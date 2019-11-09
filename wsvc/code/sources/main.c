// Copyright (c) Vincent Ycasas
// SPDX-License-Identifier: MIT

#include <wsvc/console.h>
#include <wsvc/service.h>
#include <wsvc/wsvc.h>

#include <tchar.h>
#include <strsafe.h>
#include <Windows.h>

static int const WSVC_EXIT_OK = 0;
static int const WSVC_EXIT_ERROR = -1;

static LPCTSTR const WSVC_COMMAND_INSTALL = TEXT("install");
static LPCTSTR const WSVC_COMMAND_UNINSTALL = TEXT("uninstall");

int _tmain(int const argc, TCHAR const* const argv[], TCHAR const* const envp[])
{
    LPCTSTR commandStr = NULL;
    int serviceResult = WSVC_EXIT_ERROR;
    UNREFERENCED_PARAMETER(envp);

    if (argc < 2) {
        serviceResult = wsvc_service_run();
        if (serviceResult != 0) {
            wsvc_write_to_stderr(TEXT("[WSVC] Error: Service failed to run.\n"));
            return (WSVC_EXIT_ERROR);
        }
        return (serviceResult);
    }

    commandStr = argv[1];

    if (_tcsicmp(commandStr, WSVC_COMMAND_INSTALL) == 0) {
        serviceResult = wsvc_service_install();
        if (serviceResult != 0) {
            wsvc_write_to_stderr(TEXT("[WSVC] Error: Failed to install service.\n"));
            return (WSVC_EXIT_ERROR);
        }
    }
    else if (_tcsicmp(commandStr, WSVC_COMMAND_UNINSTALL) == 0) {
        serviceResult = wsvc_service_uninstall();
        if (serviceResult != 0) {
            wsvc_write_to_stderr(TEXT("[WSVC] Error: Failed to uninstall service.\n"));
            return (WSVC_EXIT_ERROR);
        }
    }
    else {
        #define WSVC_ERROR_MESSAGE_LENGTH 64

        TCHAR errorMessage[WSVC_ERROR_MESSAGE_LENGTH];

        ZeroMemory(errorMessage, WSVC_ERROR_MESSAGE_LENGTH);

        StringCchPrintf(errorMessage, WSVC_ERROR_MESSAGE_LENGTH, TEXT("[WSVC] Error: Unknown command \"%s\".\n"), commandStr);

        wsvc_write_to_stderr(errorMessage);

        return (WSVC_EXIT_ERROR);

        #undef WSVC_ERROR_MESSAGE_LENGTH
    }

    return (WSVC_EXIT_OK);
}
