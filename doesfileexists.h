BOOLEAN
NTAPI
RtlDoesFileExists_UstrEx(IN PCUNICODE_STRING FileName,
                         IN BOOLEAN SucceedIfBusy);
BOOLEAN
NTAPI
RtlDoesFileExists_UStr(IN PUNICODE_STRING FileName);

BOOLEAN
NTAPI
RtlDoesFileExists_UEx(IN PCWSTR FileName,
                      IN BOOLEAN SucceedIfBusy);

static
BOOLEAN
NTAPI
RtlDoesFileExists_U(IN PCWSTR FileName)
{
    /* Call the new function */
    return RtlDoesFileExists_UEx(FileName, TRUE);
}
