// Copyright (c) Vincent Ycasas
// SPDX-License-Identifier: MIT

#include <wsvc/service.h>

#include <wsvc/console.h>
#include <wsvc/eventlog.h>
#include <wsvc/wsvc.h>

#include <stdbool.h>

#include <Windows.h>

static DWORD const WSVC_SERVICE_EXIT_OK = 0;
static DWORD const WSVC_SERVICE_EXIT_ERROR = (DWORD) -1;
static DWORD const WSVC_SERVICE_EXIT_ERROR_STATUS_PROBLEM = (DWORD) -2;

struct wsvc_service_status_
{
    SERVICE_STATUS_HANDLE status_handle;
    SERVICE_STATUS status;
    DWORD checkpoint;
};

typedef struct wsvc_service_status_ wsvc_service_status;
typedef wsvc_service_status* wsvc_service_status_ptr;

static VOID WINAPI wsvc_service_main(DWORD argc, LPTSTR* pArgs);

static BOOL WINAPI wsvc_service_set_status(wsvc_service_status_ptr pServiceStatus);

static DWORD WINAPI wsvc_service_control_handler(
    DWORD control,
    DWORD eventType,
    LPVOID pEventData,
    LPVOID pContext);

static DWORD WINAPI wsvc_service_start(wsvc_service_status_ptr pServiceStatus);
static DWORD WINAPI wsvc_service_stop(wsvc_service_status_ptr pServiceStatus);

static VOID WINAPI wsvc_service_main(DWORD argc, LPTSTR* pArgs)
{
    wsvc_service_status_ptr pServiceStatus = NULL;
    BOOL setServiceStatusOk = FALSE;
    LPSERVICE_STATUS pStatus = NULL;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(pArgs);

    pServiceStatus = (wsvc_service_status_ptr) malloc(sizeof(wsvc_service_status));
    if (pServiceStatus == NULL) {
        wsvc_write_to_stderr(TEXT("[WSVC RUN] ERROR: Failed to create wsvc_service_status_ptr.\n"));
        return;
    }

    ZeroMemory(pServiceStatus, sizeof(wsvc_service_status));

    pServiceStatus->status_handle = RegisterServiceCtrlHandlerEx(
        WSVC_APPLICATION_NAME,
        wsvc_service_control_handler,
        (LPVOID) pServiceStatus);

    if (pServiceStatus->status_handle == NULL) {
        wsvc_write_to_stderr(TEXT("[WSVC RUN] ERROR: Failed to register service control handler.\n"));
        return;
    }

    pStatus = &(pServiceStatus->status);

    ZeroMemory(pStatus, sizeof(SERVICE_STATUS));
    pStatus->dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    pStatus->dwCurrentState = SERVICE_START_PENDING;
    pStatus->dwControlsAccepted = SERVICE_ACCEPT_STOP;
    
    pServiceStatus->checkpoint = 1;

    setServiceStatusOk = wsvc_service_set_status(pServiceStatus);

    wsvc_service_start(pServiceStatus);

    return;
}

static BOOL WINAPI wsvc_service_set_status(wsvc_service_status_ptr pServiceStatus)
{
    BOOL setServiceStatusOk = FALSE;
    SERVICE_STATUS_HANDLE hStatus = NULL;
    LPSERVICE_STATUS pStatus = NULL;

    if (pServiceStatus == NULL) {
        wsvc_write_to_stderr(TEXT("[WSVC RUN] ERROR: wsvc_service_status_ptr was NULL when setting service status.\n"));
        return (FALSE);
    }

    hStatus = pServiceStatus->status_handle;

    pStatus = &(pServiceStatus->status);

    if ((pStatus->dwCurrentState == SERVICE_RUNNING) || (pStatus->dwCurrentState == SERVICE_STOPPED)) {
        pStatus->dwCheckPoint = 0;
    }
    else {
        pStatus->dwCheckPoint = (pServiceStatus->checkpoint)++;
    }

    setServiceStatusOk = SetServiceStatus(
        hStatus,
        pStatus);

    return (setServiceStatusOk);
}

