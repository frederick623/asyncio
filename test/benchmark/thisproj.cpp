#include <asyncio/runner.h>
#include <asyncio/start_server.h>
#include <asyncio/task.h>

using asyncio::Stream;
using asyncio::Task;

Task<> handle_echo(Stream stream) {
    while (true) {
        try {
            auto data = co_await stream.read(1024);
            // if (data.empty()) { break; }
            co_await stream.write(data);
        } 
        catch (std::exception& e)
        {
            fmt::print("echo Exception: {}\n", e.what());
        }
        catch (...) 
        {
            fmt::print("Exception\n");
            break;
        }
    }
    fmt::print("Close\n");
    stream.close(); // optional, close connection early
}

Task<> echo_server() {
    auto server = co_await asyncio::start_server(
            handle_echo, "0.0.0.0", 8888);

    fmt::print("Serving on 0.0.0.0:8888\n");

    co_await server.serve_forever();
}

int main() {
    asyncio::run(echo_server());
    return 0;
}