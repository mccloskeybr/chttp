#include <stdio.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <inttypes.h>
#include <stdbool.h>

#include "server.h"
#include "macros.h"

Status CreateServerSocket(char* address, uint16_t port, int32_t* server_fd) {
  int32_t temp_server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (temp_server_fd == INVALID_SOCKET) {
    LOG_ERROR("Could not create socket: %d", WSAGetLastError());
    return Status_Error;
  }

  int32_t reuse_addr = 0;
  if (setsockopt(temp_server_fd, SOL_SOCKET, SO_REUSEADDR, (char*) &reuse_addr, sizeof(reuse_addr)) == SOCKET_ERROR) {
    LOG_ERROR("Could not set sock option SO_REUSEADDR: %d", WSAGetLastError());
    return Status_Error;
  }

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_pton(AF_INET, address, &server_addr.sin_addr);
  if (bind(temp_server_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
    LOG_ERROR("Could not bind: %d", WSAGetLastError());
    return Status_Error;
  }

  if (listen(temp_server_fd, SOMAXCONN) == SOCKET_ERROR) {
    LOG_ERROR("Could not listen: %d", WSAGetLastError());
    return Status_Error;
  }

  *server_fd = temp_server_fd;
  return Status_Ok;
}

Status ClientAccept(int32_t server_fd, int32_t* client_fd, char (*client_address)[INET6_ADDRSTRLEN]) {
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  int32_t temp_client_fd = accept(server_fd, (struct sockaddr*) &client_addr, &client_addr_len);
  if (temp_client_fd == INVALID_SOCKET) {
    LOG_ERROR("Could not accept client connection: %d", WSAGetLastError());
    return Status_Error;
  }
  *client_fd = temp_client_fd;
  inet_ntop(AF_INET, &client_addr.sin_addr, *client_address, sizeof(*client_address));
  return Status_Ok;
}

Status ClientClose(int32_t client_fd) {
  if (closesocket(client_fd) == SOCKET_ERROR) {
    LOG_ERROR("Could not close client socket: %d", WSAGetLastError());
    return Status_Error;
  }
  return Status_Ok;
}

Status ClientSend(int32_t client_fd, char* response, int32_t response_len) {
  int32_t total = 0;
  while (total < response_len) {
    int32_t bytes_sent = send(client_fd, response, response_len, 0);
    if (bytes_sent == SOCKET_ERROR) {
      LOG_ERROR("Could not send data to the client.");
      return Status_Error;
    }
    LOG_INFO("here");
    total += bytes_sent;
  }
  return Status_Ok;
}

// TODO: parse HttpRequest struct.
Status ClientReceive(int32_t client_fd, char** request, int32_t request_len) {
  if (recv(client_fd, *request, request_len, 0) == SOCKET_ERROR) {
    LOG_ERROR("Could not recv from client.");
    return Status_Error;
  }
  return Status_Ok;
}

int32_t main() {
  char* address = "127.0.0.1";
  uint16_t port = 6969;

  WSADATA wsa_data;
  int32_t wsa_startup_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
  if (wsa_startup_result != NO_ERROR) {
    PANIC("WSAStartup failed with status: %d", wsa_startup_result);
  }

  int32_t server_fd;
  if (CreateServerSocket(address, port, &server_fd) != Status_Ok) {
    PANIC("Could not create server socket.");
  }

  LOG_INFO("Listening to http://%s:%d", address, port);

  for (;;) {
    int32_t client_fd;
    char client_address[INET6_ADDRSTRLEN];
    if (ClientAccept(server_fd, &client_fd, &client_address) != Status_Ok) {
      PANIC("Could not accept client connection.");
    }
    LOG_INFO("Accepted connection from: %s.", client_address);

    char* request = malloc(1024);
    memset(request, 0, 1024);
    if (ClientReceive(client_fd, &request, 1024) != Status_Ok) {
      PANIC("Unable to receive client request.");
    }
    LOG_INFO("%s", request);

    char* response = malloc(1024);
    memset(response, 0, 1024);
    sprintf(response, "HTTP/1.0 200\r\nContent-Type: text/html\r\nContent-Length: %d\r\nConnection: close\r\n\r\n<!DOCTYPE html><html><body>hello</body></html>", 46);
    LOG_INFO("%s", response);

    if (ClientSend(client_fd, response, (int32_t) strlen(response)) != Status_Ok) {
      PANIC("Unable to send data to client.");
    }

    free(request);
    free(response);

    LOG_INFO("Closing connection: %s", client_address);
    if (ClientClose(client_fd) != Status_Ok) {
      PANIC("Could not close client connection.");
    }
  }

  LOG_INFO("Closing...");
  WSACleanup();
}
