#include "http_message.h"

typedef struct {
  HttpVerb verb;
  char* str;
} HttpVerbLookupEntry;
static HttpVerbLookupEntry HTTP_VERB_LOOKUP[] = {
  {
    .verb = HttpVerb_Unknown,
    .str = "UNKNOWN",
  },
  {
    .verb = HttpVerb_Get,
    .str = "GET",
  },
};

HttpVerb GetHttpVerbFromString(StringView verb) {
  for (int32_t i = 0; i < HttpVerb_Count; i++) {
    if (StringViewCompare(verb, C_SV(HTTP_VERB_LOOKUP[i].str))) {
      return HTTP_VERB_LOOKUP[i].verb;
    }
  }
  return HttpVerb_Unknown;
}

typedef struct {
  HttpHeaderKey key;
  char* str;
} HttpHeaderKeyLookupEntry;
static HttpHeaderKeyLookupEntry HTTP_HEADER_KEY_LOOKUP[] = {
  {
    .key = HttpHeaderKey_Unknown,
    .str = "UNKNOWN",
  },
  {
    .key = HttpHeaderKey_ContentType,
    .str = "Content-Type",
  },
  {
    .key = HttpHeaderKey_ContentLength,
    .str = "Content-Length",
  },
  {
    .key = HttpHeaderKey_Connection,
    .str = "Connection",
  },
};

HttpHeaderKey GetHttpHeaderKeyFromString(StringView key) {
  for (int32_t i = 0; i < HttpHeaderKey_Count; i++) {
    if (StringViewCompare(key, C_SV(HTTP_HEADER_KEY_LOOKUP[i].str))) {
      return HTTP_HEADER_KEY_LOOKUP[i].key;
    }
  }
  return HttpHeaderKey_Unknown;
}

StringView GetStringViewFromHttpHeaderKey(HttpHeaderKey key) {
  return C_SV(HTTP_HEADER_KEY_LOOKUP[key].str);
}

Status HttpRequestCreateFromString(StringView request_raw, HttpRequest* request) {
  StringView request_it = request_raw;

  StringView verb_sv;
  if (!StringViewNext(&request_it, &verb_sv, C_SV(" "))) {
    return INVALID_ARGUMENT("Malformed HTTP verb in request: %s", request_raw.ptr);
  }
  StringView path_sv;
  if (!StringViewNext(&request_it, &path_sv, C_SV(" "))) {
    return INVALID_ARGUMENT("Malformed HTTP path in request: %s", request_raw.ptr);
  }
  if (!StringViewNext(&request_it, NULL, C_SV("\r\n"))) {
    return INVALID_ARGUMENT("Malformed HTTP version in request: %s", request_raw.ptr);
  }

  *request = (HttpRequest) {};
  request->verb = GetHttpVerbFromString(verb_sv);
  request->path = StringCreateFromStringView(NULL, path_sv);
  if (request->path.buf == NULL) { return RESOURCE_EXHAUSTED(""); }

  StringView header_sv;
  while (StringViewNext(&request_it, &header_sv, C_SV("\r\n"))) {
    if (header_sv.length == 0) { break; }
    StringView key_sv;
    if (!StringViewNext(&header_sv, &key_sv, C_SV(": "))) {
      return INVALID_ARGUMENT("Malformed HTTP header key in request.");
    }
    HttpHeaderKey key = GetHttpHeaderKeyFromString(header_sv);
    if (key == HttpHeaderKey_Unknown) { continue; }
    StringView value_sv = header_sv;

    HttpHeaderNode* node = malloc(sizeof(HttpHeaderNode));
    node->key = key;
    node->value = StringCreateFromStringView(NULL, value_sv);
    if (node->value.buf == NULL) { return RESOURCE_EXHAUSTED(""); }

    node->next = request->headers;
    request->headers = node;
  }

  return OK();
}

void HttpRequestFree(HttpRequest request) {
  HttpHeaderNode* curr = request.headers;
  while (curr != NULL) {
    HttpHeaderNode* to_free = curr;
    curr = curr->next;
    StringFree(NULL, to_free->value);
    free(to_free);
  }
  StringFree(NULL, request.path);
}

void HttpResponseFree(HttpResponse response) {
  HttpHeaderNode* curr = response.headers;
  while (curr != NULL) {
    HttpHeaderNode* to_free = curr;
    curr = curr->next;
    StringFree(NULL, to_free->value);
    free(to_free);
  }
  StringFree(NULL, response.body);
}

Status HttpResponseAddHeader(HttpResponse* response, HttpHeaderKey key, String value) {
  HttpHeaderNode* node = malloc(sizeof(HttpHeaderNode));
  if (node == NULL) { return RESOURCE_EXHAUSTED(""); }

  node->key = key;
  node->value = value;
  node->next = response->headers;
  response->headers = node;
  return OK();
}

StringView HttpCodeToReasonPhrase(int32_t code) {
  if (100 <= code && code < 200) {
    return C_SV("Informational");
  } else if (200 <= code && code < 300) {
    return C_SV("Success");
  } else if (300 <= code && code < 400) {
    return C_SV("Redirection");
  } else if (400 <= code && code < 500) {
    return C_SV("Client Error");
  } else if (500 <= code && code < 600) {
    return C_SV("Server Error");
  }
  LOG_ERROR("Unknown error code oberved: %d", code);
  return C_SV("Unknown Error");
}

Status HttpResponseToString(HttpResponse response, String* response_raw) {
  StringView reason_phrase = HttpCodeToReasonPhrase(response.code);
  String start_line = StringCreate(
      NULL,
      "HTTP/1.0 %d %.*s\r\n",
      response.code, reason_phrase.length, reason_phrase.ptr);
  if (start_line.buf == NULL) { return RESOURCE_EXHAUSTED(""); }
  StringConcat(NULL, response_raw, S_SV(start_line));
  StringFree(NULL, start_line);
  if (response_raw->buf == NULL) { return RESOURCE_EXHAUSTED(""); }

  for (HttpHeaderNode* curr = response.headers; curr != NULL; curr = curr->next) {
    StringView key = GetStringViewFromHttpHeaderKey(curr->key);
    StringView value = S_SV(curr->value);
    String header_line = StringCreate(
        NULL,
        "%.*s: %.*s\r\n",
        key.length, key.ptr, value.length, value.ptr);
    if (header_line.buf == NULL) { return RESOURCE_EXHAUSTED(""); }
    StringConcat(NULL, response_raw, S_SV(header_line));
    StringFree(NULL, header_line);
    if (response_raw->buf == NULL) { return RESOURCE_EXHAUSTED(""); }
  }

  String body = StringCreate(
      NULL,
      "\r\n%.*s",
      response.body.length, response.body.buf);
  if (body.buf == NULL) { return RESOURCE_EXHAUSTED(""); }
  StringConcat(NULL, response_raw, S_SV(body));
  StringFree(NULL, body);
  if (response_raw->buf == NULL) { return RESOURCE_EXHAUSTED(""); }

  return OK();
}
