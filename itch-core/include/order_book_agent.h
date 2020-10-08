#pragma once
#include "order_book.hh"
#include <boost/asio/thread_pool.hpp>

namespace helix 
{
  using boost::asio::thread_pool;
  class order_book_agent
  {
    mutable thread_pool* ob_thread{ nullptr };
    order_book* ob{ nullptr };
  public:
    order_book_agent() = default;
    order_book_agent(order_book* ob_);
    explicit order_book_agent(thread_pool* ob_thread_, order_book* ob_);
    
    std::string_view symbol() const;
    uint64_t timestamp() const;
    trading_state state() const;
    std::string_view state_name() const;
    uint16_t decimals_for_price() const;

    side_type side(uint64_t order_id) const;

    size_t bid_levels() const;
    size_t ask_levels() const;
    size_t order_count() const;

    uint64_t bid_price(size_t level) const;
    uint64_t bid_size(size_t level) const;
    uint64_t ask_price(size_t level) const;
    uint64_t ask_size(size_t level) const;
    uint64_t midprice(size_t level) const;

    price_level bid_level(size_t level) const;
    price_level ask_level(size_t level) const;
  };



} // namespace helix