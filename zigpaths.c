#include <stdarg.h>

#include "windows.h"
#include "rtl.h"
#include "unicode.h"

#include "path.h"

static struct PEB global_peb;
struct PEB *NtCurrentPeb() { return &global_peb; }









#if 0
#include "ntdll_test.h"

static NTSTATUS (WINAPI *pRtlMultiByteToUnicodeN)( LPWSTR dst, DWORD dstlen, LPDWORD reslen,
                                                   LPCSTR src, DWORD srclen );
static NTSTATUS (WINAPI *pRtlUnicodeToMultiByteN)(LPSTR,DWORD,LPDWORD,LPCWSTR,DWORD);
static UINT (WINAPI *pRtlDetermineDosPathNameType_U)( PCWSTR path );
static ULONG (WINAPI *pRtlIsDosDeviceName_U)( PCWSTR dos_name );
static NTSTATUS (WINAPI *pRtlOemStringToUnicodeString)(UNICODE_STRING *, const STRING *, BOOLEAN );
static BOOLEAN (WINAPI *pRtlIsNameLegalDOS8Dot3)(const UNICODE_STRING*,POEM_STRING,PBOOLEAN);
static DWORD (WINAPI *pRtlGetFullPathName_U)(const WCHAR*,ULONG,WCHAR*,WCHAR**);
static NTSTATUS (WINAPI *pRtlDosPathNameToNtPathName_U_WithStatus)(const WCHAR*, UNICODE_STRING*, WCHAR**, CURDIR*);
#endif

