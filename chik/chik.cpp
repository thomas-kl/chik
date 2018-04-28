// License: GPL Version 3.0. See License.txt

#define WIN32_LEAN_AND_MEAN
// As per docs of CreateJobObject function (available starting with Windows XP)
#define _WIN32_WINNT 0x0500 

#include <stdio.h>

#include "WinSDKVer.h"

#include "Windows.h"
#include <string>
#include <iostream>
#include "WinError.h"

using namespace std;

DWORD printError(wstring functionName, DWORD error) {
	wcerr << L"Error from " << functionName << L": GetLastError= " << error << endl;
	return error;
}

DWORD printLastError(wstring functionName) {
	return printError(functionName, GetLastError());
}

static void clearHMS(SYSTEMTIME *timestamp)
{
    timestamp->wHour = timestamp->wMinute = timestamp->wSecond = timestamp->wMilliseconds = 0;
}

static int getDiffDays(SYSTEMTIME *t1, SYSTEMTIME *t2)
{
    int result = 0;
    FILETIME v_ftime;
    ULARGE_INTEGER v_ui;
    __int64 v_right, v_left, v_res;

    SystemTimeToFileTime(t1, &v_ftime);
    v_ui.LowPart = v_ftime.dwLowDateTime;
    v_ui.HighPart = v_ftime.dwHighDateTime;
    v_right = (((LONGLONG)(v_ui.QuadPart - 116444736000000000)) / 10000000);

    SystemTimeToFileTime(t2, &v_ftime);
    v_ui.LowPart = v_ftime.dwLowDateTime;
    v_ui.HighPart = v_ftime.dwHighDateTime;
    v_left = (((LONGLONG)(v_ui.QuadPart - 116444736000000000)) / 10000000);

    v_res = v_right - v_left;
    v_ui.QuadPart = v_res;

    result = (int) (v_ui.QuadPart / (3600 * 24));
    return result;
}

static void _watchChildProcess(WCHAR *command, int day, int hour)
{
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    SYSTEMTIME tNow, tNowTmp, tStart, tStartTmp;
    DWORD waitResult;
    BOOL bRet;

    while (1) {
        // create child process
        bRet = CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        if (bRet) {
            // markup startup timestamp
            ::GetLocalTime(&tStart);
            tStartTmp = tStart;
            clearHMS(&tStartTmp);

            while (1) {
                waitResult = WaitForSingleObject(pi.hProcess, day > 0 ? 5 * 1000 : INFINITE);
                // child process state changed
                if (waitResult == WAIT_OBJECT_0) {
                    printf("child process=0x%x auto exit.\n", pi.hProcess);
                    break;
                } else if (waitResult == WAIT_TIMEOUT) {
                    ::GetLocalTime(&tNow);

                    tNowTmp = tNow;
                    clearHMS(&tNowTmp);
                    int diffDays = getDiffDays(&tNowTmp, &tStartTmp);

                    // day overload and hour equal
                    if (diffDays >= day && tNow.wHour == hour) {
                        ::TerminateProcess(pi.hProcess, 4);
                        break;
                    }
                } else if (waitResult == WAIT_ABANDONED) {
                    printf("child process=0x%x WaitForSingleObject=%s\n", pi.hProcess, "WAIT_ABANDONED");
                } else if (waitResult == WAIT_FAILED) {
                    printf("child process=0x%x WaitForSingleObject=%s\n", pi.hProcess, "WAIT_FAILED");
                }
            }
        } else
            printf("CreateProcess('%s') failed, GetLastError()=0x%x\n", command, GetLastError());
    }
}

int wmain(int argc, wchar_t* argv[]) {
	// 1st arg contains this executable's path. 2nd is first parameter
	if (argc < 2) {
		wcout << L"chik Version 1.2" << endl;
		wcout << L" License: GPL Version 3" << endl;
		wcout << endl;
		wcout << L"chik [-r] [day] [hour] <Command> [Arg1] [Arg2] ..." << endl;
		wcout << L"  Windows tool, that executes the given command and arguments via the system" << endl;
		wcout << L"  function, while ensuring that killing the original chik process will" << endl;
		wcout << L"  also kill all child processes spawned by command." << endl;
		wcout << endl;
		wcout << L"   [-r] (Optional) If present, will restart the command after its termination" << endl;
		return 0;
	}

	// Create an unnamed job.
	HANDLE jobHandle = CreateJobObject(NULL, NULL);
	if (jobHandle == NULL) {
		return printLastError(L"CreateJobObject");
	}

	HANDLE thisProcess = GetCurrentProcess();

	if (thisProcess == NULL) {
		return printLastError(L"GetCurrentProcess");
	}

	JOBOBJECT_BASIC_LIMIT_INFORMATION info = { 0 };
	info.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

	JOBOBJECT_EXTENDED_LIMIT_INFORMATION extendedInfo;
	extendedInfo.BasicLimitInformation = info;

	int length = sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION);

	if (!SetInformationJobObject(jobHandle, JobObjectExtendedLimitInformation, &extendedInfo, length)) {
		return printLastError(L"SetInformationJobObject");
	}

	if (AssignProcessToJobObject(jobHandle, thisProcess) == 0) {
		return printLastError(L"AssignProcessToJobObject");
	}

	if (argc > 1) {
		wstring command;
		bool restart_child = false;
        int i = 1, day = 1, hour = 3;

		/* restart child process */
		if (wcscmp(argv[1], L"-r") == 0) {
			i = 4;
			day = _wtoi(argv[2]);
			hour = _wtoi(argv[3]);
			restart_child = true;
		} else {
		    i = 1;
			restart_child = false;
		}

		// Ignore argv[0]
		for (; i < argc; i++) {
			wstring arg(argv[i]);

			// If parameter contains a space, quote the params
			//  to forward them exactly as we received them.
			if (arg.find(L" ") != string::npos) {
				arg = L"\"" + arg + L"\"";
			}
			command.append(arg);
			// Add a space to separate the final args, but no trailing space.
			if (i != argc - 1) {
				command.append(L" ");
			}
		}

		// This will cause windows to spawn a "cmd.exe /c" process
		if (restart_child) {
            WCHAR szCommandLine[128] = {0};

            wcscpy_s(szCommandLine, sizeof(szCommandLine), command.c_str());		
		    _watchChildProcess(szCommandLine, day, hour);
		} else {
			return _wsystem(command.c_str());
		}
		return 0;
	}
	return 0;
}
