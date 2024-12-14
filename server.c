#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <pthread.h>

// Required library for Winsock
#pragma comment(lib, "ws2_32.lib")

// Struct to store quiz questions, options, and the correct answer
typedef struct {
    char question[256];
    char options[4][100];
    char correct_option;
} Quiz;

// Array of questions
Quiz quiz[10] = {
    {"What is the capital of Germany?", {"A. Berlin", "B. Munich", "C. Frankfurt", "D. Hamburg"}, 'A'},
    {"Which planet is known as the Red Planet?", {"A. Venus", "B. Mars", "C. Jupiter", "D. Saturn"}, 'B'},
    {"What is the square root of 64?", {"A. 6", "B. 7", "C. 8", "D. 9"}, 'C'},
    {"Who wrote 'Hamlet'?", {"A. Charles Dickens", "B. William Shakespeare", "C. Leo Tolstoy", "D. Mark Twain"}, 'B'},
    {"What is the chemical symbol for water?", {"A. H2O", "B. CO2", "C. NaCl", "D. O2"}, 'A'},
    {"Which country is famous for the Eiffel Tower?", {"A. Italy", "B. Spain", "C. France", "D. Greece"}, 'C'},
    {"How many continents are there?", {"A. 5", "B. 6", "C. 7", "D. 8"}, 'C'},
    {"What is the largest mammal?", {"A. Elephant", "B. Blue Whale", "C. Great White Shark", "D. Giraffe"}, 'B'},
    {"Which programming language is used for iOS development?", {"A. Java", "B. Python", "C. Swift", "D. C++"}, 'C'},
    {"What is the fastest land animal?", {"A. Cheetah", "B. Lion", "C. Gazelle", "D. Horse"}, 'A'}
};

void *handle_client(void *socket_desc) {
    SOCKET sock = *(SOCKET *)socket_desc;
    char client_msg[1024] = {0};
    char buffer[256];
    int correct = 0, incorrect = 0;

    for (int i = 0; i < 10; i++) {
        // Send question and options to client
        sprintf(buffer, "Question %d: %s\n%s\n%s\n%s\n%s\nYour answer: ", 
                i + 1, 
                quiz[i].question, 
                quiz[i].options[0], 
                quiz[i].options[1], 
                quiz[i].options[2], 
                quiz[i].options[3]);
        send(sock, buffer, strlen(buffer), 0);

        // Receive client's answer
        memset(client_msg, 0, sizeof(client_msg));
        recv(sock, client_msg, sizeof(client_msg), 0);
        client_msg[strcspn(client_msg, "\n")] = 0; // Remove trailing newline

        // Print client's answer and result
        printf("Client's answer to Question %d: %s\n", i + 1, client_msg);
        if (strlen(client_msg) == 1 && client_msg[0] == quiz[i].correct_option) {
            printf("Result: Correct\n");
            correct++;
        } else {
            printf("Result: Incorrect\n");
            incorrect++;
        }
    }

    // Send results to client
    sprintf(buffer, "Quiz completed! Correct answers: %d, Incorrect answers: %d\n", correct, incorrect);
    send(sock, buffer, strlen(buffer), 0);

    closesocket(sock);
    free(socket_desc);
    return NULL;
}

int main() {
    WSADATA wsa;
    SOCKET server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock. Error code: %d\n", WSAGetLastError());
        return 1;
    }

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        printf("Failed to create socket. Error code: %d\n", WSAGetLastError());
        return 1;
    }

    // Bind socket to a port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        printf("Bind failed. Error code: %d\n", WSAGetLastError());
        return 1;
    }

    listen(server_fd, 3);
    printf("Server is running and waiting for clients...\n");

    while ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) != INVALID_SOCKET) {
        pthread_t client_thread;
        SOCKET *new_sock = malloc(sizeof(SOCKET));
        *new_sock = new_socket;

        pthread_create(&client_thread, NULL, handle_client, (void *)new_sock);
        pthread_detach(client_thread);
    }

    closesocket(server_fd);
    WSACleanup();
    getchar();
    return 0;
}
