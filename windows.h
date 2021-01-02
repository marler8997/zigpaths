#include <wchar.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define max(a,b) (((a) >= (b)) ? a : b)
#define min(a,b) (((a) <= (b)) ? a : b)

#define IN
#define OUT
#define NTAPI
#define CONST const
#define _In_
#define _Out_
#define _Out_opt_
#define _Out_z_bytecap_(sz)

#define MAXUSHORT 0xFFFF
typedef unsigned int BOOL, BOOLEAN, *PBOOLEAN;
#define LOGICAL unsigned char
#define NTSTATUS int
//#define NULL ((void*)0)
typedef int16_t SHORT;
typedef uint16_t USHORT, *PUSHORT;
typedef size_t SIZE_T;

typedef void VOID, *PVOID;
typedef uint16_t WORD;
typedef int32_t LONG;
typedef uint32_t ULONG, DWORD, UINT;
typedef int64_t LONGLONG;
typedef size_t ULONG_PTR;

#define FALSE 0
#define TRUE 1

#define LOWORD(v) ((v) & 0xFFFF)
#define HIWORD(v) (((v) >> 16) & 0xFFFF)
static DWORD MAKELONG(WORD low, WORD high)
{
  return (((DWORD)low) << 16) | (DWORD)high;
}
DWORD MAKELONG(
   WORD wLow,
   WORD wHigh
);

typedef char CHAR, *PCHAR;
typedef wchar_t WCHAR;
typedef WCHAR *PWCHAR;
typedef wchar_t *LPWSTR, *PWSTR;
typedef const wchar_t *PCWSTR, *LPCWSTR;

typedef union _LARGE_INTEGER {
  struct {
    DWORD LowPart;
    LONG  HighPart;
  } DUMMYSTRUCTNAME;
  struct {
    DWORD LowPart;
    LONG  HighPart;
  } u;
  LONGLONG QuadPart;
} LARGE_INTEGER;

//#define ANSI_NULL ((CHAR)0)
#define UNICODE_NULL ((WCHAR)0)
#define UNICODE_STRING_MAX_BYTES ((USHORT) 65534)
//#define UNICODE_STRING_MAX_CHARS (32767)

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef const UNICODE_STRING *PCUNICODE_STRING;

static inline UNICODE_STRING initUnicodeStringByteSizes(PWSTR Buffer, USHORT LengthBytes, USHORT CapacityBytes)
{
  UNICODE_STRING str = { LengthBytes, CapacityBytes, Buffer };
  return str;
}
static inline UNICODE_STRING initUnicodeStringCharSizes(PWSTR Buffer, USHORT LengthChars, USHORT CapacityChars)
{
  if (LengthChars & 0x8000) {
    fprintf(stderr, "Error: initUnicodeStringCharSizes LengthChars = %u is too big\n", LengthChars);
    exit(1);
  }
  if (CapacityChars & 0x8000) {
    fprintf(stderr, "Error: initUnicodeStringCharSizes CapacityChars = %u is too big\n", CapacityChars);
    exit(1);
  }
  return initUnicodeStringByteSizes(Buffer, LengthChars * 2, CapacityChars * 2);
}
static inline UNICODE_STRING initUnicodeStringNullTerm(PWSTR Buffer)
{
  if (!Buffer)
    return initUnicodeStringByteSizes(NULL, 0, 0);

  size_t LengthBytes = wcslen(Buffer) * sizeof(WCHAR);

  static const size_t MAX_SIZE_BYTES = (MAXUSHORT & ~1) - sizeof(WCHAR);
  if (LengthBytes > MAX_SIZE_BYTES) {
    fprintf(stderr, "Error: initUnicodeStringNullTerm Length %lu is too large (max=%lu)\n", LengthBytes, MAX_SIZE_BYTES);
    exit(1);
  }
  return initUnicodeStringByteSizes(Buffer, LengthBytes, LengthBytes + sizeof(WCHAR));
}

#define STATUS_SUCCESS 0
#define STATUS_OBJECT_NAME_INVALID 1
#define STATUS_NO_MEMORY 2
#define STATUS_NAME_TOO_LONG 3
#define STATUS_BUFFER_TOO_SMALL 4
#define STATUS_SHARING_VIOLATION 5
#define STATUS_ACCESS_DENIED 6
#define STATUS_VARIABLE_NOT_FOUND 7
#define STATUS_NOT_IMPL 8

#define NT_SUCCESS(StatCode)  ((NTSTATUS)(StatCode) >= 0)
static void ASSERT(int cond) {
  if (!cond) {
    fprintf(stderr, "assert failed\n");
    fflush(stderr);
    exit(1);
  }
}

typedef void* HANDLE;


#define MAX_PATH 260

typedef struct _CURDIR
{
    UNICODE_STRING DosPath;
    HANDLE Handle;
} CURDIR, *PCURDIR;

struct ProcessParameters {
  CURDIR CurrentDirectory;
  PWSTR Environment;
};
struct PEB {
  //PTR(struct _RTL_USER_PROCESS_PARAMETERS*) ProcessParameters;
  struct ProcessParameters *ProcessParameters;
};
typedef struct PEB *PPEB;
struct PEB *NtCurrentPeb();
static inline void *RtlGetCurrentPeb() { return NULL; }
static inline void RtlAcquirePebLock() { }
static inline void RtlReleasePebLock() { }


static LONG InterlockedIncrement(
  LONG volatile *Addend
                          ) {
  *Addend = (*Addend) + 1;
}
static LONG InterlockedDecrement(
  LONG volatile *Addend
                          ) {
  *Addend = (*Addend) - 1;
}

typedef struct _OBJECT_ATTRIBUTES {
  ULONG Length;
  HANDLE RootDirectory;
  PUNICODE_STRING ObjectName;
  ULONG Attributes;
  PVOID SecurityDescriptor;
  PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;
typedef CONST OBJECT_ATTRIBUTES *PCOBJECT_ATTRIBUTES;

#define InitializeObjectAttributes(p,n,a,r,s) { \
    (p)->Length = sizeof(OBJECT_ATTRIBUTES);    \
    (p)->RootDirectory = (r);                   \
    (p)->Attributes = (a);                      \
    (p)->ObjectName = (n);                      \
    (p)->SecurityDescriptor = (s);              \
    (p)->SecurityQualityOfService = NULL;       \
}

#define OBJ_CASE_INSENSITIVE                    0x00000040L

typedef struct _FILE_BASIC_INFORMATION
{
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    ULONG FileAttributes;
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

#define PAGED_CODE_RTL()
