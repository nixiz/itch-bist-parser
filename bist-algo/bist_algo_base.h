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
	b�t�n algoritmalar i�in base s�n�f tan�mlamas�
	mimari olarak her algoritma bir �ekirdekte ve o �ekirdek i�erisinde tek bir thread �zerinde ko�acakt�r
	ayn� �ekilde algoritmalar�n kulland��� order book'u yaratan bist_handler s�n�flar� da ayn� �ekirdek �zerinde
	ancak 2inci thread �zerinde ko�acakt�r. 
	bu sayede 
	 -	her �ekirdekte bulunan T0 thread'inde bist_handler �al��arak, sisteme gelen paketleri a��p istenilen
			semboller i�in order book yarat�rken
	 -	T1 thread'i i�erisinde �al��an algoritma, order book taraf�ndan gelen mesajlar� kendi thread'inde i�liyor 
			olabilecek. 
	!! T0'dan gelen mesajlar, algo threadinde yer alan event loop'a at�larak T1 threadinde i�lenecekler.

	 ____C0_____					 _______
	|			|		  |					|		T0	| -> Bist Packet Handler
	| T0	| T1  | =====> 	|_______|
	|			|		  |					    |
	|_____|_____|					 ___|___
												|		T1	| -> Certain Algo working with Bist Handler defined in T0
												|_______|
	
	T0 ve T1 aras�ndaki haberle�me event loop'lar aras�nda function call olarak yap�lacak. burada ipc veya dma gibi 
	bir yap� kullanmaya gerek var m� bakmak laz�m? L2 cache'ler core i�in private ise, T0'dan gelen datay� L2 cache 
	i�erisinde belirli bir adreste depolayarak, T1'in buradan okumas�n� sa�layabiliriz (dma ile).

	!! ileride boost fiber ile bu yap�y� yapmay� d���n !!

	Bir session yarat�l�rken:
	 -	subscribe olunacak semboller ile bir helix::session yarat�l�r. (buradan bist handler s�n�f� d�n�yor)
	 -	algoritma s�n�f� ilgili parametreleri ile yarat�l�r ve �al��aca�� session verilir.
	 -	algoritma s�n�f� session s�n�f�na istedi�i event'leri ile subscribe olur
	
	�leride burada d�� uygulamalar ile haberle�ecek agent'lar falan da olacak. bu sayede her algoritman�n
	monit�rleme ve emir alma/yenileme i�lemleri yap�lm�� olacak.
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
