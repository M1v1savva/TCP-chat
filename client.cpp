//=========================================================
//{      INFORMATION
//=========================================================
//!
//!     client.cpp
//!     Клиент на сокетах для локальной сети.
//!     Одновременно поддерживает максимум 16 клинтов!
//!
//!     Version 1.0.0
//!     Author: Polyakov Ivan <vk.com/programmer_c_forever>
//!                           <ivan.polyakov.01@gmail.com>
//!     Date:   23.05.2016  16:22
//!
//!     История изменений
//!         $ Версия 0.0.0
//!             ! Последовательные отправка и принятие сообщений с сервера.
//!         $ Версия 0.0.1
//!             ! Реализовано распределение клиентов на пары по их согласованию
//!         $ Версия 1.0.0
//!             ! Отдельный клиент для отправки и принятия сообщений.
//}
//=========================================================

//---------------------------------------------------------
//{         The Includes
//---------------------------------------------------------
#include "TXLib.h"
#include <Winsock2.h>                //библиотека для работы с сокетами

#pragma comment (lib, c:\\windows\system32\ws2_32.dll)    //подключаем библиотеку
//}
//---------------------------------------------------------

//---------------------------------------------------------
//{         Easy using some of std::
//---------------------------------------------------------
using std::cout;
using std::endl;
using std::cin;
using std::string;
//}
//---------------------------------------------------------

//---------------------------------------------------------
//{         Initialization
//---------------------------------------------------------
char my_name [30] = "";
bool destroy = false;

DWORD WINAPI _recv (LPVOID client_socket);
DWORD WINAPI _send (LPVOID client_socket);
//}
//---------------------------------------------------------

//---------------------------------------------------------
//{         Main
//---------------------------------------------------------
int main ()
{
    txCreateWindow (800, 600);

    char buffer [1024] = "";
    SetConsoleTitle ("TCP client 2.0");
    cout << "TCP CLIENT DEMO BY POLYAKOV IVAN\n" << endl;
    cout << "VERSION 2.0" << endl;

    //---------------------------------------------------------
    //{         Preparations
    //---------------------------------------------------------
    //переменная для проверки результатов работы функций
    int *result = new int;
    cout << "Подключение ws2_32...";
    WSADATA *w = new WSADATA {};                //бесполезная структура
    *result = WSAStartup (0x0202, &*w);         //Подгружаем ws2_32
                                                //версия библиотеки 0x0202
    if (*result != 0)                             //не удалось подключить библиотеку
    {
        cout << "ERROR: " << WSAGetLastError () << endl;
        char *er = new char [100];
        strcpy (er, "Не удалось подключиться к библиотеке.\n");
        strcpy (er, "Проверьте настройки линковщика.\n");
        strcpy (er, "Проверьте расположение библиотеки:\n");
        strcpy (er, "C:\\windows\system32\ws2_32.dll");
        MessageBox (NULL, er, "error", MB_ICONWARNING);
        delete er;
        return 1;
    }

    cout << "Проверка версии сокетов..." << endl;
    if (w -> wVersion != 0x0202)     // не та версия сокетов!
    {
        cout << "ERROR: " << WSAGetLastError () << endl;
        char *er = new char [100];
        strcpy (er, "Для корректной работы необходима версия сокетов 2x0202!\n");
        strcpy (er, "Обновите сокеты!\n");
        MessageBox (NULL, er, "error", MB_ICONWARNING);
        WSACleanup ();
        delete er;
        return 2;
    }
    delete w;

    cout << "Создание сокета" << endl;
    SOCKET alpha = socket (AF_INET, SOCK_STREAM, 0);

    if (alpha == INVALID_SOCKET)    //опять ничего не получается
    {
        cout << "ERROR: " << WSAGetLastError () << endl;
        char *er = new char [100];
        strcpy (er, "Не удалось создать сокет!");
        MessageBox (NULL, er, "error", MB_ICONWARNING);
        delete er;
        WSACleanup ();
        return 3;
    }

    sockaddr_in my_addr;

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(6666);
    my_addr.sin_addr.s_addr = inet_addr ("127.0.0.1");
    HOSTENT *hst;

                                // адрес сервера получен – пытаемся установить
                                // соединение

    cout << "Подключение..." << endl;
    if (connect (alpha, (LPSOCKADDR) &my_addr, sizeof(my_addr)) != NULL)
    {
        cout << "ERROR: " << WSAGetLastError () << endl;
        char *er = new char [100];
        strcpy (er, "Не удалось подключиться в серверу!");
        MessageBox (NULL, er, "error", MB_ICONWARNING);
        delete er;
        WSACleanup ();
        return 4;
    }
    //}
    //---------------------------------------------------------

    cout << "СОЕДИНЕНИЕ УСТАНОВЛЕНО" << endl;
    cout << "Добро пожаловать!!!" << endl;
    cout << "Введите ваше имя(максимум 30 символов): ";

    scanf ("%s", buffer);
    send (alpha, buffer, sizeof (buffer), NULL);
    strcpy (buffer, "");

    cout << "Для выхода введите \"quit\"" << endl;

    DWORD thID1;
    DWORD thID2;

    CreateThread (NULL, NULL, _recv, &alpha, NULL, &thID1);
    CreateThread (NULL, NULL, _send, &alpha, NULL, &thID2);

    delete result;
    closesocket (alpha);
    WSACleanup ();
}
//}
//---------------------------------------------------------

//---------------------------------------------------------
//{         _recv
//---------------------------------------------------------
DWORD WINAPI _recv (LPVOID client_socket)
{
    SOCKET my_sock;
    my_sock = ((SOCKET *) client_socket)[0];
    char buffer [1024] = "";

    while (int bytes_recv = recv (my_sock, buffer, sizeof (buffer), NULL)
           && bytes_recv != SOCKET_ERROR)
    {
        //printf ("%s\n", buffer);
        txTextOut (50, 100, buffer);
        if (destroy == true) return 0;
    }
    cout << "error" << endl;
    closesocket (my_sock);
}
//}
//---------------------------------------------------------

//---------------------------------------------------------
//{         _send
//---------------------------------------------------------
DWORD WINAPI _send (LPVOID client_socket)
{
    SOCKET my_sock;
    my_sock = ((SOCKET *) client_socket)[0];
    char buffer [1024] = "";

    scanf ("%s", buffer);

    if (strcmp (buffer, "quit") == NULL)
    {
        destroy = true;
        MessageBox (NULL, "The correct output", "Process finished", MB_ICONWARNING);
        return 0;
    }

    while (int bytes_send = send (my_sock, buffer, sizeof (buffer), NULL)
           && bytes_send != SOCKET_ERROR)
    {
        scanf ("%s", buffer);
        if (strcmp (buffer, "quit") == NULL)
        {
            destroy = true;
            MessageBox (NULL, "The correct output", "Process finished", MB_ICONWARNING);
            return 0;
        }
    }
    cout << "error" << endl;
    closesocket (my_sock);
}
//}
//---------------------------------------------------------

