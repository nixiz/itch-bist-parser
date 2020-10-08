#include "symbol_tracker_algo.h"

#include <sstream>
#include <memory>
#include <iomanip>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

namespace helix 
{
	struct trace_session {
		double	  bid_price = 0;
		uint64_t	bid_size = 0;
		double	  ask_price = +INFINITY;
		uint64_t	ask_size = 0;

		size_t max_price_levels = 0;
		size_t max_order_count = 0;
		uint64_t quotes = 0;
		uint64_t trades = 0;
		uint64_t volume_shs = 0;
		double volume_ccy = 0.0;
		double high = -INFINITY;
		double low = +INFINITY;
	};

	struct trace_fmt_ops 
	{
		virtual ~trace_fmt_ops() {
			if (output) {
				fflush(output);
				fclose(output);
			}
		}

		void init(std::string_view out_str) {
			if (!out_str.empty()) {
				auto res = fopen_s(&output, out_str.data(), "w");
				flush = false;
				if (!output || res != 0) {
					fprintf(stderr, "error: %s: %s\n", out_str.data(), strerror(errno));
					exit(1);
				}
			}
			else {
				output = stdout;
				flush = true;
			}
			flush = true;
		}
		
		trace_session* get_ts() {
			return &ts;
		}

		virtual void fmt_header(void) = 0;
		virtual void fmt_event(session* session, event* event) = 0;
		virtual void fmt_footer(session* session, event* event) {}
	protected:
		FILE* output = NULL;
		bool flush = false;
		trace_session ts;
	};

	const char trade_sign_c(trade_sign sign)
	{
		switch (sign)
		{
		case helix::trade_sign::buyer_initiated:  return 'B';
		case helix::trade_sign::seller_initiated: return 'S';
		case helix::trade_sign::crossing:					return 'C';
		case helix::trade_sign::non_displayable:	return 'N';
		default: return '?';
		}
	}

	static double get_price(order_book_agent* ob, uint64_t price)
	{
		double p = 0.0;
		[[unlikely]] if (auto dec = ob->decimals_for_price();
										 dec == 256)
		{
			p = static_cast<double>(price) / 256.0;
		}
		else
		{
			p = static_cast<double>(price) / std::pow(10.0, dec);
		}
		return p;
	}

	//static bool is_order_book_changed(const trace_session& ts, event* event)
	//{
	//	auto event_mask = event->get_mask();
	//	if (event_mask & ev_trade) {
	//		return true;
	//	}
	//	if (event_mask & ev_order_book_update) {
	//		auto ob = event->get_ob();
	//		auto bid_price = get_price(ob, ob->bid_price(0));
	//		auto bid_size = ob->bid_size(0);
	//		auto ask_price = get_price(ob, ob->ask_price(0));
	//		auto ask_size = ob->ask_size(0);
	//		if (!bid_price || !ask_size) {
	//			return false;
	//		}
	//		return	bid_price != ts.bid_price || 
	//						bid_size != ts.bid_size		|| 
	//						ask_price != ts.ask_price || 
	//						ask_size != ts.ask_size;
	//	}
	//	return false;
	//}

