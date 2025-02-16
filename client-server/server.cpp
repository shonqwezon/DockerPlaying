#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <csignal>

#define PING "Заяц"
#define PONG "Волк"

int server_socket;
int client_socket = -1;

void handle_signal(int signal) {
    if (signal != SIGINT) return;
    std::cout << "\nЗакрытие сервера пользователем..." << std::endl;
    if (client_socket != -1)
        close(client_socket);
    close(server_socket);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 3 || std::string(argv[1]) != "-p") {
        std::cerr << "Usage: ./server -p <port>" << std::endl;
        return EXIT_FAILURE;
    }
    const int port = std::stoi(argv[2]);
    signal(SIGINT, handle_signal);

    struct sockaddr_in server_addr{}, client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    std::string pong = PONG;

    // Создание сокета
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return EXIT_FAILURE;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Привязка сокета к адресу
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        close(server_socket);
        return EXIT_FAILURE;
    }

    // Ожидание подключений
    if (listen(server_socket, 2) < 0) {
        std::cerr << "Listen failed" << std::endl;
        close(server_socket);
        return EXIT_FAILURE;
    }

    std::cout << "Сервер запущен на порту " << port << "..." << std::endl;
    // Основной цикл обработки
    while (true) {
        if (client_socket == -1) {
            std::cout << "Сервер ожидает подключение..." << std::endl;
            client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
            // Принятие подключения

            if (client_socket < 0) {
                std::cerr << "Accept failed" << std::endl;
                continue;
            }
            std::cout << "Подключился клиент: " << inet_ntoa(client_addr.sin_addr)
                      << ":" << ntohs(client_addr.sin_port) << std::endl;
        }
        ssize_t bytes_read = read(client_socket, buffer, BUFFER_SIZE);
        if (bytes_read > 0) {
            // Создание строки с учётом количества прочитанных байт
            std::string message(buffer, bytes_read);
            std::cout << "Получено: " << message << std::endl;

            if (message == PING) {
                ssize_t bytes_sent = send(client_socket, pong.c_str(), pong.size(), 0);
                if (bytes_sent < 0) {
                    std::cerr << "Send failed" << std::endl;
                    break;
                }
                std::cout << "Отправлено: " << pong << std::endl;
            }
            else {
                std::cout << "Неизвестное сообщение: " << message << " — закрываем с клиентом соединение..."
                          << std::endl;
                close(client_socket);
            }
        }
        else if (bytes_read == 0) {
            std::cout << "Клиент закрыл соединение..." << std::endl;
            close(client_socket);
            client_socket = -1;
        }
    }
    return 0;
}
