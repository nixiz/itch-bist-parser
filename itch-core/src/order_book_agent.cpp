#include "order_book_agent.h"
#include <boost/asio/ts/executor.hpp>
#include <boost/asio/io_context.hpp>

using boost::asio::dispatch;
using boost::asio::thread_pool;
using boost::asio::use_future;

namespace helix
{
  order_book_agent::order_book_agent(order_book* ob_)
    : order_book_agent(nullptr, ob_) {}

  order_book_agent::order_book_agent(
    thread_pool* ob_thread_, 
    order_book* ob_) 
    : ob_thread(ob_thread_)
    , ob(ob_)
  { }


  void order_book_agent::set_timestamp(uint64_t timestamp) {
    auto fun = boost::bind(&order_book::set_timestamp, ob, timestamp);
    if (ob_thread) {
      dispatch(*ob_thread, std::move(fun));
    } else {
      fun();
    }
  }

  void order_book_agent::set_state(trading_state state) {
    auto fun = boost::bind(&order_book::set_state, ob, state);
    if (ob_thread) {
      dispatch(*ob_thread, std::move(fun));
    }
    else {
      fun();
    }
  }

  void order_book_agent::set_state_name(const std::string& state_name) {
    auto fun = boost::bind(&order_book::set_state_name, ob, state_name);
    if (ob_thread) {
      dispatch(*ob_thread, std::move(fun));
    }
    else {
      fun();
    }
  }

  void order_book_agent::add(order order) {
    auto fun = boost::bind(&order_book::add, ob, std::move(order));
    if (ob_thread) {
      dispatch(*ob_thread, std::move(fun));
    }
    else {
      fun();
    }
  }

  void order_book_agent::replace(uint64_t order_id, order order) {
    auto fun = boost::bind(&order_book::replace, ob, order_id, std::move(order));
    if (ob_thread) {
      dispatch(*ob_thread, std::move(fun));
    }
    else {
      fun();
    }
  }
  void order_book_agent::cancel(uint64_t order_id, uint64_t quantity) {
    auto fun = boost::bind(&order_book::cancel, ob, order_id, quantity);
    if (ob_thread) {
      dispatch(*ob_thread, std::move(fun));
    }
    else {
      fun();
    }
  }

  void order_book_agent::remove(uint64_t order_id) {
    auto fun = boost::bind(&order_book::remove, ob, order_id);
    if (ob_thread) {
      dispatch(*ob_thread, std::move(fun));
    }
    else {
      fun();
    }
  }


  void order_book_agent::set_decimals_for_price(uint16_t dec) {
    auto fun = boost::bind(&order_book::set_decimals_for_price, ob, dec);
    if (ob_thread) {
      dispatch(*ob_thread, std::move(fun));
    }
    else {
      fun();
    }
  }
  
  execution order_book_agent::execute(uint64_t order_id, uint64_t quantity) {
    if (ob_thread)
    {
      auto ret = dispatch(*ob_thread,
                          use_future(
                            boost::bind(&order_book::execute, ob, order_id, quantity)
                          ));
      return ret.get();
    }
    else
    {
      return ob->execute(order_id, quantity);
    }
  }


  std::string_view order_book_agent::symbol() const {
    if (ob_thread)
    {
      auto ret = dispatch(*ob_thread,
                      use_future([this]
                                 {
                                   return ob->symbol();
                                 }));
      return ret.get();
    }
    else
    {
      return ob->symbol();
    }
  }

  uint64_t order_book_agent::timestamp() const {
    if (ob_thread)
    {
      auto ret = dispatch(*ob_thread,
                      use_future([this]
                                 {
                                   return ob->timestamp();
                                 }));
      return ret.get();
    }
    else
    {
      return ob->timestamp();
    }
  }

  trading_state order_book_agent::state() const {
    if (ob_thread)
    {
      auto ret = dispatch(*ob_thread,
                      use_future([this]
                                 {
                                   return ob->state();
                                 }));
      return ret.get();
    }
    else
    {
      return ob->state();
    }
  }

