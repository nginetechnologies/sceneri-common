#pragma once

#if PLATFORM_DESKTOP && !PLATFORM_APPLE_MACCATALYST

#include <Common/IO/Log.h>
#include <Common/IO/Path.h>
#include <Common/Memory/Containers/String.h>
#include <iostream>

#if PLATFORM_WINDOWS
#include <Common/Platform/Windows.h>
#elif PLATFORM_POSIX
#include <unistd.h>
#endif

namespace ngine::Platform
{
	[[nodiscard]] inline bool
	StartProcess([[maybe_unused]] const IO::ConstZeroTerminatedPathView executablePath, const NativeZeroTerminatedStringView commandLine)
	{
#if PLATFORM_WINDOWS
		PROCESS_INFORMATION processInfo;
		memset(&processInfo, 0, sizeof(PROCESS_INFORMATION));

		STARTUPINFOW startupInfo;
		memset(&startupInfo, 0, sizeof(STARTUPINFOW));
		startupInfo.cb = sizeof(startupInfo);

		if (!CreateProcessW(nullptr, commandLine, nullptr, nullptr, false, CREATE_NO_WINDOW, nullptr, nullptr, &startupInfo, &processInfo))
		{
			return false;
		}

		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
		return true;
#elif PLATFORM_POSIX
		return execl(executablePath, commandLine, NULL);
#endif
	}

	[[nodiscard]] inline bool
	StartProcessAndWaitForFinish(const IO::ConstZeroTerminatedPathView executablePath, const NativeZeroTerminatedStringView commandLine)
	{
#if PLATFORM_WINDOWS
		// Most of the output redirection code was taken from here:
		// https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output
		// And only inheriting specific handles from this gem:
		// https://devblogs.microsoft.com/oldnewthing/20111216-00/?p=8873
		PROCESS_INFORMATION processInfo;
		ZeroMemory(&processInfo, sizeof(processInfo));

		SECURITY_ATTRIBUTES saAttr;
		ZeroMemory(&saAttr, sizeof(saAttr));
		saAttr.nLength = sizeof(saAttr);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		HANDLE hChildStd_IN_Wr = NULL;
		HANDLE hChildStd_OUT_Rd = NULL;
		HANDLE hChildStd_IN_Rd = NULL;
		HANDLE hChildStd_OUT_Wr = NULL;

		CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0);
		SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0);
		CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0);
		SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0);

		const DWORD cHandlesToInherit = 2;
		HANDLE rgHandlesToInherit[cHandlesToInherit] = {hChildStd_OUT_Wr, hChildStd_IN_Rd};

		SIZE_T size = 0;
		LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList = NULL;
		InitializeProcThreadAttributeList(NULL, 1, 0, &size);
		lpAttributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(HeapAlloc(GetProcessHeap(), 0, size));
		InitializeProcThreadAttributeList(lpAttributeList, 1, 0, &size);
		UpdateProcThreadAttribute(
			lpAttributeList,
			0,
			PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
			rgHandlesToInherit,
			cHandlesToInherit * sizeof(HANDLE),
			NULL,
			NULL
		);

		STARTUPINFOW startupInfo;
		ZeroMemory(&startupInfo, sizeof(startupInfo));
		startupInfo.cb = sizeof(startupInfo);
		startupInfo.hStdError = hChildStd_OUT_Wr;
		startupInfo.hStdOutput = hChildStd_OUT_Wr;
		startupInfo.hStdInput = hChildStd_IN_Rd;
		startupInfo.dwFlags |= STARTF_USESTDHANDLES;

		STARTUPINFOEXW startupInfoEx;
		ZeroMemory(&startupInfoEx, sizeof(startupInfoEx));
		startupInfoEx.StartupInfo = startupInfo;
		startupInfoEx.StartupInfo.cb = sizeof(startupInfoEx);
		startupInfoEx.lpAttributeList = lpAttributeList;

		BOOL result = CreateProcessW(
			executablePath,
			commandLine,
			NULL,
			NULL,
			TRUE,
			CREATE_NO_WINDOW | EXTENDED_STARTUPINFO_PRESENT,
			NULL,
			NULL,
			&startupInfoEx.StartupInfo,
			&processInfo
		);

		CloseHandle(hChildStd_OUT_Wr);
		CloseHandle(hChildStd_IN_Rd);

		DWORD exitCode = 1;
		if (!result)
		{
			DWORD errorId = GetLastError();
			LPSTR messageBuffer = nullptr;
			FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				errorId,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPSTR)&messageBuffer,
				0,
				NULL
			);
			LogError("CreateProcess failed '{0}'", messageBuffer);
			LocalFree(messageBuffer);
		}
		else
		{
			DWORD dwRead, dwWritten;
			CHAR chBuf[4096];
			HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
			while (ReadFile(hChildStd_OUT_Rd, chBuf, 4096, &dwRead, NULL))
			{
				WriteFile(hParentStdOut, chBuf, dwRead, &dwWritten, NULL);
			}

			GetExitCodeProcess(processInfo.hProcess, &exitCode);
		}

		CloseHandle(hChildStd_IN_Wr);
		CloseHandle(hChildStd_OUT_Rd);

		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);

		DeleteProcThreadAttributeList(lpAttributeList);
		HeapFree(GetProcessHeap(), 0, lpAttributeList);

		return exitCode == 0;

#elif PLATFORM_POSIX
		UNUSED(executablePath);
		return system(commandLine) == 0;
#endif
	}
}

#endif
