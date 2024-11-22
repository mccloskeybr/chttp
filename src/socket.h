#ifndef SOCKET_H
#define SOCKET_H

typedef struct {
  int32_t fd;
  char address[INET6_ADDRSTRLEN];
} ClientInfo;

Status CreateServerSocket(char* address, uint16_t port, int32_t* server_fd);
Status ClientAccept(int32_t server_fd, ClientInfo* client_info);
Status ClientClose(ClientInfo* client_info);
Status ClientReceive(ClientInfo* client_info, String* request_raw);
Status ClientSend(ClientInfo* client_info, StringView response_raw);

#endif
