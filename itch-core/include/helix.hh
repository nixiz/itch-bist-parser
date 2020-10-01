#pragma once

/// \mainpage
///
/// Helix is an ultra-low latency C++ feed handler.
///
/// The documentation is organized into following sections:
///
///   - \ref order-book Order book reconstruction and management.

#include "order_book.hh"

#include <cstddef>
#include <vector>
#include <string>

namespace helix {

namespace net {

class packet_view;

}

class unknown_message_type : public std::runtime_error {
public:
    explicit unknown_message_type(std::string&& cause)
        : runtime_error{std::move(cause)}
    { }
};

class truncated_packet_error : public std::runtime_error {
public:
    explicit truncated_packet_error(std::string&& cause)
        : std::runtime_error{std::move(cause)}
    { }
};

enum class trade_sign {
    buyer_initiated,
    seller_initiated,
    crossing,
    non_displayable,
};

struct trade {
    uint64_t    timestamp;
    uint64_t    price;
    uint64_t    size;
    trade_sign  sign;

    trade(uint64_t timestamp,
          uint64_t price,
          uint64_t size,
          trade_sign sign)
        : timestamp{timestamp}
        , price{price}
        , size{size}
        , sign{sign}
    { }
};

using event_mask = uint32_t;
enum {
    ev_order_book_update = 1UL << 0,
    ev_trade             = 1UL << 1,
    ev_sweep             = 1UL << 2,
};

class event {
    event_mask  _mask;
    std::string _symbol;
    uint64_t    _timestamp;
    order_book* _ob;
    trade*      _trade;
public:
    event(event_mask mask, const std::string& symbol, uint64_t timestamp, order_book* ob, trade*);
    event_mask get_mask() const;
    const std::string& get_symbol() const;
    uint64_t get_timestamp() const;
    order_book* get_ob() const;
    trade* get_trade() const;
};

event make_event(const std::string& symbol, uint64_t timestamp, order_book*, trade*, event_mask mask = 0);
event make_ob_event(const std::string& symbol, uint64_t timestamp, order_book*, event_mask mask = 0);
event make_trade_event(const std::string& symbol, uint64_t timestamp, trade*, event_mask mask = 0);

using event_callback = std::function<void(const event&)>;

using send_callback = std::function<void(char*, size_t)>;

class session {
    void* _data;
public:
    explicit session(void* data)
        : _data{data}
    { }

    virtual ~session()
    { }

    void* data() {
        return _data;
    }

    virtual bool is_rth_timestamp(uint64_t timestamp) = 0;

    virtual void subscribe(const std::string& symbol, size_t max_orders) = 0;

    virtual void register_callback(event_callback callback) = 0;

    virtual void set_send_callback(send_callback callback) = 0;

    virtual size_t process_packet(const net::packet_view& packet) = 0;
};

class protocol {
public:
    virtual ~protocol()
    { }

    virtual session* new_session(void*) = 0;
};

}
