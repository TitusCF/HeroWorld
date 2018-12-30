/*
 * Crossfire -- cooperative multi-player graphical RPG and adventure game
 *
 * Copyright (c) 1999-2014 Mark Wedel and the Crossfire Development Team
 * Copyright (c) 1992 Frank Tore Johansen
 *
 * Crossfire is free software and comes with ABSOLUTELY NO WARRANTY. You are
 * welcome to redistribute it under certain conditions. For details, please
 * see COPYING and LICENSE.
 *
 * The authors can be reached via e-mail at <crossfire@metalforge.org>.
 */

/**
 * @file
 * Windows-related compatibility functions.
 *
 * This file should probably not be used apart under Windows.
 */

#include <global.h>

#include <stdarg.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <mmsystem.h>

/** Timezone structure, for gettimeofday(). */
struct timezone {
    int tz_minuteswest; /**< Timezone. */
    int tz_dsttime; /**< Current time. */
};

/**
 * Gets the time of the day.
 *
 * @param[out] time_Info
 * will receive the time of the day.
 * @param[out] timezone_Info
 * will receive the timezone info.
 * @return
 * 0.
 */
int gettimeofday(struct timeval *time_Info, struct timezone *timezone_Info) {
    /* Get the time, if they want it */
    if (time_Info != NULL) {
        time_Info->tv_sec = time(NULL);
        time_Info->tv_usec = timeGetTime()*1000;
    }
    /* Get the timezone, if they want it */
    if (timezone_Info != NULL) {
        _tzset();
        timezone_Info->tz_minuteswest = _timezone;
        timezone_Info->tz_dsttime = _daylight;
    }
    /* And return */
    return 0;
}

/**
 * Opens a directory for reading. The handle should be disposed through closedir().
 *
 * @param dir
 * directory path.
 * @return
 * directory handle, NULL if failure.
 */
DIR *opendir(const char *dir) {
    DIR *dp;
    char *filespec;
    long handle;
    int index;

    filespec = malloc(strlen(dir)+2+1);
    strcpy(filespec, dir);
    index = strlen(filespec)-1;
    if (index >= 0 && (filespec[index] == '/' || filespec[index] == '\\'))
        filespec[index] = '\0';
    strcat(filespec, "/*");

    dp = (DIR *)malloc(sizeof(DIR));
    dp->offset = 0;
    dp->finished = 0;
    dp->dir = strdup(dir);

    if ((handle = _findfirst(filespec, &(dp->fileinfo))) < 0) {
        free(filespec);
        free(dp);
        return NULL;
    }
    dp->handle = handle;
    free(filespec);

    return dp;
}

/**
 * Returns the next file/directory for specified directory handle, obtained through
 * a call to opendir().
 *
 * @param dp
 * handle.
 * @return
 * next file/directory, NULL if end reached.
 */
struct dirent *readdir(DIR *dp) {
    if (!dp || dp->finished)
        return NULL;

    if (dp->offset != 0) {
        if (_findnext(dp->handle, &(dp->fileinfo)) < 0) {
            dp->finished = 1;
            if (ENOENT == errno)
                /* Clear error set to mean no more files else that breaks things */
                errno = 0;
            return NULL;
        }
    }
    dp->offset++;

    strncpy(dp->dent.d_name, dp->fileinfo.name, _MAX_FNAME);
    dp->dent.d_name[_MAX_FNAME] = '\0';
    dp->dent.d_ino = 1;
    /* reclen is used as meaning the length of the whole record */
    dp->dent.d_reclen = strlen(dp->dent.d_name)+sizeof(char)+sizeof(dp->dent.d_ino)+sizeof(dp->dent.d_reclen)+sizeof(dp->dent.d_off);
    dp->dent.d_off = dp->offset;

    return &(dp->dent);
}

/**
 * Dispose of a directory handle.
 *
 * @param dp
 * handle to free. Will become invalid.
 * @return
 * 0.
 */
int closedir(DIR *dp) {
    if (!dp)
        return 0;
    _findclose(dp->handle);
    free(dp->dir);
    free(dp);

    return 0;
}

/**
 * Restart a directory listing from the beginning.
 *
 * @param dir_Info
 * handle to rewing.
 */
void rewinddir(DIR *dir_Info) {
    /* Re-set to the beginning */
    char *filespec;
    long handle;
    int index;

    dir_Info->handle = 0;
    dir_Info->offset = 0;
    dir_Info->finished = 0;

    filespec = malloc(strlen(dir_Info->dir)+2+1);
    strcpy(filespec, dir_Info->dir);
    index = strlen(filespec)-1;
    if (index >= 0 && (filespec[index] == '/' || filespec[index] == '\\'))
        filespec[index] = '\0';
    strcat(filespec, "/*");

    if ((handle = _findfirst(filespec, &(dir_Info->fileinfo))) < 0) {
        if (errno == ENOENT) {
            dir_Info->finished = 1;
        }
    }
    dir_Info->handle = handle;
    free(filespec);
}

/* Service-related stuff

  Those functions are called while init is still being done, so no logging available.

  Not useful for plugins, though.

 */

/** Will be set to FALSE when the server should stop running because the service is turned off. */
int bRunning;

