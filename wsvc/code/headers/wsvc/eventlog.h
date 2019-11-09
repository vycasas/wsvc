// Copyright (c) Vincent Ycasas
// SPDX-License-Identifier: MIT

#pragma once

#include <Windows.h>

#if defined(__cplusplus)
extern "C"
{
#endif // defined(__cplusplus)

    static int const WSVC_WRITE_EVENT_LOG_OK = 0;

    static int const WSVC_WRITE_EVENT_LOG_ERROR = -1;

    static int const WSVC_WRITE_EVENT_LOG_ERROR_INVALID_TYPE = -2;

    static int const WSVC_WRITE_EVENT_LOG_ERROR_EMPTY_MESSAGE = -1;

    int wsvc_write_event_log(WORD eventLogType, TCHAR const* eventLogMessage);

#if defined(__cplusplus)
}
// extern "C"
#endif // defined(__cplusplus)