  std::string_view order_book_agent::state_name() const {
    if (ob_thread)
    {
      auto ret = dispatch(*ob_thread,
                      use_future([this]
                                 {
                                   return ob->state_name();
                                 }));
      return ret.get();
    }
    else
    {
      return ob->state_name();
    }
  }

  uint16_t order_book_agent::decimals_for_price() const {
    // price dec sürekli deðiþmeyecekse async alýp beklemeye gerek yok!
    return ob->decimals_for_price();
    //auto ret = dispatch(*ob_thread,
    //                use_future([this]
    //                           {
    //                             return ob->decimals_for_price();
    //                           }));
    //return ret.get();
  }

  side_type order_book_agent::side(uint64_t order_id) const {
    if (ob_thread)
    {
      auto ret = dispatch(*ob_thread,
                      use_future([&]
                                 {
                                   return ob->side(order_id);
                                 }));
      return ret.get();
    }
    else
    {
      return ob->side(order_id);
    }
  }

  size_t order_book_agent::bid_levels() const {
    if (ob_thread)
    {
      auto ret = dispatch(*ob_thread,
                      use_future([this]
                                 {
                                   return ob->bid_levels();
                                 }));
      return ret.get();
    }
    else
    {
      return ob->bid_levels();
    }
  }

  size_t order_book_agent::ask_levels() const {
    if (ob_thread)
    {
      auto ret = dispatch(*ob_thread,
                      use_future([this]
                                 {
                                   return ob->ask_levels();
                                 }));
      return ret.get();
    }
    else
    {
      return ob->ask_levels();
    }
  }

  size_t order_book_agent::order_count() const {
    if (ob_thread)
    {
      auto ret = dispatch(*ob_thread,
                      use_future([this]
                                 {
                                   return ob->order_count();
                                 }));
      return ret.get();
    }
    else
    {
      return ob->order_count();
    }
  }

  uint64_t order_book_agent::bid_price(size_t level) const {
    if (ob_thread)
    {
      auto ret = dispatch(*ob_thread,
                      use_future([&]
                                 {
                                   return ob->bid_price(level);
                                 }));
      return ret.get();
    }
    else
    {
      return ob->bid_price(level);
    }
  }

  uint64_t order_book_agent::bid_size(size_t level) const {
    if (ob_thread)
    {
      auto ret = dispatch(*ob_thread,
                      use_future([&]
                                 {
                                   return ob->bid_size(level);
                                 }));
      return ret.get();
    }
    else
    {
      return ob->bid_size(level);
    }
  }

  uint64_t order_book_agent::ask_price(size_t level) const {
    if (ob_thread)
    {
      auto ret = dispatch(*ob_thread,
                      use_future([&]
                                 {
                                   return ob->ask_price(level);
                                 }));
      return ret.get();
    }
    else
    {
      return ob->ask_price(level);
    }
  }

  uint64_t order_book_agent::ask_size(size_t level) const {
    if (ob_thread)
    {
      auto ret = dispatch(*ob_thread,
                      use_future([&]
                                 {
                                   return ob->ask_size(level);
                                 }));
      return ret.get();
    }
    else
    {
      return ob->ask_size(level);
    }
  }

  uint64_t order_book_agent::midprice(size_t level) const {
    if (ob_thread)
    {
      auto ret = dispatch(*ob_thread,
                      use_future([&]
                                 {
                                   return ob->midprice(level);
                                 }));
      return ret.get();
    }
    else
    {
      return ob->midprice(level);
    }
  }

  price_level order_book_agent::bid_level(size_t level) const {
    if (ob_thread)
    {
      auto ret = dispatch(*ob_thread,
                      use_future([&]
                                 {
                                   return ob->bid_level(level);
                                 }));
      return ret.get();
    }
    else
    {
      return ob->bid_level(level);
    }
  }

  price_level order_book_agent::ask_level(size_t level) const {
    if (ob_thread)
    {
      auto ret = dispatch(*ob_thread,
                      use_future([&]
                                 {
                                   return ob->ask_level(level);
                                 }));
      return ret.get();
    }
    else
    {
      return ob->ask_level(level);
    }
  }

} // namespace helix