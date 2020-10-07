#include "helix.hh"

namespace helix {

event::event(event_mask mask, std::string symbol, uint64_t timestamp, order_book* ob, trade t)
    : _mask{mask}
    , _symbol{symbol}
    , _timestamp{timestamp}
    , _ob{ob}
    , _trade{std::move(t)}
{
}

event_mask event::get_mask() const
{
    return _mask;
}

const std::string& event::get_symbol() const
{
    return _symbol;
}

uint64_t event::get_timestamp() const
{
    return _timestamp;
}

order_book* event::get_ob() const
{
    return _ob;
}

trade* event::get_trade() const
{
  return const_cast<trade*>(&_trade);
}

std::shared_ptr<event> make_event(const std::string& symbol, uint64_t timestamp, order_book* ob, trade&& t, event_mask mask)
{
  return std::make_shared<event>(mask | ev_order_book_update | ev_trade, symbol, timestamp, ob, std::move(t));
}
std::shared_ptr<event> make_sys_event(uint64_t timestamp, event_mask mask)
{
  return std::make_shared<event>(mask, "", timestamp, nullptr, trade{});
}

std::shared_ptr<event> make_ob_event(const std::string& symbol, uint64_t timestamp, order_book* ob, event_mask mask)
{
  return std::make_shared<event>(mask | ev_order_book_update, symbol, timestamp, ob, trade {});
}

std::shared_ptr<event> make_trade_event(const std::string& symbol, uint64_t timestamp, order_book* ob, trade&& t, event_mask mask)
{
  return std::make_shared<event>(mask | ev_trade, symbol, timestamp, ob, std::move(t));
}

}
