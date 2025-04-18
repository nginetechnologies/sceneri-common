#pragma once

#if PLATFORM_WINDOWS

#include "Windows.h"
#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <iostream>

namespace ngine::Platform
{
	// Handles redirection of standard output to a chosen console.
	// Most code was taken from here:
	// https://stackoverflow.com/questions/311955/redirecting-cout-to-a-console-in-windows/25927081#25927081
	bool RedirectConsoleIO()
	{
		// Re-initialize the C runtime "FILE" handles with clean handles bound to "nul". We do this because it has been
		// observed that the file number of our standard handle file objects can be assigned internally to a value of -2
		// when not bound to a valid target, which represents some kind of unknown internal invalid state. In this state our
		// call to "_dup2" fails, as it specifically tests to ensure that the target file number isn't equal to this value
		// before allowing the operation to continue. We can resolve this issue by first "re-opening" the target files to
		// use the "nul" device, which will place them into a valid state, after which we can redirect them to our target
		// using the "_dup2" function.
		FILE* dummyFile;
		freopen_s(&dummyFile, "nul", "r", stdin);
		freopen_s(&dummyFile, "nul", "w", stdout);
		freopen_s(&dummyFile, "nul", "w", stderr);

		// Redirect unbuffered stdin from the current standard input handle
		HANDLE stdHandle = GetStdHandle(STD_INPUT_HANDLE);
		if (stdHandle != INVALID_HANDLE_VALUE)
		{
			int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
			if (fileDescriptor != -1)
			{
				FILE* file = _fdopen(fileDescriptor, "r");
				if (file != NULL)
				{
					int dup2Result = _dup2(_fileno(file), _fileno(stdin));
					if (dup2Result == 0)
					{
						setvbuf(stdin, NULL, _IONBF, 0);
					}
				}
			}
		}

		// Redirect unbuffered stdout to the current standard output handle
		stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		if (stdHandle != INVALID_HANDLE_VALUE)
		{
			int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
			if (fileDescriptor != -1)
			{
				FILE* file = _fdopen(fileDescriptor, "w");
				if (file != NULL)
				{
					int dup2Result = _dup2(_fileno(file), _fileno(stdout));
					if (dup2Result == 0)
					{
						setvbuf(stdout, NULL, _IONBF, 0);
					}
				}
			}
		}

		stdHandle = GetStdHandle(STD_ERROR_HANDLE);
		if (stdHandle != INVALID_HANDLE_VALUE)
		{
			int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
			if (fileDescriptor != -1)
			{
				FILE* file = _fdopen(fileDescriptor, "w");
				if (file != NULL)
				{
					int dup2Result = _dup2(_fileno(file), _fileno(stderr));
					if (dup2Result == 0)
					{
						setvbuf(stderr, NULL, _IONBF, 0);
					}
				}
			}
		}

		// Clear the error state for each of the C++ standard stream objects. We need to do this, as attempts to access the
		// standard streams before they refer to a valid target will cause the iostream objects to enter an error state. In
		// versions of Visual Studio after 2005, this seems to always occur during startup regardless of whether anything
		// has been read from or written to the targets or not.
		std::wcin.clear();
		std::cin.clear();
		std::wcout.clear();
		std::cout.clear();
		std::wcerr.clear();
		std::cerr.clear();

		return true;
	}

	bool ReleaseConsole()
	{
		bool result = true;
		FILE* pFile;

		// Just to be safe, redirect standard IO to NUL before releasing.

		// Redirect STDIN to NUL
		if (freopen_s(&pFile, "NUL:", "r", stdin) != 0)
			result = false;
		else
			setvbuf(stdin, NULL, _IONBF, 0);

		// Redirect STDOUT to NUL
		if (freopen_s(&pFile, "NUL:", "w", stdout) != 0)
			result = false;
		else
			setvbuf(stdout, NULL, _IONBF, 0);

		// Redirect STDERR to NUL
		if (freopen_s(&pFile, "NUL:", "w", stderr) != 0)
			result = false;
		else
			setvbuf(stderr, NULL, _IONBF, 0);

		// Detach from console
		if (!FreeConsole())
			result = false;

		return result;
	}

	void AdjustConsoleBuffer(int16_t minLength)
	{
		// Set the screen buffer to be big enough to scroll some text
		CONSOLE_SCREEN_BUFFER_INFO conInfo;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &conInfo);
		if (conInfo.dwSize.Y < minLength)
			conInfo.dwSize.Y = minLength;
		SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), conInfo.dwSize);
	}

	bool CreateNewConsole(int16_t minLength)
	{
		ReleaseConsole();

		if (AllocConsole())
		{
			AdjustConsoleBuffer(minLength);
			return RedirectConsoleIO();
		}

		return false;
	}

	bool AttachParentConsole(int16_t minLength)
	{
		ReleaseConsole();
		if (AttachConsole(ATTACH_PARENT_PROCESS))
		{
			AdjustConsoleBuffer(minLength);
			return RedirectConsoleIO();
		}

		return false;
	}
}

#endif
