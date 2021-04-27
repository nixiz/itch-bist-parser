#include "order_book.hh"

#include <boost/version.hpp>
#include <stdexcept>
#include <limits>
#include <mutex>
#include <fmt/core.h>

namespace helix {
  std::mutex guard;

  execution::execution(uint64_t price, side_type side, uint64_t remaining)
    : price{ price }
    , side{ side }
    , remaining{ remaining }
    , is_valid{ true }
  { }

  order_book::order_book(std::string symbol, uint64_t timestamp, size_t max_orders)
    : order_book(symbol, timestamp, 0, max_orders) { }

  order_book::order_book(std::string symbol,
                         uint64_t timestamp,
                         uint16_t num_decimals_for_price,
                         size_t max_orders)
    : _symbol{ std::move(symbol) }
    , _timestamp{ timestamp }
    , _state{ trading_state::unknown }
    , _num_decimals_for_price(num_decimals_for_price)
    , _max_orders(max_orders)
  {
    _orders.reserve(max_orders);
  }

  void order_book::add(order order)
  {
    //std::scoped_lock lock(guard);
    switch (order.side) {
    case side_type::buy: {
      auto&& level = lookup_or_create(_bids, order.price);
      order.level = &level;
      level.size += order.quantity;
      break;
    }
    case side_type::sell: {
      auto&& level = lookup_or_create(_asks, order.price);
      order.level = &level;
      level.size += order.quantity;
      break;
    }
    default:
    throw std::invalid_argument(std::string("invalid side: ") + static_cast<char>(order.side));
    }
    _orders.emplace(std::move(order));
  }

  void order_book::replace(uint64_t order_id, order order)
  {
    remove(order_id);
    add(std::move(order));
  }

  void order_book::cancel(uint64_t order_id, uint64_t quantity)
  {
    //std::scoped_lock lock(guard);
    auto it = _orders.find(order_id);
    if (it == _orders.end()) {
      fmt::print("\norder_book::cancel()::order id: {} with symbol: {}", order_id, this->symbol());
      return;
      //throw std::invalid_argument(std::string("invalid order id: ") + std::to_string(order_id));
    }
    
    _orders.modify(it, [quantity](order& order) {
      order.quantity -= quantity;
      order.level->size -= quantity;
                   });

    if (!it->quantity) {
      remove_impl(it);
    }
  }

  execution order_book::execute(uint64_t order_id, uint64_t quantity)
  {
    //std::scoped_lock lock(guard);
    auto it = _orders.find(order_id);
    if (it == _orders.end()) {
      fmt::print("\norder_book::execute()::order id: {} with symbol: {}", order_id, this->symbol());
      return execution{};
      //throw std::invalid_argument(std::string("invalid order id: ") + std::to_string(order_id));
    }
    _orders.modify(it, [quantity](order& order) {
      order.quantity -= quantity;
      order.level->size -= quantity;
                   });
    auto result = execution(it->price, it->side, it->level->size);
    if (!it->quantity) {
      remove_impl(it);
    }
    return result;
  }

  void order_book::remove(uint64_t order_id)
  {
    //std::scoped_lock lock(guard);
    auto it = _orders.find(order_id);
    if (it == _orders.end()) {
      fmt::print("\norder_book::remove()order id: {} with symbol: {}", order_id, this->symbol());
      return;
      //throw std::invalid_argument(std::string("invalid order id: ") + std::to_string(order_id));
    }
    remove_impl(it);
  }

  void order_book::remove_impl(iterator& iter)
  {
    auto&& order = *iter;
    switch (order.side) {
    case side_type::buy: {
      remove_impl(order, _bids);
      break;
    }
    case side_type::sell: {
      remove_impl(order, _asks);
      break;
    }
    default:
    throw std::invalid_argument(std::string("invalid side: ") + static_cast<char>(order.side));
    }
    _orders.erase(iter);
  }

  template<typename T>
  void order_book::remove_impl(const order& o, T& levels)
  {
    auto it = levels.find(o.price);
    if (it == levels.end()) {
      fmt::print("\norder_book::remove_impl<T>() price: {} with symbol: {}", o.price, this->symbol());
      return;
      //throw std::invalid_argument(std::string("invalid price: ") + std::to_string(o.price));
    }
    auto&& level = it->second;
    o.level->size -= o.quantity;
    if (level.size == 0) {
      levels.erase(it);
    }
  }

  template<typename T>
  price_level& order_book::lookup_or_create(T& levels, uint64_t price)
  {
    price_level level{ price };
    auto it = levels.emplace(price, std::move(level)).first;
    return it->second;
  }

  side_type order_book::side(uint64_t order_id) const
  {
    //std::scoped_lock lock(guard);
    auto it = _orders.find(order_id);
    if (it == _orders.end()) {
      fmt::print("\norder_book::side()order id: {} with symbol: {} ", order_id, this->symbol());
      return static_cast<side_type>(0);
      //throw std::invalid_argument(std::string("invalid order id: ") + std::to_string(order_id));
    }
    return it->side;
  }


  size_t order_book::bid_levels() const
  {
    //std::scoped_lock lock(guard);
    return _bids.size();
  }

  size_t order_book::ask_levels() const
  {
    //std::scoped_lock lock(guard);
    return _asks.size();
  }

  size_t order_book::order_count() const
  {
    //std::scoped_lock lock(guard);
    return _orders.size();
  }

  uint64_t order_book::bid_price(size_t level) const
  {
    //std::scoped_lock lock(guard);
    auto it = _bids.begin();
    while (it != _bids.end() && level--) {
      it++;
    }
    if (it != _bids.end()) {
      auto&& level = it->second;
      return level.price;
    }
    return std::numeric_limits<uint64_t>::min();
  }

  uint64_t order_book::bid_size(size_t level) const
  {
    //std::scoped_lock lock(guard);
    auto it = _bids.begin();
    while (it != _bids.end() && level--) {
      it++;
    }
    if (it != _bids.end()) {
      auto&& level = it->second;
      return level.size;
    }
    return 0;
  }

  price_level order_book::bid_level(size_t level) const
  {
    auto it = _bids.begin();
    while (it != _bids.end() && level--) {
      it++;
    }
    if (it != _bids.end()) {
      auto&& level = it->second;
      return level;
    }
    return price_level{};
  }

  uint64_t order_book::ask_price(size_t level) const
  {
    //std::scoped_lock lock(guard);
    auto it = _asks.begin();
    while (it != _asks.end() && level--) {
      it++;
    }
    if (it != _asks.end()) {
      auto&& level = it->second;
      return level.price;
    }
    return std::numeric_limits<uint64_t>::max();
  }

  uint64_t order_book::ask_size(size_t level) const
  {
    //std::scoped_lock lock(guard);
    auto it = _asks.begin();
    while (it != _asks.end() && level--) {
      it++;
    }
    if (it != _asks.end()) {
      auto&& level = it->second;
      return level.size;
    }
    return 0;
  }

  price_level order_book::ask_level(size_t level) const
  {
    auto it = _asks.begin();
    while (it != _asks.end() && level--) {
      it++;
    }
    if (it != _asks.end()) {
      auto&& level = it->second;
      return level;
    }
    return price_level{};
  }

  uint64_t order_book::midprice(size_t level) const
  {
    auto bid = bid_price(level);
    auto ask = ask_price(level);
    return (bid + ask) / 2;
  }

}
