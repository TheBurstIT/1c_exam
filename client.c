#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int createSocketAndConnect(const char* serverIP, int port) {
  int client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket == -1) {
    perror("Ошибка при создании сокета");
    exit(1);
  }

  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);
  server_address.sin_addr.s_addr = inet_addr(serverIP);

  if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
    perror("Ошибка при подключении к серверу");
    close(client_socket);
    exit(1);
  }

  return client_socket;
}

void sendMessage(int client_socket, const char* message) {
  send(client_socket, message, strlen(message), 0);
}

void receiveMessage(int client_socket, char* buffer, int buffer_size) {
  recv(client_socket, buffer, buffer_size, 0);
}

int main(int argc, char* argv[]) {
  int client_socket = createSocketAndConnect(argv[1], atoi(argv[2]));

  char message[1024];
  char server_response[1024];
  while (1) {
    fgets(message, sizeof(message), stdin);
    sendMessage(client_socket, message);

    receiveMessage(client_socket, server_response, sizeof(server_response));
    printf("%s", server_response);
  }

  close(client_socket);
  return 0;
}
