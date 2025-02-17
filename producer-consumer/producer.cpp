#include <ev.h>
#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <ctime>
#include <iomanip>
#include <thread>

std::string get_message_body(int code) {
    std::time_t currentTime = std::time(nullptr);
    std::srand(currentTime);
    int randomNumber = code + std::rand() % 100;

    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&now_time_t), "sent time: %H:%M:%S") << " - code: " << randomNumber;

    return oss.str();
}

int main(int argc, char *argv[]) {
    int num_messages = 5;
    int code = 0;
    int sleep_sec = 2;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-n" && i + 1 < argc) num_messages = std::stoi(argv[++i]);
        else if (arg == "-c" && i + 1 < argc) code = std::stoi(argv[++i]);
        else if (arg == "-s" && i + 1 < argc) sleep_sec = std::stoi(argv[++i]);
        else {
            std::cerr << "Usage: ./producer [-n <num_messages>] [-c <code>] [-s <sleep_sec>]" << std::endl;
            return EXIT_FAILURE;
        }
    }

    std::cout << "./producer -n " << num_messages << " -c " << code << " -s " << sleep_sec << std::endl;

    std::string user(getenv("QUSER"));
    std::string pass(getenv("QPASS"));
    std::string host(getenv("QHOST"));
    std::string address = "amqp://" + user + ":" + pass + "@" + host + "/";

    auto *loop = EV_DEFAULT;
    AMQP::LibEvHandler handler(loop);
    AMQP::TcpConnection connection(&handler, AMQP::Address(address));
    AMQP::TcpChannel channel(&connection);

    channel.declareQueue("tasks", AMQP::durable).onSuccess(
            [&connection, &channel, &num_messages, &code, &sleep_sec](const std::string &name, uint32_t messagecount, uint32_t consumercount) {
                std::cout << "Declared queue \"" << name << "\"" << std::endl;
                for (int i = 1; i <= num_messages; ++i) {
                    std::string message = get_message_body(code);
                    channel.publish("", name, message);
                    std::cout << "Assigned task " << i << " :: " << message << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(sleep_sec));
                }
                connection.close();
            }).onError([](const char *err) {
        std::cerr << "Error declaring queue: " << err << std::endl;
    });

    ev_run(loop, 0);
    return 0;
}
