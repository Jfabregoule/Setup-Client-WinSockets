#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

SOCKET clientSocket;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_LBUTTONDOWN: // G�rer le clic gauche de la souris
    {
        // R�cup�rer les coordonn�es du clic de la souris
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        // Dessiner un pixel al�atoire en bleu � ces coordonn�es
        HDC hdc = GetDC(hwnd);
        SetPixel(hdc, x, y, RGB(0, 0, 255)); // Bleu (0, 0, 255)
        ReleaseDC(hwnd, hdc);

        char message[256];
        sprintf_s(message, "Mouse click at coordinates: (%d, %d)", x, y);
        send(clientSocket, message, strlen(message), 0);
    }
    return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

    // Initialisation Winsock

    WSADATA wsaData; // Data struct pour winsock

    // Check de l'init en version 2.2
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        MessageBox(NULL, L"Initialize Failed!", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Cr�ation de la fen�tre
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"MyWindowClass";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,                              // Options de la fen�tre
        L"MyWindowClass",               // Classe de la fen�tre
        L"Chat Client",                 // Titre de la fen�tre
        WS_OVERLAPPEDWINDOW,            // Style de la fen�tre
        CW_USEDEFAULT, CW_USEDEFAULT,   // Position de la fen�tre (par d�faut)
        800, 600,                       // Taille de la fen�tre
        NULL,                           // Handle de la fen�tre parent
        NULL,                           // Handle du menu
        hInstance,                      // Instance de l'application
        NULL                            // Donn�es de cr�ation de fen�tre suppl�mentaires
    );

    if (hwnd == NULL)
    {
        MessageBox(NULL, L"Failed to create window!", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);

    // Cr�ation de la socket
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        MessageBox(NULL, L"Failed to create socket!", L"Error", MB_OK | MB_ICONERROR);
        WSACleanup(); // Nettoyer Winsock
        return 1;
    }

    // D�finition des informations de connexion du serveur
    const char* serverIP = "127.0.0.1"; // Adresse IP du serveur
    int serverPort = 12345; // Port sur lequel le serveur �coute

    // Remplissage de la structure sockaddr_in
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);
    serverAddr.sin_port = htons(serverPort);

    // Connexion au serveur
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        MessageBox(NULL, L"Failed to connect to server!", L"Error", MB_OK | MB_ICONERROR);
        closesocket(clientSocket);
        WSACleanup(); // Nettoyer Winsock
        return 1;
    }

    // Boucle principale de messages de la fen�tre
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Boucle pour recevoir les messages du serveur
    char buffer[256];
    int bytesReceived;
    do {
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            printf("Received message from server: %s\n", buffer);
        }
    } while (bytesReceived > 0);

    return 0;
}