static void expect(int cond, const char *fmt, ...)
{
  if (!cond) {
    fprintf(stderr, "Error: test failed: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
    exit(1);
  }
}

static void test_RtlDetermineDosPathNameType_U(void)
{
    struct test
    {
        const WCHAR *path;
        UINT ret;
    };
    static const struct test tests[] =
    {
        { L"\\\\foo", RtlPathTypeUncAbsolute },
        { L"//foo"  , RtlPathTypeUncAbsolute },
        { L"\\/foo" , RtlPathTypeUncAbsolute },
        { L"/\\foo" , RtlPathTypeUncAbsolute },
        { L"\\\\"   , RtlPathTypeUncAbsolute },
        { L"//"     , RtlPathTypeUncAbsolute },

        { L"c:\\foo", RtlPathTypeDriveAbsolute },
        { L"c:/foo" , RtlPathTypeDriveAbsolute },
        { L"c://foo", RtlPathTypeDriveAbsolute },
        { L"c:\\"   , RtlPathTypeDriveAbsolute },
        { L"c:/"    , RtlPathTypeDriveAbsolute },

        { L"c:foo"    , RtlPathTypeDriveRelative },
        { L"c:f\\oo"  , RtlPathTypeDriveRelative },
        { L"c:foo/bar", RtlPathTypeDriveRelative },

        { L"\\foo", RtlPathTypeRooted },
        { L"/foo" , RtlPathTypeRooted },
        { L"\\"   , RtlPathTypeRooted },
        { L"/"    , RtlPathTypeRooted },

        { L"foo"   , RtlPathTypeRelative },
        { L""      , RtlPathTypeRelative },
        { L"\0:foo", RtlPathTypeRelative },

        { L"\\\\.\\foo", RtlPathTypeLocalDevice },
        { L"//./foo"   , RtlPathTypeLocalDevice },
        { L"/\\./foo"  , RtlPathTypeLocalDevice },
        { L"\\\\.foo"  , RtlPathTypeUncAbsolute },
        { L"//.foo"    , RtlPathTypeUncAbsolute },

        { L"\\\\.", RtlPathTypeRootLocalDevice },
        { L"//."  , RtlPathTypeRootLocalDevice },
        { NULL, 0 }
    };
    for (const struct test *test = tests; test->path; test++)
    {
        RTL_PATH_TYPE result = RtlDetermineDosPathNameType_U(test->path);
        expect(result == test->ret, "expected path type %d but got %d for path '%s'\n", result, test->ret, test->path );
    }
}

#if 0
static void test_RtlIsDosDeviceName_U(void)
{
    struct test
    {
        const char *path;
        WORD pos;
        WORD len;
        BOOL fails;
    };

    static const struct test tests[] =
    {
        { "\\\\.\\CON",    8, 6, TRUE },  /* fails on win8 */
        { "\\\\.\\con",    8, 6, TRUE },  /* fails on win8 */
        { "\\\\.\\CON2",   0, 0 },
        { "",              0, 0 },
        { "\\\\foo\\nul",  0, 0 },
        { "c:\\nul:",      6, 6 },
        { "c:\\nul\\",     0, 0 },
        { "c:\\nul\\foo",  0, 0 },
        { "c:\\nul::",     6, 6, TRUE },  /* fails on nt4 */
        { "c:\\nul::::::", 6, 6, TRUE },  /* fails on nt4 */
        { "c:prn     ",    4, 6 },
        { "c:prn.......",  4, 6 },
        { "c:prn... ...",  4, 6 },
        { "c:NUL  ....  ", 4, 6, TRUE },  /* fails on nt4 */
        { "c: . . .",      0, 0 },
        { "c:",            0, 0 },
        { " . . . :",      0, 0 },
        { ":",             0, 0 },
        { "c:nul. . . :",  4, 6 },
        { "c:nul . . :",   4, 6, TRUE },  /* fails on nt4 */
        { "c:nul0",        0, 0 },
        { "c:prn:aaa",     4, 6, TRUE },  /* fails on win9x */
        { "c:PRN:.txt",    4, 6 },
        { "c:aux:.txt...", 4, 6 },
        { "c:prn:.txt:",   4, 6 },
        { "c:nul:aaa",     4, 6, TRUE },  /* fails on win9x */
        { "con:",          0, 6 },
        { "lpt1:",         0, 8 },
        { "c:com5:",       4, 8 },
        { "CoM4:",         0, 8 },
        { "lpt9:",         0, 8 },
        { "c:\\lpt0.txt",  0, 0 },
        { "c:aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\\nul.txt", 1000, 6 },
        // ReactOS r54114
        { "c:\\nul",       6, 6 },
        { NULL, 0 }
    };

    const struct test *test;
    WCHAR buffer[2000];
    ULONG ret;

    if (!pRtlIsDosDeviceName_U)
    {
        win_skip("RtlIsDosDeviceName_U is not available\n");
        return;
    }

    for (test = tests; test->path; test++)
    {
        pRtlMultiByteToUnicodeN( buffer, sizeof(buffer), NULL, test->path, strlen(test->path)+1 );
        ret = pRtlIsDosDeviceName_U( buffer );
        ok( ret == MAKELONG( test->len, test->pos ) ||
            (test->fails && broken( ret == 0 )),
            "Wrong result (%d,%d)/(%d,%d) for %s\n",
            HIWORD(ret), LOWORD(ret), test->pos, test->len, test->path );
    }
}

static void test_RtlIsNameLegalDOS8Dot3(void)
{
    struct test
    {
        const char *path;
        BOOLEAN result;
        BOOLEAN spaces;
    };

    static const struct test tests[] =
    {
        { "12345678",     TRUE,  FALSE },
        { "123 5678",     TRUE,  TRUE  },
        { "12345678.",    FALSE, 2 /*not set*/ },
        { "1234 678.",    FALSE, 2 /*not set*/ },
        { "12345678.a",   TRUE,  FALSE },
        { "12345678.a ",  FALSE, 2 /*not set*/ },
        { "12345678.a c", TRUE,  TRUE  },
        { " 2345678.a ",  FALSE, 2 /*not set*/ },
        { "1 345678.abc", TRUE,  TRUE },
        { "1      8.a c", TRUE,  TRUE },
        { "1 3 5 7 .abc", FALSE, 2 /*not set*/ },
        { "12345678.  c", TRUE,  TRUE },
        { "123456789.a",  FALSE, 2 /*not set*/ },
        { "12345.abcd",   FALSE, 2 /*not set*/ },
        { "12345.ab d",   FALSE, 2 /*not set*/ },
        { ".abc",         FALSE, 2 /*not set*/ },
        { "12.abc.d",     FALSE, 2 /*not set*/ },
        { ".",            TRUE,  FALSE },
        { "..",           TRUE,  FALSE },
        { "...",          FALSE, 2 /*not set*/ },
        { "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", FALSE, 2 /*not set*/ },
        { NULL, 0 }
    };

    const struct test *test;
    UNICODE_STRING ustr;
    OEM_STRING oem, oem_ret;
    WCHAR buffer[200];
    char buff2[12];
    BOOLEAN ret, spaces;

    if (!pRtlIsNameLegalDOS8Dot3)
    {
        win_skip("RtlIsNameLegalDOS8Dot3 is not available\n");
        return;
    }

    ustr.MaximumLength = sizeof(buffer);
    ustr.Buffer = buffer;
    for (test = tests; test->path; test++)
    {
        char path[100];
        strcpy(path, test->path);
        oem.Buffer = path;
        oem.Length = strlen(test->path);
        oem.MaximumLength = oem.Length + 1;
        pRtlOemStringToUnicodeString( &ustr, &oem, FALSE );
        spaces = 2;
        oem_ret.Length = oem_ret.MaximumLength = sizeof(buff2);
        oem_ret.Buffer = buff2;
        ret = pRtlIsNameLegalDOS8Dot3( &ustr, &oem_ret, &spaces );
        ok( ret == test->result, "Wrong result %d/%d for '%s'\n", ret, test->result, test->path );
        ok( spaces == test->spaces, "Wrong spaces value %d/%d for '%s'\n", spaces, test->spaces, test->path );
        if (strlen(test->path) <= 12)
        {
            char str[13];
            int i;
            strcpy( str, test->path );
            for (i = 0; str[i]; i++) str[i] = toupper(str[i]);
            ok( oem_ret.Length == strlen(test->path), "Wrong length %d/%d for '%s'\n",
                oem_ret.Length, lstrlenA(test->path), test->path );
            ok( !memcmp( oem_ret.Buffer, str, oem_ret.Length ),
                "Wrong string '%.*s'/'%s'\n", oem_ret.Length, oem_ret.Buffer, str );
        }
    }
}
static void test_RtlGetFullPathName_U(void)
{
    static const WCHAR emptyW[] = {0};
    static const WCHAR deadbeefW[] = {'d','e','a','d','b','e','e','f',0};

    struct test
    {
        const char *path;
        const char *rname;
        const char *rfile;
        const char *alt_rname;
        const char *alt_rfile;
    };

    static const struct test tests[] =
        {
            { "c:/test",                     "c:\\test",         "test"},
            { "c:/test     ",                "c:\\test",         "test"},
            { "c:/test.",                    "c:\\test",         "test"},
            { "c:/test  ....   ..   ",       "c:\\test",         "test"},
            { "c:/test/  ....   ..   ",      "c:\\test\\",       NULL},
            { "c:/test/..",                  "c:\\",             NULL},
            { "c:/test/.. ",                 "c:\\test\\",       NULL},
            { "c:/TEST",                     "c:\\TEST",         "TEST"},
            { "c:/test/file",                "c:\\test\\file",   "file"},
            { "c:/test./file",               "c:\\test\\file",   "file"},
            { "c:/test.. /file",             "c:\\test.. \\file","file"},
            { "c:/test/././file",            "c:\\test\\file",   "file"},
            { "c:/test\\.\\.\\file",         "c:\\test\\file",   "file"},
            { "c:/test/\\.\\.\\file",        "c:\\test\\file",   "file"},
            { "c:/test\\\\.\\.\\file",       "c:\\test\\file",   "file"},
            { "c:/test\\test1\\..\\.\\file", "c:\\test\\file",   "file"},
            { "c:///test\\.\\.\\file//",     "c:\\test\\file\\", NULL,
                                             "c:\\test\\file",   "file"},  /* nt4 */
            { "c:///test\\..\\file\\..\\//", "c:\\",             NULL},
            { "c:/test../file",              "c:\\test.\\file",  "file",
                                             "c:\\test..\\file", "file"},  /* vista */
            { "c:\\test",                    "c:\\test",         "test"},
            { NULL, NULL, NULL}
        };

    const struct test *test;
    WCHAR pathbufW[2*MAX_PATH], rbufferW[MAX_PATH];
    CHAR  rbufferA[MAX_PATH], rfileA[MAX_PATH];
    ULONG ret;
    WCHAR *file_part;
    DWORD reslen;
    UINT len;

    if (!pRtlGetFullPathName_U)
    {
        win_skip("RtlGetFullPathName_U is not available\n");
        return;
    }

    file_part = (WCHAR *)0xdeadbeef;
    lstrcpyW(rbufferW, deadbeefW);
    ret = pRtlGetFullPathName_U(NULL, MAX_PATH, rbufferW, &file_part);
    ok(!ret, "Expected RtlGetFullPathName_U to return 0, got %u\n", ret);
    ok(!lstrcmpW(rbufferW, deadbeefW),
       "Expected the output buffer to be untouched, got %s\n", wine_dbgstr_w(rbufferW));
    ok(file_part == (WCHAR *)0xdeadbeef ||
       file_part == NULL, /* Win7 */
       "Expected file part pointer to be untouched, got %p\n", file_part);

    file_part = (WCHAR *)0xdeadbeef;
    lstrcpyW(rbufferW, deadbeefW);
    ret = pRtlGetFullPathName_U(emptyW, MAX_PATH, rbufferW, &file_part);
    ok(!ret, "Expected RtlGetFullPathName_U to return 0, got %u\n", ret);
    ok(!lstrcmpW(rbufferW, deadbeefW),
       "Expected the output buffer to be untouched, got %s\n", wine_dbgstr_w(rbufferW));
    ok(file_part == (WCHAR *)0xdeadbeef ||
       file_part == NULL, /* Win7 */
       "Expected file part pointer to be untouched, got %p\n", file_part);

    for (test = tests; test->path; test++)
    {
        len= strlen(test->rname) * sizeof(WCHAR);
        pRtlMultiByteToUnicodeN(pathbufW , sizeof(pathbufW), NULL, test->path, strlen(test->path)+1 );
        ret = pRtlGetFullPathName_U( pathbufW,MAX_PATH, rbufferW, &file_part);
        ok( ret == len || (test->alt_rname && ret == strlen(test->alt_rname)*sizeof(WCHAR)),
            "Wrong result %d/%d for \"%s\"\n", ret, len, test->path );
        ok(pRtlUnicodeToMultiByteN(rbufferA,MAX_PATH,&reslen,rbufferW,(lstrlenW(rbufferW) + 1) * sizeof(WCHAR)) == STATUS_SUCCESS,
           "RtlUnicodeToMultiByteN failed\n");
        ok(!lstrcmpA(rbufferA,test->rname) || (test->alt_rname && !lstrcmpA(rbufferA,test->alt_rname)),
           "Got \"%s\" expected \"%s\"\n",rbufferA,test->rname);
        if (file_part)
        {
            ok(pRtlUnicodeToMultiByteN(rfileA,MAX_PATH,&reslen,file_part,(lstrlenW(file_part) + 1) * sizeof(WCHAR)) == STATUS_SUCCESS,
               "RtlUnicodeToMultiByteN failed\n");
            ok((test->rfile && !lstrcmpA(rfileA,test->rfile)) ||
               (test->alt_rfile && !lstrcmpA(rfileA,test->alt_rfile)),
               "Got \"%s\" expected \"%s\"\n",rfileA,test->rfile);
        }
        else
        {
            ok( !test->rfile, "Got NULL expected \"%s\"\n", test->rfile );
        }
    }
}

static void test_RtlDosPathNameToNtPathName_U_WithStatus(void)
{
    static const WCHAR emptyW[] = { 0 };
    WCHAR path[MAX_PATH];
    UNICODE_STRING nameW;
    NTSTATUS status;

    if (!pRtlDosPathNameToNtPathName_U_WithStatus)
    {
        win_skip("RtlDosPathNameToNtPathName_U_WithStatus() is not supported.\n");
        return;
    }

    GetCurrentDirectoryW( MAX_PATH, path );

    status = pRtlDosPathNameToNtPathName_U_WithStatus( path, &nameW, NULL, NULL );
    ok(!status, "Failed convert to nt path, %#x.\n", status);

    status = pRtlDosPathNameToNtPathName_U_WithStatus( NULL, &nameW, NULL, NULL );
    ok(status == STATUS_OBJECT_NAME_INVALID || broken(status == STATUS_OBJECT_PATH_NOT_FOUND) /* W2k3 */,
        "Unexpected status %#x.\n", status);

    status = pRtlDosPathNameToNtPathName_U_WithStatus( emptyW, &nameW, NULL, NULL );
    ok(status == STATUS_OBJECT_NAME_INVALID || broken(status == STATUS_OBJECT_PATH_NOT_FOUND) /* W2k3 */,
        "Unexpected status %#x.\n", status);

    RtlFreeUnicodeString( &nameW );
}

START_TEST(path)
{
    HMODULE mod = GetModuleHandleA("ntdll.dll");
    if (!mod)
    {
        win_skip("Not running on NT, skipping tests\n");
        return;
    }

    pRtlMultiByteToUnicodeN = (void *)GetProcAddress(mod,"RtlMultiByteToUnicodeN");
    pRtlUnicodeToMultiByteN = (void *)GetProcAddress(mod,"RtlUnicodeToMultiByteN");
    pRtlDetermineDosPathNameType_U = (void *)GetProcAddress(mod,"RtlDetermineDosPathNameType_U");
    pRtlIsDosDeviceName_U = (void *)GetProcAddress(mod,"RtlIsDosDeviceName_U");
    pRtlOemStringToUnicodeString = (void *)GetProcAddress(mod,"RtlOemStringToUnicodeString");
    pRtlIsNameLegalDOS8Dot3 = (void *)GetProcAddress(mod,"RtlIsNameLegalDOS8Dot3");
    pRtlGetFullPathName_U = (void *)GetProcAddress(mod,"RtlGetFullPathName_U");
    pRtlDosPathNameToNtPathName_U_WithStatus = (void *)GetProcAddress(mod, "RtlDosPathNameToNtPathName_U_WithStatus");

    test_RtlDetermineDosPathNameType_U();
    test_RtlIsDosDeviceName_U();
    test_RtlIsNameLegalDOS8Dot3();
    test_RtlGetFullPathName_U();
    test_RtlDosPathNameToNtPathName_U_WithStatus();
}
#endif







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



  test_RtlDetermineDosPathNameType_U();


  test(L"foo", L"\\??\\C:\\tmp\\foo");
  test(L"foo*o", L"\\??\\C:\\tmp\\foo*o");
  //test(L"LPT", L"LPT");
  // TODO: this is causing an infinite loop!
  //test(L"CON");


  fprintf(stderr, "Success!\n");
}
