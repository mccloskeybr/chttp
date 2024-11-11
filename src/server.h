#ifndef SERVER_H
#define SERVER_H

typedef enum {
  Status_Ok,
  Status_Error,
} Status;

typedef enum {
  HttbVerb_Get,
} HttpVerb;

typedef enum {
  HttpStatusCode_Ok = 200,
} HttpStatusCode;

typedef struct HttpHeaderNode {
  char* key;
  char* value;
  struct HttpHeaderNode* next;
} HttpHeaderNode;

typedef struct {
  HttpVerb verb;
  char path[32];
  HttpHeaderNode* headers;
} HttpRequest;

typedef struct {
  HttpStatusCode status_code;
  HttpHeaderNode* headers;
  char* body;
} HttpResponse;

#endif