#ifndef PYTHON_PLUGIN_EXPORTS

/** Status when the server is started as a service. */
SERVICE_STATUS m_ServiceStatus;
/** Handle to the service the server is started as. */
SERVICE_STATUS_HANDLE m_ServiceStatusHandle;
/** Internal name of the service. */
#define SERVICE_NAME        "Crossfire"
/** Name that will appear in the service list. */
#define SERVICE_DISPLAY     "Crossfire server"
/** Description of the service. */
#define SERVICE_DESCRIPTION "Crossfire is a multiplayer online RPG game."

#include <winsvc.h>

/**
 * Registers the server to the service manager.
 * @sa service_unregister().
 */
void service_register(void) {
    char strDir[1024];
    HANDLE schSCManager, schService;
    char *strDescription = SERVICE_DESCRIPTION;

    GetModuleFileName(NULL, strDir, 1024);
    strcat(strDir, " -srv");

    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (schSCManager == NULL) {
        printf("openscmanager failed");
        exit(1);
    }

    schService = CreateService(schSCManager,
        SERVICE_NAME,
        SERVICE_DISPLAY,           /* service name to display */
        SERVICE_ALL_ACCESS,        /* desired access */
        SERVICE_WIN32_OWN_PROCESS, /* service type */
        SERVICE_DEMAND_START,      /* start type */
        SERVICE_ERROR_NORMAL,      /* error control type */
        strDir,                    /* service's binary */
        NULL,                      /* no load ordering group */
        NULL,                      /* no tag identifier */
        NULL,                      /* no dependencies */
        NULL,                      /* LocalSystem account */
        NULL);                     /* no password */

    if (schService == NULL) {
        printf("createservice failed");
        exit(1);
    }

    ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &strDescription);

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    exit(0);
}

/**
 * Removes the Crossfire service from the service manager.
 * @sa service_register().
 */
void service_unregister(void) {
    HANDLE schSCManager;
    SC_HANDLE hService;

    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (schSCManager == NULL) {
        printf("open failed");
        exit(1);
    }

    hService = OpenService(schSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);

    if (hService == NULL) {
        printf("openservice failed");
        exit(1);
    }

    if (DeleteService(hService) == 0) {
        printf("Delete failed");
        exit(1);
    }

    if (CloseServiceHandle(hService) == 0) {
        printf("close failed");
        exit(1);
    }

    if (!CloseServiceHandle(schSCManager)) {
        printf("close schSCManager failed");
        exit(1);
    }

    exit(0);
}

/**
 * Main service dispatch routine.
 *
 * @param Opcode
 * service operation, like pause/start/stop.
 */
void WINAPI ServiceCtrlHandler(DWORD Opcode) {
    switch (Opcode) {
    case SERVICE_CONTROL_PAUSE:
        m_ServiceStatus.dwCurrentState = SERVICE_PAUSED;
        break;

    case SERVICE_CONTROL_CONTINUE:
        m_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
        break;

    case SERVICE_CONTROL_STOP:
        m_ServiceStatus.dwWin32ExitCode = 0;
        m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        m_ServiceStatus.dwCheckPoint = 0;
        m_ServiceStatus.dwWaitHint = 0;

        SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);

        bRunning = 0;

        LOG(llevInfo, "Service stopped.\n");

        break;

    case SERVICE_CONTROL_INTERROGATE:
        break;
    }
    return;
}

extern int main(int argc, char **argv);

/**
 * Main service entrypoint.
 *
 * @param argc
 * @param argv
 * arguments to the service.
 */
void WINAPI ServiceMain(DWORD argc, LPTSTR *argv) {
    char strDir[1024];
    char *strSlash;

    GetModuleFileName(NULL, strDir, 1024);
    strSlash = strrchr(strDir, '\\');
    if (strSlash)
        *strSlash = '\0';
    chdir(strDir);

    m_ServiceStatus.dwServiceType        = SERVICE_WIN32;
    m_ServiceStatus.dwCurrentState       = SERVICE_START_PENDING;
    m_ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP;
    m_ServiceStatus.dwWin32ExitCode      = 0;
    m_ServiceStatus.dwServiceSpecificExitCode = 0;
    m_ServiceStatus.dwCheckPoint         = 0;
    m_ServiceStatus.dwWaitHint           = 0;

    m_ServiceStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);
    if (m_ServiceStatusHandle == (SERVICE_STATUS_HANDLE)0) {
        return;
    }

    m_ServiceStatus.dwCurrentState       = SERVICE_RUNNING;
    m_ServiceStatus.dwCheckPoint         = 0;
    m_ServiceStatus.dwWaitHint           = 0;
    SetServiceStatus(m_ServiceStatusHandle, &m_ServiceStatus);

    bRunning = 1;
    main(0, NULL);

    return;
}

/**
 * Service entry point.
 */
void service_handle(void) {
    SERVICE_TABLE_ENTRY DispatchTable[] = {
        { SERVICE_NAME, ServiceMain },
        { NULL, NULL }
    };
    StartServiceCtrlDispatcher(DispatchTable);
    exit(0);
}
#endif /* PYTHON_PLUGIN_EXPORTS */
