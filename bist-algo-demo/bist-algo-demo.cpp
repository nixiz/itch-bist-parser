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
#include "nasdaq/itch_bist_messages.h"
#include "symbol_tracker_algo.h"
#include "net.hh"

using namespace helix;
using clock_type = std::chrono::high_resolution_clock;

class dyn_file_reader {
public:
	explicit dyn_file_reader(std::string_view file_path) : pos(0)
	{
		input_fd.open(file_path.data(), std::ios::in | std::ios::binary);
		if (!input_fd.is_open()) {
			throw std::runtime_error("failed to open file: " + std::string(file_path));
		}
		fsize = input_fd.seekg(0, std::ios::end).tellg();
		input_fd.seekg(0, std::ios::beg);
	}

	~dyn_file_reader() {
		if (input_fd.is_open()) {
			input_fd.close();
		}
	}

	std::string_view read(size_t n) {
		auto data = peek(n);
		pos += data.size();
		return data;
	}

	std::string_view peek(size_t n = 256) {
		if (!input_fd.is_open() || n > buffer.size()) {
			throw std::runtime_error("file is not open or read size is too large");
		}
		// if requested read size exceeds remaining file size, adjust n to read only remaining bytes
		if (n > remaining()) {
			n = remaining();
		}
		input_fd.seekg(pos);
		input_fd.read(buffer.data(), n);
		size_t bytes_read = input_fd.gcount();
		return std::string_view(buffer.data(), bytes_read);
	}

	std::size_t advance(std::size_t n) {
		pos += n;
		return pos;
	}

	std::size_t size() const {
		return fsize;
	}

	std::size_t remaining() const {
		return fsize - pos;
	}

private:
	std::ifstream input_fd;
	std::array<char, 256> buffer;
	std::size_t pos;
	std::size_t fsize;
};


int main(int argc, const char* argv[])
{
	std::string_view input_file;
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
		input_file = "/Users/okatli/Software/borsa-istanbul-demo-data/TR-1.VRD";
	} else {
		input_file = argv[1];
	}

	helix::nasdaq::itch_bist_protocol protocol{ "nasdaq-binaryfile-itch-bist" };
	std::shared_ptr<session> session(protocol.new_session(nullptr));

	std::vector<std::shared_ptr<algo_base>> algos
	{
		symbol_tracker_algo::create_new_algo(session, {"ACSEL.E ", "AKBNK.E ", "GARAN.E ", "HALKB.E "}),
	};

	std::chrono::nanoseconds nmap_dur;
	auto perf_start = clock_type::now();

	if (!input_file.empty())
	{
		dyn_file_reader input_fd(input_file);
		do
		{
			const auto packet = input_fd.peek(132);
			try
			{
				const auto nr = session->process_packet(helix::net::packet_view{ packet.data(), packet.size() });
				input_fd.advance(nr);
			}
			catch(...){ }
			
		} while (input_fd.remaining() > 0);
	}
	//session->stop();
	//std::this_thread::sleep_for(std::chrono::seconds(100));
	algos.clear();
	session.reset();

	auto perf_end = clock_type::now();
	const auto perf_dur = (perf_end - perf_start).count() - nmap_dur.count();
	std::cout << "bist-algo-demo performance results: " << perf_dur << " ns" << std::endl;
	std::cout << "bist-algo-demo performance results: " << perf_dur / 1000 << " us" << std::endl;
	std::cout << "bist-algo-demo performance results: " << perf_dur / 1000000 << " ms" << std::endl;

}
