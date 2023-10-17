#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

const uint32_t MAX_CLIENTS = 128;

typedef struct client_profile {
  int socket;
  struct sockaddr_in addr;
} client_profile;

client_profile client_profiles[1024];

int client_count = 0;

int createServerSocket(int port) {
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) {
    perror("Ошибка при создании сокета");
    exit(1);
  }

  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);
  server_address.sin_addr.s_addr = INADDR_ANY;

  if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
    perror("Ошибка при привязке сокета");
    close(server_socket);
    exit(1);
  }

  return server_socket;
}

void sendMessageToAllClients(const char* message) {
  for (int i = 0; i < client_count; i++) {
    write(client_profiles[i].socket, message, strlen(message));
  }
}

void sendMessageToClient(int client_socket, const char* message) {
  send(client_socket, message, strlen(message), 0);
}

void* clientHandler(int num, int client_socket) {
  printf("%s connected\n", inet_ntoa(client_profiles[num].addr.sin_addr));
  char message[1024];
  while (1) {
    int bytes_received = read(client_socket, &message, 1024);
    if (bytes_received <= 0) {
      printf("%s disconnected\n", inet_ntoa(client_profiles[num].addr.sin_addr));
      close(client_socket);
      client_count--;
      return 0;
    }
    printf("%s:: %s\n", inet_ntoa(client_profiles[num].addr.sin_addr), message);
  }
}

void PrintClients() {
  for (int i = 0 ; i < client_count; ++i) {
    printf("%d : %s\n", i, inet_ntoa(client_profiles[client_count].addr.sin_addr));
  }
}

void* adminConsole(void* arg) {
  char input[1024];
  while (1) {
    scanf("%s", input);
    if (input[0] == '+') {
      char message[1024];
      scanf("%s", message);
      sendMessageToAllClients(message);
    }
    if (input[0] == '*') {
      int client_num = 0;
      scanf("%d", &client_num);
      char message[1024];
      scanf("%s", message);
      sendMessageToClient(client_profiles[client_num].socket, message);
    }
    if (input[0] == '-') {
      PrintClients();
    }
  }
}

int main(int argc, char* argv[]) {
  int server_socket = createServerSocket(atoi(argv[1]));

  if (listen(server_socket, 10) == -1) {
    perror("Ошибка при прослушивании");
    close(server_socket);
    exit(1);
  }

  pthread_t admin_thread;
  pthread_create(&admin_thread, NULL, adminConsole, &server_socket);

  while (1) {
    struct sockaddr_in client_address;
    socklen_t client_addr_size = sizeof(client_address);
    int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_addr_size);
    if (client_socket == -1) {
      perror("Ошибка при принятии соединения");
      continue;
    }
    if (client_count >= MAX_CLIENTS) {
      printf("Сервер переполнен. Отказано в подключении клиента.\n");
      close(client_socket);
      continue;
    }
    client_profiles[client_count].addr = client_address;
    client_profiles[client_count].socket = client_socket;
    client_count++;
    clientHandler(client_count, client_socket);
  }
  close(server_socket);
  return 0;
}
