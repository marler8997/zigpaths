#include "windows.h"

#define DPRINT(fmt,...) do { fprintf(stderr, fmt, ##__VA_ARGS__); fflush(stderr); } while (0)

PUSHORT NlsUnicodeUpcaseTable = NULL;
PUSHORT NlsUnicodeLowercaseTable = NULL;

NTSTATUS
NTAPI
RtlInitUnicodeStringEx(
    OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString)
{

    if (SourceString)
    {
        CONST SIZE_T MAX_SIZE_BYTES = (MAXUSHORT & ~1) - sizeof(WCHAR); // an even number

        size_t SizeBytes = wcslen(SourceString) * sizeof(WCHAR);
        if (SizeBytes > MAX_SIZE_BYTES) return STATUS_NAME_TOO_LONG;
        DestinationString->Length = (USHORT)SizeBytes;
        DestinationString->MaximumLength = (USHORT)SizeBytes + sizeof(WCHAR);
    }
    else
    {
        DestinationString->Length = 0;
        DestinationString->MaximumLength = 0;
    }

    DestinationString->Buffer = (PWCHAR)SourceString;
    return STATUS_SUCCESS;
}

WCHAR NTAPI
RtlpUpcaseUnicodeChar(IN WCHAR Source)
{
    USHORT Offset;

    if (Source < 'a')
        return Source;

    if (Source <= 'z')
        return (Source - ('a' - 'A'));

    Offset = ((USHORT)Source >> 8) & 0xFF;
    Offset = NlsUnicodeUpcaseTable[Offset];

    Offset += ((USHORT)Source >> 4) & 0xF;
    Offset = NlsUnicodeUpcaseTable[Offset];

    Offset += ((USHORT)Source & 0xF);
    Offset = NlsUnicodeUpcaseTable[Offset];

    return Source + (SHORT)Offset;
}

LONG
NTAPI
RtlCompareUnicodeString(
    IN PCUNICODE_STRING s1,
    IN PCUNICODE_STRING s2,
    IN BOOLEAN  CaseInsensitive)
{
    unsigned int len;
    LONG ret = 0;
    LPCWSTR p1, p2;

    len = min(s1->Length, s2->Length) / sizeof(WCHAR);
    p1 = s1->Buffer;
    p2 = s2->Buffer;

    if (CaseInsensitive)
    {
        while (!ret && len--) ret = RtlpUpcaseUnicodeChar(*p1++) - RtlpUpcaseUnicodeChar(*p2++);
    }
    else
    {
        while (!ret && len--) ret = *p1++ - *p2++;
    }

    if (!ret) ret = s1->Length - s2->Length;

    return ret;
}

BOOLEAN
NTAPI
RtlEqualUnicodeString(
    IN CONST UNICODE_STRING *s1,
    IN CONST UNICODE_STRING *s2,
    IN BOOLEAN  CaseInsensitive)
{
    if (s1->Length != s2->Length) return FALSE;
    return !RtlCompareUnicodeString(s1, s2, CaseInsensitive );
}

WCHAR NTAPI
RtlpDowncaseUnicodeChar(IN WCHAR Source)
{
    USHORT Offset;

    PAGED_CODE_RTL();

    if (Source < L'A')
        return Source;

    if (Source <= L'Z')
        return Source + (L'a' - L'A');

    if (Source < 0x80)
        return Source;

    Offset = ((USHORT)Source >> 8);
    DPRINT("Offset: %hx\n", Offset);

    Offset = NlsUnicodeLowercaseTable[Offset];
    DPRINT("Offset: %hx\n", Offset);

    Offset += (((USHORT)Source & 0x00F0) >> 4);
    DPRINT("Offset: %hx\n", Offset);

    Offset = NlsUnicodeLowercaseTable[Offset];
    DPRINT("Offset: %hx\n", Offset);

    Offset += ((USHORT)Source & 0x000F);
    DPRINT("Offset: %hx\n", Offset);

    Offset = NlsUnicodeLowercaseTable[Offset];
    DPRINT("Offset: %hx\n", Offset);

    DPRINT("Result: %hx\n", Source + (SHORT)Offset);

    return Source + (SHORT)Offset;
}

