#include <iostream>
#include <Windows.h>
#include <cstdio>
#include <tchar.h> 
#include <tlhelp32.h>
#include <WinBase.h>
#include <string>
#include <vector>
#include <fstream>

 using namespace std;

 const int EXPERT_WIDTH = 30;
 const int EXPERT_HEIGHT = 16;
 const int GARBAGE_CHAR = 16;
 const int MINE_CHAR = -113;
 const int BLANK_CHAR = 15;
 const int NO_NUM = 64; //1 starts at 65 (A)
 const int MAX_LET_ASCII = 90;
 const int MAX_TABLE_SIZE_EXPERT = 480;
 const unsigned int MAP_MEMORY_ADDRESS = 0x01005340;
 const unsigned int GAME_STATE_ADDRESS = 0x01005160;

 PDWORD ListProcessModules(DWORD);
 vector<vector<char> > tableParse(string parser);
 bool stillPlaying(HANDLE handle);
 bool didWin(HANDLE handle);
 pair<int, int> getMove(vector<vector<char> > boardMain);
 vector<vector<char> > translateBoard(vector<vector<char> > board);

 int main(int argc, TCHAR *argv[]){

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD creationFlags = PROCESS_VM_READ | PROCESS_VM_WRITE;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
  
    char pathToProgram[] = "C:\\Users\\Graham\\Documents\\My Games\\Winmine__XP.exe";
    //IMPORTANT
    //change this to the path to your minesweeper executable.

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

    
    //Play the game
    while(stillPlaying(pi.hProcess)){
        char buffer[1000];
        PDWORD numBytes;
        string temp ="";

        //get the map
        ReadProcessMemory(pi.hProcess, (PVOID)MAP_MEMORY_ADDRESS, buffer, sizeof(buffer), 0);
        string parser(buffer);
        vector<vector<char> > mineMap = tableParse(parser);

        pair<int, int> move = pair<int, int>(0, 0);
        move = getMove(mineMap);
        
        SendMessage(windowHandle, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(20+(16*move.first),70+(16*move.second))); //click the board
        SendMessage(windowHandle, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(20+(16*move.first),70+(16*move.second)));
        Sleep(30);// For effect only, can comment out.
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
*Find where the table/win variable are stored.
*Returns base address of the minesweeper process
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

/*
* Checks if the bot has won or lost
*
*/
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

/*
*Checks if bot won
*
*/
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

/*
* Returns string version of board state
*/
string printBoard(vector<vector<char> > b){
    string finalstr = "";
    for(int i = 0; i < b.size(); i++){
        for(int j = 0; j < b[0].size(); j++){
            finalstr.append(1, b[i][j]);
        }
        finalstr.append("\n");
    }
    return finalstr;
}


/*
* Gets the next move for the bot. Does not make smart guesses.
*
*/
pair<int, int> getMove(vector<vector<char> > boardMain){
    vector<vector<char> > board = translateBoard(boardMain);

    ofstream myfile;
    myfile.open ("example.txt");
        
    int height = EXPERT_HEIGHT;
    int width  = EXPERT_WIDTH;
    vector<pair<int, int> > coords;
    
    for (int i=0;i<height;i++){
        myfile << printBoard(board) << endl;
        for (int j=0;j<width;j++){
            if(board[i][j]=='0'){ 
                for(int jj=-1;jj<2;jj++){
                        for(int ii=-1;ii<2;ii++){
                                if(i+ii>=0 && j+jj>=0 && i+ii<height && j+jj<width){
                                    if(board[i+ii][j+jj]=='o'){ 
                                        coords.push_back(pair<int, int>(j+jj, i+ii));
                                    }
                                }
                        }
                }
            }
            int num=0;

            for(int I=-1;I<2;I++){
                    for(int J=-1;J<2;J++){
                            if(i+I>=0 && j+J>=0 && i+I<height && j+J<width)
                                   if(board[i+I][j+J]=='o')
                                                           num++;
                    }
            }  
            bool changed = false;
            if (num == (int)board[i][j]-48){                                
                for(int x=-1;x<2;x++){
                    for(int y=-1;y<2;y++){
                            if(i+x>=0 && j+y>=0 && i+x<height && j+y<width)
                                   if(board[i+x][j+y]=='o' ){
                                          board[i+x][j+y]='x';
                                          changed = true;
                                          for(int xx=-1;xx<2;xx++){
                                               for(int yy=-1;yy<2;yy++){
                                                       if(i+x+xx>=0 && j+y+yy>=0 && i+x+xx<height && j+y+yy<width)
                                                                   if(board[i+x+xx][j+y+yy] == '1' ||
                                                                      board[i+x+xx][j+y+yy] == '2' ||
                                                                      board[i+x+xx][j+y+yy] == '3' ||
                                                                      board[i+x+xx][j+y+yy] == '4' ||
                                                                      board[i+x+xx][j+y+yy] == '5' ||
                                                                      board[i+x+xx][j+y+yy] == '6' ||
                                                                      board[i+x+xx][j+y+yy] == '7' ||
                                                                      board[i+x+xx][j+y+yy] == '8' ||
                                                                      board[i+x+xx][j+y+yy] == '9' )
                                                                               board[i+x+xx][j+y+yy] = (char)((int)board[i+x+xx][j+y+yy]-49)+48;
                                               }
                                          }
                                   }
                    }
                }
                if(changed){
                    j=0;
                    i=-1;
                    break;
                }               
            }
        }
    }

    if (coords.size()==0){ //Backup case, fills coords with all possible unflagged moves
         
         for (int r=0;r<height;r++){
             for(int c=0;c<width;c++){
                     if(board[r][c]=='o'){
                          coords.push_back(pair<int, int>(c, r));
                     }
             }
         }
    }
    int randomInt = rand()%coords.size();//picks random move of possible moves
    pair<int, int> move = pair<int, int>((coords[randomInt].first), (coords[randomInt].second));
    myfile.close();
    return move;   
}

/**
* Translates the board from garbage characters to ones that make sense (A -> 1, @ -> " ", etc.)
*
*/
vector<vector<char> > translateBoard(vector<vector<char> > board){
    vector<vector<char> > finalBoard = board;
    int j = 0;
    for(int i = 0; i < EXPERT_HEIGHT; i++){
        for(int j = 0; j < EXPERT_WIDTH; j++){
            if(finalBoard[i][j] == MINE_CHAR || finalBoard[i][j] == BLANK_CHAR){
                finalBoard[i][j] = 'o';
            }
            else if(finalBoard[i][j] == NO_NUM){
                finalBoard[i][j] = ' ';
            }
            else if(finalBoard[i][j] > NO_NUM && finalBoard[i][j] <= MAX_LET_ASCII){
                finalBoard[i][j] = finalBoard[i][j] - 16;
            }
            else{
                finalBoard[i][j] = 'o';
            }
        }
    }
    return finalBoard;
}
