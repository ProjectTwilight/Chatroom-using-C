#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <wchar.h>
#include <windows.h>
#include <string.h>

#define WIDTH 400   
#define HEIGHT 600
#define WM_UPDATE_CHAT (WM_USER + 1)

#define ED_EDIT 1
#define ID_BUTTON 2
#define ID_USERNAME_EDIT 3
#define SEND_BUTTON 4
#define MAX_MESSAGE_SIZE 50000

#define IP_TARGET "127.0.0.1"

//#define IP_TARGET "192.168.254.137"


#define SRC_PORT 12345
#define DST_PORT 54321


// #define SRC_PORT 54321
// #define DST_PORT 12345


HWND hWndMain, hName, lChatBox, rChatBox, hTypeBox, hSendButton, hLogo, hSetNameButton, hBackground;
wchar_t  message[MAX_MESSAGE_SIZE], lFullMessage[MAX_MESSAGE_SIZE], rFullMessage[MAX_MESSAGE_SIZE], userName[MAX_MESSAGE_SIZE] = L"", messageToSend[MAX_MESSAGE_SIZE];
char sentMessage[MAX_MESSAGE_SIZE], encryptedMessage[MAX_MESSAGE_SIZE], newMessage[MAX_MESSAGE_SIZE];
BOOL bEnd = FALSE;
SOCKET sock;

void toBinary(unsigned char c, char* binStr) {
    for (int i = 7; i >= 0; --i) {
        binStr[i] = (c & 1) + '0';
        c >>= 1;
    }
}

unsigned char fromBinary(char* binStr) {
    unsigned char c = 0;
    for (int i = 7; i >= 0; --i) {
        c <<= 1;
        c |= binStr[i] - '0';
    }
    return c;
}

void encrypt(char* input, char* output) {
    memset(output, 0, MAX_MESSAGE_SIZE); // Clear output buffer
    while (*input) {
        char binStr[9] = {0};
        toBinary(*input, binStr);

        // Complement the binary
        for (int i = 4; i < 8; ++i) {
            binStr[i] = (binStr[i] == '0') ? '1' : '0';
        }

        *output = fromBinary(binStr);
        ++input;
        ++output;
    }
    *output = '\0'; // Null-terminate the output string
}

void decrypt(char* input, char* output) {
    memset(output, 0, MAX_MESSAGE_SIZE); // Clear output buffer
    while (*input) {
        char binStr[9] = {0};
        toBinary(*input, binStr);

        // Complement the binary
        for (int i = 0; i < 8; ++i) {
            binStr[i] = (binStr[i] == '0') ? '1' : '0';
        }

        *output = fromBinary(binStr);
        ++input;
        ++output;
    }
    *output = '\0'; // Null-terminate the output string
}


BOOL SendData(char* msg){
    SOCKADDR_IN sendAddR = {0};
    sendAddR.sin_family = AF_INET;
    sendAddR.sin_port = htons(DST_PORT);
    sendAddR.sin_addr.s_addr = inet_addr(IP_TARGET);

    int sendResult = sendto(sock, msg, strlen(msg), 0, (SOCKADDR*)&sendAddR, sizeof(sendAddR));
    if (sendResult == SOCKET_ERROR) {
        MessageBoxW(NULL, L"Failed to send message!", L"Error", MB_OK | MB_ICONERROR);
        return FALSE;
    }
    return TRUE;
}

