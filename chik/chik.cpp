// License: GPL Version 3.0. See License.txt
// Author: Thomas Klambauer
// Email: Thomas AT Klambauer.info

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

int main(int argc, char* argv[]) {

	// 1st arg contains this executable's path. 2nd is first parameter
	if (argc < 2) {
		wcout << L"chik Version 1.1" << endl;
		wcout << L" Author: Thomas AT Klambauer.info" << endl;
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
		string command;
		int i = 0;
		bool restart_child = false;

		/* restart child process */
		if (strcmp(argv[1], "-r") == 0) {
			i = 2;
			restart_child = true;
		} else {
			i = 1;
			restart_child = false;
		}

		// Ignore argv[0]
		for (; i < argc; i++) {
			string arg(argv[i]);

			// If parameter contains a space, quote the params
			//  to forward them exactly as we received them.
			if (arg.find(" ") != string::npos) {
				arg = "\"" + arg + "\"";
			}
			command.append(arg);
			// Add a space to separate the final args, but no trailing space.
			if (i != argc - 1) {
				command.append(" ");
			}
		}

		// This will cause windows to spawn a "cmd.exe /c" process
		if (restart_child) {
			while (true) {
				system(command.c_str());
			}
		}
		else {
			return system(command.c_str());
		}
		return 0;
	}

	return 0;
}

