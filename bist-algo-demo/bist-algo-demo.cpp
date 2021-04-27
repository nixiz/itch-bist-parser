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
using clock_type = std::chrono::high_resolution_clock;

int main(int argc, char* argv[])
{
	std::string input = argv[1];
	std::ifstream input_fd;

	helix::nasdaq::itch_bist_protocol protocol{ "nasdaq-binaryfile-itch-bist" };
	std::shared_ptr<session> session(protocol.new_session(nullptr));

	std::vector<algo_base*> algos
	{
		symbol_tracker_algo::create_new_algo(session, {"ACSEL.E ", "AKBNK.E ", "GARAN.E ", "HALKB.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ADEL.E  "}),
		symbol_tracker_algo::create_new_algo(session, {"ADESE.E "}),
		symbol_tracker_algo::create_new_algo(session, {"AEFES.E "}),
		symbol_tracker_algo::create_new_algo(session, {"AFYON.E "}),
		symbol_tracker_algo::create_new_algo(session, {"AGHOL.E "}),
		symbol_tracker_algo::create_new_algo(session, {"AGYO.E  "}),
		symbol_tracker_algo::create_new_algo(session, {"AKBNK.E "}),
		symbol_tracker_algo::create_new_algo(session, {"AKCNS.E "}),
		symbol_tracker_algo::create_new_algo(session, {"AKENR.E "}),
		symbol_tracker_algo::create_new_algo(session, {"AKFGY.E "}),
		symbol_tracker_algo::create_new_algo(session, {"AKGRT.E "}),
		symbol_tracker_algo::create_new_algo(session, {"AKMGY.E "}),
		symbol_tracker_algo::create_new_algo(session, {"AKSA.E  "}),
		symbol_tracker_algo::create_new_algo(session, {"AKSEN.E "}),
		symbol_tracker_algo::create_new_algo(session, {"AKSGY.E "}),
		symbol_tracker_algo::create_new_algo(session, {"AKSUE.E "}),
		symbol_tracker_algo::create_new_algo(session, {"AKYHO.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ALARK.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ALBRK.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ALCAR.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ALCTL.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ALGYO.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ALKA.E  "}),
		symbol_tracker_algo::create_new_algo(session, {"ALKIM.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ANELE.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ANHYT.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ANSGR.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ARCLK.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ARDYZ.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ARENA.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ARMDA.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ARSAN.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ASELS.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ASUZU.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ATAGY.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ATEKS.E "}),
		symbol_tracker_algo::create_new_algo(session, {"AVGYO.E "}),
		symbol_tracker_algo::create_new_algo(session, {"AVHOL.E "}),
		symbol_tracker_algo::create_new_algo(session, {"AVISA.E "}),
		symbol_tracker_algo::create_new_algo(session, {"AVOD.E  "}),
		symbol_tracker_algo::create_new_algo(session, {"AVTUR.E "}),
		symbol_tracker_algo::create_new_algo(session, {"AYCES.E "}),
		symbol_tracker_algo::create_new_algo(session, {"AYEN.E  "}),
		symbol_tracker_algo::create_new_algo(session, {"AYGAZ.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BAGFS.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BAKAB.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BANVT.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BAYRK.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BERA.E  "}),
		symbol_tracker_algo::create_new_algo(session, {"BEYAZ.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BFREN.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BIMAS.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BIZIM.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BJKAS.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BLCYT.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BNTAS.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BOSSA.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BRISA.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BRKSN.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BRMEN.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BRSAN.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BRYAT.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BSOKE.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BTCIM.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BUCIM.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BURCE.E "}),
		symbol_tracker_algo::create_new_algo(session, {"BURVA.E "}),
		symbol_tracker_algo::create_new_algo(session, {"CCOLA.E "}),
		symbol_tracker_algo::create_new_algo(session, {"CELHA.E "}),
		symbol_tracker_algo::create_new_algo(session, {"CEMAS.E "}),
		symbol_tracker_algo::create_new_algo(session, {"CEMTS.E "}),
		symbol_tracker_algo::create_new_algo(session, {"CEOEM.E "}),
		symbol_tracker_algo::create_new_algo(session, {"CIMSA.E "}),
		symbol_tracker_algo::create_new_algo(session, {"CLEBI.E "}),
		symbol_tracker_algo::create_new_algo(session, {"CMBTN.E "}),
		symbol_tracker_algo::create_new_algo(session, {"CMENT.E "}),
		symbol_tracker_algo::create_new_algo(session, {"CRDFA.E "}),
		symbol_tracker_algo::create_new_algo(session, {"CRFSA.E "}),
		symbol_tracker_algo::create_new_algo(session, {"CUSAN.E "}),
		symbol_tracker_algo::create_new_algo(session, {"DAGHL.E "}),
		symbol_tracker_algo::create_new_algo(session, {"DAGI.E  "}),
		symbol_tracker_algo::create_new_algo(session, {"DERAS.E "}),
		symbol_tracker_algo::create_new_algo(session, {"DERIM.E "}),
		symbol_tracker_algo::create_new_algo(session, {"DESA.E  "}),
		symbol_tracker_algo::create_new_algo(session, {"DESPC.E "}),
		symbol_tracker_algo::create_new_algo(session, {"DEVA.E  "}),
		symbol_tracker_algo::create_new_algo(session, {"DGATE.E "}),
		symbol_tracker_algo::create_new_algo(session, {"DGGYO.E "}),
		symbol_tracker_algo::create_new_algo(session, {"DGKLB.E "}),
		symbol_tracker_algo::create_new_algo(session, {"DITAS.E "}),
		symbol_tracker_algo::create_new_algo(session, {"DMSAS.E "}),
		symbol_tracker_algo::create_new_algo(session, {"DNISI.E "}),
		symbol_tracker_algo::create_new_algo(session, {"DOAS.E  "}),
		symbol_tracker_algo::create_new_algo(session, {"DOBUR.E "}),
		symbol_tracker_algo::create_new_algo(session, {"DOCO.E  "}),
		symbol_tracker_algo::create_new_algo(session, {"DOGUB.E "}),
		symbol_tracker_algo::create_new_algo(session, {"DOHOL.E "}),
		symbol_tracker_algo::create_new_algo(session, {"DOKTA.E "}),
		symbol_tracker_algo::create_new_algo(session, {"DURDO.E "}),
		symbol_tracker_algo::create_new_algo(session, {"DYOBY.E "}),
		symbol_tracker_algo::create_new_algo(session, {"DZGYO.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ECILC.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ECZYT.E "}),
		symbol_tracker_algo::create_new_algo(session, {"EDIP.E  "}),
		symbol_tracker_algo::create_new_algo(session, {"EGEEN.E "}),
		symbol_tracker_algo::create_new_algo(session, {"EGGUB.E "}),
		symbol_tracker_algo::create_new_algo(session, {"EGPRO.E "}),
		symbol_tracker_algo::create_new_algo(session, {"EGSER.E "}),
		symbol_tracker_algo::create_new_algo(session, {"EKGYO.E "}),
		symbol_tracker_algo::create_new_algo(session, {"EMKEL.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ENJSA.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ENKAI.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ERBOS.E "}),
		symbol_tracker_algo::create_new_algo(session, {"EREGL.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ERSU.E  "}),
		symbol_tracker_algo::create_new_algo(session, {"ESCOM.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ESEN.E  "}),
		symbol_tracker_algo::create_new_algo(session, {"EUHOL.E "}),
		symbol_tracker_algo::create_new_algo(session, {"FADE.E  "}),
		symbol_tracker_algo::create_new_algo(session, {"FENER.E "}),
		symbol_tracker_algo::create_new_algo(session, {"FLAP.E  "}),
		symbol_tracker_algo::create_new_algo(session, {"FMIZP.E "}),
		symbol_tracker_algo::create_new_algo(session, {"FONET.E "}),
		symbol_tracker_algo::create_new_algo(session, {"FORMT.E "}),
		symbol_tracker_algo::create_new_algo(session, {"FROTO.E "}),
		symbol_tracker_algo::create_new_algo(session, {"GARAN.E "}),
		symbol_tracker_algo::create_new_algo(session, {"GARFA.E "}),
		symbol_tracker_algo::create_new_algo(session, {"GEDIK.E "}),
		symbol_tracker_algo::create_new_algo(session, {"GEDZA.E "}),
		symbol_tracker_algo::create_new_algo(session, {"GENTS.E "}),
		symbol_tracker_algo::create_new_algo(session, {"GEREL.E "}),
		symbol_tracker_algo::create_new_algo(session, {"GLBMD.E "}),
		symbol_tracker_algo::create_new_algo(session, {"GLRYH.E "}),
		symbol_tracker_algo::create_new_algo(session, {"GLYHO.E "}),
		symbol_tracker_algo::create_new_algo(session, {"GOLTS.E "}),
		symbol_tracker_algo::create_new_algo(session, {"GOODY.E "}),
		symbol_tracker_algo::create_new_algo(session, {"GOZDE.E "}),
		symbol_tracker_algo::create_new_algo(session, {"GSDDE.E "}),
		symbol_tracker_algo::create_new_algo(session, {"GSDHO.E "}),
		symbol_tracker_algo::create_new_algo(session, {"GSRAY.E "}),
		symbol_tracker_algo::create_new_algo(session, {"GUBRF.E "}),
		symbol_tracker_algo::create_new_algo(session, {"HALKB.E "}),
		symbol_tracker_algo::create_new_algo(session, {"HATEK.E "}),
		symbol_tracker_algo::create_new_algo(session, {"HDFGS.E "}),
		symbol_tracker_algo::create_new_algo(session, {"HEKTS.E "}),
		symbol_tracker_algo::create_new_algo(session, {"HLGYO.E "}),
		symbol_tracker_algo::create_new_algo(session, {"HUBVC.E "}),
		symbol_tracker_algo::create_new_algo(session, {"HURGZ.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ICBCT.E "}),
		symbol_tracker_algo::create_new_algo(session, {"IDEAS.E "}),
		//symbol_tracker_algo::create_new_algo(session, {"IDGYO.E "}),
		symbol_tracker_algo::create_new_algo(session, {"IEYHO.E "}),
		symbol_tracker_algo::create_new_algo(session, {"IHEVA.E "}),
		symbol_tracker_algo::create_new_algo(session, {"IHGZT.E "}),
		symbol_tracker_algo::create_new_algo(session, {"IHLAS.E "}),
		symbol_tracker_algo::create_new_algo(session, {"IHLGM.E "}),
		symbol_tracker_algo::create_new_algo(session, {"IHYAY.E "}),
		symbol_tracker_algo::create_new_algo(session, {"INDES.E "}),
		symbol_tracker_algo::create_new_algo(session, {"INFO.E  "}),
		symbol_tracker_algo::create_new_algo(session, {"INTEM.E "}),
		symbol_tracker_algo::create_new_algo(session, {"INVEO.E "}),
		symbol_tracker_algo::create_new_algo(session, {"IPEKE.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ISATR.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ISBTR.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ISCTR.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ISDMR.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ISFIN.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ISGSY.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ISGYO.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ISMEN.E "}),
		symbol_tracker_algo::create_new_algo(session, {"ITTFH.E "}),
		symbol_tracker_algo::create_new_algo(session, {"IZFAS.E "}),
		symbol_tracker_algo::create_new_algo(session, {"IZMDC.E "}),
		symbol_tracker_algo::create_new_algo(session, {"IZTAR.E "}),
		symbol_tracker_algo::create_new_algo(session, {"JANTS.E "}),
	};

	std::chrono::nanoseconds nmap_dur;
	auto perf_start = clock_type::now();

	if (!input.empty())
	{
		auto nmap_start = clock_type::now();

		input_fd.open(input, std::ios::in | std::ios::binary);

		// read all data into local buffer
		std::vector<char> buffer(std::istreambuf_iterator<char>(input_fd), {});

		auto nmap_end = clock_type::now();

		nmap_dur = nmap_end - nmap_start;

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
	//std::this_thread::sleep_for(std::chrono::seconds(100));
	for (auto&& algo : algos) {
		delete algo;
	}
	algos.clear();
	session.reset();

	auto perf_end = clock_type::now();
	const auto perf_dur = (perf_end - perf_start).count() - nmap_dur.count();
	std::cout << "bist-algo-demo performance results: " << perf_dur << " ns" << std::endl;
	std::cout << "bist-algo-demo performance results: " << perf_dur / 1000 << " us" << std::endl;
	std::cout << "bist-algo-demo performance results: " << perf_dur / 1000000 << " ms" << std::endl;

}
