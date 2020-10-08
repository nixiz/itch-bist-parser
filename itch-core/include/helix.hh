#pragma once

/// \mainpage
///
/// Helix is an ultra-low latency C++ feed handler.
///
/// The documentation is organized into following sections:
///
///   - \ref order-book Order book reconstruction and management.

#include "order_book_agent.h"

#include <cstddef>
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

namespace helix {

  namespace net {

    class packet_view;

  }

  class unknown_message_type : public std::runtime_error {
  public:
    explicit unknown_message_type(std::string&& cause)
      : runtime_error{ std::move(cause) }
    { }
  };

  class truncated_packet_error : public std::runtime_error {
  public:
    explicit truncated_packet_error(std::string&& cause)
      : std::runtime_error{ std::move(cause) }
    { }
  };

  enum class trade_sign {
    buyer_initiated,
    seller_initiated,
    crossing,
    non_displayable,
  };

  struct trade {
    uint64_t    timestamp{0};
    uint64_t    price{ 0 };
    uint64_t    size{ 0 };
    trade_sign  sign{ trade_sign::non_displayable };
    trade() = default;
    trade(uint64_t timestamp,
          uint64_t price,
          uint64_t size,
          trade_sign sign)
      : timestamp{ timestamp }
      , price{ price }
      , size{ size }
      , sign{ sign }
    { }
  };

  using event_mask = uint32_t;
  enum {
    ev_order_book_update = 1UL << 0,
    ev_trade = 1UL << 1,
    ev_sweep = 1UL << 2,
    ev_opened = 1UL << 3,
    ev_closed = 1UL << 4,
  };

  class event {
    uint64_t    _timestamp;
    order_book_agent _ob;
    trade _trade;
    event_mask  _mask;
    std::string_view _symbol;
  public:
    event(event_mask mask, std::string_view symbol, uint64_t timestamp, order_book_agent&& ob, trade&&);
    event_mask get_mask() const;
    std::string_view get_symbol() const;
    uint64_t get_timestamp() const;
    order_book_agent* get_ob() const;
    trade* get_trade() const;
  };

  std::shared_ptr<event> make_event(std::string_view symbol, uint64_t timestamp, order_book_agent&&, trade&&, event_mask mask = 0);
  std::shared_ptr<event> make_sys_event(uint64_t timestamp, event_mask mask = 0);
  std::shared_ptr<event> make_ob_event(std::string_view symbol, uint64_t timestamp, order_book_agent&&, event_mask mask = 0);
  std::shared_ptr<event> make_trade_event(std::string_view symbol, uint64_t timestamp, order_book_agent&&, trade&&, event_mask mask = 0);

  //typedef void (*event_callback)(std::shared_ptr<event>);
  using event_callback = std::function<void(std::shared_ptr<event>)>;

  using send_callback = std::function<void(char*, size_t)>;

  class session {
  public:
    explicit session(void* data)
      : _data{ data }
    { }

    virtual ~session() = default;

    void* data() const {
      return _data;
    }

    void register_for_symbol(std::string symbol, 
                             event_callback callback, 
                             size_t max_orders = 1000)
    {
      symbol = subscribe(symbol, max_orders);
      _subs[symbol].push_back(std::move(callback));
      register_callback([&] (std::shared_ptr<event> ev)
                        {
                          std::string symb(ev->get_symbol());
                          if (auto it = _subs.find(symb);
                                   it != _subs.end())
                          {
                            for (auto&& cb : it->second) {
                              cb(ev);
                            }
                          }
                          else if (symb.empty()) // means broadcast message
                          {
                            for (auto&& kvp : _subs) {
                              for (auto&& cb : kvp.second) {
                                cb(ev);
                              }
                            }
                          }
                        });
    }

    virtual void stop() {}

    virtual std::string subscribe(const std::string& symbol, size_t max_orders) = 0;

    virtual void register_callback(event_callback callback) = 0;

    virtual void set_send_callback(send_callback callback) = 0;

    virtual bool is_rth_timestamp(uint64_t timestamp) = 0;

    virtual size_t process_packet(const net::packet_view& packet) = 0;
  private:
    std::unordered_map<std::string, std::vector<event_callback>> _subs;
    void* _data;
  };

  class protocol {
  public:
    virtual ~protocol()
    { }

    virtual session* new_session(void*) = 0;
  };

} // namespace helix.
