char *GetPathTypeString(RTL_PATH_TYPE type);

RTL_PATH_TYPE
NTAPI
RtlDetermineDosPathNameType_U(IN PCWSTR Path);

ULONG
NTAPI
RtlIsDosDeviceName_Ustr(IN PCUNICODE_STRING PathString);

ULONG
RtlpCollapsePath(PWSTR Path, /* ULONG PathBufferSize, ULONG PathLength, */ ULONG mark, BOOLEAN SkipTrailingPathSeparators);

ULONG
NTAPI
RtlGetFullPathName_Ustr(
    _In_ PUNICODE_STRING FileName,
    _In_ ULONG Size,
    _Out_z_bytecap_(Size) PWSTR Buffer,
    _Out_opt_ PCWSTR *ShortName,
    _Out_opt_ PBOOLEAN InvalidName,
    _Out_ RTL_PATH_TYPE *PathType);

BOOLEAN
NTAPI
RtlDosPathNameToRelativeNtPathName_Ustr(IN PCUNICODE_STRING DosName,
                                        OUT PUNICODE_STRING NtName,
                                        OUT PCWSTR *PartName,
                                        OUT PRTL_RELATIVE_NAME_U RelativeName);

NTSTATUS
NTAPI
RtlpDosPathNameToRelativeNtPathName_Ustr(IN BOOLEAN HaveRelative,
                                         IN PCUNICODE_STRING DosName,
                                         OUT PUNICODE_STRING NtName,
                                         OUT PCWSTR *PartName,
                                         OUT PRTL_RELATIVE_NAME_U RelativeName);
