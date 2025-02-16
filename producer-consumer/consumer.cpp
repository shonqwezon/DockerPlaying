#include <ev.h>
#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <iomanip>
#include <thread>

std::tm *get_current_time() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    return std::localtime(&now_time_t);
}

int main(int argc, char *argv[]) {
    int handle_sleep_sec = 5;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-s" && i + 1 < argc) handle_sleep_sec = std::stoi(argv[++i]);
        else {
            std::cerr << "Usage: ./consumer [-s <handle_sleep_sec>]" << std::endl;
            return EXIT_FAILURE;
        }
    }

    std::cout << "./consumer -s " << handle_sleep_sec << std::endl;

    std::string user(getenv("QUSER"));
    std::string pass(getenv("QPASS"));
    std::string host(getenv("QHOST"));
    std::string address = "amqp://" + user + ":" + pass + "@" + host + "/";

    auto *loop = EV_DEFAULT;
    AMQP::LibEvHandler handler(loop);
    AMQP::TcpConnection connection(&handler, AMQP::Address(address));
    AMQP::TcpChannel channel(&connection);

    channel.declareQueue("tasks", AMQP::durable).onSuccess(
            [&channel, &handle_sleep_sec](const std::string &name, uint32_t messagecount, uint32_t consumercount) {
                std::cout << "Declared queue \"" << name << "\"" << std::endl;

                channel.consume(name)
                        .onReceived([&channel, &handle_sleep_sec](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
                            char msg[message.bodySize() + 1];
                            strcpy(msg, message.body());
                            msg[message.bodySize()] = '\0';

                            std::cout << std::put_time(get_current_time(), "[%H:%M:%S] Received task\t:: ") << msg << std::endl;

                            std::this_thread::sleep_for(std::chrono::seconds(handle_sleep_sec));

                            std::cout << std::put_time(get_current_time(), "[%H:%M:%S] Processed task\t:: ") << msg << std::endl;

                            channel.ack(deliveryTag);
                        });

            }).onError([](const char *err) {
        std::cerr << "Error declaring queue: " << err << std::endl;
    });

    ev_run(loop, 0);
    return 0;
}
