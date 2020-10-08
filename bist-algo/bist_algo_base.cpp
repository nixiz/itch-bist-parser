#include "bist_algo_base.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/ts/executor.hpp>

using boost::asio::post;
using boost::asio::thread_pool;
using boost::asio::use_future;

namespace helix
{
	algo_base::algo_base(std::weak_ptr<session> s)
		: _session(std::move(s))
	{
		if constexpr (true)
		{
			_session.lock()->register_callback(
				[this](std::shared_ptr<helix::event> ev)
				{
					// use internal event pool to trampoline event_handled in algo thread.
					post(_pool, boost::bind(&algo_base::event_handled, this, ev));
				});
		}
		else
		{
			_session.lock()->register_callback(std::bind(&algo_base::event_handled, this, std::placeholders::_1));
		}
		_working = true;
	}

	algo_base::~algo_base() = default;

	// will call run() loop after initializing order book handler and registering for necessary events
	void algo_base::start() {
		_working = true;
	}

	// will terminate loop as soon as possible and return the algo to idle state
	void algo_base::stop() {
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