static DWORD WINAPI wsvc_service_control_handler(
    DWORD control,
    DWORD eventType,
    LPVOID pEventData,
    LPVOID pContext)
{
    DWORD result = WSVC_SERVICE_EXIT_ERROR;
    wsvc_service_status_ptr pServiceStatus = NULL;

    UNREFERENCED_PARAMETER(eventType);
    UNREFERENCED_PARAMETER(pEventData);

    pServiceStatus = (wsvc_service_status_ptr) pContext;
    if (pServiceStatus == NULL) {
        return (WSVC_SERVICE_EXIT_ERROR_STATUS_PROBLEM);
    }

    switch (control) {
    case SERVICE_CONTROL_STOP:
        result = wsvc_service_stop(pServiceStatus);
        break;
    default:
        break;
    }

    return (result);
}

static DWORD WINAPI wsvc_service_start(wsvc_service_status_ptr pServiceStatus)
{
    if (pServiceStatus == NULL) {
        wsvc_write_to_stderr(TEXT("[WSVC RUN] ERROR: Invalid wsvc_service_status_ptr while starting the service.\n"));
        return (WSVC_SERVICE_EXIT_ERROR_STATUS_PROBLEM);
    }
    
    pServiceStatus->status.dwCurrentState = SERVICE_START_PENDING;
    wsvc_service_set_status(pServiceStatus);

    wsvc_write_event_log(EVENTLOG_SUCCESS, TEXT("[WSVC] Service is running."));

    pServiceStatus->status.dwCurrentState = SERVICE_RUNNING;
    wsvc_service_set_status(pServiceStatus);

    return (WSVC_SERVICE_EXIT_OK);
}

static DWORD WINAPI wsvc_service_stop(wsvc_service_status_ptr pServiceStatus)
{
    if (pServiceStatus == NULL) {
        wsvc_write_to_stderr(TEXT("[WSVC RUN] ERROR: Invalid wsvc_service_status_ptr while stopping the service.\n"));
        return (WSVC_SERVICE_EXIT_ERROR_STATUS_PROBLEM);
    }

    pServiceStatus->status.dwCurrentState = SERVICE_STOP_PENDING;
    wsvc_service_set_status(pServiceStatus);

    wsvc_write_event_log(EVENTLOG_SUCCESS, TEXT("[WSVC] Service is stopping."));

    pServiceStatus->status.dwCurrentState = SERVICE_STOPPED;
    wsvc_service_set_status(pServiceStatus);

    return (WSVC_SERVICE_EXIT_OK);
}

int wsvc_service_install()
{
    static LPCTSTR const WSVC_SERVICE_USER_ACCOUNT = TEXT("NT AUTHORITY\\LocalService");

    int result = WSVC_SERVICE_INSTALL_ERROR;
    TCHAR moduleFileName[MAX_PATH];
    DWORD getModuleFileNameResult = 0;
    SC_HANDLE scmHandle = NULL;
    SC_HANDLE serviceHandle = NULL;

    wsvc_write_to_stdout(TEXT("[WSVC INSTALL] Starting installation process.\n"));

    ZeroMemory(moduleFileName, sizeof(TCHAR) * _countof(moduleFileName));

    getModuleFileNameResult = GetModuleFileName(NULL, moduleFileName, MAX_PATH);

    if (getModuleFileNameResult == 0) {
        wsvc_write_to_stderr(TEXT("[WSVC INSTALL] ERROR: Failed to get module filename during installation.\n"));
        return (WSVC_SERVICE_INSTALL_ERROR_CANNOT_FIND_MODULE);
    }

    do {
        scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);

        if (scmHandle == NULL) {
            wsvc_write_to_stderr(TEXT("[WSVC INSTALL] ERROR: Failed to get the handle to the SCM.\n"));
            result = WSVC_SERVICE_INSTALL_ERROR_FAILED_TO_GET_SCM_HANDLE;
            break;
        }

        serviceHandle = CreateService(
            scmHandle,
            WSVC_APPLICATION_NAME,
            WSVC_APPLICATION_NAME,
            SERVICE_QUERY_STATUS,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_DEMAND_START,
            SERVICE_ERROR_NORMAL,
            moduleFileName,
            NULL,
            NULL,
            NULL,
            WSVC_SERVICE_USER_ACCOUNT,
            NULL);

        if (serviceHandle == NULL) {
            wsvc_write_to_stderr(TEXT("[WSVC INSTALL] ERROR: Failed to create service.\n"));
            result = WSVC_SERVICE_INSTALL_ERROR_FAILED_TO_CREATE_SERVICE;
            break;
        }

        wsvc_write_to_stdout(TEXT("[WSVC INSTALL] Service successfully installed.\n"));
        result = WSVC_SERVICE_INSTALL_OK;
    }
    while (false);

    if (serviceHandle != NULL)
        CloseServiceHandle(serviceHandle);

    if (scmHandle != NULL)
        CloseServiceHandle(scmHandle);

    return (result);
}

