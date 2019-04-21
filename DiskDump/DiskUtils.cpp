// DiskDump.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

unsigned long long writtenGlobal;
unsigned long long sizeGlobal;
DWORD lastTickGlobal;

BOOL GetDriveGeometry(LPSTR wszPath, DISK_GEOMETRY *pdg)
{
	HANDLE hDevice = INVALID_HANDLE_VALUE;  
	BOOL bResult = FALSE;             
	DWORD junk = 0;                  

	hDevice = CreateFileA(wszPath,      
		0,                
		FILE_SHARE_READ | 
		FILE_SHARE_WRITE,
		NULL,      
		OPEN_EXISTING,    
		0,         
		NULL);  

	if (hDevice == INVALID_HANDLE_VALUE)   
	{
		return (FALSE);
	}

	bResult = DeviceIoControl(hDevice,    
		IOCTL_DISK_GET_DRIVE_GEOMETRY,
		NULL, 0,         
		pdg, sizeof(*pdg),  
		&junk,   
		(LPOVERLAPPED)NULL);  

	CloseHandle(hDevice);

	return (bResult);
}

ULONGLONG GetDriveSize(char *Drive)
{
	DISK_GEOMETRY pdg = { 0 }; 
	BOOL bResult = FALSE;      
	ULONGLONG DiskSize = 0;   

	bResult = GetDriveGeometry(Drive, &pdg);

	if (bResult)
	{
		wprintf(L"Drive path      = %ws\n", L"G:\\");
		wprintf(L"Cylinders       = %I64d\n", pdg.Cylinders);
		wprintf(L"Tracks/cylinder = %ld\n", (ULONG)pdg.TracksPerCylinder);
		wprintf(L"Sectors/track   = %ld\n", (ULONG)pdg.SectorsPerTrack);
		wprintf(L"Bytes/sector    = %ld\n", (ULONG)pdg.BytesPerSector);

		DiskSize = pdg.Cylinders.QuadPart * (ULONG)pdg.TracksPerCylinder *
			(ULONG)pdg.SectorsPerTrack * (ULONG)pdg.BytesPerSector;
		wprintf(L"Disk size       = %I64d (Bytes)\n"
			L"                = %.2f (Gb)\n",
			DiskSize, (double)DiskSize / (1024 * 1024 * 1024));
	}
	else
	{
		wprintf(L"GetDriveGeometry failed. Error %ld.\n", GetLastError());
	}

	return DiskSize;
}

void printError() {
	DWORD error = GetLastError();
	printf("Error code: %d\n", error);
	if (error == 5) {
		printf("Access is denied. Try running the program with admin access\n");
	}
	else if (error == 32) {
		printf("The drive was already in use. Try closing programs that might be using it\n");
	}
}
void formatTimeCount(long double time) {
	time /= 1000;
	if (time > 60) {
		DWORD minutes = ((DWORD)time) / 60;
		time = time - minutes * 60;
		printf("%dmin %.2Lfs\n", minutes, time);
	}
	else {
		printf("%.2Lfs\n", time);
	}
}

void formatTimeCount(DWORD time) {
	time /= 1000;
	if (time > 60) {
		DWORD minutes = time / 60;
		time = time % 60;
		printf("Time taken: %dmin %ds\n", minutes, time);
	}
	else {
		printf("Time taken: %ds\n", time);
	}
}

void formatSpeed(DWORD count) {
	if (count >= MEGA_BYTE) {
		DWORD mb = count / MEGA_BYTE;
		count %= MEGA_BYTE;
		if (count >= KILO_BYTE) {
			DWORD kb = count / KILO_BYTE;
			DWORD b = count % KILO_BYTE;
			printf("Speed: %d MB %d KB %d Bytes / second\n", mb, kb, b);
		}
		else {
			DWORD b = count % KILO_BYTE;
			printf("Speed: %d MB %d Bytes / second\n", mb, b);
		}
	}
	else if (count >= KILO_BYTE) {
		DWORD kb = count / KILO_BYTE;
		DWORD b = count % KILO_BYTE;
		printf("Speed: %d KB %d Bytes / second\n", kb, b);
	}
	else {
		DWORD b = count % KILO_BYTE;
		printf("Speed: %d Bytes / second\n", b);
	}
}

void showProgressBar(unsigned long long from, unsigned long long to) {
	unsigned long long chunk = to / PROGRESS_BAR_LENGTH;
	DWORD bars, empty;
	if (chunk) {
		bars=from / chunk;
		empty = PROGRESS_BAR_LENGTH - bars;
	}
	else {
		bars = 0;
		empty = PROGRESS_BAR_LENGTH;
	}
	printf("<");
	while (bars--) {
		printf("=");
	}
	while (empty--) {
		printf(" ");
	}
	printf(">\n");
}

