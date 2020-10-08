#include "helix.hh"

namespace helix {

event::event(event_mask mask, 
             std::string_view symbol, 
             uint64_t timestamp, 
             order_book_agent&& ob, 
             trade&& t)
  : _mask{mask}
  , _symbol{ symbol }
  , _timestamp{ timestamp }
  , _ob{ std::move(ob) }
  , _trade{ std::move(t) }
{ 
}

event_mask event::get_mask() const
{
    return _mask;
}

std::string_view event::get_symbol() const
{
    return _symbol;
}

uint64_t event::get_timestamp() const
{
    return _timestamp;
}

order_book_agent* event::get_ob() const
{
  return const_cast<order_book_agent*>(&_ob);
}

trade* event::get_trade() const
{
  return const_cast<trade*>(&_trade);
}

std::shared_ptr<event> make_event(std::string_view symbol, uint64_t timestamp, order_book_agent&& ob, trade&& t, event_mask mask)
{
  return std::make_shared<event>(mask | ev_order_book_update | ev_trade, symbol, timestamp, std::move(ob), std::move(t));
}
std::shared_ptr<event> make_sys_event(uint64_t timestamp, event_mask mask)
{
  return std::make_shared<event>(mask, "", timestamp, order_book_agent{}, trade{});
}

std::shared_ptr<event> make_ob_event(std::string_view symbol, uint64_t timestamp, order_book_agent&& ob, event_mask mask)
{
  return std::make_shared<event>(mask | ev_order_book_update, symbol, timestamp, std::move(ob), trade {});
}

std::shared_ptr<event> make_trade_event(std::string_view symbol, uint64_t timestamp, order_book_agent&& ob, trade&& t, event_mask mask)
{
  return std::make_shared<event>(mask | ev_trade, symbol, timestamp, std::move(ob), std::move(t));
}

}
