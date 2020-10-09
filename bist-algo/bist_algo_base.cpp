#include "bist_algo_base.h"
#include <boost/asio/io_context.hpp>
#include <algorithm>

namespace helix
{
	algo_base::algo_base(std::weak_ptr<session> s)
		: _session(std::move(s))
	{
		_working = true;
		register_callback();
	}
	algo_base::~algo_base() = default;

	void algo_base::create_ob_with_symbols(std::vector<std::pair<std::string, size_t>> symbols)
	{
		ob_sym_map.reserve(symbols.size());
		for (const auto& [symb, max_order] : symbols) {
			ob_sym_map.insert({symb, order_book{symb, 0, max_order} });
			auto& ob = ob_sym_map.at(symb);
			get_session()->register_for_symbol(symb, std::make_unique<order_book_agent>(&_pool, &ob));
		}
	}

	helix::order_book* algo_base::get_ob_for_sym(std::string sym) {
		if (auto it = ob_sym_map.find(sym);
				it != ob_sym_map.end())
		{
			return std::addressof(it->second);
		}
		return nullptr;
	}

	helix::order_book const* algo_base::get_ob_for_sym(std::string sym) const
	{
		if (auto it = ob_sym_map.find(sym);
				it != ob_sym_map.end())
		{
			return std::addressof(it->second);
		}
		return nullptr;
	}

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