DWORD WINAPI RecVThread(LPVOID pParam){
    SOCKADDR_IN RecVAddr = {0};
    int iRet, iRecVSize;
    char buf[MAX_MESSAGE_SIZE];

    while(!bEnd){
        iRecVSize = sizeof(RecVAddr);
        iRet = recvfrom(sock, buf, MAX_MESSAGE_SIZE, 0, (SOCKADDR*)&RecVAddr, &iRecVSize);

        if(iRet == SOCKET_ERROR){
            continue;
        }
        buf[iRet] = '\0';

        

        // Decrypt the message
        char decryptedMessage[MAX_MESSAGE_SIZE];
        decrypt(buf, decryptedMessage);

        // Convert the decrypted message to wchar_t* for displaying in the chatbox
        wchar_t* wBuf = (wchar_t*)malloc(MAX_MESSAGE_SIZE * sizeof(wchar_t));
        MultiByteToWideChar(CP_ACP, 0, decryptedMessage, -1, wBuf, MAX_MESSAGE_SIZE);
        PostMessage(hWndMain, WM_UPDATE_CHAT, 0, (LPARAM)wBuf);
    }
    return 0;
}


void AddControls(HWND hWnd){
    HBITMAP hBitmap = (HBITMAP)LoadImageW(NULL, L"Logo.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (hBitmap == NULL) {
        MessageBoxW(NULL, L"Failed to load bitmap!", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    hName = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD |WS_BORDER | SS_LEFT| ES_MULTILINE | ES_AUTOVSCROLL, 
            10, 5, 195, 30, hWnd, NULL, NULL, NULL);
            
    hBackground = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | SS_LEFT| ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_DISABLED, 
            10, 40, 364, 442, hWnd, NULL, NULL, NULL);

    hSetNameButton = CreateWindowW(L"Button", L"✓", WS_VISIBLE | WS_CHILD, 
            210, 5, 30, 30, hWnd, (HMENU)ID_BUTTON, NULL, NULL);
    lChatBox = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | SS_LEFT| ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_DISABLED, 
            15, 45, 175.5, 432, hWnd, NULL, NULL, NULL);
    rChatBox = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | SS_RIGHT| ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_DISABLED, 
            194.5, 45, 175.5, 432, hWnd, NULL, NULL, NULL);
    hTypeBox = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | SS_LEFT| ES_MULTILINE | ES_AUTOVSCROLL | WS_DISABLED, 
            10, 490, 266, 60, hWnd, NULL, NULL, NULL);
    hSendButton = CreateWindowW(L"Button", L"Send", WS_VISIBLE | WS_CHILD | WS_DISABLED | BS_BITMAP, 
            274, 490, 100, 60, hWnd, (HMENU)SEND_BUTTON, NULL, NULL);
    SendMessage(hSendButton, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap);

    MessageBoxW(hWnd, L"Please Enter Your Name First", L"Info", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
    static HBRUSH hbrBackground = NULL;
    static HBRUSH hbrSendButton = NULL;

    switch(msg)
    {
        case WM_COMMAND:
            switch(wp)
            {
                case ID_BUTTON:
                    GetWindowTextW(hName, userName, 50000);
                    if(wcscmp(userName, L"") == 0){
                        MessageBoxW(hWnd, L"Username Field is Blank!", L"Info", MB_OK | MB_ICONEXCLAMATION);
                    }
                    else{
                        EnableWindow(hSetNameButton, FALSE);
                        SendMessageW(hName, EM_SETREADONLY, TRUE, 0);
                        SendMessageW(hName, WS_DISABLED, TRUE, 0);

                        EnableWindow(hTypeBox, TRUE);
                        EnableWindow(hSendButton, TRUE);
                    }
                    break;
                case SEND_BUTTON:
                    GetWindowTextW(hTypeBox, message, 50000);

                    if(wcscmp(message, L"") == 0){
                        break;
                    }

                    SendMessageW(hTypeBox, WM_SETTEXT, 0, (LPARAM)L"");
                    
                    wcscpy(messageToSend, userName); // Copy username
                    wcscat(messageToSend, L": "); // Add separator
                    wcscat(messageToSend, message); // Add the actual message

                    WideCharToMultiByte(CP_ACP, 0, messageToSend, -1, sentMessage, sizeof(sentMessage), NULL, NULL);
                    encrypt(sentMessage, newMessage); // Encrypt the message
                    SendData(newMessage); // Send the encrypted message

                    // Update lChatBox with the original message
                    wcscpy(rFullMessage, L" ");
                    wcscat(rFullMessage, message);
                    wcscat(rFullMessage, L"\r\n\r\n");
                    SendMessageW(rChatBox, EM_SETSEL, 0xFFFFFFFF, 0xFFFFFFFF);
                    SendMessageW(rChatBox, EM_REPLACESEL, TRUE, (LPARAM)rFullMessage);

                    // Prepare a modified message with asterisks for rChatBox, handling new lines
                    wchar_t modifiedMsg[50000];
                    wcscpy(modifiedMsg, L" ");
                    for (int i = 0; message[i] != L'\0' && i < 49994; i++) {
                        if (message[i] == L'\n' || (message[i] == L'\r' && message[i+1] == L'\n')) {
                            wcscat(modifiedMsg, L"\r\n");
                            if (message[i] == L'\r') {
                                i++;
                            }
                        } else {
                            wcscat(modifiedMsg, L" ");
                        }
                    }
                    wcscat(modifiedMsg, L"\r\n\r\n");
                    SendMessageW(lChatBox, EM_SETSEL, 0xFFFFFFFF, 0xFFFFFFFF);
                    SendMessageW(lChatBox, EM_REPLACESEL, TRUE, (LPARAM)modifiedMsg);
                    
                    break;
            }
            break;

        case WM_UPDATE_CHAT:
            {
                wchar_t* receivedMsg = (wchar_t*)lp;

                wcscpy(lFullMessage, L"");
                wcscat(lFullMessage, receivedMsg);
                wcscat(lFullMessage, L"\r\n\r\n");
                SendMessageW(lChatBox, EM_SETSEL, 0xFFFFFFFF, 0xFFFFFFFF); // Move cursor to end
                SendMessageW(lChatBox, EM_REPLACESEL, TRUE, (LPARAM)lFullMessage);
                

                wchar_t modifiedMsg[50000];
                wcscpy(modifiedMsg, L"");
                for (int i = 0; receivedMsg[i] != L'\0' && i < 49994; i++) {
                    if (receivedMsg[i] == L'\n' || (receivedMsg[i] == L'\r' && receivedMsg[i+1] == L'\n')) {
                        wcscat(modifiedMsg, L"\r\n");
                        if (receivedMsg[i] == L'\r') {
                            i++;
                        }
                    } else {
                        wcscat(modifiedMsg, L" ");
                    }
                }
                wcscat(modifiedMsg, L"\r\n\r\n");
                SendMessageW(rChatBox, EM_SETSEL, 0xFFFFFFFF, 0xFFFFFFFF);
                SendMessageW(rChatBox, EM_REPLACESEL, TRUE, (LPARAM)modifiedMsg);

                free(receivedMsg);
            }
            break;

        case WM_CREATE:
            hWndMain = hWnd;
            AddControls(hWnd);
            break;

        case WM_CTLCOLORSTATIC:
            {
                HDC hdcStatic = (HDC)wp;
                HWND hCtrl = (HWND)lp;
                if (hCtrl == hBackground)
                {
                    if (!hbrBackground)
                    {
                        hbrBackground = CreateSolidBrush(RGB(69, 69, 69)); 
                    }
                    SetBkColor(hdcStatic, RGB(69, 69, 69)); 
                    SetTextColor(hdcStatic, RGB(0, 0, 0)); 
                    return (INT_PTR)hbrBackground;
                }
                else if (hCtrl == lChatBox)
                {
                    static HBRUSH hbrLChatBoxBkgnd = NULL;
                    if (!hbrLChatBoxBkgnd)
                    {
                        hbrLChatBoxBkgnd = CreateSolidBrush(RGB(245, 247, 247)); 
                    }
                    SetBkColor(hdcStatic, RGB(245, 247, 247)); 
                    SetTextColor(hdcStatic, RGB(0, 0, 0)); 
                    return (INT_PTR)hbrLChatBoxBkgnd;
                }
                else if (hCtrl == rChatBox)
                {
                    static HBRUSH hbrRChatBoxBkgnd = NULL;
                    if (!hbrRChatBoxBkgnd)
                    {
                        hbrRChatBoxBkgnd = CreateSolidBrush(RGB(245, 247, 247)); 
                    }
                    SetBkColor(hdcStatic, RGB(245, 247, 247)); 
                    SetTextColor(hdcStatic, RGB(0, 0, 0)); 
                    return (INT_PTR)hbrRChatBoxBkgnd;
                }
                else if (hCtrl == hName)
                {
                    static HBRUSH hbrHName = NULL;
                    if (!hbrHName)
                    {
                        hbrHName = CreateSolidBrush(RGB(255, 255, 255)); 
                    }
                    SetBkColor(hdcStatic, RGB(255, 255, 255)); 
                    SetTextColor(hdcStatic, RGB(0, 0, 0)); 
                    return (INT_PTR)hbrHName;
                }
                else if (hCtrl == hTypeBox)
                {
                    static HBRUSH hbrTypeBox = NULL;
                    if (!hbrTypeBox)
                    {
                        hbrTypeBox = CreateSolidBrush(RGB(255, 255, 255));
                    }
                    SetBkColor(hdcStatic, RGB(255, 255, 255)); 
                    SetTextColor(hdcStatic, RGB(0, 0, 0));
                    return (INT_PTR)hbrTypeBox;
                }
                return DefWindowProcW(hWnd, msg, wp, lp);
            }
            break;

        case WM_DESTROY:
            if (hbrBackground)
            {
                DeleteObject(hbrBackground);
                hbrBackground = NULL;
            }
            if (hbrSendButton)
            {
                DeleteObject(hbrSendButton);
                hbrSendButton = NULL;
            }
            PostQuitMessage(0);
            break;


        default:
            return DefWindowProcW(hWnd, msg, wp, lp);
    }
}

