#include <extern-c/helix.h>
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>

#include <stdexcept>
#include <string>
#include <vector>
#include <memory>
#include <istream>
#include <fstream>
#include <chrono>
#include <iomanip>

static const char* program;

size_t max_price_levels = 0;
size_t max_order_count = 0;
uint64_t quotes = 0;
uint64_t trades = 0;
uint64_t volume_shs = 0;
double volume_ccy = 0.0;
double high = -INFINITY;
double low = +INFINITY;

struct socket_address {
	std::string addr;
	int port;
	socket_address()
	{}
	socket_address(std::string addr, int port)
		: addr{ addr }, port{ port }
	{}
};

struct config {
	std::vector<std::string> symbols;
	size_t max_orders;
	std::string proto;
	std::string multicast_addr;
	int multicast_port;
	std::string request_server;
	std::string format;
	std::string input;
	std::string output;
};

struct trace_session {
	socket_address addr;
	double	bid_price = 0;
	uint64_t	bid_size = 0;
	double	ask_price = UINT64_MAX;
	uint64_t	ask_size = 0;
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
	virtual void fmt_event(helix_session_t session, helix_event_t event) = 0;
protected:
	FILE* output = NULL;
	bool flush = false;
};

socket_address parse_socket_address(std::string raw_addr)
{
	size_t pos = raw_addr.find_first_of(':');
	if (pos == std::string::npos) {
		throw std::invalid_argument(raw_addr + " is not a valid socket address");
	}
	try {
		auto addr = raw_addr.substr(0, pos);
		auto port = raw_addr.substr(pos + 1);
		return socket_address{ addr, std::stoi(port) };
	}
	catch (...) {
		throw std::invalid_argument(raw_addr + " is not a valid socket address");
	}
}

const char trade_sign(helix_trade_sign_t sign)
{
	switch (sign) {
	case HELIX_TRADE_SIGN_BUYER_INITIATED:	return 'B';
	case HELIX_TRADE_SIGN_SELLER_INITIATED:	return 'S';
	case HELIX_TRADE_SIGN_CROSSING:		return 'C';
	case HELIX_TRADE_SIGN_NON_DISPLAYABLE:	return 'N';
	default:				return '?';
	}
}