	struct fmt_pretty_ops final
		: trace_fmt_ops
	{
		void fmt_header(void) override {
			fprintf(output, "Timestamp: hours:minutes:seconds:milliseconds(6 precision)\n");
			fprintf(output, "T: Trade Sign:\n");
			fprintf(output, "\tB: Buyer initiated\n");
			fprintf(output, "\tS: Seller initiated\n");
			fprintf(output, "\tC: Sign crossing\n");
			fprintf(output, "\tN: Sign non displayable\n");
			fprintf(output, "Y: Sweep Event Flag\n");
			fprintf(output, " SYMBOL  | Time Stamp USec |  BidSz   Bid$   Ask$   AskSz  |  Last$  LSize | T |  VWAP   | Y |\n");
		}

		void fmt_footer(session* session, event* event) override 
		{
			if (event->get_mask() & ev_closed) {
				fprintf(output, "quotes: %" PRId64  ", trades: %" PRId64 " , max levels: %zu, max orders: %zu\n", ts.quotes, ts.trades, ts.max_price_levels, ts.max_order_count);
				fprintf(output, "volume (mio): %.4lf, notional (mio): %.4lf, VWAP: %.3lf, high: %.3lf, low: %.3lf\n", (double)ts.volume_shs * 1e-6, ts.volume_ccy * 1e-6, (ts.volume_ccy / (double)ts.volume_shs), ts.high, ts.low);
			}
		}

		void fmt_event(session* session, event* event) override
		{
			using namespace std::chrono;
			auto timestamp = event->get_timestamp();
			if (!session->is_rth_timestamp(timestamp)) {
				return;
			}
			nanoseconds ns(*reinterpret_cast<uint64_t*>(&timestamp));
			time_point<system_clock, seconds> tp(duration_cast<seconds>(ns));
			auto tm = system_clock::to_time_t(tp);
			auto lcltm = std::localtime(&tm);

			const uint64_t hours = lcltm->tm_hour;
			const uint64_t minutes = lcltm->tm_min;
			const uint64_t seconds = lcltm->tm_sec;
			const uint64_t milliseconds = timestamp % 1000000;

			auto ob = event->get_ob();

			auto bid_level = ob->bid_level(0);
			auto ask_level = ob->ask_level(0);

			auto bid_price = get_price(ob, bid_level.price);
			auto bid_size  = bid_level.size;
			auto ask_price = get_price(ob, ask_level.price);
			auto ask_size  = ask_level.size;

			// burasý neden böyle mutlaka öðren!
			if (!bid_price || !ask_size) {
				//fprintf(output, " ___*-*-*-*-*-*-*-*-*-*-*-*___ |");
				return;
			}

			const bool has_ob_changed = bid_price != ts.bid_price || 
																	ask_price != ts.ask_price || 
																	bid_size  != ts.bid_size  || 
																	ask_size  != ts.ask_size;
			if (!has_ob_changed) return;

			fprintf(output, "%s | %02" PRIu64":%02" PRIu64":%02" PRIu64".%06" PRIu64 " |",
							event->get_symbol().data(),
							hours, minutes, seconds, milliseconds);
			auto event_mask = event->get_mask();

			if (event_mask & ev_order_book_update) 
			{				
				fprintf(output, "%6" PRIu64"  %6.3f  %6.3f  %-6" PRIu64" |",
								bid_size,
								bid_price,
								ask_price,
								ask_size
				);
				ts.bid_price = bid_price;
				ts.bid_size = bid_size;
				ts.ask_price = ask_price;
				ts.ask_size = ask_size;
			}
			else {
				fprintf(output, "                               |");
			}

			if (event_mask & ev_trade) {
				auto ob = event->get_ob();
				auto trade = event->get_trade();
				fprintf(output, " %6.3f %6" PRIu64 " | %c | %6.3f  |",
								get_price(ob, trade->price),
								trade->size,
								trade_sign_c(trade->sign),
								ts.volume_ccy / (double)ts.volume_shs
				);
			}
			else {
				fprintf(output, "               |   |         |");
			}
			
			std::string sweep_event;
			if (event_mask & ev_sweep) {
				sweep_event = " Y |";
			}
			else {
				sweep_event = "   |";
			}
			
			fprintf(output, "%s", sweep_event.c_str());
			fprintf(output, "\n");
			if (flush) {
				fflush(output);
			}
		}
	};

	static void process_ob_event(trace_session* ts, order_book_agent* ob, event_mask event_mask)
	{
		size_t bid_levels = ob->bid_levels();
		size_t ask_levels = ob->ask_levels();
		size_t order_count = ob->order_count();

		ts->max_price_levels = bid_levels > ts->max_price_levels ? bid_levels : ts->max_price_levels;
		ts->max_price_levels = ask_levels > ts->max_price_levels ? ask_levels : ts->max_price_levels;
		ts->max_order_count = order_count > ts->max_order_count ? order_count : ts->max_order_count;
		ts->quotes++;
	}

	static void process_trade_event(trace_session* ts, order_book_agent* ob, trade* trade, event_mask event_mask)
	{
		double trade_price = get_price(ob, trade->price);
		uint64_t trade_size = trade->size;
		ts->volume_shs += trade_size;
		ts->volume_ccy += (double)trade_size * trade_price;
		ts->high = trade_price > ts->high ? trade_price : ts->high;
		ts->low = trade_price < ts->low ? trade_price : ts->low;
		ts->trades++;
	}

  symbol_tracker_algo* symbol_tracker_algo::create_new_algo(
		std::weak_ptr<session> session, 
		std::string symbol)
  {
    return new symbol_tracker_algo(session, symbol);
  }

  symbol_tracker_algo::symbol_tracker_algo(
		std::weak_ptr<session> s,
		std::string symbol)
    : algo_base(s)
  {
		impl.reset(new fmt_pretty_ops);
		std::stringstream s_str;
		s_str << "D:/hft/results/" << "result_" << symbol << ".out";
		impl->init(s_str.str());
		impl->fmt_header();
		register_and_subscribe(symbol, 1000);
		//get_session()->subscribe(symbol, 1000);
	}

	symbol_tracker_algo::~symbol_tracker_algo() {
		//_pool.stop();
		//_pool.join();
		get_event_pool()->join();
		impl.reset();
	}

  int symbol_tracker_algo::tick(event* ev) {
		auto mask = ev->get_mask();
		auto session = get_session();
		if (mask & ev_opened || mask & ev_closed) {
			// TODO(): do whatever when bist opened or closed!
			puts("bist opened/closed event consumed.");
			impl->fmt_footer(session.get(), ev);
			return 0;
		}
		if (mask & ev_order_book_update) {
			auto ob = ev->get_ob();
			process_ob_event(impl->get_ts(), ob, mask);
		}
		if (mask & ev_trade) {
			auto ob = ev->get_ob();
			auto trade = ev->get_trade();
			process_trade_event(impl->get_ts(), ob, trade, mask);
		}
		impl->fmt_event(session.get(), ev);
		return 0;
	}

} // namespace helix