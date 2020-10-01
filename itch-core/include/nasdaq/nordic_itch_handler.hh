#pragma once

#include "nasdaq/nordic_itch_messages.h"
#include "order_book.hh"
#include "helix.hh"
#include "net.hh"

#include <unordered_map>
#include <vector>
#include <memory>
#include <set>

namespace helix {

namespace nasdaq {

// NASDAQ OXM Nordic ITCH feed.
//
// This is a feed handler for Nordic ITCH. The handler assumes that transport
// protocol framing such as SoupTCP or MoldUDP has already been parsed and
// works directly on ITCH messages.
//
// The feed handler reconstructs a full depth order book from a ITCH message
// flow using an algorithm that is specified in Appendix A of the protocol
// specification. As not all messages include an order book ID, the handler
// keeps mapping from every order ID it encounters to the order book the order
// is part of.
//
// The ITCH variant processed by this feed handler is specified by NASDAQ OMX
// in:
//
//   Nordic Equity TotalView-ITCH
//   Version 2.02.2
//   June 4, 2015
//
class nordic_itch_handler {
    //! Seconds since midnight in CET (Central European Time).
    uint64_t time_sec;
    //! Milliseconds since @time_sec.
    uint64_t time_msec;
    //! Callback function for processing events.
    event_callback _process_event;
    //! A map of order books by order book ID.
    std::unordered_map<uint64_t, helix::order_book> order_book_id_map;
    //! A map of order books by order ID.
    std::unordered_map<uint64_t, helix::order_book&> order_id_map;
    //! A set of symbols that we are interested in.
    std::set<std::string> _symbols;
    //! A map of pre-allocation size by symbol.
    std::unordered_map<std::string, size_t> _symbol_max_orders;
public:
    nordic_itch_handler();
    bool is_rth_timestamp(uint64_t timestamp) const;
    void subscribe(std::string sym, size_t max_orders);
    void register_callback(event_callback callback);
    size_t process_packet(const net::packet_view& packet);
private:
    template<typename T>
    size_t process_msg(const net::packet_view& packet);
    void process_msg(const itch_seconds* m);
    void process_msg(const itch_milliseconds* m);
    void process_msg(const itch_market_segment_state* m);
    void process_msg(const itch_system_event* m);
    void process_msg(const itch_order_book_directory* m);
    void process_msg(const itch_order_book_trading_action* m);
    void process_msg(const itch_add_order* m);
    void process_msg(const itch_add_order_mpid* m);
    void process_msg(const itch_order_executed* m);
    void process_msg(const itch_order_executed_with_price* m);
    void process_msg(const itch_order_cancel* m);
    void process_msg(const itch_order_delete* m);
    void process_msg(const itch_trade* m);
    void process_msg(const itch_cross_trade* m);
    void process_msg(const itch_broken_trade* m);
    void process_msg(const itch_noii* m);
    //! Generate a sweep event if execution cleared a price level.
    event_mask sweep_event(const execution&) const;
    //! Timestamp in milliseconds
    uint64_t timestamp() const;
};

}

}