int wsvc_service_uninstall()
{
    int result = WSVC_SERVICE_UNINSTALL_ERROR;
    SC_HANDLE scmHandle = NULL;
    SC_HANDLE serviceHandle = NULL;
    BOOL queryStatusOk = FALSE;
    SERVICE_STATUS serviceStatus;
    BOOL deleteServiceOk = FALSE;

    ZeroMemory(&serviceStatus, sizeof(SERVICE_STATUS));

    wsvc_write_to_stdout(TEXT("[WSVC UNINSTALL] Starting uninstallation process.\n"));

    do {
        scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

        if (scmHandle == NULL) {
            wsvc_write_to_stderr(TEXT("[WSVC UNINSTALL] ERROR: Failed to get the handle to the SCM.\n"));
            result = WSVC_SERVICE_UNINSTALL_ERROR_FAILED_TO_GET_SCM_HANDLE;
            break;
        }

        serviceHandle = OpenService(scmHandle, WSVC_APPLICATION_NAME, SERVICE_QUERY_STATUS | DELETE);

        if (serviceHandle == NULL) {
            wsvc_write_to_stderr(TEXT("[WSVC UNINSTALL] ERROR: Failed to open service.\n"));
            result = WSVC_SERVICE_UNINSTALL_ERROR_FAILED_TO_OPEN_SERVICE;
            break;
        }

        queryStatusOk = QueryServiceStatus(serviceHandle, &serviceStatus);

        if (queryStatusOk != TRUE) {
            wsvc_write_to_stderr(TEXT("[WSVC UNINSTALL] ERROR: Failed to query service status.\n"));
            result = WSVC_SERVICE_UNINSTALL_ERROR_FAILED_TO_QUERY_SERVICE_STATUS;
            break;
        }

        if (serviceStatus.dwCurrentState != SERVICE_STOPPED) {
            wsvc_write_to_stderr(TEXT("[WSVC UNINSTALL] ERROR: Service is not stopped. Please stop the service first before uninstalling.\n"));
            result = WSVC_SERVICE_UNINSTALL_ERROR_SERVICE_NOT_STOPPED;
            break;
        }

        deleteServiceOk = DeleteService(serviceHandle);

        if (deleteServiceOk != TRUE) {
            wsvc_write_to_stderr(TEXT("[WSVC UNINSTALL] ERROR: Failed to delete service.\n"));
            result = WSVC_SERVICE_UNINSTALL_ERROR;
            break;
        }

        wsvc_write_to_stdout(TEXT("[WSVC UNINSTALL] Service successfully uninstalled.\n"));
        result = WSVC_SERVICE_UNINSTALL_OK;
    }
    while (false);

    if (serviceHandle != NULL)
        CloseServiceHandle(serviceHandle);

    if (scmHandle != NULL)
        CloseServiceHandle(scmHandle);

    return (result);
}

int wsvc_service_run()
{
    BOOL startOk = FALSE;
    SERVICE_TABLE_ENTRY serviceTableEntries[] = {
        { NULL, NULL },
        { NULL, NULL }
    };

    wsvc_write_to_stdout(TEXT("[WSVC RUN] Registering service table entries.\n"));

    serviceTableEntries[0].lpServiceName = (LPTSTR) WSVC_APPLICATION_NAME;
    serviceTableEntries[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION) &wsvc_service_main;
    
    startOk = StartServiceCtrlDispatcher(serviceTableEntries);

    if (startOk != TRUE) {
        wsvc_write_to_stderr(TEXT("[WSVC RUN] Error: Failed to start service control dispatcher.\n"));
        return (WSVC_SERVICE_RUN_ERROR);
    }

    wsvc_write_to_stdout(TEXT("[WSVC RUN] Service control dispatcher has started.\n"));
    return (WSVC_SERVICE_RUN_OK);
}
