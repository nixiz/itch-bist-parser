#include "helix.hh"

namespace helix {

event::event(event_mask mask, 
             std::string symbol,
             uint64_t timestamp, 
             trade&& t)
  : _mask{mask}
  , _symbol{ symbol }
  , _timestamp{ timestamp }
  , _trade{ std::move(t) }
{ 
}

event_mask event::get_mask() const
{
    return _mask;
}

std::string event::get_symbol() const
{
    return _symbol;
}

uint64_t event::get_timestamp() const
{
    return _timestamp;
}

trade* event::get_trade() const
{
  return const_cast<trade*>(&_trade);
}

std::shared_ptr<event> make_event(std::string symbol, uint64_t timestamp, trade&& t, event_mask mask)
{
  return std::make_shared<event>(mask | ev_order_book_update | ev_trade, symbol, timestamp, std::move(t));
}
std::shared_ptr<event> make_sys_event(uint64_t timestamp, event_mask mask)
{
  return std::make_shared<event>(mask, "", timestamp, trade{});
}

std::shared_ptr<event> make_ob_event(std::string symbol, uint64_t timestamp, event_mask mask)
{
  return std::make_shared<event>(mask | ev_order_book_update, symbol, timestamp, trade {});
}

std::shared_ptr<event> make_trade_event(std::string symbol, uint64_t timestamp, trade&& t, event_mask mask)
{
  return std::make_shared<event>(mask | ev_trade, symbol, timestamp, std::move(t));
}

}
