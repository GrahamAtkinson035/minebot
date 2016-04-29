#include <iostream>
#include <Windows.h>
#include <cstdio>
#include <tchar.h> 
#include <tlhelp32.h>
#include <WinBase.h>
#include <string>
#include <vector>

 using namespace std;

 const int EXPERT_WIDTH = 30;
 const int EXPERT_HEIGHT = 16;
 const int GARBAGE_CHAR = 16;
 const int MINE_CHAR = -113;
 const int BLANK_CHAR = 15;
 const int MAX_TABLE_SIZE_EXPERT = 480;
 const unsigned int MAP_MEMORY_ADDRESS = 0x01005340;
 const unsigned int GAME_STATE_ADDRESS = 0x01005160;

 PDWORD ListProcessModules(DWORD);
 vector<vector<char> > tableParse(string parser);
 bool stillPlaying(HANDLE handle);
 bool didWin(HANDLE handle);

int main(int argc, TCHAR *argv[]){

	STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD creationFlags = PROCESS_VM_READ | PROCESS_VM_WRITE;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
  
    char pathToProgram[] = "C:\\Users\\olivp\\Documents\\My Games\\Winmine__XP.exe";

	if( !CreateProcessA( NULL,
		pathToProgram,         
        NULL,           // process handle
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        creationFlags,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    ) 
    {
        printf( "CreateProcess failed (%d).\n", GetLastError() );
        return 1;
    }

    Sleep(1000);
    HWND windowHandle = FindWindow(NULL, "Minesweeper");

    PDWORD baseId = ListProcessModules((pi.dwProcessId)); // <---get the baseAddress from modules

    char buffer[1000];
    PDWORD numBytes;
    string temp ="";
    ReadProcessMemory(pi.hProcess, (PVOID)MAP_MEMORY_ADDRESS, buffer, sizeof(buffer), 0);
    	

    string parser(buffer);
    
    vector<vector<char> > mineMap = tableParse(parser);
    
    int row = 0;
    int column  = 0;
    while(stillPlaying(pi.hProcess)){
        if(mineMap[row][column] == MINE_CHAR){
            SendMessage(windowHandle, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(20+(16*column),70+(16*row)));
            SendMessage(windowHandle, WM_RBUTTONUP, MK_RBUTTON, MAKELPARAM(20+(16*column),70+(16*row)));
        }
        if(mineMap[row][column] == BLANK_CHAR){
            SendMessage(windowHandle, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(20+(16*column),70+(16*row)));
            SendMessage(windowHandle, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(20+(16*column),70+(16*row)));
        }
        column++;
        if(column >= EXPERT_WIDTH){
            column = 0;
            row++;
        }
        //Sleep(30);
    }
    if(didWin(pi.hProcess)){
        cout << "Good Job bot! :--D" << endl;
    }
    else{
        cout << "You lost baddie! D--:<" << endl;
    }

	return 1;
}

/**
*
*
*
**/
PDWORD ListProcessModules( DWORD dwPID ) {
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE; 
  MODULEENTRY32 me32; 
 
//  Take a snapshot of all modules in the specified process. 
  hModuleSnap = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, dwPID ); 
  if( hModuleSnap == INVALID_HANDLE_VALUE ) 
  { 
    cerr << ( ("CreateToolhelp32Snapshot (of modules)") ) << endl; 
  } 
 
//  Set the size of the structure before using it. 
  me32.dwSize = sizeof( MODULEENTRY32 ); 
 
//  Retrieve information about the first module, 
//  and exit if unsuccessful 
  if( !Module32First( hModuleSnap, &me32 ) ) 
  { 
    cerr << ( TEXT("Module32First") ) << endl;  // Show cause of failure 
    CloseHandle( hModuleSnap );     // Must clean up the snapshot object! 

  } 

  PDWORD baseAddress = ((PDWORD)(me32.modBaseAddr));
 
  CloseHandle( hModuleSnap ); 
  return( baseAddress ); 
}

vector<vector<char> > tableParse(string parser){
    vector<vector<char> > mineTable(EXPERT_HEIGHT);

    int j = 0;
    for(int i = 0; i < parser.length(); i++){
        if(parser[i] == GARBAGE_CHAR || i >= MAX_TABLE_SIZE_EXPERT){
            parser.erase(i, 1);
            i--;
        }
        else{
            mineTable[j].push_back(parser[i]);
            if(((i+1)%EXPERT_WIDTH) == 0 && i > 0){
                j++;
            }
        }
    }
    return mineTable;
}

bool stillPlaying(HANDLE handle){
    int buffer = 0;
    ReadProcessMemory(handle, (PVOID)GAME_STATE_ADDRESS, &buffer, sizeof(buffer), 0);

    if(buffer == 2 || buffer == 3){
        return false;
    }
    else{
        return true;
    }
}

bool didWin(HANDLE handle){
    int buffer = 0;
    ReadProcessMemory(handle, (PVOID)GAME_STATE_ADDRESS, &buffer, sizeof(buffer), 0);
    if(buffer == 2){
        return false;
    }
    else{
        return true;
    }
}