SOCKET makeSocket(){
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock == INVALID_SOCKET){
        MessageBoxW(NULL, L"Socket creation failed!", L"Error", MB_OK | MB_ICONERROR);
        return (SOCKET)NULL;
    }

    SOCKADDR_IN addR = {0};
    addR.sin_family = AF_INET;
    addR.sin_port = htons(SRC_PORT);
    addR.sin_addr.s_addr = INADDR_ANY;
    
    if(bind(sock, (SOCKADDR*)&addR, sizeof(addR)) == SOCKET_ERROR){
        closesocket(sock);
        MessageBoxW(NULL, L"Socket binding failed!", L"Error", MB_OK | MB_ICONERROR);
        return (SOCKET)NULL;
    }
    return sock;
}



int main() {
    WSADATA wsaData = {0};
    WNDCLASSW wc = {0};
    
    WSAStartup(MAKEWORD(2,2), &wsaData);

    HBRUSH hbrMainBkgnd = CreateSolidBrush(RGB(121, 121, 121));
    wc.hbrBackground = hbrMainBkgnd;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"myWindowClass";
    wc.lpfnWndProc = WindowProcedure;

    if(makeSocket() != (SOCKET)NULL){
        HANDLE hThread = CreateThread(NULL, 0, RecVThread, NULL, 0, NULL);
    }

    if(!RegisterClassW(&wc)){
        MessageBoxW(NULL, L"Failed to register window class", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    CreateWindowW(L"myWindowClass", L"My Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
        GetSystemMetrics(SM_CXSCREEN)/2 - (WIDTH/2), GetSystemMetrics(SM_CYSCREEN)/2 - (HEIGHT/2), 
        WIDTH, HEIGHT, NULL, NULL, NULL, NULL);

    MSG msg = {0};

    while(GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    WSACleanup();
    return 0;
}
