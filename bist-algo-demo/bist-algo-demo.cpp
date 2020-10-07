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
	std::vector<algo_base*> algos
	{
		symbol_tracker_algo::create_new_algo("AKBNK.E"),
		//symbol_tracker_algo::create_new_algo("ASELS.E"),
		//symbol_tracker_algo::create_new_algo("ALCTL.E"),
		//symbol_tracker_algo::create_new_algo("BRSAN.E"),
		//symbol_tracker_algo::create_new_algo("HEKTS.E"),
		//symbol_tracker_algo::create_new_algo("GARAN.E"),
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
			for (auto alg : algos) {
				nr = alg->get_session()->received_packet(helix::net::packet_view{ p, size });
			}
			p += nr;
			size -= nr;
		}
	}
	for (auto&& algo : algos) {
		delete algo;
	}
	algos.clear();
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
