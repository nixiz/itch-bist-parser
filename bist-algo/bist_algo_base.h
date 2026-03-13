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
	bütün algoritmalar için base sınıf tanımlaması
	mimari olarak her algoritma bir çekirdekte ve o çekirdek içerisinde tek bir thread üzerinde koşacaktır
	aynı şekilde algoritmaların kullandığı order book'u yaratan bist_handler sınıfları da aynı çekirdek üzerinde
	ancak 2inci thread üzerinde koşacaktır. 
	bu sayede 
	 -	her çekirdekte bulunan T0 thread'inde bist_handler çalışarak, sisteme gelen paketleri açıp istenilen
			semboller için order book yaratırken
	 -	T1 thread'i içerisinde çalışan algoritma, order book tarafından gelen mesajları kendi thread'inde işliyor 
			olabilecek. 
	!! T0'dan gelen mesajlar, algo threadinde yer alan event loop'a atılarak T1 threadinde işlenecekler.

	 ____C0_____					 _______
	|			|		  |					|		T0	| -> Bist Packet Handler
	| T0	| T1  | =====> 	|_______|
	|			|		  |					    |
	|_____|_____|					 ___|___
												|		T1	| -> Certain Algo working with Bist Handler defined in T0
												|_______|
	
	T0 ve T1 arasındaki haberleşme event loop'lar arasında function call olarak yapılacak. burada ipc veya dma gibi 
	bir yapı kullanmaya gerek var mı bakmak lazım? L2 cache'ler core için private ise, T0'dan gelen datayı L2 cache 
	içerisinde belirli bir adreste depolayarak, T1'in buradan okumasını sağlayabiliriz (dma ile).

	!! ileride boost fiber ile bu yapıyı yapmayı düşün !!

	Bir session yaratılırken:
	 -	subscribe olunacak semboller ile bir helix::session yaratılır. (buradan bist handler sınıfı dönüyor)
	 -	algoritma sınıfı ilgili parametreleri ile yaratılır ve çalışacağı session verilir.
	 -	algoritma sınıfı session sınıfına istediği event'leri ile subscribe olur
	
	İleride burada dış uygulamalar ile haberleşecek agent'lar falan da olacak. bu sayede her algoritmanın
	monitörleme ve emir alma/yenileme işlemleri yapılmış olacak.
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
		virtual int run() { return 0; } // run bi dursun �imdilik. boost thread pool olunca gerek kalmad� sanki.
	private:
		std::unordered_map<std::string, helix::order_book> ob_sym_map;

		// for event_callback register
		void event_handled(std::shared_ptr<event> ev);
		bool _working {false};
		mutable thread_pool _pool{ 1 };
		std::weak_ptr<session> _session;
	};


} // namespace helix.
