#pragma once

/// \defgroup order-book Order book management
///
/// Order book management provides support for reconstructing and
/// querying per-asset order book state such as top and depth of book bid
/// and ask price and size.

#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>

#include <unordered_map>
#include <cstdint>
#include <utility>
#include <memory>
#include <string>
#include <list>
#include <map>
#include <unordered_map>
#include <memory_resource>

namespace helix {

/// \addtogroup order-book
/// @{

/// \brief Order side.
enum class side_type : uint8_t {
    /// Buy order
    buy = 1,
    /// Sell order
    sell = 2,
};

/// \brief Trading state.
enum class trading_state : uint8_t {
    /// Trading state is unknown.
    unknown,
    /// Trading is halted.
    halted,
    /// Trading is paused.
    paused,
    /// Quotation only period.
    quotation_only,
    /// Trading is ongoing.
    trading,
    /// Auction period.
    auction,
};

struct price_level;

/// \brief Order is a request to buy or sell quantity of asset at a
/// specified price.
struct order final {
    price_level* level;
    uint64_t     id;
    uint64_t     price;
    uint64_t     quantity;
    uint64_t     timestamp;
    side_type    side;

    order(uint64_t id, uint64_t price, uint64_t quantity, side_type side, uint64_t timestamp)
        : level{nullptr}
        , id{id}
        , price{price}
        , quantity{quantity}
        , side{side}
        , timestamp{timestamp}
    {}
};

struct order_hash {
  size_t operator()(const order& ord) const {
    return std::hash<decltype(order::id)>()(ord.id);
  }
};

struct order_equal_to {
  bool operator()(const order& lhs, const order& rhs) const {
    return lhs.id == rhs.id;
  }
};

/// \brief Price level is a time-prioritized list of orders with the same price.
struct price_level {
    explicit price_level(uint64_t price_ = 0)
        : price(price_)
        , size(0)
    { }

    uint64_t price;
    uint64_t size;
};

/// \brief Order execution details.
struct execution {
  //! The price order was executed with.
  uint64_t price {0};
  //! The side of the liquidity taker of the trade.
  side_type side{0};
  //! The number of remaining quantity on the traded price level.
  uint64_t remaining{ 0 };
  bool is_valid{ false };
  execution() = default;
  execution(uint64_t price, side_type side, uint64_t remaining);
  bool valid() const { return is_valid; }
};

/// \brief Order book is a price-time prioritized list of buy and sell
/// orders.
class order_book {
    using order_set = boost::multi_index::multi_index_container<
                          order,
                          boost::multi_index::indexed_by<
                              boost::multi_index::hashed_unique<
                                  boost::multi_index::member<order, uint64_t, &order::id>
                              >
                          >
                      >;

    /* boyle olacak herhalde?!?
        using order_set = boost::multi_index::multi_index_container<
                          order,
                          boost::multi_index::indexed_by<
                              boost::multi_index::hashed_non_unique<
                                  boost::multi_index::composite_key<
                                    order,
                                    boost::multi_index::member<order, uint64_t, &order::id>,
                                    boost::multi_index::member<order, side_type, &order::side>
                                  > 
                              >
                          >
                      >;
    */
    /*
    * ayn? order::id ile hem buy hem de sell order ekleme mesaj? gelebiliyor. "IDGYO.E" i?in b?yle bir data var. 
    * bu nedenle multi_index_container unique olmak yerine order::id ve order::side'a g?re tutulmal?
    */
    //using order_set = std::unordered_map<decltype(order::id), order>;
    using iterator = order_set::const_iterator;

    std::string _symbol;
    std::string _state_name;
    uint64_t _timestamp;
    trading_state _state;
    uint16_t _num_decimals_for_price; // A value of 256 means that the instrument is traded in fractions (each fraction is 1/256). 
    order_set _orders;
    size_t _max_orders;
    std::map<uint64_t, price_level, std::greater<uint64_t>> _bids;
    std::map<uint64_t, price_level, std::less   <uint64_t>> _asks;
public:
    order_book(std::string symbol, uint64_t timestamp, size_t max_orders = 0);
    order_book(std::string symbol, uint64_t timestamp, uint16_t num_decimals_for_price, size_t max_orders = 0);

    std::string_view symbol() const {
        return _symbol;
    }

    void set_timestamp(uint64_t timestamp) {
        _timestamp = timestamp;
    }

    uint64_t timestamp() const {
        return _timestamp;
    }

    void set_state(trading_state state) {
        _state = state;
    }

    trading_state state() const {
        return _state;
    }

    void set_state_name(std::string state_name) {
      _state_name = std::move(state_name);
    }

    std::string_view state_name() const {
      return _state_name;
    }

    void set_decimals_for_price(uint16_t dec) {
      _num_decimals_for_price = dec;
    }

    uint16_t decimals_for_price() const {
      return _num_decimals_for_price;
    }

    size_t max_orders() const {
      return _max_orders;
    }

    void add(order order);
    void replace(uint64_t order_id, order order);
    void cancel(uint64_t order_id, uint64_t quantity);
    execution execute(uint64_t order_id, uint64_t quantity);
    void remove(uint64_t order_id);
    side_type side(uint64_t order_id) const;

    size_t bid_levels() const;
    size_t ask_levels() const;
    size_t order_count() const;

    uint64_t bid_price(size_t level) const;
    uint64_t bid_size (size_t level) const;
    price_level bid_level(size_t level) const;

    uint64_t ask_price(size_t level) const;
    uint64_t ask_size (size_t level) const;
    price_level ask_level(size_t level) const;
    uint64_t midprice (size_t level) const;

private:
    void remove_impl(iterator& iter);

    template<typename T>
    void remove_impl(const order& o, T& levels);

    template<typename T>
    price_level& lookup_or_create(T& levels, uint64_t price);

    friend class order_book_agent;
};

/// @}

}