static double get_price(helix_order_book_t ob, helix_price_t price)
{
	double p = 0.0;
	[[unlikely]] if (auto dec = helix_order_book_price_decimals(ob);
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

static bool is_order_book_changed(helix_session_t session, helix_event_t event)
{
	auto event_mask = helix_event_mask(event);
	if (event_mask & HELIX_EVENT_TRADE) {
		return true;
	}
	if (event_mask & HELIX_EVENT_ORDER_BOOK_UPDATE) {
		auto* ts = reinterpret_cast<trace_session*>(helix_session_data(session));
		auto ob = helix_event_order_book(event);
		auto bid_price = get_price(ob, helix_order_book_bid_price(ob, 0));
		auto bid_size  = helix_order_book_bid_size(ob, 0);
		auto ask_price = get_price(ob, helix_order_book_ask_price(ob, 0));
		auto ask_size  = helix_order_book_ask_size(ob, 0);
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
	void fmt_event(helix_session_t session, helix_event_t event) override
	{
		using namespace std::chrono;
		auto* ts = reinterpret_cast<trace_session*>(helix_session_data(session));
		auto timestamp = helix_event_timestamp(event);
		if (!helix_session_is_rth_timestamp(session, timestamp) || !is_order_book_changed(session, event)) {
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
						helix_event_symbol(event),
						hours, minutes, seconds, milliseconds);
		auto event_mask = helix_event_mask(event);
		if (event_mask & HELIX_EVENT_ORDER_BOOK_UPDATE) {
			auto ob = helix_event_order_book(event);

			auto bid_price = get_price(ob, helix_order_book_bid_price(ob, 0));
			auto bid_size  = helix_order_book_bid_size(ob, 0);
			auto ask_price = get_price(ob, helix_order_book_ask_price(ob, 0));
			auto ask_size  = helix_order_book_ask_size(ob, 0);

			fprintf(output, "%6" PRIu64"  %6.3f  %6.3f  %-6" PRIu64" |",
							bid_size,
							(double)bid_price,
							(double)ask_price,
							ask_size
			);

			ts->bid_price = bid_price;
			ts->bid_size  = bid_size;
			ts->ask_price = ask_price;
			ts->ask_size  = ask_size;
		}
		else {
			fprintf(output, "                               |");
		}
		if (event_mask & HELIX_EVENT_TRADE) {
			auto ob = helix_event_order_book(event);
			auto trade = helix_event_trade(event);
			fprintf(output, " %6.3f %6" PRIu64 " | %c | %6.3f  |",
							get_price(ob, helix_trade_price(trade)),
							helix_trade_size(trade),
							trade_sign(helix_trade_sign(trade)),
							volume_ccy / (double)volume_shs
			);
		}
		else {
			fprintf(output, "               |   |         |");
		}
		std::string sweep_event;
		if (event_mask & HELIX_EVENT_SWEEP) {
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
struct fmt_csv_ops final
	: trace_fmt_ops
{
	void fmt_header(void) override {
		fprintf(output, "Symbol,Timestamp,BidPrice,BidSize,AskPrice,AskSize,LastPrice,LastSize,LastSign,VWAP,SweepEvent\n");
		if (flush) fflush(output);
	}

	void fmt_event(helix_session_t session, helix_event_t event) override {
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
			auto bid_size  = helix_order_book_bid_size(ob, 0);
			auto ask_price = get_price(ob, helix_order_book_ask_price(ob, 0));
			auto ask_size  = helix_order_book_ask_size(ob, 0);

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

std::unique_ptr<trace_fmt_ops> fmt_ops;

static void process_ob_event(helix_session_t session, helix_order_book_t ob, helix_event_mask_t event_mask)
{
	size_t bid_levels = helix_order_book_bid_levels(ob);
	size_t ask_levels = helix_order_book_ask_levels(ob);
	size_t order_count = helix_order_book_order_count(ob);

	max_price_levels = bid_levels > max_price_levels ? bid_levels : max_price_levels;
	max_price_levels = ask_levels > max_price_levels ? ask_levels : max_price_levels;
	max_order_count = order_count > max_order_count ? order_count : max_order_count;
	quotes++;
}

static void process_trade_event(helix_session_t session, helix_order_book_t ob, helix_trade_t trade, helix_event_mask_t event_mask)
{
	double trade_price  = get_price(ob, helix_trade_price(trade));
	uint64_t trade_size = helix_trade_size(trade);
	volume_shs += trade_size;
	volume_ccy += (double)trade_size * trade_price;
	high = trade_price > high ? trade_price : high;
	low = trade_price < low ? trade_price : low;
	trades++;
}

static void process_event(helix_session_t session, helix_event_t event)
{
	helix_event_mask_t mask = helix_event_mask(event);

	if (mask & HELIX_EVENT_OPENED || mask & HELIX_EVENT_CLOSED) {
		// TODO(): do whatever when bist opened or closed!
		puts("bist opened/closed event consumed.");
	}
	if (mask & HELIX_EVENT_ORDER_BOOK_UPDATE) {
		helix_order_book_t ob = helix_event_order_book(event);
		process_ob_event(session, ob, mask);
	}
	if (mask & HELIX_EVENT_TRADE) {
		helix_order_book_t ob = helix_event_order_book(event);
		helix_trade_t trade = helix_event_trade(event);
		process_trade_event(session, ob, trade, mask);
	}
	fmt_ops->fmt_event(session, event);
}

static void process_send(helix_session_t session, char* base, size_t len)
{
	auto* ts = reinterpret_cast<trace_session*>(helix_session_data(session));

	static char tx_buf[1024];
	memcpy(tx_buf, base, len);

	// ...
}

static void usage(void)
{
	fprintf(stdout,
					"usage: %s [options]\n"
					"  options:\n"
					"    -s, --symbol symbol            Ticker symbol to listen to.\n"
					"    -m, --max-orders number        Maximum number of orders per symbol (for pre-allocation).\n"
					"    -P, --proto proto              Market data protocol to listen to\n"
					"          or read from. Supported values:\n"
					"              nasdaq-nordic-moldudp-itch\n"
					"              nasdaq-nordic-soupfile-itch\n"
					"              nasdaq-binaryfile-itch50\n"
					"              parity-moldudp64-pmd\n"
					"    -a, --multicast-addr addr      UDP multicast address to listen to.\n"
					"    -p, --multicast-port port      UDP multicast port to listen to.\n"
					"    -r, --request-server addr:port UDP request server to connect to.\n"
					"    -i, --input filename           Input filename.\n"
					"    -o, --output filename          Output filename.\n"
					"    -f, --format format            Output format (pretty, csv).\n"
					"    -h, --help                     display this help and exit\n",
					program);
	exit(1);
}


int main(int argc, char* argv[])
{
	helix_session_t session;
	helix_protocol_t proto;
	struct config cfg = {};
	trace_session ts;
	std::ifstream input_fd;

	cfg.output = argv[2];
	fmt_ops.reset(new fmt_pretty_ops);
	fmt_ops->init(cfg.output);

	cfg.proto = "nasdaq-binaryfile-itch-bist";
	proto = helix_protocol_lookup(cfg.proto.c_str());
	if (!proto) {
		fprintf(stderr, "error: protocol '%s' is not supported\n", cfg.proto.c_str());
		exit(1);
	}

	session = helix_session_create(proto, process_event, &ts);
	if (!session) {
		fprintf(stderr, "error: unable to create new session\n");
		exit(1);
	}

	cfg.symbols = { "GARAN.E" };
	//cfg.symbols = { "TSKB.E" };
	cfg.max_orders = 200000;
	for (auto&& symbol : cfg.symbols) {
		helix_session_subscribe(session, symbol.c_str(), cfg.max_orders);
	}

	helix_session_set_send_callback(session, process_send);
	
	// first param as input itch data
	cfg.input = argv[1];
	if (!cfg.input.empty()) 
	{
		input_fd.open(cfg.input, std::ios::in | std::ios::binary);

		// read all data into local buffer
		std::vector<char> buffer(std::istreambuf_iterator<char>(input_fd), {});

		fmt_ops->fmt_header();

		const char* p = buffer.data();
		size_t size = buffer.size();
		while (size > 0) {
			int nr;

			nr = helix_session_process_packet(session, p, size);
			if (nr < 0) {
				fprintf(stderr, "error: %s: %s\n", cfg.input.c_str(), helix_strerror(nr));
				exit(1);
			}
			p += nr;
			size -= nr;
		}
	}
	helix_session_destroy(session);

	fprintf(stderr, "quotes: %" PRId64  ", trades: %" PRId64 " , max levels: %zu, max orders: %zu\n", quotes, trades, max_price_levels, max_order_count);
	fprintf(stderr, "volume (mio): %.4lf, notional (mio): %.4lf, VWAP: %.3lf, high: %.3lf, low: %.3lf\n", (double)volume_shs * 1e-6, volume_ccy * 1e-6, (volume_ccy / (double)volume_shs), high, low);
}
