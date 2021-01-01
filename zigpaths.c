#include <wchar.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define IN
#define OUT
#define NTAPI
#define CONST const
#define _In_
#define _Out_
#define _Out_opt_
#define _Out_z_bytecap_(sz)

#define MAXUSHORT 0xFFFF
typedef unsigned int BOOLEAN, *PBOOLEAN;
#define LOGICAL unsigned char
#define NTSTATUS int
//#define NULL ((void*)0)
typedef uint16_t USHORT;
typedef size_t SIZE_T;

typedef int32_t LONG;
typedef uint32_t ULONG;
typedef size_t ULONG_PTR;

#define MAX_PATH 1045

#define FALSE 0
#define TRUE 1

#define LOWORD(v) ((v) & 0xFFFF)
#define HIWORD(v) (((v) >> 16) & 0xFFFF)
typedef char CHAR, *PCHAR;
typedef wchar_t WCHAR;
typedef WCHAR *PWCHAR;
typedef WCHAR* WSTR;
typedef const WCHAR* CWSTR;
typedef wchar_t *LPWSTR, *PWSTR;
typedef CWSTR *PCWSTR;

//#define ANSI_NULL ((CHAR)0)
#define UNICODE_NULL ((WCHAR)0)
#define UNICODE_STRING_MAX_BYTES ((USHORT) 65534)
//#define UNICODE_STRING_MAX_CHARS (32767)

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING, *PCUNICODE_STRING;

#define STATUS_SUCCESS 0
#define STATUS_OBJECT_NAME_INVALID 1
#define STATUS_NO_MEMORY 2
#define STATUS_NAME_TOO_LONG 3
#define STATUS_BUFFER_TOO_SMALL 4
#define NT_SUCCESS(StatCode)  ((NTSTATUS)(StatCode) >= 0)
#define ASSERT(c) do { \
    if (!c) { fprintf(stderr, "assert failed\n"); fflush(stderr); exit(1); } \
} while(0)

typedef void* HANDLE;


