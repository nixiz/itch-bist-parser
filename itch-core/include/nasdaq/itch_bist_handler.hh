#pragma once

#include "nasdaq/itch_bist_messages.h"
#include "order_book.hh"
#include "helix.hh"
#include "net.hh"

#include <unordered_map>
#include <vector>
#include <memory>
#include <set>
#include <chrono>

namespace helix {

namespace nasdaq {

// NASDAQ TotalView-ITCH BIST v.4.5
//
// This is a feed handler for Total-View ITCH. The handler assumes that
// transport protocol framing such as SoupTCP or MoldUDP has already been
// parsed and works directly on ITCH messages.
//
// The ITCH variant processed by this feed handler is specified by NASDAQ in:
//
//   NASDAQ TotalView-ITCH BIST v.4.5
//   Version 5.0
//   03/06/2015
//
class itch_bist_handler {
private:
    //! Callback function for processing events.
    event_callback _process_event;
    //! A map of order books by order book ID.
    std::unordered_map<uint64_t, helix::order_book> order_book_id_map;
    //! A set of symbols that we are interested in.
    std::set<std::string> _symbols;
    //! A map of pre-allocation size by symbol.
    std::unordered_map<std::string, size_t> _symbol_max_orders;
    //! Working utc time seconds. nanoseconds will be padded on all other messages
    std::chrono::seconds time_secs {0};
public:
    itch_bist_handler();
    bool is_rth_timestamp(uint64_t timestamp) const;
    void subscribe(std::string sym, size_t max_orders);
    void register_callback(event_callback callback);
    size_t process_packet(const net::packet_view& packet);
private:
    template<typename T>
    size_t process_msg(const net::packet_view& packet);
    void process_msg(const itch_bist_seconds* m);
    void process_msg(const itch_bist_order_book_directory* m);
    void process_msg(const itch_bist_combination_order_book_leg* m);
    void process_msg(const itch_bist_tick_size_table_entry* m);
    void process_msg(const itch_bist_system_event* m);
    void process_msg(const itch_bist_order_book_state* m);
    void process_msg(const itch_bist_add_order* m);
    void process_msg(const itch_bist_add_order_mpid* m);
    void process_msg(const itch_bist_order_executed* m);
    void process_msg(const itch_bist_order_executed_with_price* m);
    void process_msg(const itch_bist_order_replace* m);
    void process_msg(const itch_bist_order_delete* m);
    void process_msg(const itch_bist_trade* m);
    void process_msg(const itch_bist_equilibrium_price_update* m);
    //! Generate a sweep event if execution cleared a price level.
    event_mask sweep_event(const execution&) const;
    //! Generate timestamp with nanoseconds
    uint64_t itch_bist_timestamp(uint32_t raw_timestamp);
};

}

}
