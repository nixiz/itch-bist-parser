#include "symbol_tracker_algo.h"
#include "nasdaq/itch_bist_protocol.hh"
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

	struct trace_fmt_ops {
		trace_fmt_ops() = default;
		virtual ~trace_fmt_ops() {
			if (output) {
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
		}

		virtual void fmt_header(void) = 0;
		virtual void fmt_event(session* session, event* event) = 0;
	protected:
		FILE* output = NULL;
		bool flush = false;
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

	static double get_price(order_book* ob, uint64_t price)
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

	static bool is_order_book_changed(session* session, event* event)
	{
		auto event_mask = event->get_mask();
		if (event_mask & ev_trade) {
			return true;
		}
		if (event_mask & ev_order_book_update) {
			auto* ts = reinterpret_cast<trace_session*>(session->data());
			auto ob = event->get_ob();
			auto bid_price = get_price(ob, ob->bid_price(0));
			auto bid_size = ob->bid_size(0);
			auto ask_price = get_price(ob, ob->ask_price(0));
			auto ask_size = ob->ask_size(0);
			if (!bid_price || !ask_size) {
				return false;
			}
			return bid_price != ts->bid_price || bid_size != ts->bid_size || ask_price != ts->ask_price || ask_size != ts->ask_size;
		}
		return false;
	}

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

		void fmt_event(session* session, event* event) override
		{
			using namespace std::chrono;
			auto* ts = reinterpret_cast<trace_session*>(session->data());
			auto timestamp = event->get_timestamp();
			if (!session->check_is_working_time(timestamp) || 
					!is_order_book_changed(session, event)) 
			{
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
			fprintf(output, "%s | %02" PRIu64":%02" PRIu64":%02" PRIu64".%06" PRIu64 " |",
							event->get_symbol().c_str(),
							hours, minutes, seconds, milliseconds);
			auto event_mask = event->get_mask();
			if (event_mask & ev_order_book_update) {
				auto ob = event->get_ob();

				auto bid_price = get_price(ob, ob->bid_price(0));
				auto bid_size = ob->bid_size(0);
				auto ask_price = get_price(ob, ob->ask_price(0));
				auto ask_size = ob->ask_size(0);

				fprintf(output, "%6" PRIu64"  %6.3f  %6.3f  %-6" PRIu64" |",
								bid_size,
								(double)bid_price,
								(double)ask_price,
								ask_size
				);

				ts->bid_price = bid_price;
				ts->bid_size = bid_size;
				ts->ask_price = ask_price;
				ts->ask_size = ask_size;
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
								ts->volume_ccy / (double)ts->volume_shs
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


			if (event_mask & ev_closed) {
				fprintf(output, "quotes: %" PRId64  ", trades: %" PRId64 " , max levels: %zu, max orders: %zu\n", ts->quotes, ts->trades, ts->max_price_levels, ts->max_order_count);
				fprintf(output, "volume (mio): %.4lf, notional (mio): %.4lf, VWAP: %.3lf, high: %.3lf, low: %.3lf\n", (double)ts->volume_shs * 1e-6, ts->volume_ccy * 1e-6, (ts->volume_ccy / (double)ts->volume_shs), ts->high, ts->low);
			}

			if (flush) {
				fflush(output);
			}
		}
	};

#if 0
	struct fmt_csv_ops final
		: trace_fmt_ops
	{
		void fmt_header(void) override {
			fprintf(output, "Symbol,Timestamp,BidPrice,BidSize,AskPrice,AskSize,LastPrice,LastSize,LastSign,VWAP,SweepEvent\n");
			if (flush) fflush(output);
		}

		void fmt_event(session* session, event* event) override {
			auto* ts = reinterpret_cast<trace_session*>(helix_session_data(session));
			auto timestamp = helix_event_timestamp(event);
			if (!helix_session_is_rth_timestamp(session, timestamp) || !is_order_book_changed(session, event)) {
				return;
			}
			auto symbol = helix_event_symbol(event);
			fprintf(output, "%s,%" PRIu64 ",", symbol, timestamp);
			auto event_mask = helix_event_mask(event);
			if (event_mask & HELIX_EVENT_ORDER_BOOK_UPDATE) {
				auto ob = helix_event_order_book(event);

				auto bid_price = get_price(ob, helix_order_book_bid_price(ob, 0));
				auto bid_size = helix_order_book_bid_size(ob, 0);
				auto ask_price = get_price(ob, helix_order_book_ask_price(ob, 0));
				auto ask_size = helix_order_book_ask_size(ob, 0);

				fprintf(output, "%f,%" PRIu64",%f,%" PRIu64",",
								bid_price,
								bid_size,
								ask_price,
								ask_size
				);

				ts->bid_price = bid_price;
				ts->bid_size = bid_size;
				ts->ask_price = ask_price;
				ts->ask_size = ask_size;
			}
			else {
				fprintf(output, ",,,,");
			}
			if (event_mask & HELIX_EVENT_TRADE) {
				auto ob = helix_event_order_book(event);
				auto trade = helix_event_trade(event);
				fprintf(output, "%f,%" PRIu64 ",%c,%f,",
								get_price(ob, helix_trade_price(trade)),
								helix_trade_size(trade),
								trade_sign(helix_trade_sign(trade)),
								volume_ccy / (double)volume_shs
				);
			}
			else {
				fprintf(output, ",,,,");
			}
			std::string sweep_event;
			if (event_mask & HELIX_EVENT_SWEEP) {
				sweep_event = "Y";
			}
			fprintf(output, "%s", sweep_event.c_str());
			fprintf(output, "\n");
			if (flush) {
				fflush(output);
			}
		}
	};
#endif

	static void process_ob_event(session* session, order_book* ob, event_mask event_mask)
	{
		auto* ts = reinterpret_cast<trace_session*>(session->data());
		size_t bid_levels = ob->bid_levels();
		size_t ask_levels = ob->ask_levels();
		size_t order_count = ob->order_count();

		ts->max_price_levels = bid_levels > ts->max_price_levels ? bid_levels : ts->max_price_levels;
		ts->max_price_levels = ask_levels > ts->max_price_levels ? ask_levels : ts->max_price_levels;
		ts->max_order_count = order_count > ts->max_order_count ? order_count : ts->max_order_count;
		ts->quotes++;
	}

	static void process_trade_event(session* session, order_book* ob, trade* trade, event_mask event_mask)
	{
		auto* ts = reinterpret_cast<trace_session*>(session->data());
		double trade_price = get_price(ob, trade->price);
		uint64_t trade_size = trade->size;
		ts->volume_shs += trade_size;
		ts->volume_ccy += (double)trade_size * trade_price;
		ts->high = trade_price > ts->high ? trade_price : ts->high;
		ts->low = trade_price < ts->low ? trade_price : ts->low;
		ts->trades++;
	}

  symbol_tracker_algo* symbol_tracker_algo::create_new_algo(std::string symbol) 
  {
    helix::nasdaq::itch_bist_protocol protocol{ "nasdaq-binaryfile-itch-bist" };
    std::unique_ptr<session> s(protocol.new_session(new trace_session));
    return new symbol_tracker_algo(std::move(s), symbol);
  }

  symbol_tracker_algo::symbol_tracker_algo(
		std::unique_ptr<session> s, 
		std::string symbol)
    : algo_base(std::move(s))
  {
		impl.reset(new fmt_pretty_ops);
		std::stringstream s_str;
		s_str << "D:/hft/results/" << "result_" << symbol << ".out";
		impl->init(s_str.str());
		impl->fmt_header();
		get_session()->subscribe(symbol, 1000);
	}

	symbol_tracker_algo::~symbol_tracker_algo() {
		auto* ts = reinterpret_cast<trace_session*>(get_session()->data());
		delete ts;
	}

  int symbol_tracker_algo::tick(event* ev) {
		auto mask = ev->get_mask();

		if (mask & ev_opened || mask & ev_closed) {
			// TODO(): do whatever when bist opened or closed!
			puts("bist opened/closed event consumed.");
		}

		if (mask & ev_order_book_update) {
			auto ob = ev->get_ob();
			process_ob_event(get_session(), ob, mask);
		}
		if (mask & ev_trade) {
			auto ob = ev->get_ob();
			auto trade = ev->get_trade();
			process_trade_event(get_session(), ob, trade, mask);
		}
		impl->fmt_event(get_session(), ev);
		return 0;
	}

} // namespace helix