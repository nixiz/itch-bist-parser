#include "bist_algo_base.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/ts/executor.hpp>

using boost::asio::post;
using boost::asio::thread_pool;
using boost::asio::use_future;

namespace helix
{
	algo_base::algo_base(std::unique_ptr<session> s)
		: _session(std::move(s))
	{
		_session->register_callback(
			[this](std::shared_ptr<helix::event> ev)
			{
				// use internal event pool to trampoline event_handled in algo thread.
				post(_pool, boost::bind(&algo_base::event_handled, this, ev));
			});
		_working = true;
	}

	algo_base::~algo_base() {
		_pool.stop();
		_pool.join();
	}

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

}