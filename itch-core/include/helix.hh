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
#include <memory>
#include <boost/config.hpp>
#include <boost/asio/thread_pool.hpp>

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
    order_book* _ob;
    trade _trade;
    event_mask  _mask;
    std::string _symbol;
  public:
    event(event_mask mask, std::string symbol, uint64_t timestamp, order_book* ob, trade);
    event_mask get_mask() const;
    const std::string& get_symbol() const;
    uint64_t get_timestamp() const;
    order_book* get_ob() const;
    trade* get_trade() const;
  };

  std::shared_ptr<event> make_event(const std::string& symbol, uint64_t timestamp, order_book*, trade&&, event_mask mask = 0);
  std::shared_ptr<event> make_sys_event(uint64_t timestamp, event_mask mask = 0);
  std::shared_ptr<event> make_ob_event(const std::string& symbol, uint64_t timestamp, order_book*, event_mask mask = 0);
  std::shared_ptr<event> make_trade_event(const std::string& symbol, uint64_t timestamp, order_book*, trade&&, event_mask mask = 0);

  using event_callback = std::function<void(std::shared_ptr<event>)>;

  using send_callback = std::function<void(char*, size_t)>;

  class session {
  public:
    explicit session(void* data)
      : _data{ data }
    { }

    virtual ~session()
    {
      //_pool.stop();
      _pool.join();
    }

    void* data() const {
      return _data;
    }

    bool check_is_working_time(uint64_t timestamp);

    virtual void subscribe(const std::string& symbol, size_t max_orders) = 0;

    virtual void register_callback(event_callback callback) = 0;

    virtual void set_send_callback(send_callback callback) = 0;

    size_t received_packet(const net::packet_view& packet);

    boost::asio::thread_pool* get_event_pool() const {
      return &_pool;
    }

  protected:
    void set_support_async_packet_handling(bool b) { support_async_packet_handling = b; }
    virtual bool is_rth_timestamp(uint64_t timestamp) = 0;
    virtual size_t get_packet_length(const net::packet_view& packet) const { return static_cast<size_t>(-1); }
    virtual size_t process_packet(const net::packet_view& packet) = 0;
  private:
    bool support_async_packet_handling {false};
    void* _data;
    mutable boost::asio::thread_pool _pool{ 1 };
  };

  class protocol {
  public:
    virtual ~protocol()
    { }

    virtual session* new_session(void*) = 0;
  };

} // namespace helix.
