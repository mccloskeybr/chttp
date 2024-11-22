#include "server.h"

static Status GenerateStatusResponse(Status status, HttpResponse* response) {
  *response = (HttpResponse) {};
  int http_status_code = StatusCodeToHttpStatusCode(status.code);
  response->code = http_status_code;
  String body = StringCreate(NULL,
        "<!DOCTYPE html>"
        "<html>"
        "  <body>"
        "    STATUS: %d, message: %s"
        "  </body>"
        "</html>"
      , http_status_code, status.msg);
  if (body.buf == NULL) { return RESOURCE_EXHAUSTED(""); }
  response->body = body;
  RETURN_IF_ERROR(HttpResponseAddHeader(
        response, HttpHeaderKey_ContentType, StringCreate(NULL, "text/html")));
  RETURN_IF_ERROR(HttpResponseAddHeader(
        response, HttpHeaderKey_Connection, StringCreate(NULL, "close")));
  RETURN_IF_ERROR(HttpResponseAddHeader(
        response, HttpHeaderKey_ContentLength, StringCreate(NULL, "%d", body.length)));
  return OK();
}

static Status GenerateIndexResponse(HttpRequest* request, HttpResponse* response) {
  *response = (HttpResponse) {};
  response->code = StatusCodeToHttpStatusCode(StatusCode_Ok);;
  String body = StringCreate(NULL,
        "<!DOCTYPE html>"
        "<html>"
        "  <body>"
        "    hello world"
        "  </body>"
        "</html>");
  if (body.buf == NULL) { return RESOURCE_EXHAUSTED(""); }
  response->body = body;
  RETURN_IF_ERROR(HttpResponseAddHeader(
        response, HttpHeaderKey_ContentType, StringCreate(NULL, "text/html")));
  RETURN_IF_ERROR(HttpResponseAddHeader(
        response, HttpHeaderKey_Connection, StringCreate(NULL, "close")));
  RETURN_IF_ERROR(HttpResponseAddHeader(
        response, HttpHeaderKey_ContentLength, StringCreate(NULL, "%d", body.length)));
  return OK();
}

Status ProcessRequest(HttpRequest* request, HttpResponse* response) {
  LOG_INFO("Request: %s", request->path.buf);
  if (StringViewCompare(S_SV(request->path), C_SV("/")) ||
      StringViewCompare(S_SV(request->path), C_SV("/index.html"))) {
    return GenerateIndexResponse(request, response);
  } else {
    return GenerateStatusResponse(NOT_FOUND("Path not found: %s", request->path.buf), response);
  }
}
