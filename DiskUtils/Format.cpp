#include "stdafx.h"

int formatDrive(std::string drive) {
	printf("Drive %s will be formated. Type Y to continue\n", drive.c_str());
	std::string str;
	std::getline(std::cin, str);
	if (str.length() == 0 || str[0] != 'Y')
		return 0;
	sizeGlobal = GetDriveSize((char*)drive.c_str());
	if (!sizeGlobal) {
		printf("Invalid drive %s\n", drive.c_str());
		return 0;
	}
	HANDLE hDrive = CreateFileA(drive.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hDrive == INVALID_HANDLE_VALUE) {
		DWORD error = GetLastError();
		printf("Failed to open drive %s with write access\n", drive.c_str());
		printError();
		return 0;
	}
	DWORD status;

	// lock volume
	if (!DeviceIoControl(hDrive, FSCTL_LOCK_VOLUME,
		NULL, 0, NULL, 0, &status, NULL))
	{
		printf("Failed to lock drive\n");
		printError();
		CloseHandle(hDrive);
		return 0;
		// error handling; not sure if retrying is useful
	}
	int ret = 1;
	char buffer[4096];
	memset(buffer, 0, sizeof(buffer));
	writtenGlobal = 0;
	DWORD temp;
	printf("Formating drive...\n");
	HANDLE infoThread = CreateThread(0, 0, statusPrint, 0, 0, 0);

	while (writtenGlobal < sizeGlobal) {
		ULONGLONG toWrite = min(sizeGlobal - writtenGlobal, sizeof(buffer));
		if (!WriteFile(hDrive, buffer, toWrite, &temp, 0)) {
			printf("Write to drive failed\n");
			printError();
			if (!DeviceIoControl(hDrive, FSCTL_UNLOCK_VOLUME,
				NULL, 0, NULL, 0, &status, NULL))
			{
				printf("Failed to unlock drive\n");
				printError();
			}
			ret = 0;
			break;
		}
		writtenGlobal += temp;
	}
	writtenGlobal = sizeGlobal;
	if (!DeviceIoControl(hDrive, FSCTL_UNLOCK_VOLUME,
		NULL, 0, NULL, 0, &status, NULL))
	{
		printf("Failed to unlock drive\n");
	}
	CloseHandle(hDrive);
	WaitForSingleObject(infoThread, INFINITE);
	return ret;
}