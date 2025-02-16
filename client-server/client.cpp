#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <csignal>
#include <netdb.h>

#define PING "Заяц"
#define PONG "Волк"
bool stop = false;

void handle_signal(int signal) {
    if (signal != SIGINT) return;
    std::cout << "\nЗакрытие клиента пользователем..." << std::endl;
    stop = true;
}

int main(int argc, char *argv[]) {
    int port = 8080, timeout = 1, num_packets = 4;
    bool infinite = false;
    std::string host = "127.0.0.1";
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-w" && i + 1 < argc) timeout = std::stoi(argv[++i]);
        else if (arg == "-n" && i + 1 < argc) num_packets = std::stoi(argv[++i]);
        else if (arg == "-t") infinite = true;
        else if (arg.find(':') != std::string::npos) {
            host = arg.substr(0, arg.find(':'));
            port = std::stoi(arg.substr(arg.find(':') + 1));
        } else {
            std::cerr << "Usage: ./client <host>:<port> [-w <timeout>] [-n <packets>] [-t]" << std::endl;
            return EXIT_FAILURE;
        }
    }
    signal(SIGINT, handle_signal);

    int sock;
    struct sockaddr_in serv_addr{};
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    std::string ping = PING;

    // Создание сокета
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return EXIT_FAILURE;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    struct hostent *he = gethostbyname(host.c_str());
    if (he == nullptr) {
        std::cerr << "Ошибка при получении IP сервера" << std::endl;
        close(sock);
        return EXIT_FAILURE;
    }
    memcpy(&serv_addr.sin_addr, he->h_addr_list[0], he->h_length);

    int sent_packets = 0;
    // Основной цикл отправки и получения сообщений
    int conn = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    while (!stop && (infinite || sent_packets < num_packets)) {
        sent_packets += 1;

        // Подключение к серверу
        if (conn == -1) {
            std::cerr << "Connection failed" << std::endl;
            conn = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        } else {
            ssize_t bytes_sent = send(sock, ping.c_str(), ping.size(), 0);
            if (bytes_sent < 0) {
                std::cerr << "Send failed" << std::endl;
                break;
            }
            std::cout << "Отправлено: " << ping << std::endl;

            ssize_t bytes_read = read(sock, buffer, BUFFER_SIZE);
            if (bytes_read > 0) {
                // Создание строки с учётом количества прочитанных байт
                std::string message(buffer, bytes_read);
                std::cout << "Получено: " << message << std::endl;
                if (message != PONG) {
                    std::cout << "Неизвестное сообщение: " << message << " — закрываем с сервером соединение..."
                              << std::endl;
                    close(sock);
                }
            }
            else if (bytes_read == 0) {
                std::cout << "Сервер закрыл соединение..." << std::endl;
                break;
            }
            else {
                std::cerr << "Не удалось получить ответ от сервера" << std::endl;
                break;
            }
        }
        sleep(timeout);
    }

    close(sock);
    return 0;
}
