// orderbook-perf-test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <order_book.hh>
#include <chrono>
#include <time.h>
#include <vector>

using namespace helix;

using clock_type = std::chrono::high_resolution_clock;

static constexpr uint64_t quantity = 10;

auto test_add(order_book& ob, size_t count)
{
  auto start = clock_type::now();
  for (size_t i = 0; i < count; i++) {
    order o{ i, 8000, quantity, side_type::buy, i };
    ob.add(std::move(o));
  }
  auto end = clock_type::now();
  return end - start;
}

auto test_add_rnd_price(order_book& ob, size_t count)
{
  srand(time(NULL));
  std::vector<uint32_t> rnds; rnds.reserve(count);
  for (size_t i = 0; i < (count/4); i++) {
    rnds.emplace_back(static_cast<uint32_t>(rand()));
    rnds.emplace_back(static_cast<uint32_t>(rand()));
    rnds.emplace_back(static_cast<uint32_t>(rand()));
    rnds.emplace_back(static_cast<uint32_t>(rand()));
  }
  if (rnds.size() != count)
  {
    std::cout << "oupss!\n";
  }

  auto start = clock_type::now();
  for (size_t i = 0; i < count; i++) {
    order o{ i, rnds[i], quantity, rnds[i] % 2 == 0 ? side_type::buy : side_type::sell, i };
    ob.add(std::move(o));
  }
  auto end = clock_type::now();
  return end - start;
}

auto test_cancel(order_book& ob, size_t count)
{
  auto start = clock_type::now();
  for (size_t i = 0; i < count; i++) {
    ob.cancel(i, quantity / 2);
  }
  auto end = clock_type::now();
  return end - start;
}

auto test_remove(order_book& ob, size_t count)
{
  auto start = clock_type::now();
  for (size_t i = 0; i < count; i++) {
    ob.remove(i);
  }
  auto end = clock_type::now();
  return end - start;
}

int main()
{
  size_t count = 20000000;

  order_book ob{ "AXP", 0, count };
  auto add_duration = test_add(ob, count);
  auto cancel_duration = test_cancel(ob, count);
  auto remove_duration = test_remove(ob, count);
  auto add_duration_rnd_price = test_add_rnd_price(ob, count);

  std::cout << "order_book::add()     " << std::chrono::duration_cast<std::chrono::nanoseconds>(add_duration).count() / count << " ns/op" << std::endl;
  std::cout << "order_book::add_rnd() " << std::chrono::duration_cast<std::chrono::nanoseconds>(add_duration_rnd_price).count() / count << " ns/op" << std::endl;
  std::cout << "order_book::cancel()  " << std::chrono::duration_cast<std::chrono::nanoseconds>(cancel_duration).count() / count << " ns/op" << std::endl;
  std::cout << "order_book::remove()  " << std::chrono::duration_cast<std::chrono::nanoseconds>(remove_duration).count() / count << " ns/op" << std::endl;
}
