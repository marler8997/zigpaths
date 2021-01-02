static void* RtlGetProcessHeap()
{
  return NULL;
}
static void* RtlAllocateHeap(void* heap_handle, ULONG flags, size_t size)
{
  return malloc(size);
}
static LOGICAL RtlFreeHeap(void* heap_handle, ULONG flags, void* addr)
{
  free(addr);
  return TRUE;
}

static void RtlZeroMemory(void* mem, size_t len)
{
  memset(mem, 0, len);
}
static void RtlCopyMemory(void* dest, const void* src, size_t len)
{
  memcpy(dest, src, len);
}
static void RtlMoveMemory(void* dest, const void* src, size_t len)
{
  memmove(dest, src, len);
}

static void RtlInitEmptyUnicodeString(
    PUNICODE_STRING UnicodeString,
    PWCHAR Buffer,
    USHORT BufferSize)
{
    UnicodeString->Length = 0;
    UnicodeString->MaximumLength = BufferSize;
    UnicodeString->Buffer = Buffer;
}

typedef enum _RTL_PATH_TYPE
{
    RtlPathTypeUnknown,
    RtlPathTypeUncAbsolute,
    RtlPathTypeDriveAbsolute,
    RtlPathTypeDriveRelative,
    RtlPathTypeRooted,
    RtlPathTypeRelative,
    RtlPathTypeLocalDevice,
    RtlPathTypeRootLocalDevice,
} RTL_PATH_TYPE;

typedef struct _CURDIR
{
    UNICODE_STRING DosPath;
    HANDLE Handle;
} CURDIR, *PCURDIR;

typedef struct _RTLP_CURDIR_REF
{
    LONG RefCount;
    HANDLE Handle;
} RTLP_CURDIR_REF, *PRTLP_CURDIR_REF;

typedef struct _RTL_RELATIVE_NAME_U
{
    UNICODE_STRING RelativeName;
    HANDLE ContainingDirectory;
    PRTLP_CURDIR_REF CurDirRef;
} RTL_RELATIVE_NAME_U, *PRTL_RELATIVE_NAME_U;

#define RTL_CONSTANT_STRING(__SOURCE_STRING__)                  \
{                                                               \
    sizeof(__SOURCE_STRING__) - sizeof((__SOURCE_STRING__)[0]), \
    sizeof(__SOURCE_STRING__),                                  \
    (__SOURCE_STRING__)                                         \
}
