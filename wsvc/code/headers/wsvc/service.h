// Copyright (c) Vincent Ycasas
// SPDX-License-Identifier: MIT

#pragma once

#if defined(__cplusplus)
extern "C"
{
#endif // defined(__cplusplus)

    static int const WSVC_SERVICE_INSTALL_OK = 0;
    static int const WSVC_SERVICE_INSTALL_ERROR = -1;
    static int const WSVC_SERVICE_INSTALL_ERROR_CANNOT_FIND_MODULE = -2;
    static int const WSVC_SERVICE_INSTALL_ERROR_FAILED_TO_GET_SCM_HANDLE = -3;
    static int const WSVC_SERVICE_INSTALL_ERROR_FAILED_TO_CREATE_SERVICE = -4;

    static int const WSVC_SERVICE_UNINSTALL_OK = 0;
    static int const WSVC_SERVICE_UNINSTALL_ERROR = -1;
    static int const WSVC_SERVICE_UNINSTALL_ERROR_FAILED_TO_GET_SCM_HANDLE = -2;
    static int const WSVC_SERVICE_UNINSTALL_ERROR_FAILED_TO_OPEN_SERVICE = -3;
    static int const WSVC_SERVICE_UNINSTALL_ERROR_FAILED_TO_QUERY_SERVICE_STATUS = -4;
    static int const WSVC_SERVICE_UNINSTALL_ERROR_SERVICE_NOT_STOPPED = -5;

    static int const WSVC_SERVICE_RUN_OK = 0;
    static int const WSVC_SERVICE_RUN_ERROR = -1;

    int wsvc_service_install();

    int wsvc_service_uninstall();
   
    int wsvc_service_run();

#if defined(__cplusplus)
}
// extern "C"
#endif // defined(__cplusplus)