void formatTimeLeft(DWORD seconds) {
	if (seconds > HOUR) {
		DWORD hours = seconds / HOUR;
		seconds %= HOUR;
		if (seconds > MINUTE) {
			DWORD minutes = seconds / MINUTE;
			seconds %= MINUTE;
			printf("Estimated time left: %d Hours %d Minutes %d Seconds\n", hours, minutes, seconds);
		}
		else {
			seconds %= MINUTE;
			printf("Estimated time left: %d Hours %d Seconds\n", hours, seconds);
		}
	}
	else if (seconds > MINUTE) {
		DWORD minutes = seconds / MINUTE;
		seconds %= MINUTE;
		printf("Estimated time left: %d Minutes %d Seconds\n", minutes, seconds);
	}
	else {
		printf("Estimated time left: %d Seconds\n", seconds);
	}
}

DWORD CALLBACK statusPrint(LPVOID lpParam) {
	unsigned long long lastWritten=0;
	DWORD startTime = GetTickCount();
	while (writtenGlobal < sizeGlobal) {
		system("cls");
		long double d = 100 - ((((long double)sizeGlobal - writtenGlobal) / sizeGlobal) * 100);
		unsigned long long speed = writtenGlobal - lastWritten;
		lastWritten = writtenGlobal;
		if (speed > 0) {
			unsigned long long secondsLeft = (sizeGlobal - lastWritten) / speed;
			formatSpeed(speed);
			formatTimeLeft(secondsLeft);
		}
		printf("%.2Lf %%\n", d);
		showProgressBar(writtenGlobal, sizeGlobal);
		Sleep(1000);
	}
	system("cls");
	long double d = 100 - ((((long double)sizeGlobal - writtenGlobal) / sizeGlobal) * 100);
	unsigned long long speed = writtenGlobal - lastWritten;
	lastWritten = writtenGlobal;
	unsigned long long secondsLeft = (sizeGlobal - lastWritten) / speed;
	formatSpeed(speed);
	formatTimeLeft(secondsLeft);
	printf("%.2Lf %%\n", d);
	showProgressBar(writtenGlobal, sizeGlobal);

	DWORD end = GetTickCount();
	formatTimeCount(end - startTime);
	return 1;
}

int dumpDrive(std::string drive, std::string dumpPath) {
	ULONGLONG driveSize = GetDriveSize((char*)drive.c_str());
	if (!driveSize) {
		printf("Invalid drive %s\n", drive.c_str());
		return 0;
	}
	HANDLE hDrive = CreateFileA(drive.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hDrive == INVALID_HANDLE_VALUE) {
		DWORD error = GetLastError();
		printf("Failed to open drive %s with read access\n", drive.c_str());
		printError();
		return 0;
	}
	HANDLE hDest = CreateFileA(dumpPath.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hDest == INVALID_HANDLE_VALUE) {
		DWORD error = GetLastError();
		printf("Failed to open dump file %s with write access\n", drive.c_str());
		printError();
		return 0;
	}
	char buffer[4096];
	ULONGLONG readTotal = 0;
	DWORD temp, written;
	int counter = 0;
	printf("Dumping drive to: %s ...\n", dumpPath.c_str());
	DWORD startTime = GetTickCount();
	while (readTotal < driveSize) {
		ULONGLONG toRead = min(driveSize - readTotal, sizeof(buffer));
		if (!ReadFile(hDrive, buffer, toRead, &temp, 0)) {
			printf("Read from drive failed\n");
			printError();
			CloseHandle(hDrive);
			CloseHandle(hDest);
			return 0;
		}
		if (!WriteFile(hDest, buffer, temp, &written, 0) || written != temp) {
			printf("Failed to write to dump file!\n");
			printError();
			CloseHandle(hDrive);
			CloseHandle(hDest);
			return 0;
		}
		counter++;
		readTotal += temp;
		if (counter == 1000) {
			long double d = 100 - ((((long double)driveSize - readTotal) / driveSize) * 100);
			printf("%.2Lf %%\n", d);
			counter = 0;
		}
	}
	printf("100 %%\n");
	DWORD end = GetTickCount();
	long double time = (((long double)end) - startTime);
	formatTimeCount(time);
	CloseHandle(hDrive);
	CloseHandle(hDest);
	return 1;
}

