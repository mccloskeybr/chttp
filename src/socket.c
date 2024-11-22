#include "socket.h"

Status CreateServerSocket(char* address, uint16_t port, int32_t* server_fd) {
  int32_t temp_server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (temp_server_fd == INVALID_SOCKET) {
    return INTERNAL("Could not create socket: %d", WSAGetLastError());
  }

  int32_t reuse_addr = 0;
  if (setsockopt(temp_server_fd, SOL_SOCKET, SO_REUSEADDR, (char*) &reuse_addr, sizeof(reuse_addr)) == SOCKET_ERROR) {
    return INTERNAL("Could not set sock option SO_REUSEADDR: %d", WSAGetLastError());
  }

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_pton(AF_INET, address, &server_addr.sin_addr);
  if (bind(temp_server_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
    return INTERNAL("Could not bind: %d", WSAGetLastError());
  }

  if (listen(temp_server_fd, SOMAXCONN) == SOCKET_ERROR) {
    return INTERNAL("Could not listen: %d", WSAGetLastError());
  }

  *server_fd = temp_server_fd;
  return OK();
}

Status ClientAccept(int32_t server_fd, ClientInfo* client_info) {
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  int32_t temp_client_fd = accept(server_fd, (struct sockaddr*) &client_addr, &client_addr_len);
  if (temp_client_fd == INVALID_SOCKET) {
    return INTERNAL("Could not accept client connection: %d", WSAGetLastError());
  }
  client_info->fd = temp_client_fd;
  inet_ntop(AF_INET, &client_addr.sin_addr, client_info->address, sizeof(client_info->address));
  return OK();
}

Status ClientClose(ClientInfo* client_info) {
  if (closesocket(client_info->fd) == SOCKET_ERROR) {
    return INTERNAL("Could not close client socket: %d", WSAGetLastError());
  }
  return OK();
}

Status ClientSend(ClientInfo* client_info, StringView response) {
  int32_t total = 0;
  while (total < response.length) {
    int32_t bytes_sent = send(client_info->fd, response.ptr, response.length, 0);
    if (bytes_sent == SOCKET_ERROR) {
      return INTERNAL("Could not send data to the client.");
    }
    total += bytes_sent;
  }
  return OK();
}

Status ClientReceive(ClientInfo* client_info, String* request_raw) {
  while (true) {
    char buffer[1024];
    int recv_ret = recv(client_info->fd, buffer, sizeof(buffer), 0);
    int error_code = 0;
    if (recv_ret == SOCKET_ERROR) { error_code = WSAGetLastError(); }
    if (recv_ret == SOCKET_ERROR && error_code != WSAEMSGSIZE) {
      return INTERNAL("Could not receive data from the client: %d", recv_ret);
    }

    StringView buffer_view = {};
    buffer_view.ptr = buffer;
    buffer_view.length = recv_ret == SOCKET_ERROR ? sizeof(buffer) : recv_ret;
    StringConcat(NULL, request_raw, buffer_view);

    if (recv_ret < sizeof(buffer)) { break; }
  }
  return OK();
}
