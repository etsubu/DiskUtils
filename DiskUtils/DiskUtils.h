#pragma once
#include <windows.h>
BOOL GetDriveGeometry(LPSTR wszPath, DISK_GEOMETRY *pdg);
ULONGLONG GetDriveSize(char *Drive);
void printError();
void formatTimeCount(long double time);
DWORD CALLBACK statusPrint(LPVOID lpParam);

extern unsigned long long writtenGlobal;
extern unsigned long long sizeGlobal;
extern DWORD lastTickGlobal;

#define MEGA_BYTE (1024 * 1024)
#define KILO_BYTE (1024)

#define HOUR (60 * 60)
#define MINUTE (60)

#define PROGRESS_BAR_LENGTH 40