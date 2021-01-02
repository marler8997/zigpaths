#include "windows.h"
#include "rtl.h"

#include "path.h"

static struct TEB TEB;
struct PEB *NtCurrentPeb() { return TEB.ProcessEnvironmentBlock; }

int main(int argc, char *argv[]) {
  {
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

