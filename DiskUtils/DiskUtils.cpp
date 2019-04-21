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
		std::printf("%dmin %.2Lfs\n", minutes, time);
	}
	else {
		std::printf("%.2Lfs\n", time);
	}
}

void formatTimeCount(DWORD64 time) {
	time /= 1000;
	if (time > 60) {
		DWORD64 minutes = time / 60;
		time = time % 60;
		std::printf("Time taken: %llumin %llus\n", minutes, time);
	}
	else {
		std::printf("Time taken: %llus\n", time);
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
	DWORD64 startTime = GetTickCount64();
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
		std::printf("%.2Lf %%\n", d);
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
	std::printf("%.2Lf %%\n", d);
	showProgressBar(writtenGlobal, sizeGlobal);

	DWORD64 end = GetTickCount64();
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
		std::printf("Failed to open drive %s with read access\n", drive.c_str());
		printError();
		return 0;
	}
	HANDLE hDest = CreateFileA(dumpPath.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hDest == INVALID_HANDLE_VALUE) {
		DWORD error = GetLastError();
		std::printf("Failed to open dump file %s with write access\n", drive.c_str());
		printError();
		return 0;
	}
	char buffer[4096];
	ULONGLONG readTotal = 0;
	DWORD temp, written;
	int counter = 0;
	std::printf("Dumping drive to: %s ...\n", dumpPath.c_str());
	DWORD64 startTime = GetTickCount64();
	while (readTotal < driveSize) {
		ULONGLONG toRead = min(driveSize - readTotal, sizeof(buffer));
		if (!ReadFile(hDrive, buffer, toRead, &temp, 0)) {
			std::printf("Read from drive failed\n");
			printError();
			CloseHandle(hDrive);
			CloseHandle(hDest);
			return 0;
		}
		if (!WriteFile(hDest, buffer, temp, &written, 0) || written != temp) {
			std::printf("Failed to write to dump file!\n");
			printError();
			CloseHandle(hDrive);
			CloseHandle(hDest);
			return 0;
		}
		counter++;
		readTotal += temp;
		if (counter == 1000) {
			long double d = 100 - ((((long double)driveSize - readTotal) / driveSize) * 100);
			std::printf("%.2Lf %%\n", d);
			counter = 0;
		}
	}
	std::printf("100 %%\n");
	DWORD64 end = GetTickCount64();
	long double time = (((long double)end) - startTime);
	formatTimeCount(time);
	CloseHandle(hDrive);
	CloseHandle(hDest);
	return 1;
}

int burnDrive(std::string drive, std::string imagePath) {
	ULONGLONG driveSize = GetDriveSize((char*)drive.c_str());
	if (!driveSize) {
		std::printf("Invalid drive %s\n", drive.c_str());
		return 0;
	}
	HANDLE hImage = CreateFileA(imagePath.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hImage == INVALID_HANDLE_VALUE) {
		std::printf("Failed to open image file: %s\n", imagePath.c_str());
		return 0;
	}
	LARGE_INTEGER li;
	if (!GetFileSizeEx(hImage, &li)) {
		CloseHandle(hImage);
		std::printf("Failed to get image size\n");
		return 0;
	}
	if (driveSize < li.QuadPart) {
		CloseHandle(hImage);
		std::printf("Image won't fit in the drive\n");
		return 0;
	}
	//Open the drive for write
	HANDLE hDrive = CreateFileA(drive.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hDrive == INVALID_HANDLE_VALUE) {
		DWORD error = GetLastError();
		std::printf("Failed to open drive %s with write access", drive.c_str());
		printError();
		return 0;
	}
	DWORD status;

	// lock volume
	if (!DeviceIoControl(hDrive, FSCTL_LOCK_VOLUME,
		NULL, 0, NULL, 0, &status, NULL))
	{
		std::printf("Failed to lock drive\n");
		printError();
		CloseHandle(hDrive);
		CloseHandle(hImage);
		return 0;
	}
	char buffer[4096];
	unsigned long long written = 0;
	DWORD temp, temp2;
	int ret = 1;
	std::printf("Writing image to disk...\n");
	while (written < li.QuadPart) {
		DWORD toRead = min(sizeof(buffer), li.QuadPart - written);
		if (!ReadFile(hImage, buffer, toRead, &temp, 0)) {
			std::printf("Failed to read buffer from image file!\n");
			printError();
			ret = 0;
			goto cleanup;
		}
		if (!WriteFile(hDrive, buffer, temp, &temp2, 0) || temp2 != temp) {
			std::printf("Failed to write buffer to the disk!\n");
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
		std::printf("Failed to unlock drive\n");
		printError();
		// error handling; not sure if retrying is useful
	}
	CloseHandle(hDrive);
	return 1;
}

std::string queryDrive() {
	DWORD drives = GetLogicalDrives();
	char drivePath[32];
	drivePath[1] = ':';
	drivePath[2] = '\\';
	drivePath[3] = '\0';
	DWORD p = 1;
	for (int i = 0; i < 31; i++) {
		if (drives & (1 << i)) {
			drivePath[0] = 'A' + i;
			UINT driveType = GetDriveTypeA(drivePath);
			switch (driveType) {
			case DRIVE_UNKNOWN:
				std::printf("%d. DRIVE_UNKNOWN: %s\n", i, drivePath);
				break;
			case DRIVE_REMOVABLE:
				std::printf("%d. DRIVE_REMOVABLE: %s\n", i, drivePath);
				break;
			case DRIVE_FIXED:
				std::printf("%d. DRIVE_FIXED: %s\n", i, drivePath);
				break;
			case DRIVE_REMOTE:
				std::printf("%d. DRIVE_REMOTE: %s\n", i, drivePath);
				break;
			case DRIVE_CDROM:
				std::printf("%d. DRIVE_CDROM: %s\n", i, drivePath);
				break;
			case DRIVE_RAMDISK:
				std::printf("%d. DRIVE_RAMDISK: %s\n", i, drivePath);
				break;
			default:
				std::printf("%d. DRIVE_UNKNOWN: %s\n", i, drivePath);
				break;
			}
		}
	}
	int selected;
	std::string str;
	do {
		std::printf("Select a drive (0-31): ");
		std::getline(std::cin, str);
		try {
			selected = std::stoi(str);
		}
		catch (std::invalid_argument& e) {

		}
		catch (std::out_of_range& e) {

		}
	} while (!(drives & (1 << selected)));
	char drive = 'A' + selected;
	std::string path = "\\\\.\\";
	path.push_back(drive);
	path.push_back(':');
	return path;
}

int main(int argc, char *argv[])
{
	std::printf("Disk Dump 1.0\n\n");
	std::string disk;
	std::string dumpPath;
	std::string burnPath;
	bool formatFlag = false, dumpFlag = false, burnFlag = false;
	if (argc <= 1) {
		std::string flagsStr;
		std::printf("Input flags (-dump *PATH_TO_DUMP* OR -format OR -burn *PATH_TO_ISO_IMAGE*): ");
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
			std::printf("Invalid flags\n");
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
	disk = queryDrive();
	std::cout << disk << std::endl;
	if (formatFlag) {
		if (formatDrive(disk)) {
			printf("Disk was successfully formated!\n");
		}
		else {
			printf("Disk was not erased!\n");
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
    return 0;
}

