// Copyright (c) Vincent Ycasas
// SPDX-License-Identifier: MIT

#pragma once

#include <Windows.h>

#if defined(__cplusplus)
extern "C"
{
#endif // defined(__cplusplus)

    static int const WSVC_CONSOLE_ERROR = -1;
    static int const WSVC_CONSOLE_ERROR_CANNOT_FIND_CONSOLE = -2;
    static int const WSVC_CONSOLE_ERROR_FAILED_TO_GET_MESSAGE_LENGTH = -3;

    int wsvc_write_to_stdout(LPCTSTR const message);

    int wsvc_write_to_stderr(LPCTSTR const message);

#if defined(__cplusplus)
}
// extern "C"
#endif // defined(__cplusplus)
