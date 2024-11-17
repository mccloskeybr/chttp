#ifndef SERVER_H
#define SERVER_H

Status CreateServerSocket(char* address, uint16_t port, int32_t* server_fd);
Status ClientAccept(int32_t server_fd, int32_t* client_fd, char (*client_address)[INET6_ADDRSTRLEN]);
Status ClientClose(int32_t client_fd);
Status ClientReceive(int32_t client_fd, String* request_raw);
Status ClientSend(int32_t client_fd, StringView response_raw);

#endif
