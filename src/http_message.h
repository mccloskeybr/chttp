#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H

typedef enum {
  HttpVerb_Unknown,
  HttpVerb_Get,
  HttpVerb_Count,
} HttpVerb;

typedef enum {
  HttpHeaderKey_Unknown,
  HttpHeaderKey_ContentType,
  HttpHeaderKey_ContentLength,
  HttpHeaderKey_Connection,
  HttpHeaderKey_Count,
} HttpHeaderKey;

typedef struct HttpHeaderNode {
  HttpHeaderKey key;
  String value;
  struct HttpHeaderNode* next;
} HttpHeaderNode;

typedef struct {
  HttpVerb verb;
  String path;
  HttpHeaderNode* headers;
} HttpRequest;

typedef struct {
  int code;
  HttpHeaderNode* headers;
  String body;
} HttpResponse;

Status HttpRequestCreateFromString(StringView request_raw, HttpRequest* request);
void HttpRequestFree(HttpRequest request);

void HttpResponseFree(HttpResponse response);
Status HttpResponseAddHeader(HttpResponse* response, HttpHeaderKey key, String value);
Status HttpResponseToString(HttpResponse response, String* response_raw);

#endif
