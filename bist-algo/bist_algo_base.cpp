#include "bist_algo_base.h"
#include <boost/asio/io_context.hpp>

namespace helix
{
	algo_base::algo_base(std::weak_ptr<session> s)
		: _session(std::move(s))
	{
		//defer(boost::bind(&algo_base::register_and_subscribe, this));
		_working = true;
	}

	algo_base::~algo_base() = default;

	// will call run() loop after initializing order book handler and registering for necessary events
	void algo_base::start() {
		_working = true;
	}

	// will terminate loop as soon as possible and return the algo to idle state
	void algo_base::stop() {
		_pool.join();
		_working = false;
	}

	void algo_base::event_handled(std::shared_ptr<event> ev) 
	{
		auto res = tick(ev.get());
	}

	std::shared_ptr<session> algo_base::get_session() {
		return _session.lock();
	}

	std::shared_ptr<session> algo_base::get_session() const {
		return _session.lock();
	}

}