#include "windows.h"
#include "unicode.h"

#define DPRINT(fmt,...) do { fprintf(stderr, fmt, ##__VA_ARGS__); fflush(stderr); } while (0)

NTSTATUS NTAPI
RtlQueryEnvironmentVariable_U(PWSTR Environment,
                              PCUNICODE_STRING Name,
                              PUNICODE_STRING Value)
{
   NTSTATUS Status;
   PWSTR wcs;
   UNICODE_STRING var;
   PWSTR val;
   BOOLEAN SysEnvUsed = FALSE;

   DPRINT("RtlQueryEnvironmentVariable_U Environment %p Variable '%.*S' Value %p\n",
          Environment, Name->Length/2, Name->Buffer, Value);

   if (Environment == NULL)
   {
      PPEB Peb = RtlGetCurrentPeb();
      if (Peb) {
          RtlAcquirePebLock();
          Environment = Peb->ProcessParameters->Environment;
          SysEnvUsed = TRUE;
      }
   }

   if (Environment == NULL)
   {
      if (SysEnvUsed)
         RtlReleasePebLock();
      return(STATUS_VARIABLE_NOT_FOUND);
   }

   Value->Length = 0;

   wcs = Environment;
   DPRINT("Starting search at :%p\n", wcs);
   while (*wcs)
   {
      var.Buffer = wcs++;
      wcs = wcschr(wcs, L'=');
      if (wcs == NULL)
      {
         wcs = var.Buffer + wcslen(var.Buffer);
         DPRINT("Search at :%S\n", wcs);
      }
      if (*wcs)
      {
         var.Length = var.MaximumLength = (USHORT)(wcs - var.Buffer) * sizeof(WCHAR);
         val = ++wcs;
         wcs += wcslen(wcs);
         DPRINT("Search at :%S\n", wcs);

         if (RtlEqualUnicodeString(&var, Name, TRUE))
         {
            Value->Length = (USHORT)(wcs - val) * sizeof(WCHAR);
            if (Value->Length <= Value->MaximumLength)
            {
               memcpy(Value->Buffer, val,
                      min(Value->Length + sizeof(WCHAR), Value->MaximumLength));
               DPRINT("Value %S\n", val);
               DPRINT("Return STATUS_SUCCESS\n");
               Status = STATUS_SUCCESS;
            }
            else
            {
               DPRINT("Return STATUS_BUFFER_TOO_SMALL\n");
               Status = STATUS_BUFFER_TOO_SMALL;
            }

            if (SysEnvUsed)
               RtlReleasePebLock();

            return(Status);
         }
      }
      wcs++;
   }

   if (SysEnvUsed)
      RtlReleasePebLock();

   DPRINT("Return STATUS_VARIABLE_NOT_FOUND: '%.*S'\n", Name->Length/2, Name->Buffer);
   return(STATUS_VARIABLE_NOT_FOUND);
}
