NTSTATUS
NTAPI
RtlInitUnicodeStringEx(
    OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString);

BOOLEAN
NTAPI
RtlEqualUnicodeString(
    IN CONST UNICODE_STRING *s1,
    IN CONST UNICODE_STRING *s2,
    IN BOOLEAN  CaseInsensitive);

WCHAR NTAPI
RtlpDowncaseUnicodeChar(IN WCHAR Source);

WCHAR NTAPI
RtlpUpcaseUnicodeChar(IN WCHAR Source);
