
#include <iostream>
#include <string>
#include <asyncio/runner.h>
#include <asyncio/task.h>
#include <asyncio/sleep.h>

using namespace asyncio;

int main()
{
    auto say_after = [&](auto delay, std::string what) -> Task<std::string> {
        co_await asyncio::sleep(delay);
        co_return  what + "\n";
    };
    auto async_main = [&]() -> Task<> {
        auto task1 = schedule_task(say_after(100ms, "hello"));
        auto task2 = schedule_task(say_after(200ms, "world"));

        co_await task1;
        task2.cancel();

    };
    run(async_main());
    return 0;
}
    