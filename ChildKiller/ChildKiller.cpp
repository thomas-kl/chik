// License: GPL Version 3.0. See License.txt
// Author: Thomas Klambauer
// Email: Thomas AT Klambauer.info

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT _WIN32_WINNT_WIN7

#include "WinSDKVer.h"

#include "Windows.h"
#include <string>
#include <iostream>

using namespace std;

void printLastError() {
	wcerr << L"Error: GetLastError= " << GetLastError() << endl;
}

int main(int argc, char* argv[]) {

	if( argc < 2 ) {
		wcout << "ChildKiller Version 1" << endl;
		wcout << " Author: Thomas AT Klambauer.info" << endl;
		wcout << " License: GPL Version 3" << endl;
		wcout << endl;
		wcout << L"ChildKiller [Command] [Arg1] [Arg2] ..." << endl;
		wcout << L"  Windows tool, that executes the given command and arguments via the system" << endl;
		wcout << L"  function, while ensuring that killing the original ChildKiller process will" << endl;
		wcout << L"  also kill all child processes spawned by command." << endl;
		return 0;
	}

	HANDLE jobHandle = CreateJobObject(NULL, NULL);
	if( jobHandle == NULL ) {
		wcerr << L"CreateJobObject failed" << endl;
		printLastError();
		return 1;
	}

	HANDLE thisProcess = GetCurrentProcess();

	if( thisProcess == NULL ) {
		wcerr << L"GetCurrentProcess failed" << endl;
		printLastError();
		return 1;
	}

	JOBOBJECT_BASIC_LIMIT_INFORMATION info = {0};
    info.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION extendedInfo;
    extendedInfo.BasicLimitInformation = info;

    int length = sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION);

    if (!SetInformationJobObject(jobHandle, JobObjectExtendedLimitInformation, &extendedInfo, length)) {
		wcerr << L"SetInformationJobObject failed" << endl;
		printLastError();
		return 1;
	}

	if( AssignProcessToJobObject(jobHandle, thisProcess) == 0 ) {
		wcerr << L"AssignProcessToJobObject failed" << endl;
		printLastError();
		return 1;
	}

	if( argc > 1 ) {
		string command;
		// System also only uses cmd /c to execute the given string.
		// Add another cmd with /S to ensure that quote-processing behavior
		// as specified by cmd doc. That behavior is to strip the first quote
		// character and the last one in the string and leave the rest.
		// For that, a leading quote is added here... and one at the end.
		command.append("cmd /S /C \"");
		// Ignore argv[0] (contains this executable's path).
		for( int i = 1; i < argc; i++) {
			// Quote all the params to forward them exactly as we received them.
			command.append("\"");
			command.append(argv[i]);
			command.append("\" ");
		}
		// This quote is also removed by cmd.
		command.append("\"");
		return system(command.c_str());
	}

	return 0;
}

