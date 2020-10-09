#pragma once

#include <helix.hh>
#include <order_book.hh>
#include <unordered_map>

#include <boost/asio/thread_pool.hpp>
#include <boost/asio/ts/executor.hpp>

using boost::asio::post;
using boost::asio::thread_pool;
using boost::asio::use_future;

namespace helix
{
	/*
	bütün algoritmalar için base sýnýf tanýmlamasý
	mimari olarak her algoritma bir çekirdekte ve o çekirdek içerisinde tek bir thread üzerinde koþacaktýr
	ayný þekilde algoritmalarýn kullandýðý order book'u yaratan bist_handler sýnýflarý da ayný çekirdek üzerinde
	ancak 2inci thread üzerinde koþacaktýr. 
	bu sayede 
	 -	her çekirdekte bulunan T0 thread'inde bist_handler çalýþarak, sisteme gelen paketleri açýp istenilen
			semboller için order book yaratýrken
	 -	T1 thread'i içerisinde çalýþan algoritma, order book tarafýndan gelen mesajlarý kendi thread'inde iþliyor 
			olabilecek. 
	!! T0'dan gelen mesajlar, algo threadinde yer alan event loop'a atýlarak T1 threadinde iþlenecekler.

	 ____C0_____					 _______
	|			|		  |					|		T0	| -> Bist Packet Handler
	| T0	| T1  | =====> 	|_______|
	|			|		  |					    |
	|_____|_____|					 ___|___
												|		T1	| -> Certain Algo working with Bist Handler defined in T0
												|_______|
	
	T0 ve T1 arasýndaki haberleþme event loop'lar arasýnda function call olarak yapýlacak. burada ipc veya dma gibi 
	bir yapý kullanmaya gerek var mý bakmak lazým? L2 cache'ler core için private ise, T0'dan gelen datayý L2 cache 
	içerisinde belirli bir adreste depolayarak, T1'in buradan okumasýný saðlayabiliriz (dma ile).

	!! ileride boost fiber ile bu yapýyý yapmayý düþün !!

	Bir session yaratýlýrken:
	 -	subscribe olunacak semboller ile bir helix::session yaratýlýr. (buradan bist handler sýnýfý dönüyor)
	 -	algoritma sýnýfý ilgili parametreleri ile yaratýlýr ve çalýþacaðý session verilir.
	 -	algoritma sýnýfý session sýnýfýna istediði event'leri ile subscribe olur
	
	Ýleride burada dýþ uygulamalar ile haberleþecek agent'lar falan da olacak. bu sayede her algoritmanýn
	monitörleme ve emir alma/yenileme iþlemleri yapýlmýþ olacak.
	*/
	class algo_base
	{
	public:
		algo_base(std::weak_ptr<session> s);
		virtual ~algo_base();

		// will call run() loop after initializing order book handler and registering for necessary events
		virtual void start();
		
		// will terminate loop as soon as possible and return the algo to idle state
		virtual void stop();
		
		// will schedule algo for given event or time to either run algo or do whatever its possible
		virtual void schedule(/*schedule time or event would be here*/) { }

		thread_pool* get_event_pool() const {
			return &_pool;
		}

		// all algo's should implement tick(). caclulation steps in every algorithm would be done in this thick event
		virtual int tick(event* ev) = 0;

		helix::order_book const* get_ob_for_sym(std::string sym) const;
		helix::order_book* get_ob_for_sym(std::string sym);
	protected:
		std::shared_ptr<session> get_session();
		std::shared_ptr<session> get_session() const;
		
		void register_callback(std::string symbol)
		{
			auto session = _session.lock();
			session->register_event(
				symbol,
				[this](std::shared_ptr<helix::event> ev)
				{
					// use internal event pool to trampoline event_handled in algo thread.
					dispatch(_pool, boost::bind(&algo_base::event_handled, this, ev));
				});
		}
		virtual void create_ob_with_symbols(std::vector<std::pair<std::string, size_t>> symbols);

		// all algo's should implement tick() loop and all the things will go under this loop
		virtual int run() { return 0; } // run bi dursun þimdilik. boost thread pool olunca gerek kalmadý sanki.
	private:
		std::unordered_map<std::string, helix::order_book> ob_sym_map;

		// for event_callback register
		void event_handled(std::shared_ptr<event> ev);
		bool _working {false};
		mutable thread_pool _pool{ 1 };
		std::weak_ptr<session> _session;
	};


} // namespace helix.
