#include <Windows.h>
#include <stdio.h>
#include "clientNetwork.h"

void createChildrens(HWND hWnd);
void TextEdit_append(HWND hWnd, const PWSTR newText);
void networkRoutine();
int WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND* childrenList;
int SEND = 0;


int WINAPI wWinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) {
    WNDCLASS wnd = {0};
    wnd.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    wnd.hIcon = LoadIcon(0, IDI_APPLICATION);
    wnd.hCursor = LoadCursor(0, IDC_ARROW);
    wnd.lpszClassName = L"Application";
    wnd.lpfnWndProc = WndProc;
    RegisterClass(&wnd);
    HWND hWnd = CreateWindow(L"Application", L"Чятик", WS_VISIBLE | WS_OVERLAPPEDWINDOW, 0, 0, 600, 300, 0, 0, 0, 0);
    MSG msg = {0};
    while(GetMessage(&msg, hWnd, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

int WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps = {0};
    switch(uMsg) {
        case WM_CREATE:
            createChildrens(hWnd);
            CreateThread(0, 0, networkRoutine, 0, 0, 0);
            break;
        case WM_DESTROY:
            ExitProcess(0);
            break;
        case WM_COMMAND:
            if (LOWORD(wParam) == 3) {
                SEND = 1;
            }
            break;
        case WM_PAINT:
            HDC hdc = BeginPaint(hWnd, &ps);

            EndPaint(hWnd, &ps);
            break;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

void createChildrens(HWND hWnd) {
    childrenList = (HWND*)calloc(10, sizeof(*childrenList));
    childrenList[0] = CreateWindow(L"Edit", L" ", WS_VISIBLE | WS_CHILDWINDOW | ES_MULTILINE 
    | ES_READONLY | ES_AUTOVSCROLL, 0, 0, 210, 200, hWnd, 0, 0, 0);
    childrenList[1] = CreateWindow(L"Edit", L" ", WS_VISIBLE | WS_CHILDWINDOW | ES_MULTILINE  
    , 220, 0, 150, 50, hWnd, 0, 0, 0);
    childrenList[2] = CreateWindow(L"Button", L"Отправить", WS_VISIBLE | WS_CHILDWINDOW | BS_PUSHBUTTON 
    , 220, 70, 90, 60, hWnd, (HMENU)3, 0, 0);
}

void TextEdit_append(HWND hWnd, const PWSTR newText) {
    int textLen = GetWindowTextLength(hWnd) + lstrlen(newText) + 3; // +2 для "\r\n" и +1 для '\0'
    PWSTR buf = (PWSTR)malloc(textLen * sizeof(wchar_t)); // wchar_t, если работаем с Unicode
    GetWindowText(hWnd, buf, textLen);
    wcscat(buf, newText);
    wcscat(buf, L"\r\n"); // Для Windows стиль перевода строки - это возврат каретки и перевод строки
    SetWindowText(hWnd, buf);
    free(buf);
}


void networkRoutine() {
    Client* client = createClient();
    unsigned long mode = 1; 
    ioctlsocket(client_getSocket(client), FIONBIO, &mode);
    char buffer[BUFSIZ];
    wchar_t w_buffer[BUFSIZ];
    if(connectToServer(client, "", 8080) == TRUE) {
        TextEdit_append(childrenList[0], L"Connected to server successfully.\r\n");
    } else {
        TextEdit_append(childrenList[0], L"Connected to server ERROR.\r\n");
    }
    while(1) {
        int bytesReceived  = 0;
        if((bytesReceived = recv(client_getSocket(client), buffer, BUFSIZ, 0)) > 0) {
            int wideSize = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, NULL, 0);
            wchar_t* wideStr = (wchar_t*)malloc(wideSize * sizeof(wchar_t));
            MultiByteToWideChar(CP_UTF8, 0, buffer, bytesReceived, wideStr, wideSize);
            TextEdit_append(childrenList[0], wideStr);
            free(wideStr);
        }
           if (SEND == 1) {
        wchar_t w_buffer[BUFSIZ]; // Buffer for wide characters
        GetWindowTextW(childrenList[1], w_buffer, BUFSIZ); // Getting text in wchar_t format

        int size = GetWindowTextLength(childrenList[1]);

        char hostname[BUFSIZ];
        gethostname(hostname, BUFSIZ);

        int hostnameLength = MultiByteToWideChar(CP_UTF8, 0, hostname, -1, NULL, 0);
        wchar_t* w_hostname = (wchar_t*)malloc(hostnameLength * sizeof(wchar_t));
        MultiByteToWideChar(CP_UTF8, 0, hostname, -1, w_hostname, hostnameLength);

        wchar_t* finalMessage = (wchar_t*)malloc((size + hostnameLength + 3) * sizeof(wchar_t)); // +3: два для "\r\n" и один для '\0'
        wcscpy(finalMessage, w_hostname);
        wcscat(finalMessage, L": "); // Добавляем разделитель
        wcscat(finalMessage, w_buffer);

        // Converting from wchar_t* to a multibyte string (UTF-8)
        int utf8Size = WideCharToMultiByte(CP_UTF8, 0, finalMessage, -1, NULL, 0, NULL, NULL); // -1, так как finalMessage является строкой, завершающейся нулем
        char* utf8Text = (char*)malloc(utf8Size);
        WideCharToMultiByte(CP_UTF8, 0, finalMessage, -1, utf8Text, utf8Size, NULL, NULL);

        int sended = send(client_getSocket(client), utf8Text, utf8Size, 0); 

        free(w_hostname);
        free(finalMessage);
        free(utf8Text); 
        SEND = 0;
    }
    }
}