#include "nasdaq/itch50_handler.hh"

#include "compat/endian.h"
#include "order_book.hh"

#include <stdexcept>
#include <chrono>

namespace helix {

namespace nasdaq {

static side_type itch50_side(char c)
{
    switch (c) {
    case 'B': return side_type::buy;
    case 'S': return side_type::sell;
    default:  throw std::invalid_argument(std::string("invalid argument: ") + std::to_string(c));
    }
}

trade_sign itch50_trade_sign(side_type s)
{
    switch (s) {
    case side_type::buy:  return trade_sign::seller_initiated;
    case side_type::sell: return trade_sign::buyer_initiated;
    default:              throw std::invalid_argument(std::string("invalid argument"));
    }
}

static uint64_t itch50_timestamp(uint64_t raw_timestamp)
{
    return be64toh(raw_timestamp << 16);
}

itch50_handler::itch50_handler()
{
}

bool itch50_handler::is_rth_timestamp(uint64_t timestamp) const
{
    using namespace std::chrono_literals;
    using namespace std::chrono;
    constexpr uint64_t rth_start = duration_cast<nanoseconds>(9h + 30min).count();
    constexpr uint64_t rth_end   = duration_cast<nanoseconds>(16h).count();
    return timestamp >= rth_start && timestamp < rth_end;
}

void itch50_handler::subscribe(std::string sym, size_t max_orders) {
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

void itch50_handler::register_callback(event_callback callback) {
    _process_event = callback;
}

size_t itch50_handler::process_packet(const net::packet_view& packet)
{
    auto* msg = packet.cast<itch50_message>();
    switch (msg->MessageType) {
    case 'S': return process_msg<itch50_system_event>(packet);
    case 'R': return process_msg<itch50_stock_directory>(packet);
    case 'H': return process_msg<itch50_stock_trading_action>(packet);
    case 'Y': return process_msg<itch50_reg_sho_restriction>(packet);
    case 'L': return process_msg<itch50_market_participant_position>(packet);
    case 'V': return process_msg<itch50_mwcb_decline_level>(packet);
    case 'W': return process_msg<itch50_mwcb_breach>(packet);
    case 'K': return process_msg<itch50_ipo_quoting_period_update>(packet);
    case 'A': return process_msg<itch50_add_order>(packet);
    case 'F': return process_msg<itch50_add_order_mpid>(packet);
    case 'E': return process_msg<itch50_order_executed>(packet);
    case 'C': return process_msg<itch50_order_executed_with_price>(packet);
    case 'X': return process_msg<itch50_order_cancel>(packet);
    case 'D': return process_msg<itch50_order_delete>(packet);
    case 'U': return process_msg<itch50_order_replace>(packet);
    case 'P': return process_msg<itch50_trade>(packet);
    case 'Q': return process_msg<itch50_cross_trade>(packet);
    case 'B': return process_msg<itch50_broken_trade>(packet);
    case 'I': return process_msg<itch50_noii>(packet);
    case 'N': return process_msg<itch50_rpii>(packet);
    default: throw unknown_message_type("unknown type: " + std::string(1, msg->MessageType));
    }
}

template<typename T>
size_t itch50_handler::process_msg(const net::packet_view& packet)
{
    process_msg(packet.cast<T>());
    return sizeof(T);
}

void itch50_handler::process_msg(const itch50_system_event* m)
{
}

void itch50_handler::process_msg(const itch50_stock_directory* m)
{
    std::string sym{m->Stock, ITCH_SYMBOL_LEN};
    if (_symbols.count(sym) > 0) {
        order_book ob{sym, itch50_timestamp(m->Timestamp), _symbol_max_orders.at(sym)};
        order_book_id_map.insert({m->StockLocate, std::move(ob)});
    }
}

void itch50_handler::process_msg(const itch50_stock_trading_action* m)
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

void itch50_handler::process_msg(const itch50_reg_sho_restriction* m)
{
}

void itch50_handler::process_msg(const itch50_market_participant_position* m)
{
}

void itch50_handler::process_msg(const itch50_mwcb_decline_level* m)
{
}

void itch50_handler::process_msg(const itch50_mwcb_breach* m)
{
}

void itch50_handler::process_msg(const itch50_ipo_quoting_period_update* m)
{
}

void itch50_handler::process_msg(const itch50_add_order* m)
{
    auto it = order_book_id_map.find(m->StockLocate);
    if (it != order_book_id_map.end()) {
        auto& ob = it->second;

        uint64_t order_id = m->OrderReferenceNumber;
        uint64_t price    = be32toh(m->Price);
        uint32_t quantity = be32toh(m->Shares);
        auto     side     = itch50_side(m->BuySellIndicator);
        uint64_t timestamp = itch50_timestamp(m->Timestamp);
        order o{order_id, price, quantity, side, timestamp};
        ob.add(std::move(o));
        ob.set_timestamp(timestamp);
        _process_event(make_ob_event(ob.symbol(), timestamp, &ob));
    }
}

void itch50_handler::process_msg(const itch50_add_order_mpid* m)
{
    auto it = order_book_id_map.find(m->StockLocate);
    if (it != order_book_id_map.end()) {
        auto& ob = it->second;

        uint64_t order_id = m->OrderReferenceNumber;
        uint64_t price    = be32toh(m->Price);
        uint32_t quantity = be32toh(m->Shares);
        auto     side     = itch50_side(m->BuySellIndicator);
        uint64_t timestamp = itch50_timestamp(m->Timestamp);
        order o{order_id, price, quantity, side, timestamp};
        ob.add(std::move(o));
        ob.set_timestamp(timestamp);
        _process_event(make_ob_event(ob.symbol(), timestamp, &ob));
    }
}

void itch50_handler::process_msg(const itch50_order_executed* m)
{
    auto it = order_book_id_map.find(m->StockLocate);
    if (it != order_book_id_map.end()) {
        uint64_t quantity = be32toh(m->ExecutedShares);
        uint64_t timestamp = itch50_timestamp(m->Timestamp);
        auto& ob = it->second;
        auto result = ob.execute(m->OrderReferenceNumber, quantity);
        ob.set_timestamp(timestamp);
        trade t{timestamp, result.price, quantity, itch50_trade_sign(result.side)};
        _process_event(make_event(ob.symbol(), timestamp, &ob, &t, sweep_event(result)));
    }
}

void itch50_handler::process_msg(const itch50_order_executed_with_price* m)
{
    auto it = order_book_id_map.find(m->StockLocate);
    if (it != order_book_id_map.end()) {
        uint64_t quantity = be32toh(m->ExecutedShares);
        uint64_t price = be32toh(m->ExecutionPrice);
        uint64_t timestamp = itch50_timestamp(m->Timestamp);
        auto& ob = it->second;
        auto result = ob.execute(m->OrderReferenceNumber, quantity);
        ob.set_timestamp(timestamp);
        trade t{timestamp, price, quantity, itch50_trade_sign(result.side)};
        _process_event(make_event(ob.symbol(), timestamp, &ob, &t, sweep_event(result)));
    }
}

void itch50_handler::process_msg(const itch50_order_cancel* m)
{
    auto it = order_book_id_map.find(m->StockLocate);
    if (it != order_book_id_map.end()) {
        auto& ob = it->second;
        ob.cancel(m->OrderReferenceNumber, be32toh(m->CanceledShares));
        auto timestamp = itch50_timestamp(m->Timestamp);
        ob.set_timestamp(timestamp);
        _process_event(make_ob_event(ob.symbol(), timestamp, &ob));
    }
}

void itch50_handler::process_msg(const itch50_order_delete* m)
{
    auto it = order_book_id_map.find(m->StockLocate);
    if (it != order_book_id_map.end()) {
        auto& ob = it->second;
        ob.remove(m->OrderReferenceNumber);
        auto timestamp = itch50_timestamp(m->Timestamp);
        ob.set_timestamp(timestamp);
        _process_event(make_ob_event(ob.symbol(), timestamp, &ob));
    }
}

void itch50_handler::process_msg(const itch50_order_replace* m)
{
    auto it = order_book_id_map.find(m->StockLocate);
    if (it != order_book_id_map.end()) {
        auto& ob = it->second;
        auto side = ob.side(m->OriginalOrderReferenceNumber);
        uint64_t order_id = m->NewOrderReferenceNumber;
        uint64_t price    = be32toh(m->Price);
        uint32_t quantity = be32toh(m->Shares);
        uint64_t timestamp = itch50_timestamp(m->Timestamp);
        order o{order_id, price, quantity, side, timestamp};
        ob.replace(m->OriginalOrderReferenceNumber, std::move(o));
        ob.set_timestamp(timestamp);
        _process_event(make_ob_event(ob.symbol(), timestamp, &ob));
    }
}

void itch50_handler::process_msg(const itch50_trade* m)
{
    auto it = order_book_id_map.find(m->StockLocate);
    if (it != order_book_id_map.end()) {
        uint64_t trade_price = be32toh(m->Price);
        uint32_t quantity = be32toh(m->Shares);
        auto& ob = it->second;
        auto timestamp = itch50_timestamp(m->Timestamp);
        trade t{timestamp, trade_price, quantity, trade_sign::non_displayable};
        _process_event(make_trade_event(ob.symbol(), timestamp, &t));
    }
}

void itch50_handler::process_msg(const itch50_cross_trade* m)
{
    auto it = order_book_id_map.find(m->StockLocate);
    if (it != order_book_id_map.end()) {
        uint64_t cross_price = be32toh(m->CrossPrice);
        uint64_t quantity = be64toh(m->Shares);
        auto& ob = it->second;
        auto timestamp = itch50_timestamp(m->Timestamp);
        trade t{timestamp, cross_price, quantity, trade_sign::crossing};
        _process_event(make_trade_event(ob.symbol(), timestamp, &t));
    }
}

void itch50_handler::process_msg(const itch50_broken_trade* m)
{
}

void itch50_handler::process_msg(const itch50_noii* m)
{
}

void itch50_handler::process_msg(const itch50_rpii* m)
{
}

event_mask itch50_handler::sweep_event(const execution& e) const
{
    if (e.remaining > 0) {
        return 0;
    }
    return ev_sweep;
}

}

}
