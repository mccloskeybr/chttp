#ifndef CDEFAULT_STATUS_H
#define CDEFAULT_STATUS_H

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

#define CDEFAULT_STATUS_LOG_ON_ERROR true
#ifdef CDEFAULT_STATUS_LOG_ON_ERROR
#include <cdefault_log.h>
#endif

typedef enum {
  StatusCode_Ok,
  StatusCode_Cancelled,
  StatusCode_Unknown,
  StatusCode_InvalidArgument,
  StatusCode_DeadlineExceeded,
  StatusCode_NotFound,
  StatusCode_AlreadyExists,
  StatusCode_PermissionDenied,
  StatusCode_ResourceExhausted,
  StatusCode_FailedPrecondition,
  StatusCode_Aborted,
  StatusCode_OutOfRange,
  StatusCode_Internal,
  StatusCode_Unavailable,
  StatusCode_DataLoss,
  StatusCode_Unauthenticated,
  StatusCode_Unimplemented,
} StatusCode;

typedef struct {
  StatusCode code;
  char msg[256];
  char* filename;
  int32_t loc;
} Status;

char* StatusCodeToString(StatusCode code);
int32_t StatusCodeToHttpStatusCode(StatusCode code);

Status _StatusCreate(StatusCode code, char* filename, int32_t loc, char* fmt, ...);
#define CDEFAULT_STATUS_FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define OK() _StatusCreate(StatusCode_Ok, CDEFAULT_STATUS_FILENAME, __LINE__, NULL)
#define CANCELLED(fmt, ...) _StatusCreate(StatusCode_Cancelled, CDEFAULT_STATUS_FILENAME, __LINE__, fmt, __VA_ARGS__)
#define UNKNOWN(fmt, ...) _StatusCreate(StatusCode_Unknown, CDEFAULT_STATUS_FILENAME, __LINE__, fmt, __VA_ARGS__)
#define INVALID_ARGUMENT(fmt, ...) _StatusCreate(StatusCode_InvalidArgument, CDEFAULT_STATUS_FILENAME, __LINE__, fmt, __VA_ARGS__)
#define DEADLINE_EXCEEDED(fmt, ...) _StatusCreate(StatusCode_DeadlineExceeded, CDEFAULT_STATUS_FILENAME, __LINE__, fmt, __VA_ARGS__)
#define NOT_FOUND(fmt, ...) _StatusCreate(StatusCode_NotFound, CDEFAULT_STATUS_FILENAME, __LINE__, fmt, __VA_ARGS__)
#define ALREADY_EXISTS(fmt, ...) _StatusCreate(StatusCode_AlreadyExists, CDEFAULT_STATUS_FILENAME, __LINE__, fmt, __VA_ARGS__)
#define PERMISSION_DENIED(fmt, ...) _StatusCreate(StatusCode_PermissionDenied, CDEFAULT_STATUS_FILENAME, __LINE__, fmt, __VA_ARGS__)
#define RESOURCE_EXHAUSTED(fmt, ...) _StatusCreate(StatusCode_ResourceExhausted, CDEFAULT_STATUS_FILENAME, __LINE__, fmt, __VA_ARGS__)
#define FAILED_PRECONDITION(fmt, ...) _StatusCreate(StatusCode_FailedPrecondition, CDEFAULT_STATUS_FILENAME, __LINE__, fmt, __VA_ARGS__)
#define ABORTED(fmt, ...) _StatusCreate(StatusCode_Aborted, CDEFAULT_STATUS_FILENAME, __LINE__, fmt, __VA_ARGS__)
#define OUT_OF_RANGE(fmt, ...) _StatusCreate(StatusCode_OutOfRange, CDEFAULT_STATUS_FILENAME, __LINE__, fmt, __VA_ARGS__)
#define INTERNAL(fmt, ...) _StatusCreate(StatusCode_Internal, CDEFAULT_STATUS_FILENAME, __LINE__, fmt, __VA_ARGS__)
#define UNAVAILABLE(fmt, ...) _StatusCreate(StatusCode_Unavailable, CDEFAULT_STATUS_FILENAME, __LINE__, fmt, __VA_ARGS__)
#define DATA_LOSS(fmt, ...) _StatusCreate(StatusCode_DataLoss, CDEFAULT_STATUS_FILENAME, __LINE__, fmt, __VA_ARGS__)
#define UNAUTHENTICATED(fmt, ...) _StatusCreate(StatusCode_Unauthenticated, CDEFAULT_STATUS_FILENAME, __LINE__, fmt, __VA_ARGS__)
#define UNIMPLEMENTED(fmt, ...) _StatusCreate(StatusCode_Unimplemented, CDEFAULT_STATUS_FILENAME, __LINE__, fmt, __VA_ARGS__)

