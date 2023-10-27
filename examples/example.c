#define CCMD_IMPLEMENTATION
#include "../ccmd.h"

bool sync_test()
{
    CCMD cmd = {0};
    ccmd_append(&cmd, "gcc");
    ccmd_append(&cmd, "-o", "hello_ccmd");
    ccmd_append(&cmd, "../example.c");
#ifndef _WIN32
    ccmd_append(&cmd, "-lpthread");
#endif
    ccmd_append(&cmd, "-Wall", "-Wextra", "-Werror");

    return ccmd_run_sync(&cmd);
}

bool async_test()
{
    CCMD cmd = {0};
#ifndef _WIN32
    ccmd_append(&cmd, "sh", "-c", "for i in {1..5}; do echo Nyaa; sleep 1; done");
#else
    ccmd_append(&cmd, "for ($i = 0; $i -lt 5; $i++) { Write-Host Nyaa; Start-Sleep -Seconds 1; }");
#endif
    Process process = ccmd_run_async(&cmd);
    printf("Running code while printing Nyaa!\n");
#ifndef _WIN32
#include <unistd.h>
    sleep(3);
#else
#include <windows.h>
    Sleep(3000);
#endif
    printf("Still here!\n");
    ccmd_await(process);
    ccmd_unload(&cmd);
    return true;
}

int main()
{
    printf("Sync test: %s\n", sync_test() ? "Success" : "Failure");
    printf("Async test: %s\n", async_test() ? "Success" : "Failure");
    return 0;
}
