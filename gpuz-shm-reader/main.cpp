#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

// https://msdn.microsoft.com/en-us/library/windows/desktop/aa366551(v=vs.85).aspx
// https://www.techpowerup.com/forums/threads/gpu-z-shared-memory-layout.65258/

#define SHMEM_NAME _T("GPUZShMem")
#define MAX_RECORDS 128

#pragma pack(push, 1)
struct GPUZ_RECORD
{
	WCHAR key[256];
	WCHAR value[256];
};

struct GPUZ_SENSOR_RECORD
{
	WCHAR name[256];
	WCHAR unit[8];
	UINT32 digits;
	double value;
};

struct GPUZ_SH_MEM
{
	UINT32 version;		// Version number, 1 for the struct here
	volatile LONG busy;	// Is data being accessed?
	UINT32 lastUpdate;	// GetTickCount() of last update
	GPUZ_RECORD data[MAX_RECORDS];
	GPUZ_SENSOR_RECORD sensors[MAX_RECORDS];
};
#pragma pack(pop)

#define BUF_SIZE (sizeof(GPUZ_SH_MEM))



int main()
{
	HANDLE hMapFile;
	LPCTSTR pBuf;

	hMapFile = OpenFileMapping(
		FILE_MAP_READ,   // read/write access
		FALSE,           // do not inherit the name
		SHMEM_NAME);     // name of mapping object

	if (hMapFile == NULL) {
		wprintf(L"Could not open file mapping object (%d).\n", GetLastError());
		return 1;
	}

	pBuf = (LPTSTR)MapViewOfFile(hMapFile,		// handle to map object
								 FILE_MAP_READ,	// read/write permission
								 0,
								 0,
								 BUF_SIZE);

	if (pBuf == NULL) {
		wprintf(L"Could not map view of file (%d).\n", GetLastError());
		CloseHandle(hMapFile);
		return 1;
	}

	GPUZ_SH_MEM* gpuz = (GPUZ_SH_MEM*)(pBuf);

	wprintf(L"version:\t%u\n", gpuz->version);
	wprintf(L"busy:\t%ld\n", gpuz->busy);
	wprintf(L"lastUpdate:\t%u\n", gpuz->lastUpdate);

	for (int i = 0; i < MAX_RECORDS; i++) {
		GPUZ_RECORD& data = gpuz->data[i];
		if (wcscmp(data.key, L"") == 0) { break; }
		wprintf(L"%ls:\t%ls\n", data.key, data.value);
	}

	wchar_t wvalue[1024] = { 0 };
	for (int i = 0; i < MAX_RECORDS; i++) {
		GPUZ_SENSOR_RECORD& sensor = gpuz->sensors[i];
		if (wcscmp(sensor.name, L"") == 0) { break; }
		wprintf(L"%ls:\t%lf%ls\n", sensor.name, sensor.value, sensor.unit);
	}

	UnmapViewOfFile(pBuf);

	CloseHandle(hMapFile);

	return 0;
}