#define RETURN_IF_ERROR(exp)            \
  {                                     \
    Status status = (exp);              \
    if (status.code != StatusCode_Ok) { \
      return status;                    \
    }                                   \
  }
#define FATAL_IF_ERROR(exp)             \
  {                                     \
    Status status = (exp);              \
    if (status.code != StatusCode_Ok) { \
      exit(1);                          \
    }                                   \
  }

#ifdef CDEFAULT_STATUS_IMPL


Status _StatusCreate(StatusCode code, char* filename, int32_t loc, char* fmt, ...) {
  Status status;
  status.code = code;

  if (code == StatusCode_Ok) {
    return status;
  } else {
    status.filename = filename;
    status.loc = loc;
    int32_t offset = snprintf(status.msg, sizeof(status.msg), "%s: ", StatusCodeToString(code));
    va_list args;
    va_start(args, fmt);
    vsnprintf(status.msg + offset, sizeof(status.msg) - offset, fmt, args);
    va_end(args);
#ifdef CDEFAULT_STATUS_LOG_ON_ERROR
    Log(stderr, "ERROR", filename, loc, status.msg);
#endif
    return status;
  }
}

char* StatusCodeToString(StatusCode code) {
  switch(code) {
    case StatusCode_Ok: return "OK";
    case StatusCode_Cancelled: return "CANCELLED";
    case StatusCode_Unknown: return "UNKNOWN";
    case StatusCode_InvalidArgument: return "INVALID_ARGUMENT";
    case StatusCode_DeadlineExceeded: return "DEADLINE_EXCEEDED";
    case StatusCode_NotFound: return "NOT_FOUND";
    case StatusCode_AlreadyExists: return "ALREADY_EXISTS";
    case StatusCode_PermissionDenied: return "PERMISSION_DENIED";
    case StatusCode_ResourceExhausted: return "RESOURCE_EXHAUSTED";
    case StatusCode_FailedPrecondition: return "FAILED_PRECONDITION";
    case StatusCode_Aborted: return "ABORTED";
    case StatusCode_OutOfRange: return "OUT_OF_RANGE";
    case StatusCode_Internal: return "INTERNAL";
    case StatusCode_Unavailable: return "UNAVAILABLE";
    case StatusCode_DataLoss: return "DATA_LOSS";
    case StatusCode_Unauthenticated: return "UNAUTHENTICATED";
    case StatusCode_Unimplemented: return "UNIMPLEMENTED";
    default: return "UNKNOWN_STATUS_CODE";
  }
}

int32_t StatusCodeToHttpStatusCode(StatusCode code) {
  switch(code) {
    case StatusCode_Ok: return 200;
    case StatusCode_Cancelled: return 499;
    case StatusCode_Unknown: return 500;
    case StatusCode_InvalidArgument: return 400;
    case StatusCode_DeadlineExceeded: return 504;
    case StatusCode_NotFound: return 404;
    case StatusCode_AlreadyExists: return 409;
    case StatusCode_PermissionDenied: return 403;
    case StatusCode_ResourceExhausted: return 429;
    case StatusCode_FailedPrecondition: return 400;
    case StatusCode_Aborted: return 409;
    case StatusCode_OutOfRange: return 400;
    case StatusCode_Internal: return 500;
    case StatusCode_Unavailable: return 403;
    case StatusCode_DataLoss: return 400;
    case StatusCode_Unauthenticated: return 401;
    case StatusCode_Unimplemented: return 501;
    default: return 500;
  }
}

inline bool IsOk(Status* status) {
  return status->code == StatusCode_Ok;
}

#endif
#endif
