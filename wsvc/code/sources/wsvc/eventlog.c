// Copyright (c) Vincent Ycasas
// SPDX-License-Identifier: MIT

#include <wsvc/eventlog.h>

#include <wsvc/wsvc.h>

int wsvc_write_event_log(WORD eventLogType, TCHAR const* eventLogMessage)
{
    BOOL reportEventOk = FALSE;
    HANDLE hEventSource = INVALID_HANDLE_VALUE;

    if (eventLogMessage == NULL)
        return (WSVC_WRITE_EVENT_LOG_ERROR_EMPTY_MESSAGE);

    hEventSource = RegisterEventSource(NULL, WSVC_APPLICATION_NAME);

    if ((hEventSource == INVALID_HANDLE_VALUE) || (hEventSource == NULL))
        return (WSVC_WRITE_EVENT_LOG_ERROR);


    reportEventOk = ReportEvent(
        hEventSource,
        eventLogType,
        0,
        0,
        NULL,
        1,
        0,
        &eventLogMessage,
        NULL);

    DeregisterEventSource(hEventSource);

    if (reportEventOk != TRUE)
        return (WSVC_WRITE_EVENT_LOG_ERROR);

    return (WSVC_WRITE_EVENT_LOG_OK);
}