struct CurrentDirectory {
  UNICODE_STRING DosPath;
};
struct ProcessParameters {
  struct CurrentDirectory CurrentDirectory;
};
struct PEB {
  //PTR(struct _RTL_USER_PROCESS_PARAMETERS*) ProcessParameters;
  struct ProcessParameters *ProcessParameters;
};
struct {
    struct PEB *ProcessEnvironmentBlock;
} TEB;
#define NtCurrentPeb() (TEB.ProcessEnvironmentBlock)



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
static void RtlAcquirePebLock() { }
static void RtlReleasePebLock() { }
static void RtlZeroMemory(void* mem, size_t len)
{
  memset(mem, 0, len);
}
static void RtlCopyMemory(void* dest, const void* src, size_t len)
{
  memcpy(dest, src, len);
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

LONG InterlockedIncrement(
  LONG volatile *Addend
                          ) {
  *Addend = (*Addend) + 1;
}

#define DPRINT(fmt,...) do { fprintf(stderr, fmt, ##__VA_ARGS__); fflush(stderr); } while (0)








NTSTATUS
NTAPI
RtlInitUnicodeStringEx(
    OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString)
{
    SIZE_T Size;
    CONST SIZE_T MaxSize = (MAXUSHORT & ~1) - sizeof(WCHAR); // an even number

    if (SourceString)
    {
        Size = wcslen(SourceString) * sizeof(WCHAR);
        if (Size > MaxSize) return STATUS_NAME_TOO_LONG;
        DestinationString->Length = (USHORT)Size;
        DestinationString->MaximumLength = (USHORT)Size + sizeof(WCHAR);
    }
    else
    {
        DestinationString->Length = 0;
        DestinationString->MaximumLength = 0;
    }

    DestinationString->Buffer = (PWCHAR)SourceString;
    return STATUS_SUCCESS;
}









#define OBJ_NAME_PATH_SEPARATOR                 L'\\'





const UNICODE_STRING DeviceRootString = RTL_CONSTANT_STRING(L"\\\\.\\");

const UNICODE_STRING RtlpDosDevicesPrefix    = RTL_CONSTANT_STRING(L"\\??\\");
const UNICODE_STRING RtlpDosDevicesUncPrefix = RTL_CONSTANT_STRING(L"\\??\\UNC\\");
const UNICODE_STRING RtlpWin32NtRootSlash    = RTL_CONSTANT_STRING(L"\\\\?\\");



PRTLP_CURDIR_REF RtlpCurDirRef;

#define IS_PATH_SEPARATOR(x) (((x)==L'\\')||((x)==L'/'))

RTL_PATH_TYPE
NTAPI
RtlDetermineDosPathNameType_U(IN PCWSTR Path)
{
    DPRINT("RtlDetermineDosPathNameType_U %S\n", Path);

    /* Unlike the newer RtlDetermineDosPathNameType_U we assume 4 characters */
    if (IS_PATH_SEPARATOR(Path[0]))
    {
        if (!IS_PATH_SEPARATOR(Path[1])) return RtlPathTypeRooted;                /* \x             */
        if ((Path[2] != L'.') && (Path[2] != L'?')) return RtlPathTypeUncAbsolute;/* \\x            */
        if (IS_PATH_SEPARATOR(Path[3])) return RtlPathTypeLocalDevice;            /* \\.\x or \\?\x */
        if (Path[3]) return RtlPathTypeUncAbsolute;                               /* \\.x or \\?x   */
        return RtlPathTypeRootLocalDevice;                                        /* \\. or \\?     */
    }
    else
    {
        if (!(Path[0]) || (Path[1] != L':')) return RtlPathTypeRelative;          /* x              */
        if (IS_PATH_SEPARATOR(Path[2])) return RtlPathTypeDriveAbsolute;          /* x:\            */
        return RtlPathTypeDriveRelative;                                          /* x:             */
    }
}

ULONG
NTAPI
RtlGetFullPathName_Ustr(
    _In_ PUNICODE_STRING FileName,
    _In_ ULONG Size,
    _Out_z_bytecap_(Size) PWSTR Buffer,
    _Out_opt_ PCWSTR *ShortName,
    _Out_opt_ PBOOLEAN InvalidName,
    _Out_ RTL_PATH_TYPE *PathType)
{
    NTSTATUS Status;
    PWCHAR FileNameBuffer;
    ULONG FileNameLength, FileNameChars, DosLength, DosLengthOffset, FullLength;
    BOOLEAN SkipTrailingPathSeparators;
    WCHAR c;


    ULONG               reqsize = 0;
    PCWSTR              ptr;

    PCUNICODE_STRING    CurDirName;
    UNICODE_STRING      EnvVarName, EnvVarValue;
    WCHAR EnvVarNameBuffer[4];

    ULONG  PrefixCut    = 0;    // Where the path really starts (after the skipped prefix)
    PWCHAR Prefix       = NULL; // pointer to the string to be inserted as the new path prefix
    ULONG  PrefixLength = 0;
    PWCHAR Source;
    ULONG  SourceLength;


    /* For now, assume the name is valid */
    DPRINT("Filename: %wZ\n", FileName);
    DPRINT("Size and buffer: %lx %p\n", Size, Buffer);
    if (InvalidName) *InvalidName = FALSE;

    /* Handle initial path type and failure case */
    *PathType = RtlPathTypeUnknown;
    if ((FileName->Length == 0) || (FileName->Buffer[0] == UNICODE_NULL)) return 0;

    /* Break filename into component parts */
    FileNameBuffer = FileName->Buffer;
    FileNameLength = FileName->Length;
    FileNameChars  = FileNameLength / sizeof(WCHAR);

    /* Kill trailing spaces */
    c = FileNameBuffer[FileNameChars - 1];
    while ((FileNameLength != 0) && (c == L' '))
    {
        /* Keep going, ignoring the spaces */
        FileNameLength -= sizeof(WCHAR);
        if (FileNameLength != 0) c = FileNameBuffer[FileNameLength / sizeof(WCHAR) - 1];
    }

    /* Check if anything is left */
    if (FileNameLength == 0) return 0;

    /*
     * Check whether we'll need to skip trailing path separators in the
     * computed full path name. If the original file name already contained
     * trailing separators, then we keep them in the full path name. On the
     * other hand, if the original name didn't contain any trailing separators
     * then we'll skip it in the full path name.
     */
    SkipTrailingPathSeparators = !IS_PATH_SEPARATOR(FileNameBuffer[FileNameChars - 1]);

    /* Check if this is a DOS name */
    DosLength = RtlIsDosDeviceName_Ustr(FileName);
    DPRINT("DOS length for filename: %lx %wZ\n", DosLength, FileName);
    if (DosLength != 0)
    {
        /* Zero out the short name */
        if (ShortName) *ShortName = NULL;

        /* See comment for RtlIsDosDeviceName_Ustr if this is confusing... */
        DosLengthOffset = HIWORD(DosLength);
        DosLength       = LOWORD(DosLength);

        /* Do we have a DOS length, and does the caller want validity? */
        if (InvalidName && (DosLengthOffset != 0))
        {
            /* Do the check */
            Status = RtlpCheckDeviceName(FileName, DosLengthOffset, InvalidName);

            /* If the check failed, or the name is invalid, fail here */
            if (!NT_SUCCESS(Status)) return 0;
            if (*InvalidName) return 0;
        }

        /* Add the size of the device root and check if it fits in the size */
        FullLength = DosLength + DeviceRootString.Length;
        if (FullLength < Size)
        {
            /* Add the device string */
            RtlMoveMemory(Buffer, DeviceRootString.Buffer, DeviceRootString.Length);

            /* Now add the DOS device name */
            RtlMoveMemory((PCHAR)Buffer + DeviceRootString.Length,
                          (PCHAR)FileNameBuffer + DosLengthOffset,
                          DosLength);

            /* Null terminate */
            *(PWCHAR)((ULONG_PTR)Buffer + FullLength) = UNICODE_NULL;
            return FullLength;
        }

        /* Otherwise, there's no space, so return the buffer size needed */
        if ((FullLength + sizeof(UNICODE_NULL)) > UNICODE_STRING_MAX_BYTES) return 0;
        return FullLength + sizeof(UNICODE_NULL);
    }

    /* Zero-out the destination buffer. FileName must be different from Buffer */
    RtlZeroMemory(Buffer, Size);

    /* Get the path type */
    *PathType = RtlDetermineDosPathNameType_U(FileNameBuffer);



    /**********************************************
     **    CODE REWRITING IS HAPPENING THERE     **
     **********************************************/
    Source       = FileNameBuffer;
    SourceLength = FileNameLength;
    EnvVarValue.Buffer = NULL;

    /* Lock the PEB to get the current directory */
    RtlAcquirePebLock();
    CurDirName = &NtCurrentPeb()->ProcessParameters->CurrentDirectory.DosPath;

    switch (*PathType)
    {
        case RtlPathTypeUncAbsolute:        /* \\foo   */
        {
            PrefixCut = RtlpSkipUNCPrefix(FileNameBuffer);
            break;
        }

        case RtlPathTypeLocalDevice:        /* \\.\foo */
        {
            PrefixCut = 4;
            break;
        }

        case RtlPathTypeDriveAbsolute:      /* c:\foo  */
        {
            ASSERT(FileNameBuffer[1] == L':');
            ASSERT(IS_PATH_SEPARATOR(FileNameBuffer[2]));

            // FileNameBuffer[0] = RtlpUpcaseUnicodeChar(FileNameBuffer[0]);
            Prefix = FileNameBuffer;
            PrefixLength = 3 * sizeof(WCHAR);
            Source += 3;
            SourceLength -= 3 * sizeof(WCHAR);

            PrefixCut = 3;
            break;
        }

        case RtlPathTypeDriveRelative:      /* c:foo   */
        {
            WCHAR CurDrive, NewDrive;

            Source += 2;
            SourceLength -= 2 * sizeof(WCHAR);

            CurDrive = RtlpUpcaseUnicodeChar(CurDirName->Buffer[0]);
            NewDrive = RtlpUpcaseUnicodeChar(FileNameBuffer[0]);

            if ((NewDrive != CurDrive) || CurDirName->Buffer[1] != L':')
            {
                EnvVarNameBuffer[0] = L'=';
                EnvVarNameBuffer[1] = NewDrive;
                EnvVarNameBuffer[2] = L':';
                EnvVarNameBuffer[3] = UNICODE_NULL;

                EnvVarName.Length = 3 * sizeof(WCHAR);
                EnvVarName.MaximumLength = EnvVarName.Length + sizeof(WCHAR);
                EnvVarName.Buffer = EnvVarNameBuffer;

                // FIXME: Is it possible to use the user-given buffer ?
                // RtlInitEmptyUnicodeString(&EnvVarValue, NULL, Size);
                EnvVarValue.Length = 0;
                EnvVarValue.MaximumLength = (USHORT)Size;
                EnvVarValue.Buffer = RtlAllocateHeap(RtlGetProcessHeap(), 0, Size);
                if (EnvVarValue.Buffer == NULL)
                {
                    Prefix       = NULL;
                    PrefixLength = 0;
                    goto Quit;
                }

                Status = RtlQueryEnvironmentVariable_U(NULL, &EnvVarName, &EnvVarValue);
                switch (Status)
                {
                    case STATUS_SUCCESS:
                        /*
                         * (From Wine)
                         * FIXME: Win2k seems to check that the environment
                         * variable actually points to an existing directory.
                         * If not, root of the drive is used (this seems also
                         * to be the only place in RtlGetFullPathName that the
                         * existence of a part of a path is checked).
                         */
                        EnvVarValue.Buffer[EnvVarValue.Length / sizeof(WCHAR)] = L'\\';
                        Prefix       = EnvVarValue.Buffer;
                        PrefixLength = EnvVarValue.Length + sizeof(WCHAR); /* Append trailing '\\' */
                        break;

                    case STATUS_BUFFER_TOO_SMALL:
                        reqsize = EnvVarValue.Length + SourceLength + sizeof(UNICODE_NULL);
                        goto Quit;

                    default:
                        DPRINT1("RtlQueryEnvironmentVariable_U(\"%wZ\") returned 0x%08lx\n", &EnvVarName, Status);

                        EnvVarNameBuffer[0] = NewDrive;
                        EnvVarNameBuffer[1] = L':';
                        EnvVarNameBuffer[2] = L'\\';
                        EnvVarNameBuffer[3] = UNICODE_NULL;
                        Prefix       = EnvVarNameBuffer;
                        PrefixLength = 3 * sizeof(WCHAR);

                        RtlFreeHeap(RtlGetProcessHeap(), 0, EnvVarValue.Buffer);
                        EnvVarValue.Buffer = NULL;
                        break;
                }
                PrefixCut = 3;
                break;
            }
            /* Fall through */
            DPRINT("RtlPathTypeDriveRelative - Using fall-through to RtlPathTypeRelative\n");
        }

        case RtlPathTypeRelative:           /* foo     */
        {
            Prefix       = CurDirName->Buffer;
            PrefixLength = CurDirName->Length;
            if (CurDirName->Buffer[1] != L':')
            {
                PrefixCut = RtlpSkipUNCPrefix(CurDirName->Buffer);
            }
            else
            {
                PrefixCut = 3;
            }
            break;
        }

        case RtlPathTypeRooted:             /* \xxx    */
        {
            if (CurDirName->Buffer[1] == L':')
            {
                // The path starts with "C:\"
                ASSERT(CurDirName->Buffer[1] == L':');
                ASSERT(IS_PATH_SEPARATOR(CurDirName->Buffer[2]));

                Prefix = CurDirName->Buffer;
                PrefixLength = 3 * sizeof(WCHAR); // Skip "C:\"

                PrefixCut = 3;      // Source index location incremented of + 3
            }
            else
            {
                PrefixCut = RtlpSkipUNCPrefix(CurDirName->Buffer);
                PrefixLength = PrefixCut * sizeof(WCHAR);
                Prefix = CurDirName->Buffer;
            }
            break;
        }

        case RtlPathTypeRootLocalDevice:    /* \\.     */
        {
            Prefix       = DeviceRootString.Buffer;
            PrefixLength = DeviceRootString.Length;
            Source += 3;
            SourceLength -= 3 * sizeof(WCHAR);

            PrefixCut = 4;
            break;
        }

        case RtlPathTypeUnknown:
            goto Quit;
    }

    /* Do we have enough space for storing the full path? */
    reqsize = PrefixLength;
    if (reqsize + SourceLength + sizeof(WCHAR) > Size)
    {
        /* Not enough space, return needed size (including terminating '\0') */
        reqsize += SourceLength + sizeof(WCHAR);
        goto Quit;
    }

    /*
     * Build the full path
     */
    /* Copy the path's prefix */
    if (PrefixLength) RtlMoveMemory(Buffer, Prefix, PrefixLength);
    /* Copy the remaining part of the path */
    RtlMoveMemory(Buffer + PrefixLength / sizeof(WCHAR), Source, SourceLength + sizeof(WCHAR));

    /* Some cleanup */
    Prefix = NULL;
    if (EnvVarValue.Buffer)
    {
        RtlFreeHeap(RtlGetProcessHeap(), 0, EnvVarValue.Buffer);
        EnvVarValue.Buffer = NULL;
    }

    /*
     * Finally, put the path in canonical form (remove redundant . and ..,
     * (back)slashes...) and retrieve the length of the full path name
     * (without its terminating null character) (in chars).
     */
    reqsize = RtlpCollapsePath(Buffer, /* Size, reqsize, */ PrefixCut, SkipTrailingPathSeparators);

    /* Find the file part, which is present after the last path separator */
    if (ShortName)
    {
        ptr = wcsrchr(Buffer, L'\\');
        if (ptr) ++ptr; // Skip it

        /*
         * For UNC paths, the file part is after the \\share\dir part of the path.
         */
        PrefixCut = (*PathType == RtlPathTypeUncAbsolute ? PrefixCut : 3);

        if (ptr && *ptr && (ptr >= Buffer + PrefixCut))
        {
            *ShortName = ptr;
        }
        else
        {
            /* Zero-out the short name */
            *ShortName = NULL;
        }
    }

Quit:
    /* Release PEB lock */
    RtlReleasePebLock();

    return reqsize;
}

ULONG
NTAPI
RtlGetFullPathName_UEx(
    PWSTR FileName,
    ULONG BufferLength,
    PWSTR Buffer,
    PWSTR *FilePart,
    RTL_PATH_TYPE *InputPathType)
{
    UNICODE_STRING FileNameString;
    NTSTATUS status;

    if (InputPathType)
        *InputPathType = 0;

    /* Build the string */
    status = RtlInitUnicodeStringEx(&FileNameString, FileName);
    if (!NT_SUCCESS(status)) return 0;

    /* Call the extended function */
    return RtlGetFullPathName_Ustr(
        &FileNameString,
        BufferLength,
        Buffer,
        (PCWSTR*)FilePart,
        NULL,
        InputPathType);
}

ULONG
NTAPI
RtlGetFullPathName_U(
    PCWSTR FileName,
    ULONG Size,
    PWSTR Buffer,
    PWSTR *ShortName)
{
    RTL_PATH_TYPE PathType;

    /* Call the extended function */
    return RtlGetFullPathName_UEx((PWSTR)FileName,
                                   Size,
                                   Buffer,
                                   ShortName,
                                   &PathType);
}
RTL_PATH_TYPE
NTAPI
RtlDetermineDosPathNameType_Ustr(IN PCUNICODE_STRING PathString)
{
    PWCHAR Path;
    ULONG Chars;

    Path = PathString->Buffer;
    Chars = PathString->Length / sizeof(WCHAR);

    /* Return if there are no characters */
    if (!Chars) return RtlPathTypeRelative;

    /*
     * The algorithm is similar to RtlDetermineDosPathNameType_U but here we
     * actually check for the path length before touching the characters
     */
    if (IS_PATH_SEPARATOR(Path[0]))
    {
        if ((Chars < 2) || !(IS_PATH_SEPARATOR(Path[1]))) return RtlPathTypeRooted;                /* \x             */
        if ((Chars < 3) || ((Path[2] != L'.') && (Path[2] != L'?'))) return RtlPathTypeUncAbsolute;/* \\x            */
        if ((Chars >= 4) && (IS_PATH_SEPARATOR(Path[3]))) return RtlPathTypeLocalDevice;           /* \\.\x or \\?\x */
        if (Chars != 3) return RtlPathTypeUncAbsolute;                                             /* \\.x or \\?x   */
        return RtlPathTypeRootLocalDevice;                                                         /* \\. or \\?     */
    }
    else
    {
        if ((Chars < 2) || (Path[1] != L':')) return RtlPathTypeRelative;                          /* x              */
        if ((Chars < 3) || !(IS_PATH_SEPARATOR(Path[2]))) return RtlPathTypeDriveRelative;         /* x:             */
        return RtlPathTypeDriveAbsolute;                                                           /* x:\            */
    }
}

NTSTATUS
NTAPI
RtlpWin32NTNameToNtPathName_U(IN PUNICODE_STRING DosPath,
                              OUT PUNICODE_STRING NtPath,
                              OUT PCWSTR *PartName,
                              OUT PRTL_RELATIVE_NAME_U RelativeName)
{
    ULONG DosLength;
    PWSTR NewBuffer, p;

    /* Validate the input */
    if (!DosPath) return STATUS_OBJECT_NAME_INVALID;

    /* Validate the DOS length */
    DosLength = DosPath->Length;
    if (DosLength >= UNICODE_STRING_MAX_BYTES) return STATUS_NAME_TOO_LONG;

    /* Make space for the new path */
    NewBuffer = RtlAllocateHeap(RtlGetProcessHeap(),
                                0,
                                DosLength + sizeof(UNICODE_NULL));
    if (!NewBuffer) return STATUS_NO_MEMORY;

    /* Copy the prefix, and then the rest of the DOS path, and NULL-terminate */
    RtlCopyMemory(NewBuffer, RtlpDosDevicesPrefix.Buffer, RtlpDosDevicesPrefix.Length);
    RtlCopyMemory((PCHAR)NewBuffer + RtlpDosDevicesPrefix.Length,
                  DosPath->Buffer + RtlpDosDevicesPrefix.Length / sizeof(WCHAR),
                  DosPath->Length - RtlpDosDevicesPrefix.Length);
    NewBuffer[DosLength / sizeof(WCHAR)] = UNICODE_NULL;

    /* Did the caller send a relative name? */
    if (RelativeName)
    {
        /* Zero initialize it */
        RtlInitEmptyUnicodeString(&RelativeName->RelativeName, NULL, 0);
        RelativeName->ContainingDirectory = NULL;
        RelativeName->CurDirRef = 0;
    }

    /* Did the caller request a partial name? */
    if (PartName)
    {
        /* Loop from the back until we find a path separator */
        p = &NewBuffer[DosLength / sizeof(WCHAR)];
        while (--p > NewBuffer)
        {
            /* We found a path separator, move past it */
            if (*p == OBJ_NAME_PATH_SEPARATOR)
            {
                ++p;
                break;
            }
        }

        /* Check whether a separator was found and if something remains */
        if ((p > NewBuffer) && *p)
        {
            /* What follows the path separator is the partial name */
            *PartName = p;
        }
        else
        {
            /* The path ends with a path separator, no partial name */
            *PartName = NULL;
        }
    }

    /* Build the final NT path string */
    NtPath->Buffer = NewBuffer;
    NtPath->Length = (USHORT)DosLength;
    NtPath->MaximumLength = (USHORT)DosLength + sizeof(UNICODE_NULL);
    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
RtlpDosPathNameToRelativeNtPathName_Ustr(IN BOOLEAN HaveRelative,
                                         IN PCUNICODE_STRING DosName,
                                         OUT PUNICODE_STRING NtName,
                                         OUT PCWSTR *PartName,
                                         OUT PRTL_RELATIVE_NAME_U RelativeName)
{
    WCHAR BigBuffer[MAX_PATH + 1];
    PWCHAR PrefixBuffer, NewBuffer, Buffer;
    ULONG MaxLength, PathLength, PrefixLength, PrefixCut, LengthChars, Length;
    UNICODE_STRING CapturedDosName, PartNameString, FullPath;
    BOOLEAN QuickPath;
    RTL_PATH_TYPE InputPathType, BufferPathType;
    NTSTATUS Status;
    BOOLEAN NameInvalid;
    PCURDIR CurrentDirectory;

    /* Assume MAX_PATH for now */
    //DPRINT("Relative: %d DosName: %wZ NtName: %p, PartName: %p, RelativeName: %p\n",
    //        HaveRelative, DosName, NtName, PartName, RelativeName);
    MaxLength = sizeof(BigBuffer);

    /* Validate the input */
    if (!DosName) return STATUS_OBJECT_NAME_INVALID;

    /* Capture input string */
    CapturedDosName = *DosName;

    /* Check for the presence or absence of the NT prefix "\\?\" form */
    // if (!RtlPrefixUnicodeString(&RtlpWin32NtRootSlash, &CapturedDosName, FALSE))
    if ((CapturedDosName.Length <= RtlpWin32NtRootSlash.Length) ||
        (CapturedDosName.Buffer[0] != RtlpWin32NtRootSlash.Buffer[0]) ||
        (CapturedDosName.Buffer[1] != RtlpWin32NtRootSlash.Buffer[1]) ||
        (CapturedDosName.Buffer[2] != RtlpWin32NtRootSlash.Buffer[2]) ||
        (CapturedDosName.Buffer[3] != RtlpWin32NtRootSlash.Buffer[3]))
    {
        /* NT prefix not present */

        /* Quick path won't be used */
        QuickPath = FALSE;

        /* Use the static buffer */
        Buffer = BigBuffer;
        MaxLength += RtlpDosDevicesUncPrefix.Length;

        /* Allocate a buffer to hold the path */
        NewBuffer = RtlAllocateHeap(RtlGetProcessHeap(), 0, MaxLength);
        DPRINT("MaxLength: %d\n", MaxLength);
        if (!NewBuffer) return STATUS_NO_MEMORY;
    }
    else
    {
        /* NT prefix present */

        /* Use the optimized path after acquiring the lock */
        QuickPath = TRUE;
        NewBuffer = NULL;
    }

    /* Lock the PEB and check if the quick path can be used */
    RtlAcquirePebLock();
    if (QuickPath)
    {
        /* Some simple fixups will get us the correct path */
        DPRINT("Quick path\n");
        Status = RtlpWin32NTNameToNtPathName_U(&CapturedDosName,
                                               NtName,
                                               PartName,
                                               RelativeName);

        /* Release the lock, we're done here */
        RtlReleasePebLock();
        return Status;
    }

    /* Call the main function to get the full path name and length */
    PathLength = RtlGetFullPathName_Ustr(&CapturedDosName,
                                         MAX_PATH * sizeof(WCHAR),
                                         Buffer,
                                         PartName,
                                         &NameInvalid,
                                         &InputPathType);
    if ((NameInvalid) || !(PathLength) || (PathLength > (MAX_PATH * sizeof(WCHAR))))
    {
        /* Invalid name, fail */
        DPRINT("Invalid name: %lx Path Length: %lx\n", NameInvalid, PathLength);
        RtlFreeHeap(RtlGetProcessHeap(), 0, NewBuffer);
        RtlReleasePebLock();
        return STATUS_OBJECT_NAME_INVALID;
    }

    /* Start by assuming the path starts with \??\ (DOS Devices Path) */
    PrefixLength = RtlpDosDevicesPrefix.Length;
    PrefixBuffer = RtlpDosDevicesPrefix.Buffer;
    PrefixCut = 0;

    /* Check where it really is */
    BufferPathType = RtlDetermineDosPathNameType_U(Buffer);
    DPRINT("Buffer: %S Type: %lx\n", Buffer, BufferPathType);
    switch (BufferPathType)
    {
        /* It's actually a UNC path in \??\UNC\ */
        case RtlPathTypeUncAbsolute:
            PrefixLength = RtlpDosDevicesUncPrefix.Length;
            PrefixBuffer = RtlpDosDevicesUncPrefix.Buffer;
            PrefixCut = 2;
            break;

        case RtlPathTypeLocalDevice:
            /* We made a good guess, go with it but skip the \??\ */
            PrefixCut = 4;
            break;

        case RtlPathTypeDriveAbsolute:
        case RtlPathTypeDriveRelative:
        case RtlPathTypeRooted:
        case RtlPathTypeRelative:
            /* Our guess was good, roll with it */
            break;

        /* Nothing else is expected */
        default:
            ASSERT(FALSE);
    }

    /* Now copy the prefix and the buffer */
    RtlCopyMemory(NewBuffer, PrefixBuffer, PrefixLength);
    RtlCopyMemory((PCHAR)NewBuffer + PrefixLength,
                  Buffer + PrefixCut,
                  PathLength - (PrefixCut * sizeof(WCHAR)));

    /* Compute the length */
    Length = PathLength + PrefixLength - PrefixCut * sizeof(WCHAR);
    LengthChars = Length / sizeof(WCHAR);

    /* Setup the actual NT path string and terminate it */
    NtName->Buffer = NewBuffer;
    NtName->Length = (USHORT)Length;
    NtName->MaximumLength = (USHORT)MaxLength;
    NewBuffer[LengthChars] = UNICODE_NULL;
    DPRINT("New buffer: %S\n", NewBuffer);
    DPRINT("NT Name: %wZ\n", NtName);

    /* Check if a partial name was requested */
    if ((PartName) && (*PartName))
    {
        /* Convert to Unicode */
        Status = RtlInitUnicodeStringEx(&PartNameString, *PartName);
        if (NT_SUCCESS(Status))
        {
            /* Set the partial name */
            *PartName = &NewBuffer[LengthChars - (PartNameString.Length / sizeof(WCHAR))];
        }
        else
        {
            /* Fail */
            RtlFreeHeap(RtlGetProcessHeap(), 0, NewBuffer);
            RtlReleasePebLock();
            return Status;
        }
    }

    /* Check if a relative name was asked for */
    if (RelativeName)
    {
        /* Setup the structure */
        RtlInitEmptyUnicodeString(&RelativeName->RelativeName, NULL, 0);
        RelativeName->ContainingDirectory = NULL;
        RelativeName->CurDirRef = NULL;

        /* Check if the input path itself was relative */
        if (InputPathType == RtlPathTypeRelative)
        {
            /* Get current directory */
            CurrentDirectory = &(NtCurrentPeb()->ProcessParameters->CurrentDirectory);
            if (CurrentDirectory->Handle)
            {
                Status = RtlInitUnicodeStringEx(&FullPath, Buffer);
                if (!NT_SUCCESS(Status))
                {
                    RtlFreeHeap(RtlGetProcessHeap(), 0, NewBuffer);
                    RtlReleasePebLock();
                    return Status;
                }

                /* If current directory is bigger than full path, there's no way */
                if (CurrentDirectory->DosPath.Length > FullPath.Length)
                {
                    RtlReleasePebLock();
                    return Status;
                }

                /* File is in current directory */
                if (RtlEqualUnicodeString(&FullPath, &CurrentDirectory->DosPath, TRUE))
                {
                    /* Make relative name string */
                    RelativeName->RelativeName.Buffer = (PWSTR)((ULONG_PTR)NewBuffer + PrefixLength + FullPath.Length - PrefixCut * sizeof(WCHAR));
                    RelativeName->RelativeName.Length = (USHORT)(PathLength - FullPath.Length);
                    /* If relative name starts with \, skip it */
                    if (RelativeName->RelativeName.Buffer[0] == OBJ_NAME_PATH_SEPARATOR)
                    {
                        RelativeName->RelativeName.Buffer++;
                        RelativeName->RelativeName.Length -= sizeof(WCHAR);
                    }
                    RelativeName->RelativeName.MaximumLength = RelativeName->RelativeName.Length;
                    DPRINT("RelativeName: %wZ\n", &(RelativeName->RelativeName));

                    if (!HaveRelative)
                    {
                        RelativeName->ContainingDirectory = CurrentDirectory->Handle;
                        return Status;
                    }

                    /* Give back current directory data & reference counter */
                    RelativeName->CurDirRef = RtlpCurDirRef;
                    if (RelativeName->CurDirRef)
                    {
                        InterlockedIncrement(&RtlpCurDirRef->RefCount);
                    }

                    RelativeName->ContainingDirectory = CurrentDirectory->Handle;
                }
            }
        }
    }

    /* Done */
    RtlReleasePebLock();
    return STATUS_SUCCESS;
}

int main(int argc, char *argv[]) {

  {
    //WCHAR *dos_name = L"foo";
    //UNICODE_STRING dos_name_u = {6, 6, &dos_name};
    const UNICODE_STRING dos_name = RTL_CONSTANT_STRING(L"dos_name");
    UNICODE_STRING nt_name;
    NTSTATUS status = RtlpDosPathNameToRelativeNtPathName_Ustr
    (0,
     &dos_name,
     &nt_name,
     NULL,
     NULL);
  }
}
