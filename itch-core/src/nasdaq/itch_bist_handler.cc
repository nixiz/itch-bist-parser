#include "nasdaq/itch_bist_handler.hh"

#include "compat/endian.h"
#include "order_book.hh"

#include <stdexcept>
#include <chrono>
#include <ctime>
#include <iostream>
#include <iomanip>

namespace helix {

namespace nasdaq {

static side_type itch_bist_side(char c)
{
  switch (c) {
  case 'B': return side_type::buy;
  case 'S': return side_type::sell;
  default:  throw std::invalid_argument(std::string("invalid argument: ") + std::to_string(c));
  }
}

trade_sign itch_bist_trade_sign(side_type s)
{
  switch (s) {
  case side_type::buy:  return trade_sign::seller_initiated;
  case side_type::sell: return trade_sign::buyer_initiated;
  default:              throw std::invalid_argument(std::string("invalid argument"));
  }
}

uint64_t itch_bist_handler::itch_bist_timestamp(uint32_t raw_timestamp)
{
  using namespace std::chrono;
  auto timestamp = duration_cast<nanoseconds>(time_secs).count();
  timestamp += swap_bytes(raw_timestamp);
  return timestamp;
}

bool itch_bist_handler::is_rth_timestamp(uint64_t timestamp) const
{
  using namespace std::chrono_literals;
  using namespace std::chrono;

  constexpr uint64_t rth_start = duration_cast<nanoseconds>(9h + 30min).count();
  constexpr uint64_t rth_end = duration_cast<nanoseconds>(16h).count();
  // TODO(oguzhank): burada gun basindan itibaren bakiyor ama bizim timestamp utc nanosecond!
  // bunu cozmek gerekiyor
  //return timestamp >= rth_start && timestamp < rth_end;
  return true;
}

void itch_bist_handler::subscribe(std::string sym, size_t max_orders) {
  auto padding = ITCH_SYMBOL_LEN - sym.size();
  if (padding > 0) {
    sym.insert(sym.size(), padding, ' ');
  }
  _symbols.insert(sym);
  _symbol_max_orders.emplace(sym, max_orders);
  size_t max_all_orders = 0;
  for (auto&& kv : _symbol_max_orders) {
    max_all_orders += kv.second;
  }
  order_book_id_map.reserve(max_all_orders);
}

void itch_bist_handler::register_callback(event_callback callback) {
  _process_event = callback;
}

size_t itch_bist_handler::process_packet(const net::packet_view& packet)
{
  auto* msg = packet.cast<itch_bist_message>();
  switch (msg->MessageType) {
  case 'T': return process_msg<itch_bist_seconds>(packet);
  case 'R': return process_msg<itch_bist_order_book_directory>(packet);
  case 'M': return process_msg<itch_bist_combination_order_book_leg>(packet);
  case 'L': return process_msg<itch_bist_tick_size_table_entry>(packet);
  case 'S': return process_msg<itch_bist_system_event>(packet);
  case 'O': return process_msg<itch_bist_order_book_state>(packet);
  case 'A': return process_msg<itch_bist_add_order>(packet);
  case 'F': return process_msg<itch_bist_add_order_mpid>(packet);
  case 'E': return process_msg<itch_bist_order_executed>(packet);
  case 'C': return process_msg<itch_bist_order_executed_with_price>(packet);
  case 'U': return process_msg<itch_bist_order_replace>(packet);
  case 'D': return process_msg<itch_bist_order_delete>(packet);
  case 'P': return process_msg<itch_bist_trade>(packet);
  case 'Z': return process_msg<itch_bist_equilibrium_price_update>(packet);
  default: throw unknown_message_type("unknown type: " + std::string(1, msg->MessageType));
  }
}

template<typename T>
size_t itch_bist_handler::process_msg(const net::packet_view& packet)
{
  // TODO(oguzhank): consider to return size of T through some trait helper like:
  //                 itch_bist_msg_traits<T>::packet_size;
  // Bu sayede hangi mesajin islenip islenmeyecegi, ne yapmasi gerektigi falan da
  // trait ozellikleri ile belirtilebilir!
  // if constexpr (itch_bist_msg_traits<T>::should_process) { process_msg(..); }
  process_msg(packet.cast<T>());
  return sizeof(T);
}

template <class dur_t>
inline void print_utc_time(const char* msg, dur_t const& tsecs) {
  using namespace std::chrono;
  time_point<system_clock, system_clock::duration> tp(tsecs);
  auto tm = system_clock::to_time_t(tp);
  std::cout << msg << std::put_time(std::localtime(&tm), "%F %T") << "\n";
}

void itch_bist_handler::process_msg(const itch_bist_seconds* m)
{
  auto new_sec = std::chrono::seconds(swap_bytes(m->UtcSeconds));
  if (new_sec - time_secs > std::chrono::seconds(10))
  {
    print_utc_time("working utc time: ", time_secs);
  }
  time_secs = new_sec;
}

void itch_bist_handler::process_msg(const itch_bist_order_book_directory* m)
{
  std::string sym{ m->Symbol, ITCH_SYMBOL_LEN };
  if (_symbols.count(sym) > 0) {
    order_book ob{ 
      sym, 
      itch_bist_timestamp(m->TimestampNanoseconds),  
      swap_bytes(m->NumberOfDecimalsInPrice), 
      _symbol_max_orders.at(sym) 
    };
    order_book_id_map.insert({ m->OrderBookID, std::move(ob) });
  }
}

void itch_bist_handler::process_msg(const itch_bist_combination_order_book_leg* m)
{
}

void itch_bist_handler::process_msg(const itch_bist_tick_size_table_entry* m)
{
}

void itch_bist_handler::process_msg(const itch_bist_system_event* m)
{
  using namespace std::chrono;
  auto ts = itch_bist_timestamp(m->TimestampNanoseconds);
  nanoseconds ts_ns(ts);
  switch (m->EventCode)
  {
  case system_event_code_e::StartMessage:
  {
    print_utc_time("borsa basladi: ", duration_cast<seconds>(ts_ns));
  }
  break;
  case system_event_code_e::EndMessage:
  {
    print_utc_time("borsa kapandi: ", duration_cast<seconds>(ts_ns));
  }
  break;
  default:
  break;
  }
  _process_event(make_sys_event(ts, m->EventCode == system_event_code_e::StartMessage ? ev_opened : ev_closed));
}

void itch_bist_handler::process_msg(const itch_bist_order_book_state* m)
{
  auto it = order_book_id_map.find(m->OrderBookID);
  if (it != order_book_id_map.end()) {
    auto& ob = it->second;
    ob.set_state_name(std::move(std::string(m->StateName)));
  }
}

void itch_bist_handler::process_msg(const itch_bist_add_order* m)
{  
  if (auto it = order_book_id_map.find(m->OrderBookID); 
      it != order_book_id_map.end()) 
  {
    auto& ob = it->second;

    auto order_id = m->OrderID;
    auto price = swap_bytes(m->Price);
    auto quantity = swap_bytes(m->Quantity);
    auto side = itch_bist_side(m->Side);
    auto timestamp = itch_bist_timestamp(m->TimestampNanoseconds);
    order o{ order_id, price, quantity, side, timestamp };
    ob.add(std::move(o));
    ob.set_timestamp(timestamp);
    _process_event(make_ob_event(ob.symbol(), timestamp, &ob));
  }
}

void itch_bist_handler::process_msg(const itch_bist_add_order_mpid* m)
{ 
  if (auto it = order_book_id_map.find(m->OrderBookID); 
      it != order_book_id_map.end()) 
  {
    auto& ob = it->second;

    auto order_id = m->OrderID;
    auto price = swap_bytes(m->Price);
    auto quantity = swap_bytes(m->Quantity);
    auto side = itch_bist_side(m->Side);
    auto timestamp = itch_bist_timestamp(m->TimestampNanoseconds);
    order o{ order_id, price, quantity, side, timestamp };
    ob.add(std::move(o));
    ob.set_timestamp(timestamp);
    _process_event(make_ob_event(ob.symbol(), timestamp, &ob));
  }
}

void itch_bist_handler::process_msg(const itch_bist_order_executed* m)
{  
  if (auto it = order_book_id_map.find(m->OrderBookID); 
      it != order_book_id_map.end()) 
  {
    auto quantity = swap_bytes(m->ExecutedQuantity);
    auto timestamp = itch_bist_timestamp(m->TimestampNanoseconds);
    auto& ob = it->second;
    auto result = ob.execute(m->OrderID, quantity);
    ob.set_timestamp(timestamp);
    trade t{ timestamp, result.price, quantity, itch_bist_trade_sign(result.side) };
    _process_event(make_event(ob.symbol(), timestamp, &ob, std::move(t), sweep_event(result)));
  }
}

void itch_bist_handler::process_msg(const itch_bist_order_executed_with_price* m)
{
  if (auto it = order_book_id_map.find(m->OrderBookID);
      it != order_book_id_map.end()) 
  {
    auto quantity = swap_bytes(m->ExecutedQuantity);
    auto price = swap_bytes(m->TradePrice);
    auto timestamp = itch_bist_timestamp(m->TimestampNanoseconds);
    auto& ob = it->second;
    switch (m->OccurredAtCross)
    {
    case 'Y':
    {
      trade t{ timestamp, price, quantity, trade_sign::crossing };
      _process_event(make_trade_event(ob.symbol(), timestamp, &ob, std::move(t)));
    }
    break;
    case 'N':
    default:
    {
      auto result = ob.execute(m->OrderID, quantity);
      ob.set_timestamp(timestamp);
      trade t{ timestamp, price, quantity, itch_bist_trade_sign(result.side) };
      _process_event(make_event(ob.symbol(), timestamp, &ob, std::move(t), sweep_event(result)));
    }
    break;
    }
  }
}

void itch_bist_handler::process_msg(const itch_bist_order_replace* m)
{  
  if (auto it = order_book_id_map.find(m->OrderBookID);
      it != order_book_id_map.end()) 
  {
    auto& ob = it->second;
    auto side = ob.side(m->OrderID);
    auto order_id = m->NewOrderBookPosition;
    auto price = swap_bytes(m->Price);
    auto quantity = swap_bytes(m->Quantity);
    auto timestamp = itch_bist_timestamp(m->TimestampNanoseconds);
    order o{ order_id, price, quantity, side, timestamp };
    ob.replace(m->OrderID, std::move(o));
    ob.set_timestamp(timestamp);
    _process_event(make_ob_event(ob.symbol(), timestamp, &ob));
  }
}

void itch_bist_handler::process_msg(const itch_bist_order_delete* m)
{
  if (auto it = order_book_id_map.find(m->OrderBookID); 
      it != order_book_id_map.end()) 
  {
    auto& ob = it->second;
    ob.remove(m->OrderID);
    auto timestamp = itch_bist_timestamp(m->TimestampNanoseconds);
    ob.set_timestamp(timestamp);
    _process_event(make_ob_event(ob.symbol(), timestamp, &ob));
  }
}

void itch_bist_handler::process_msg(const itch_bist_trade* m)
{
  if (auto it = order_book_id_map.find(m->OrderBookID); 
      it != order_book_id_map.end()) 
  {
    auto trade_price = swap_bytes(m->TradePrice);
    auto quantity = swap_bytes(m->Quantity);
    auto& ob = it->second;
    auto timestamp = itch_bist_timestamp(m->TimestampNanoseconds);
    trade t{ timestamp, trade_price, quantity, trade_sign::non_displayable };
    _process_event(make_trade_event(ob.symbol(), timestamp, &ob, std::move(t)));
  }
}

void itch_bist_handler::process_msg(const itch_bist_equilibrium_price_update* m)
{

}

event_mask itch_bist_handler::sweep_event(const execution& e) const
{
  if (e.remaining > 0) {
    return 0;
  }
  return ev_sweep;
}

#if 0
void itch_bist_handler::process_msg(const itch_bist_stock_trading_action* m)
{
    auto it = order_book_id_map.find(m->StockLocate);
    if (it != order_book_id_map.end()) {
        auto& ob = it->second;

        switch (m->TradingState) {
        case 'H': ob.set_state(trading_state::halted); break;
        case 'P': ob.set_state(trading_state::paused); break;
        case 'Q': ob.set_state(trading_state::quotation_only); break;
        case 'T': ob.set_state(trading_state::trading); break;
        default:  throw std::invalid_argument(std::string("invalid trading state: ") + std::to_string(m->TradingState));
        }
    }
}

void itch_bist_handler::process_msg(const itch_bist_reg_sho_restriction* m)
{
}


void itch_bist_handler::process_msg(const itch_bist_mwcb_decline_level* m)
{
}

void itch_bist_handler::process_msg(const itch_bist_mwcb_breach* m)
{
}

void itch_bist_handler::process_msg(const itch_bist_ipo_quoting_period_update* m)
{
}

void itch_bist_handler::process_msg(const itch_bist_order_cancel* m)
{
    auto it = order_book_id_map.find(m->StockLocate);
    if (it != order_book_id_map.end()) {
        auto& ob = it->second;
        ob.cancel(m->OrderReferenceNumber, be32toh(m->CanceledShares));
        auto timestamp = itch_bist_timestamp(m->Timestamp);
        ob.set_timestamp(timestamp);
        _process_event(make_ob_event(ob.symbol(), timestamp, &ob));
    }
}

void itch_bist_handler::process_msg(const itch_bist_cross_trade* m)
{
    auto it = order_book_id_map.find(m->StockLocate);
    if (it != order_book_id_map.end()) {
        uint64_t cross_price = be32toh(m->CrossPrice);
        uint64_t quantity = be64toh(m->Shares);
        auto& ob = it->second;
        auto timestamp = itch_bist_timestamp(m->Timestamp);
        trade t{timestamp, cross_price, quantity, trade_sign::crossing};
        _process_event(make_trade_event(ob.symbol(), timestamp, &t));
    }
}

void itch_bist_handler::process_msg(const itch_bist_broken_trade* m)
{
}

void itch_bist_handler::process_msg(const itch_bist_noii* m)
{
}

void itch_bist_handler::process_msg(const itch_bist_rpii* m)
{
}

#endif

} // namespace nasdaq
} // namespace helix
