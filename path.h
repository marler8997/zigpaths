char *GetPathTypeString(RTL_PATH_TYPE type);

RTL_PATH_TYPE
NTAPI
RtlDetermineDosPathNameType_U(IN PCWSTR Path);

ULONG
NTAPI
RtlIsDosDeviceName_Ustr(IN PCUNICODE_STRING PathString);

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
