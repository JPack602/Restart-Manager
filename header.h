#ifndef __HEADER__
#define __HEADER__

#pragma comment(lib, "Rstrtmgr.lib")

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <locale.h>
#include <stddef.h>

#include <Windows.h>
#include <restartmanager.h>
#include <Windows.h>

extern int ForceShutdown(WCHAR* dest);

#endif