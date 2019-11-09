// Copyright (c) Vincent Ycasas
// SPDX-License-Identifier: MIT

#include <wsvc/console.h>

#include <stdbool.h>
#include <strsafe.h>

static LPCTSTR const WSVC_LOGFILE_PATH = TEXT("C:\\wsvc.log");

static int const WSVC_LOGFILE_OK = 0;
static int const WSVC_LOGFILE_ERROR = -1;
static int const WSVC_LOGFILE_ERROR_INVALID_FILE_HANDLE = -2;

static size_t wsvc_convert_string_to_utf8(
    LPCTSTR const inputString,
    char* outputUtf8String,
    size_t outputUtf8StringLength)
{
    size_t bytesWritten = 0;

#if defined(UNICODE)

    if ((outputUtf8String == NULL) || (outputUtf8StringLength == 0)) {
        bytesWritten = (size_t) WideCharToMultiByte(
            CP_UTF8,
            WC_ERR_INVALID_CHARS,
            inputString,
            -1,
            NULL,
            0,
            NULL,
            NULL);
    }
    else {
        bytesWritten = (size_t) WideCharToMultiByte(
            CP_UTF8,
            WC_ERR_INVALID_CHARS,
            inputString,
            -1,
            outputUtf8String,
            (int) outputUtf8StringLength,
            NULL,
            NULL);
    }

#else // defined(UNICODE)
    if ((outputUtf8String == NULL) || (outputUtf8StringLength == 0)) {
        bytesWritten = strlen(inputString);
    }
    else {
        // There are no multibyte to multibyte conversion functions, so strings are just copied directly.
        memcpy_s(outputUtf8String, outputUtf8StringLength, (void const*)inputString, inputStringLength);
        bytesWritten = strlen(outputUtf8String);
    }
#endif // defined(UNICODE)

    return (bytesWritten);
}

static int wsvc_append_to_log_file(LPCTSTR const logFilePath, LPCTSTR const message)
{
    int result = WSVC_LOGFILE_ERROR;
    HANDLE hLogFile = INVALID_HANDLE_VALUE;

    if (message == NULL)
        return (0);

    hLogFile = CreateFile(
        logFilePath,
        FILE_APPEND_DATA,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if ((hLogFile == NULL) || (hLogFile == INVALID_HANDLE_VALUE))
        return (WSVC_LOGFILE_ERROR_INVALID_FILE_HANDLE);

    do {
        #define WSVC_UTF8_STRING_LENGTH 128

        BOOL writeOk = FALSE;
        char utf8String[WSVC_UTF8_STRING_LENGTH];
        size_t bytesWritten = 0;

        ZeroMemory(utf8String, WSVC_UTF8_STRING_LENGTH);
        bytesWritten = wsvc_convert_string_to_utf8(message, utf8String, WSVC_UTF8_STRING_LENGTH);
        if (bytesWritten == 0)
            break;

        writeOk = WriteFile(hLogFile, (LPCVOID) utf8String, (DWORD) strlen(utf8String), NULL, NULL);

        result = (writeOk == TRUE) ? WSVC_LOGFILE_OK : WSVC_LOGFILE_ERROR;

        #undef WSVC_UTF8_STRING_LENGTH
    }
    while (false);

    if ((hLogFile != NULL) && (hLogFile != INVALID_HANDLE_VALUE)) {
        CloseHandle(hLogFile);
    }

    return (result);
}

static int wsvc_write_to_console(HANDLE hConsole, LPCTSTR const message)
{
    int result = WSVC_CONSOLE_ERROR;
    size_t messageLength = 0;
    HRESULT strlenResult = S_OK;

    if (message == NULL)
        return (0);

    if ((hConsole == NULL) || (hConsole == INVALID_HANDLE_VALUE))
        return (WSVC_CONSOLE_ERROR_CANNOT_FIND_CONSOLE);

    strlenResult = StringCchLength(message, STRSAFE_MAX_CCH, &messageLength);

    if (FAILED(strlenResult))
        return (WSVC_CONSOLE_ERROR_FAILED_TO_GET_MESSAGE_LENGTH);

    result = WriteConsole(hConsole, (VOID CONST*) message, (DWORD) messageLength, NULL, NULL);
        
#if defined(DEBUG)
    wsvc_append_to_log_file(WSVC_LOGFILE_PATH, message);
#endif // defined(DEBUG)

    return (result);
}

int wsvc_write_to_stdout(LPCTSTR const message)
{
    int result = WSVC_CONSOLE_ERROR;
    HANDLE hConsole = INVALID_HANDLE_VALUE;

    if (message == NULL)
        return (0);

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if ((hConsole == NULL) || (hConsole == INVALID_HANDLE_VALUE))
        return (WSVC_CONSOLE_ERROR_CANNOT_FIND_CONSOLE);

    result = wsvc_write_to_console(hConsole, message);

    return (result);
}

int wsvc_write_to_stderr(LPCTSTR const message)
{
    int result = WSVC_CONSOLE_ERROR;
    HANDLE hConsole = INVALID_HANDLE_VALUE;

    if (message == NULL)
        return (0);

    hConsole = GetStdHandle(STD_ERROR_HANDLE);

    if ((hConsole == NULL) || (hConsole == INVALID_HANDLE_VALUE))
        return (WSVC_CONSOLE_ERROR_CANNOT_FIND_CONSOLE);

    result = wsvc_write_to_console(hConsole, message);

    return (result);
}
