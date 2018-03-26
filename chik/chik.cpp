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

int wmain(int argc, wchar_t* argv[]) {

	// 1st arg contains this executable's path. 2nd is first parameter
	if (argc < 2) {
		wcout << L"chik Version 1.2" << endl;
		wcout << L" License: GPL Version 3" << endl;
		wcout << endl;
		wcout << L"chik [-r] <Command> [Arg1] [Arg2] ..." << endl;
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
		int i = 0;
		bool restart_child = false;

		/* restart child process */
		if (wcscmp(argv[1], L"-r") == 0) {
			i = 2;
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
			while (true) {
				_wsystem(command.c_str());
			}
		}
		else {
			return _wsystem(command.c_str());
		}
		return 0;
	}

	return 0;
}

