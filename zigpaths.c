#include "windows.h"
#include "rtl.h"

#include "path.h"

static struct PEB global_peb;
struct PEB *NtCurrentPeb() { return &global_peb; }

int main(int argc, char *argv[]) {
  struct ProcessParameters peb = {
    .CurrentDirectory = {.DosPath = RTL_CONSTANT_STRING(L"C:\\tmp\\")},
    .Environment = L"FOO=Bar\0BAR=Bas\0\0"
  };
  global_peb.ProcessParameters = &peb;

  {
    UNICODE_STRING dos_name = RTL_CONSTANT_STRING(L"dos_name");
    UNICODE_STRING nt_name;
    NTSTATUS status = RtlpDosPathNameToRelativeNtPathName_Ustr
    (0,
     &dos_name,
     &nt_name,
     NULL,
     NULL);
  }
}
