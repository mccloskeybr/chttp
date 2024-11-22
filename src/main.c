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
#include "socket.c"
#include "thread.c"
#include "server.c"

#define MAX_IN_FLIGHT_REQUESTS THREAD_COUNT

typedef struct {
  ClientInfo* client_info;
  String request_raw;
  HttpRequest request;
  String response_raw;
  HttpResponse response;
} ConnectionContext;

void ConnectionClose(ConnectionContext context) {
  LOG_INFO("Closing connection: %s", context.client_info->address);

  ClientClose(context.client_info);
  free(context.client_info);
  StringFree(NULL, context.request_raw);
  HttpRequestFree(context.request);
  StringFree(NULL, context.response_raw);
  HttpResponseFree(context.response);
}

void HandleRequest(void* data) {
  ConnectionContext context = {};
  context.client_info = (ClientInfo*) data;
  Status status = {}; // TODO: attempt to return an error page instead of just giving up.

  status = ClientReceive(context.client_info, &context.request_raw);
  if (status.code != StatusCode_Ok) { ConnectionClose(context); return; }

  status = HttpRequestCreateFromString(S_SV(context.request_raw), &context.request);
  if (status.code != StatusCode_Ok) { ConnectionClose(context); return; }

  status = ProcessRequest(&context.request, &context.response);
  if (status.code != StatusCode_Ok) { ConnectionClose(context); return; }

  status = HttpResponseToString(context.response, &context.response_raw);
  if (status.code != StatusCode_Ok) { ConnectionClose(context); return; }

  status = ClientSend(context.client_info, S_SV(context.response_raw));
  if (status.code != StatusCode_Ok) { ConnectionClose(context); return; }

  ConnectionClose(context);
}

int32_t main(int argc, char** argv) {
  char* address = "127.0.0.1";
  uint16_t port = 8080;
  for (int32_t i = 0; i < argc; i++) {
    if (StringViewCompare(C_SV(argv[i]), C_SV("--address"))) {
      address = argv[++i];
    } else if (StringViewCompare(C_SV(argv[i]), C_SV("--port"))) {
      port = (int16_t) atoi(argv[++i]);
    } else if (StringViewCompare(C_SV(argv[i]), C_SV("--help"))) {
      printf("Usage: --address <address> --port <port>.\nEx. --address 127.0.0.1 --port 8080\n");
      exit(0);
    }
  }

  WSADATA wsa_data;
  int32_t wsa_startup_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
  if (wsa_startup_result != NO_ERROR) {
    LOG_FATAL("WSAStartup failed with status: %d", wsa_startup_result);
  }

  int32_t server_fd;
  FATAL_IF_ERROR(CreateServerSocket(address, port, &server_fd));
  LOG_INFO("Listening to http://%s:%d", address, port);

  ThreadPool thread_pool;
  FATAL_IF_ERROR(ThreadPoolInit(&thread_pool));

  for (;;) {
    while (thread_pool.in_flight_requests > MAX_IN_FLIGHT_REQUESTS) {}

    ClientInfo* client_info = malloc(sizeof(ClientInfo));
    if (client_info == NULL) { LOG_ERROR("Unable to allocate client info!"); continue; }
    *client_info = (ClientInfo) {};
    if (ClientAccept(server_fd, client_info).code != StatusCode_Ok) { free(client_info); continue; }
    LOG_INFO("Accepted connection from: %s.", client_info->address);

    WorkItem item = {};
    item.callback = HandleRequest;
    item.data = client_info;
    while (!ThreadPoolAddWorkItem(&thread_pool, item)) {}
  }

  LOG_INFO("Closing...");
  ThreadPoolClose(&thread_pool);
  WSACleanup();
}
