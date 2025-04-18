#pragma once

#if PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define UNICODE

#include "UndefineWindowsMacros.h"

#include <Windows.h>
#include <ShlObj.h>
#include <shellapi.h>

#include "UndefineWindowsMacros.h"
#endif
