#pragma once

#include "nasdaq/itch_bist_messages.h"
#include "order_book.hh"
#include "helix.hh"
#include "net.hh"

#include <unordered_map>
#include <vector>
#include <memory>
#include <set>

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
public:
    itch_bist_handler();
    bool is_rth_timestamp(uint64_t timestamp) const;
    void subscribe(std::string sym, size_t max_orders);
    void register_callback(event_callback callback);
    size_t process_packet(const net::packet_view& packet);
private:
    template<typename T>
    size_t process_msg(const net::packet_view& packet);
    void process_msg(const itch_bist_system_event* m);
    void process_msg(const itch_bist_stock_directory* m);
    void process_msg(const itch_bist_stock_trading_action* m);
    void process_msg(const itch_bist_reg_sho_restriction* m);
    void process_msg(const itch_bist_market_participant_position* m);
    void process_msg(const itch_bist_mwcb_decline_level* m);
    void process_msg(const itch_bist_mwcb_breach* m);
    void process_msg(const itch_bist_ipo_quoting_period_update* m);
    void process_msg(const itch_bist_add_order* m);
    void process_msg(const itch_bist_add_order_mpid* m);
    void process_msg(const itch_bist_order_executed* m);
    void process_msg(const itch_bist_order_executed_with_price* m);
    void process_msg(const itch_bist_order_cancel* m);
    void process_msg(const itch_bist_order_delete* m);
    void process_msg(const itch_bist_order_replace* m);
    void process_msg(const itch_bist_trade* m);
    void process_msg(const itch_bist_cross_trade* m);
    void process_msg(const itch_bist_broken_trade* m);
    void process_msg(const itch_bist_noii* m);
    void process_msg(const itch_bist_rpii* m);
    //! Generate a sweep event if execution cleared a price level.
    event_mask sweep_event(const execution&) const;
};

}

}
