#ifndef CDEFAULT_STRING_H
#define CDEFAULT_STRING_H

#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  char* buf;
  int32_t length;
} String;

typedef struct {
  char* ptr;
  int32_t length;
} StringView;

String StringCreate(void* allocator, char* fmt, ...);
String StringCreateSized(void* allocator, int32_t length, char* fmt, ...);
String StringCreateCopy(void* allocator, String to_copy);
String StringCreateFromStringView(void* allocator, StringView string_view);
void StringFree(void* allocator, String string);
void StringConcat(void* allocator, String* dest, StringView source);

StringView StringViewCreateFromString(String string);
StringView StringViewCreateFromCString(char* ptr);
int32_t StringViewFindFirstOf(StringView string_view, char c);
int32_t StringViewFindLastOf(StringView string_view, char c);
bool StringViewCompare(StringView a, StringView b);
int32_t StringViewFindSubstring(StringView string_view, StringView substring);
bool StringViewForward(StringView* string_view, int32_t offset);
bool StringViewNext(StringView* string_view, StringView* extract, StringView substring);

#define S_SV(s) StringViewCreateFromString(s)
#define C_SV(p) StringViewCreateFromCString(p)

#ifdef CDEFAULT_STRING_IMPL

#ifndef ALLOCATE
#define ALLOCATE(allocator, size) malloc(size)
#define REALLOCATE(allocator, ptr, new_size) realloc(ptr, new_size)
#define FREE(allocator, ptr) free(ptr)
#endif

String StringCreate(void* allocator, char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int32_t length = vsnprintf(NULL, 0, fmt, args);
  va_end(args);
  char* buf = ALLOCATE(allocator, (length + 1) * sizeof(char));
  va_start(args, fmt);
  vsnprintf(buf, length + 1, fmt, args);
  va_end(args);

  String string = {};
  string.buf = buf;
  string.length = length;
  return string;
}

String StringCreateSized(void* allocator, int32_t length, char* fmt, ...) {
  char* buf = ALLOCATE(allocator, (length + 1) * sizeof(char));
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, length + 1, fmt, args);
  va_end(args);

  String string = {};
  string.buf = buf;
  string.length = length;
  return string;
}

String StringCreateCopy(void* allocator, String to_copy) {
  char* buf = ALLOCATE(allocator, (to_copy.length + 1) * sizeof(char));
  snprintf(buf, to_copy.length + 1, "%s", to_copy.buf);
  String string = {};
  string.buf = buf;
  string.length = to_copy.length;
  return string;
}

String StringCreateFromStringView(void* allocator, StringView string_view) {
  String string = {};
  string.buf = ALLOCATE(allocator, string_view.length + 1);
  snprintf(string.buf, string_view.length + 1, "%s", string_view.ptr);
  string.length = string_view.length;
  return string;
}

void StringFree(void* allocator, String string) {
  FREE(allocator, string.buf);
}

void StringConcat(void* allocator, String* dest, StringView source) {
  dest->buf = REALLOCATE(allocator, dest->buf, (dest->length + source.length + 1) * sizeof(char));
  snprintf(dest->buf + dest->length, source.length + 1, "%s", source.ptr);
  dest->length += source.length;
}

StringView StringViewCreateFromString(String string) {
  StringView string_view = {};
  string_view.ptr = string.buf;
  string_view.length = string.length;
  return string_view;
}

StringView StringViewCreateFromCString(char* ptr) {
  StringView string_view = {};
  string_view.ptr = ptr;
  while (*ptr != '\0') { string_view.length++; ptr++; }
  return string_view;
}

int32_t StringViewFindFirstOf(StringView string_view, char c) {
  for (int32_t i = 0; i < string_view.length; i++) {
    if (string_view.ptr[i] == c) { return i; }
  }
  return -1;
}

int32_t StringViewFindLastOf(StringView string_view, char c) {
  int index = -1;
  for (int32_t i = 0; i < string_view.length; i++) {
    if (string_view.ptr[i] == c) { index = i; }
  }
  return index;
}

bool StringViewCompare(StringView a, StringView b) {
  if (a.length != b.length) { return false; }
  for (int32_t i = 0; i < a.length; i++) {
    if (a.ptr[i] != b.ptr[i]) { return false; }
  }
  return true;
}

int32_t StringViewFindSubstring(StringView string_view, StringView substring) {
  for (int32_t i = 0; i < string_view.length; i++) {
    int32_t j;
    for (j = 0; j < substring.length && string_view.ptr[i + j] == substring.ptr[j]; j++);
    if (j == substring.length) { return i; }
  }
  return -1;
}

bool StringViewForward(StringView* string_view, int32_t offset) {
  if (string_view->length < offset) { return false;}
  string_view->ptr += offset;
  string_view->length -= offset;
  return true;
}

bool StringViewNext(StringView* string_view, StringView* extract, StringView substring) {
  int32_t offset = StringViewFindSubstring(*string_view, substring);
  if (offset == -1) { return false; }
  if (extract != NULL) {
    extract->ptr = string_view->ptr;
    extract->length = offset;
  }
  string_view->ptr += offset + substring.length;
  string_view->length -= offset + substring.length;
  return true;
}

#endif
#endif
