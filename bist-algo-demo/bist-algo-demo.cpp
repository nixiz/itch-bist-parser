// bist-algo-demo.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <istream>
#include <fstream>
#include <chrono>
#include <iomanip>

#include "nasdaq/itch_bist_protocol.hh"
#include "symbol_tracker_algo.h"
#include "net.hh"

using namespace helix;

int main(int argc, char* argv[])
{
	std::string input = argv[1];
	std::ifstream input_fd;
	/*
		cfg.symbols = { "GARAN.E" };
	//cfg.symbols = { "TSKB.E" };
	cfg.max_orders = 200000;
	*/
	helix::nasdaq::itch_bist_protocol protocol{ "nasdaq-binaryfile-itch-bist" };
	std::shared_ptr<session> session(protocol.new_session(nullptr));

	std::vector<algo_base*> algos
	{
		symbol_tracker_algo::create_new_algo(session, "AKBNK.E"),
		//symbol_tracker_algo::create_new_algo(session, "ASELS.E"),
		//symbol_tracker_algo::create_new_algo(session, "ALCTL.E"),
		//symbol_tracker_algo::create_new_algo(session, "BRSAN.E"),
		//symbol_tracker_algo::create_new_algo(session, "HEKTS.E"),
		//symbol_tracker_algo::create_new_algo(session, "GARAN.E"),
	};
	if (!input.empty())
	{
		input_fd.open(input, std::ios::in | std::ios::binary);

		// read all data into local buffer
		std::vector<char> buffer(std::istreambuf_iterator<char>(input_fd), {});

		const char* p = buffer.data();
		size_t size = buffer.size();
		while (size > 0) {
			int nr = 0;
			nr = session->process_packet(helix::net::packet_view{ p, size });
			p += nr;
			size -= nr;
		}
	}
	//session->stop();
	//std::this_thread::sleep_for(std::chrono::seconds(1));
	for (auto&& algo : algos) {
		delete algo;
	}
	algos.clear();
	session.reset();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
