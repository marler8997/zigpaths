#include "windows.h"
#include "rtl.h"
#include "unicode.h"

#include "path.h"

static struct PEB global_peb;
struct PEB *NtCurrentPeb() { return &global_peb; }

static void test(PCWSTR DosPathName, PCWSTR Expected) {
  UNICODE_STRING DosPathNameUstr = initUnicodeStringNullTerm((PWSTR)DosPathName);
  UNICODE_STRING NtName;
  NTSTATUS status = RtlpDosPathNameToRelativeNtPathName_Ustr
    (0,
     &DosPathNameUstr,
     &NtName,
     NULL,
     NULL);
  if (!NT_SUCCESS(status)) {
    fprintf(stderr, "test '%S' failed with %d\n", DosPathName, status);
    exit(1);
  }
  fprintf(stderr, "--------\ntest     '%S'\nexpected '%S'\nactual   '%.*S'\n",
          DosPathName,
          Expected,
          NtName.Length, NtName.Buffer);

  UNICODE_STRING ExpectedUstr = initUnicodeStringNullTerm((PWSTR)Expected);
  if (!RtlEqualUnicodeString(&ExpectedUstr, &NtName, 0)) {
    fprintf(stderr, "Error: strings are not equal\n");
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  struct ProcessParameters peb = {
    .CurrentDirectory = {.DosPath = RTL_CONSTANT_STRING(L"C:\\tmp\\")},
    .Environment = L"FOO=Bar\0BAR=Bas\0\0"
  };
  global_peb.ProcessParameters = &peb;

  test(L"foo", L"\\??\\C:\\tmp\\foo");
  test(L"foo*o", L"\\??\\C:\\tmp\\foo*o");
  //test(L"LPT", L"LPT");
  // TODO: this is causing an infinite loop!
  //test(L"CON");


  fprintf(stderr, "Success!\n");
}
