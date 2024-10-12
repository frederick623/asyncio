

#include <catch2/catch_test_macros.hpp>
#include <string_view>
#include <asyncio/runner.h>
#include <asyncio/task.h>
#include <asyncio/sleep.h>

using namespace asyncio;

SCENARIO("sched cancel test") {
	auto say_after = [&](auto delay, std::string_view what) -> Task<> {
		co_await sleep(delay);
		fmt::print("{}\n", what);
	};

	GIVEN("schedule sleep and cancel") {
		auto async_main = [&]() -> Task<> {
			auto task1 = schedule_task(say_after(100ms, "hello"));
			auto task2 = schedule_task(say_after(200ms, "world"));

			co_await task1;
			task2.cancel();
		};
		auto before_wait = get_event_loop().time();
		run(async_main());
		auto after_wait = get_event_loop().time();
		auto diff = after_wait - before_wait;
		REQUIRE(diff >= 100ms);
		REQUIRE(diff < 200ms);
	}
}
