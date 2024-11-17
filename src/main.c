#include <stdio.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <inttypes.h>
#include <stdbool.h>

#define CDEFAULT_LOG_IMPL
#include "cdefault_log.h"
#define CDEFAULT_STRING_IMPL
#include "cdefault_string.h"
#define CDEFAULT_STATUS_IMPL
#include "cdefault_status.h"

#include "http_message.c"
#include "server.c"

typedef struct {
  int32_t fd;
  char address[INET6_ADDRSTRLEN];
  String request_raw;
  HttpRequest request;
  String response_raw;
  HttpResponse response;
} ClientContext;

void ClientContextClose(ClientContext client) {
  LOG_INFO("Closing connection: %s", client.address);

  ClientClose(client.fd);
  StringFree(NULL, client.request_raw);
  HttpRequestFree(client.request);
  StringFree(NULL, client.response_raw);
  HttpResponseFree(client.response);
}

void ClientContextTrySendStatusAndClose(ClientContext client, Status status) {
  HttpResponse status_response = {};
  if (HttpResponseCreateFromStatus(status, &status_response).code != StatusCode_Ok) {
    HttpResponseFree(status_response);
    ClientContextClose(client);
    return;
  }

  String status_response_raw = {};
  if (HttpResponseToString(status_response, &status_response_raw).code != StatusCode_Ok) {
    StringFree(NULL, status_response_raw);
    HttpResponseFree(status_response);
    ClientContextClose(client);
    return;
  }

  ClientSend(client.fd, S_SV(status_response_raw));
  StringFree(NULL, status_response_raw);
  HttpResponseFree(status_response);
  ClientContextClose(client);
}

int32_t main() {
  char* address = "127.0.0.1";
  uint16_t port = 6969;

  WSADATA wsa_data;
  int32_t wsa_startup_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
  if (wsa_startup_result != NO_ERROR) {
    LOG_FATAL("WSAStartup failed with status: %d", wsa_startup_result);
  }

  int32_t server_fd;
  FATAL_IF_ERROR(CreateServerSocket(address, port, &server_fd));
  LOG_INFO("Listening to http://%s:%d", address, port);

  for (;;) {
    ClientContext client = {};
    if (ClientAccept(server_fd, &client.fd, &client.address).code != StatusCode_Ok) {
      continue;
    }
    LOG_INFO("Accepted connection from: %s.", client.address);

    Status status = {};
    status = ClientReceive(client.fd, &client.request_raw);
    if (status.code != StatusCode_Ok) {
      ClientContextTrySendStatusAndClose(client, status);
      continue;
    }

    status = HttpRequestCreateFromString(S_SV(client.request_raw), &client.request);
    if (status.code != StatusCode_Ok) {
      ClientContextTrySendStatusAndClose(client, status);
      continue;
    }

    status = HttpResponseCreateTestMessage(&client.response);
    if (status.code != StatusCode_Ok) {
      ClientContextTrySendStatusAndClose(client, status);
      continue;
    }

    status = HttpResponseToString(client.response, &client.response_raw);
    if (status.code != StatusCode_Ok) {
      ClientContextTrySendStatusAndClose(client, status);
      continue;
    }

    status = ClientSend(client.fd, S_SV(client.response_raw));
    if (status.code != StatusCode_Ok) {
      ClientContextTrySendStatusAndClose(client, status);
      continue;
    }

    ClientContextClose(client);
  }

  LOG_INFO("Closing...");
  WSACleanup();
}
