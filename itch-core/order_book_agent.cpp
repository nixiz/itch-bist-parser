#include "order_book_agent.h"
#include <boost/asio/ts/executor.hpp>
#include <boost/asio/io_context.hpp>

using boost::asio::post;
using boost::asio::thread_pool;
using boost::asio::use_future;

namespace helix
{
  order_book_agent::order_book_agent(
    thread_pool* ob_thread_, 
    order_book* ob_) 
    : ob_thread(ob_thread_)
    , ob(ob_) { }

  std::string_view order_book_agent::symbol() const {
    auto ret = post(*ob_thread, 
                    use_future([this]
                               {
                                 return ob->symbol();
                               }));
    return ret.get();
  }

  uint64_t order_book_agent::timestamp() const {
    auto ret = post(*ob_thread,
                    use_future([this]
                               {
                                 return ob->timestamp();
                               }));
    return ret.get();
  }

  trading_state order_book_agent::state() const {
    auto ret = post(*ob_thread,
                    use_future([this]
                               {
                                 return ob->state();
                               }));
    return ret.get();
  }

  std::string_view order_book_agent::state_name() const {
    auto ret = post(*ob_thread,
                    use_future([this]
                               {
                                 return ob->state_name();
                               }));
    return ret.get();
  }

  uint16_t order_book_agent::decimals_for_price() const {
    // price dec sürekli deðiþmeyecekse async alýp beklemeye gerek yok!
    return ob->decimals_for_price();
    //auto ret = post(*ob_thread,
    //                use_future([this]
    //                           {
    //                             return ob->decimals_for_price();
    //                           }));
    //return ret.get();
  }

  side_type order_book_agent::side(uint64_t order_id) const {
    auto ret = post(*ob_thread,
                    use_future([=]
                               {
                                 return ob->side(order_id);
                               }));
    return ret.get();
  }

  size_t order_book_agent::bid_levels() const {
    auto ret = post(*ob_thread,
                    use_future([this]
                               {
                                 return ob->bid_levels();
                               }));
    return ret.get();
  }

  size_t order_book_agent::ask_levels() const {
    auto ret = post(*ob_thread,
                    use_future([this]
                               {
                                 return ob->ask_levels();
                               }));
    return ret.get();
  }

  size_t order_book_agent::order_count() const {
    auto ret = post(*ob_thread,
                    use_future([this]
                               {
                                 return ob->order_count();
                               }));
    return ret.get();
  }

  uint64_t order_book_agent::bid_price(size_t level) const {
    auto ret = post(*ob_thread,
                    use_future([=]
                               {
                                 return ob->bid_price(level);
                               }));
    return ret.get();
  }

  uint64_t order_book_agent::bid_size(size_t level) const {
    auto ret = post(*ob_thread,
                    use_future([=]
                               {
                                 return ob->bid_size(level);
                               }));
    return ret.get();
  }

  uint64_t order_book_agent::ask_price(size_t level) const {
    auto ret = post(*ob_thread,
                    use_future([=]
                               {
                                 return ob->ask_price(level);
                               }));
    return ret.get();
  }

  uint64_t order_book_agent::ask_size(size_t level) const {
    auto ret = post(*ob_thread,
                    use_future([=]
                               {
                                 return ob->ask_size(level);
                               }));
    return ret.get();
  }

  uint64_t order_book_agent::midprice(size_t level) const {
    auto ret = post(*ob_thread,
                    use_future([=]
                               {
                                 return ob->midprice(level);
                               }));
    return ret.get();
  }

  price_level order_book_agent::bid_level(size_t level) const {
    auto ret = post(*ob_thread,
                    use_future([=]
                               {
                                 return ob->bid_level(level);
                               }));
    return ret.get();
  }

  price_level order_book_agent::ask_level(size_t level) const {
    auto ret = post(*ob_thread,
                    use_future([=]
                               {
                                 return ob->ask_level(level);
                               }));
    return ret.get();
  }

} // namespace helix