int burnDrive(std::string drive, std::string imagePath) {
	ULONGLONG driveSize = GetDriveSize((char*)drive.c_str());
	if (!driveSize) {
		printf("Invalid drive %s\n", drive.c_str());
		return 0;
	}
	HANDLE hImage = CreateFileA(imagePath.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hImage == INVALID_HANDLE_VALUE) {
		printf("Failed to open image file: %s\n", imagePath.c_str());
		return 0;
	}
	LARGE_INTEGER li;
	if (!GetFileSizeEx(hImage, &li)) {
		CloseHandle(hImage);
		printf("Failed to get image size\n");
		return 0;
	}
	if (driveSize < li.QuadPart) {
		CloseHandle(hImage);
		printf("Image won't fit in the drive\n");
		return 0;
	}
	//Open the drive for write
	HANDLE hDrive = CreateFileA(drive.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hDrive == INVALID_HANDLE_VALUE) {
		DWORD error = GetLastError();
		printf("Failed to open drive %s with write access", drive.c_str());
		printError();
		return 0;
	}
	DWORD status;
	if (!DeviceIoControl(hDrive, FSCTL_DISMOUNT_VOLUME,
		NULL, 0, NULL, 0, &status, NULL))
	{
		printf("Failed to dismount drive\n");
		printError();
		CloseHandle(hDrive);
		CloseHandle(hImage);
		return 0;
	}

	// lock volume
	if (!DeviceIoControl(hDrive, FSCTL_LOCK_VOLUME,
		NULL, 0, NULL, 0, &status, NULL))
	{
		printf("Failed to lock drive\n");
		printError();
		CloseHandle(hDrive);
		CloseHandle(hImage);
		return 0;
		// error handling; not sure if retrying is useful
	}
	char buffer[4096];
	unsigned long long written = 0;
	DWORD temp, temp2;
	int ret = 1;
	printf("Writing image to disk...\n");
	while (written < li.QuadPart) {
		DWORD toRead = min(sizeof(buffer), li.QuadPart - written);
		if (!ReadFile(hImage, buffer, toRead, &temp, 0)) {
			printf("Failed to read buffer from image file!\n");
			printError();
			ret = 0;
			goto cleanup;
		}
		if (!WriteFile(hDrive, buffer, temp, &temp2, 0) || temp2 != temp) {
			printf("Failed to write buffer to the disk!\n");
			printError();
			ret = 0;
			goto cleanup;
		}
		written += temp;
	}
	cleanup:
	CloseHandle(hImage);
	if (!DeviceIoControl(hDrive, FSCTL_UNLOCK_VOLUME,
		NULL, 0, NULL, 0, &status, NULL))
	{
		printf("Failed to unlock drive\n");
		printError();
		// error handling; not sure if retrying is useful
	}
	CloseHandle(hDrive);
	return 1;
}
int main(int argc, char *argv[])
{
	printf("Disk Dump 1.0\n\n");
	std::string disk;
	std::string dumpPath;
	std::string burnPath;
	bool formatFlag=false, dumpFlag=false, burnFlag=false;
	if (argc <= 1) {
		std::string flagsStr;
		printf("Input disk path: ");
		std::getline(std::cin, disk);
		printf("Input flags (-dump *PATH_TO_DUMP* OR -format OR -burn *PATH_TO_ISO_IMAGE*): ");
		std::getline(std::cin, flagsStr);
		if (flagsStr.find("-dump ") == 0) {
			dumpPath = flagsStr.substr(6);
			dumpFlag = true;
		}
		else if (flagsStr.compare("-format") == 0) {
			formatFlag = true;
		}
		else if (flagsStr.find("-burn ") == 0) {
			burnPath = flagsStr.substr(6);
			burnFlag = true;
		}
		else {
			printf("Invalid flags\n");
			return 0;
		}
	}
	else {
		for (int i = 1; i < argc; i++) {
			std::string temp = (argv[i]);
			if (temp.compare("-format") == 0) {
				formatFlag = true;
			}
			else if (temp.compare("-dump") == 0) {
				if (i < argc - 1) {
					dumpPath = (argv[i + 1]);
					dumpFlag = true;
					i++;
				}
			}
			else if (temp.compare("-burn") == 0) {
				if (i < argc - 1) {
					burnPath = (argv[i + 1]);
					burnFlag = true;
					i++;
				}
			}
			else {
				disk = (argv[i]);
			}
		}
	}
	if (disk[disk.length() - 1] == '\\')
		disk = disk.substr(0, disk.length() - 1);
	else if (disk[disk.length() - 1] == '/')
		disk = disk.substr(0, disk.length() - 1);
	disk = "\\\\.\\" + disk;
	if (formatFlag) {
		if (formatDrive(disk)) {
			printf("Disk was successfully formated!\n");
		}
		else {
			printf("Disk was not formated!\n");
		}
	}
	else if (dumpFlag) {
		if (dumpDrive(disk, dumpPath)) {
			printf("Drive dumped successfully\n");
		}
		else {
			printf("The drive was not dumped!\n");
		}
	}
	else if (burnFlag) {
		if (burnDrive(disk, burnPath)) {
			printf("Image was burned successfully!\n");
		}
		else {
			printf("The image was not burned!\n");
		}
	}
	getchar();
    return 0;
}

