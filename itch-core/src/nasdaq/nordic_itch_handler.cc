#include "nasdaq/nordic_itch_handler.hh"

#include "nasdaq/nordic_itch_messages.h"
#include "order_book.hh"

#include <unordered_map>
#include <stdexcept>
#include <cstring>
#include <cstdint>
#include <chrono>
#include <cstdio>
#include <memory>
#include <string>

using namespace std;

namespace helix {

namespace nasdaq {

nordic_itch_handler::nordic_itch_handler()
{
}

void nordic_itch_handler::subscribe(std::string sym, size_t max_orders)
{
    _symbols.insert(sym);
    _symbol_max_orders.emplace(sym, max_orders);
    size_t max_all_orders = 0;
    for (auto&& kv : _symbol_max_orders) {
         max_all_orders += kv.second;
    }
    order_id_map.reserve(max_all_orders);
}

void nordic_itch_handler::register_callback(event_callback callback)
{
    _process_event = callback;
}

static side_type itch_side(char c)
{
    switch (c) {
    case 'B': return side_type::buy;
    case 'S': return side_type::sell;
    default:  throw invalid_argument(string("invalid argument: ") + to_string(c));
    }
}

trade_sign itch_trade_sign(side_type s)
{
    switch (s) {
    case side_type::buy:  return trade_sign::seller_initiated;
    case side_type::sell: return trade_sign::buyer_initiated;
    default:              throw invalid_argument(string("invalid argument"));
    }
}

bool nordic_itch_handler::is_rth_timestamp(uint64_t timestamp) const
{
    using namespace std::chrono_literals;
    using namespace std::chrono;
    // FIXME: This is valid only for Stockholm and Helsinki equities.
    constexpr uint64_t rth_start = duration_cast<milliseconds>(9h).count();
    constexpr uint64_t rth_end   = duration_cast<milliseconds>(17h + 25min).count();
    return timestamp >= rth_start && timestamp < rth_end;
}

size_t nordic_itch_handler::process_packet(const net::packet_view& packet)
{
    auto* msg = packet.cast<itch_message>();
    switch (msg->MsgType) {
    case 'T': return process_msg<itch_seconds>(packet);
    case 'M': return process_msg<itch_milliseconds>(packet);
    case 'O': return process_msg<itch_market_segment_state>(packet);
    case 'S': return process_msg<itch_system_event>(packet);
    case 'R': return process_msg<itch_order_book_directory>(packet);
    case 'H': return process_msg<itch_order_book_trading_action>(packet);
    case 'A': return process_msg<itch_add_order>(packet);
    case 'F': return process_msg<itch_add_order_mpid>(packet);
    case 'E': return process_msg<itch_order_executed>(packet);
    case 'C': return process_msg<itch_order_executed_with_price>(packet);
    case 'X': return process_msg<itch_order_cancel>(packet);
    case 'D': return process_msg<itch_order_delete>(packet);
    case 'P': return process_msg<itch_trade>(packet);
    case 'Q': return process_msg<itch_cross_trade>(packet);
    case 'B': return process_msg<itch_broken_trade>(packet);
    case 'I': return process_msg<itch_noii>(packet);
    default:  return 0;
    }
}

template<typename T>
size_t nordic_itch_handler::process_msg(const net::packet_view& packet)
{
    process_msg(packet.cast<T>());
    return sizeof(T);
}

void nordic_itch_handler::process_msg(const itch_seconds* m)
{
    auto second = itch_uatoi(m->Second, 5);
    time_sec = second;
}

void nordic_itch_handler::process_msg(const itch_milliseconds* m)
{
    auto millisecond = itch_uatoi(m->Millisecond, 3);
    time_msec = millisecond;
}

void nordic_itch_handler::process_msg(const itch_market_segment_state* m)
{
}

void nordic_itch_handler::process_msg(const itch_system_event* m)
{
}

void nordic_itch_handler::process_msg(const itch_order_book_directory* m)
{
    auto order_book_id = itch_uatoi(m->OrderBook, sizeof(m->OrderBook));

    std::string sym{m->Symbol, ITCH_SYMBOL_LEN};
    std::size_t end = sym.find_first_of(" ");
    if (end != std::string::npos) {
        sym = sym.substr(0, end);
    }
    if (_symbols.count(sym) > 0) {
        order_book ob{sym, timestamp(), _symbol_max_orders.at(sym)};
        order_book_id_map.insert({order_book_id, std::move(ob)});
    }
}

void nordic_itch_handler::process_msg(const itch_order_book_trading_action* m)
{
    auto order_book_id = itch_uatoi(m->OrderBook, sizeof(m->OrderBook));
    auto it = order_book_id_map.find(order_book_id);
    if (it != order_book_id_map.end()) {
        auto& ob = it->second;

        switch (m->TradingState) {
        case 'H': ob.set_state(trading_state::halted ); break;
        case 'T': ob.set_state(trading_state::trading); break;
        case 'Q': ob.set_state(trading_state::auction); break;
        default : throw invalid_argument(string("invalid trading state: ") + to_string(m->TradingState));
        }
        ob.set_timestamp(timestamp());
    }
}

void nordic_itch_handler::process_msg(const itch_add_order* m)
{
    auto order_book_id = itch_uatoi(m->OrderBook, sizeof(m->OrderBook));
    auto it = order_book_id_map.find(order_book_id);
    if (it != order_book_id_map.end()) {
        auto& ob = it->second;

        uint64_t order_id = itch_uatoi(m->OrderReferenceNumber, sizeof(m->OrderReferenceNumber));
        uint64_t price    = itch_uatoi(m->Price, sizeof(m->Price));;
        uint32_t quantity = itch_uatoi(m->Quantity, sizeof(m->Quantity));;
        auto     side     = itch_side(m->BuySellIndicator);

        order o{order_id, price, quantity, side, timestamp()};
        ob.add(std::move(o));

        order_id_map.insert({order_id, ob});
        ob.set_timestamp(timestamp());
        _process_event(make_ob_event(ob.symbol(), timestamp(), &ob));
    }
}

void nordic_itch_handler::process_msg(const itch_add_order_mpid* m)
{
    auto order_book_id = itch_uatoi(m->OrderBook, sizeof(m->OrderBook));
    auto it = order_book_id_map.find(order_book_id);
    if (it != order_book_id_map.end()) {
        auto& ob = it->second;

        uint64_t order_id = itch_uatoi(m->OrderReferenceNumber, sizeof(m->OrderReferenceNumber));
        uint64_t price    = itch_uatoi(m->Price, sizeof(m->Price));;
        uint32_t quantity = itch_uatoi(m->Quantity, sizeof(m->Quantity));;
        auto     side     = itch_side(m->BuySellIndicator);

        order o{order_id, price, quantity, side, timestamp()};
        ob.add(std::move(o));

        order_id_map.insert({order_id, ob});
        ob.set_timestamp(timestamp());
        _process_event(make_ob_event(ob.symbol(), timestamp(), &ob));
    }
}

void nordic_itch_handler::process_msg(const itch_order_executed* m)
{
   uint64_t order_id = itch_uatoi(m->OrderReferenceNumber, sizeof(m->OrderReferenceNumber));
   auto it = order_id_map.find(order_id);
   if (it != order_id_map.end()) {
       uint64_t quantity = itch_uatoi(m->ExecutedQuantity, sizeof(m->ExecutedQuantity));
       auto& ob = it->second;
       auto result = ob.execute(order_id, quantity);
       ob.set_timestamp(timestamp());
       trade t{timestamp(), result.price, quantity, itch_trade_sign(result.side)};
        _process_event(make_event(ob.symbol(), timestamp(), &ob, &t, sweep_event(result)));
   }
}

void nordic_itch_handler::process_msg(const itch_order_executed_with_price* m)
{
    uint64_t order_id = itch_uatoi(m->OrderReferenceNumber, sizeof(m->OrderReferenceNumber));
    auto it = order_id_map.find(order_id);
    if (it != order_id_map.end()) {
        uint64_t quantity = itch_uatoi(m->ExecutedQuantity, sizeof(m->ExecutedQuantity));
        uint64_t price = itch_uatoi(m->TradePrice, sizeof(m->TradePrice));
        auto& ob = it->second;
        auto result = ob.execute(order_id, quantity);
        ob.set_timestamp(timestamp());
        trade t{timestamp(), price, quantity, itch_trade_sign(result.side)};
        _process_event(make_event(ob.symbol(), timestamp(), &ob, &t, sweep_event(result)));
    }
}

void nordic_itch_handler::process_msg(const itch_order_cancel* m)
{
    uint64_t order_id = itch_uatoi(m->OrderReferenceNumber, sizeof(m->OrderReferenceNumber));
    auto it = order_id_map.find(order_id);
    if (it != order_id_map.end()) {
        uint64_t quantity = itch_uatoi(m->CanceledQuantity, sizeof(m->CanceledQuantity));
        auto& ob = it->second;
        ob.cancel(order_id, quantity);
        ob.set_timestamp(timestamp());
        _process_event(make_ob_event(ob.symbol(), timestamp(), &ob));
    }
}

void nordic_itch_handler::process_msg(const itch_order_delete* m)
{
    uint64_t order_id = itch_uatoi(m->OrderReferenceNumber, sizeof(m->OrderReferenceNumber));
    auto it = order_id_map.find(order_id);
    if (it != order_id_map.end()) {
        auto& ob = it->second;
        ob.remove(order_id);
        ob.set_timestamp(timestamp());
        _process_event(make_ob_event(ob.symbol(), timestamp(), &ob));
    }
}

void nordic_itch_handler::process_msg(const itch_trade* m)
{
    auto order_book_id = itch_uatoi(m->OrderBook, sizeof(m->OrderBook));
    auto it = order_book_id_map.find(order_book_id);
    if (it != order_book_id_map.end()) {
        uint64_t trade_price = itch_uatoi(m->TradePrice, sizeof(m->TradePrice));
        uint64_t quantity = itch_uatoi(m->Quantity, sizeof(m->Quantity));
        auto& ob = it->second;
        trade t{timestamp(), trade_price, quantity, trade_sign::non_displayable};
        _process_event(make_trade_event(ob.symbol(), timestamp(), &t));
    }
}

void nordic_itch_handler::process_msg(const itch_cross_trade* m)
{
    auto order_book_id = itch_uatoi(m->OrderBook, sizeof(m->OrderBook));
    auto it = order_book_id_map.find(order_book_id);
    if (it != order_book_id_map.end()) {
        uint64_t cross_price = itch_uatoi(m->CrossPrice, sizeof(m->CrossPrice));
        uint64_t quantity = itch_uatoi(m->Quantity, sizeof(m->Quantity));
        auto& ob = it->second;
        trade t{timestamp(), cross_price, quantity, trade_sign::crossing};
        _process_event(make_trade_event(ob.symbol(), timestamp(), &t));
    }
}

void nordic_itch_handler::process_msg(const itch_broken_trade* m)
{
}

void nordic_itch_handler::process_msg(const itch_noii* m)
{
}

uint64_t nordic_itch_handler::timestamp() const
{
    return time_sec * 1000 + time_msec;
}

event_mask nordic_itch_handler::sweep_event(const execution& e) const
{
    if (e.remaining > 0) {
        return 0;
    }
    return ev_sweep;
}

}

}
