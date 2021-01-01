#include "windows.h"
#include "rtl.h"

static struct TEB TEB;
struct PEB *NtCurrentPeb() { return TEB.ProcessEnvironmentBlock; }


typedef struct _OBJECT_ATTRIBUTES {
  ULONG Length;
  HANDLE RootDirectory;
  PUNICODE_STRING ObjectName;
  ULONG Attributes;
  PVOID SecurityDescriptor;
  PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;
typedef CONST OBJECT_ATTRIBUTES *PCOBJECT_ATTRIBUTES;





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









#define InitializeObjectAttributes(p,n,a,r,s) { \
    (p)->Length = sizeof(OBJECT_ATTRIBUTES);    \
    (p)->RootDirectory = (r);                   \
    (p)->Attributes = (a);                      \
    (p)->ObjectName = (n);                      \
    (p)->SecurityDescriptor = (s);              \
    (p)->SecurityQualityOfService = NULL;       \
}

typedef struct _FILE_BASIC_INFORMATION
{
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    ULONG FileAttributes;
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

#define OBJ_CASE_INSENSITIVE                    0x00000040L

BOOLEAN
NTAPI
RtlDoesFileExists_UstrEx(IN PCUNICODE_STRING FileName,
                         IN BOOLEAN SucceedIfBusy)
{
    BOOLEAN Result;
    RTL_RELATIVE_NAME_U RelativeName;
    UNICODE_STRING NtPathName;
    PVOID Buffer;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    FILE_BASIC_INFORMATION BasicInformation;

    /* Get the NT Path */
    Result = RtlDosPathNameToRelativeNtPathName_Ustr(FileName,
                                                     &NtPathName,
                                                     NULL,
                                                     &RelativeName);
    if (!Result) return FALSE;

    /* Save the buffer */
    Buffer = NtPathName.Buffer;

    /* Check if we have a relative name */
    if (RelativeName.RelativeName.Length)
    {
        /* Use it */
        NtPathName = RelativeName.RelativeName;
    }
    else
    {
        /* Otherwise ignore it */
        RelativeName.ContainingDirectory = NULL;
    }

    /* Initialize the object attributes */
    InitializeObjectAttributes(&ObjectAttributes,
                               &NtPathName,
                               OBJ_CASE_INSENSITIVE,
                               RelativeName.ContainingDirectory,
                               NULL);

    /* Query the attributes and free the buffer now */
    Status = ZwQueryAttributesFile(&ObjectAttributes, &BasicInformation);
    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlGetProcessHeap(), 0, Buffer);

    /* Check if we failed */
    if (!NT_SUCCESS(Status))
    {
        /* Check if we failed because the file is in use */
        if ((Status == STATUS_SHARING_VIOLATION) ||
            (Status == STATUS_ACCESS_DENIED))
        {
            /* Check if the caller wants this to be considered OK */
            Result = SucceedIfBusy ? TRUE : FALSE;
        }
        else
        {
            /* A failure because the file didn't exist */
            Result = FALSE;
        }
    }
    else
    {
        /* The file exists */
        Result = TRUE;
    }

    /* Return the result */
    return Result;
}

BOOLEAN
NTAPI
RtlDoesFileExists_UStr(IN PUNICODE_STRING FileName)
{
    /* Call the updated API */
    return RtlDoesFileExists_UstrEx(FileName, TRUE);
}

BOOLEAN
NTAPI
RtlDoesFileExists_UEx(IN PCWSTR FileName,
                      IN BOOLEAN SucceedIfBusy)
{
    UNICODE_STRING NameString;

    /* Create the unicode name*/
    if (NT_SUCCESS(RtlInitUnicodeStringEx(&NameString, FileName)))
    {
        /* Call the unicode function */
        return RtlDoesFileExists_UstrEx(&NameString, SucceedIfBusy);
    }

    /* Fail */
    return FALSE;
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

