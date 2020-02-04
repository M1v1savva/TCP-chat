//=========================================================
//{      INFORMATION
//=========================================================
//!
//!     server.cpp
//!     Сервер на сокетах для локальной сети.
//!     Одновременно поддерживает максимум 16 клинтов!
//!
//!     Version 1.0.0
//!     Author: Polyakov Ivan <vk.com/programmer_c_forever>
//!                           <ivan.polyakov.01@gmail.com>
//!     Date:   23.05.2016  16:22
//!
//!     История изменений
//!         $ Версия 0.0.0
//!             !Примитивный однопоточный эхо-сервер
//!         $ Версия 0.0.1
//!             ! Реализована многопоточность (одновременное обслуживание нескольких клиентов)
//!               путем присвоения каждому клиенту определённого номера. Код вводится при входе.
//!             ! Реализовано распределение клиентов на пары по их согласованию
//!         $ Версия 1.0.0
//!             ! Сервер переделан под ядро свободного чата
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
using std::endl;                     //облегчаем код
using std::cin;
using std::cout;
using std::string;
//}
//---------------------------------------------------------

//---------------------------------------------------------
//{         Initialization
//---------------------------------------------------------
int clients = 0; //глобальная переменная - количество активных подключений
char clients_names [16] [30];
SOCKET client [16];
bool connections [16];
char hostent_data1 [16] [30];
char hostent_data2 [16] [30];

DWORD WINAPI Work_With_Client (LPVOID client_data);//работа с клиентом в отдельном потоке
//}
//---------------------------------------------------------

//---------------------------------------------------------
//{         Main
//---------------------------------------------------------
int main ()
{
    char buffer [1024] = "";

    setlocale (LC_CTYPE, "Russian");
    SetConsoleTitle ("TCP server 2.0");
    cout << "TCP SERVER DEMO BY POLYAKOV IVAN\n";   //заголовок
    cout << "VERSION 2.0\n";                        //номер версии


    //-----------------------------------------------------
    //{         Preparations
    //-----------------------------------------------------
    //переменная для проверки результатов работы функций
    int *result = new int;

    cout << endl;
    cout << "Подключение ws2_32..." << endl;
    WSADATA *w = new WSADATA {};                //бесполезная структура
    *result = WSAStartup (0x0202, &*w);         //включаем ws2_32
                                                //версия библиотеки 0x0202

    if (*result != 0)                             //не удалось подключить библиотеку
    {
        cout << "ERROR: " << WSAGetLastError () << endl; //печатаем код ошибки
        char *er = new char [100];
        strcat (er, "Не удалось подключиться к библиотеке.\n");
        strcat (er, "Проверьте настройки линковщика.\n");
        strcat (er, "Проверьте расположение библиотеки:\n");
        strcat (er, "C:\\windows\system32\ws2_32.dll");
        MessageBox (NULL, er, "error", MB_ICONWARNING);  //error message
        delete er;                                       //освобождаем память
        return 1;                                        //подробнее в README
    }

    cout << "Проверка версии сокетов..." << endl;
    if (w -> wVersion != 0x0202)     // не та версия сокетов!
    {
        cout << "ERROR: " << WSAGetLastError () << endl;
        char *er = new char [100];
        strcat (er, "Для корректной работы необходима версия сокетов 2x0202!\n");
        strcat (er, "Обновите сокеты!\n");
        MessageBox (NULL, er, "error", MB_ICONWARNING);
        WSACleanup ();                     //закрываем библиотеку
        delete er;
        return 2;
    }
    delete w;                              //удаляем лишнюю переменную


    cout << "Создание слушующего сокета..." << endl;
    SOCKET alpha = socket (AF_INET, SOCK_STREAM, 0);
    //AF_INET - семейство протоколов для работы через Интернет
    //SOCK_STREAM - сокет потоковый
    //0 - TCP протокол по умолчанию

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

    //структура, описывающая адрес
    sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;                  //семейство протоколов
    local_addr.sin_port   = htons (6666);             //номер порта, не забываем про порядок байт
    local_addr.sin_addr.s_addr = inet_addr ("127.0.0.1");  //всё дозволено! Все IP адреса доступны

    cout << "Прикрепление сокета к локальному адресу..." << endl;
    *result = bind(alpha, (sockaddr *) &local_addr, sizeof(local_addr));

    if (*result == SOCKET_ERROR)
    {
        cout << "ERROR: " << WSAGetLastError() << endl;
        char *er = new char [100];
        strcpy (er, "Не удалось привязать сокет к локальному адресу!");
        MessageBox (NULL, er, "error", MB_ICONWARNING);
        delete er;
        delete result;
        closesocket (alpha);
        WSACleanup();
        return 4;
    }

    cout << "Ожидание подключений..." << endl;
    *result = listen (alpha, 0x10);         //задаем максимальное число подключений и начинаем
                                            //прослушивание

    if (*result == SOCKET_ERROR)           //в случае провала...
    {
        cout << "ERROR: " << WSAGetLastError() << endl;
        char *er = new char [100];
        strcpy (er, "Ошибка ожидания покдлючений\n");
        MessageBox (NULL, er, "error", MB_ICONWARNING);
        delete er;
        delete result;
        closesocket (alpha);
        WSACleanup();
        return 5;
    }
    //}
    //-----------------------------------------------------

    SOCKET client_socket = INVALID_SOCKET; //промежуточный сокет для обработки подключений
    sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);
    int client_number = 0;

    cout << "Обработка подключений..." << endl;

    for (int i = 0; i < 16; i++)
    {
        client [i] = INVALID_SOCKET;
        strcpy (clients_names [i], "");
        connections [i] = false;
    }

    for (; clients < 16; client_socket = INVALID_SOCKET)    //обрабатываем подключения
    {
        //соединяемся с клиентом
        client_socket = accept (alpha, (sockaddr *) &client_addr, &client_addr_size);

        if (client_socket == INVALID_SOCKET)  //да сколько можно?!
        {
            cout << "ERROR: " << WSAGetLastError() << endl;
            char *er = new char [100];
            strcpy (er, "Ошибка соединения с клиентом\n");
            MessageBox (NULL, er, "error", MB_ICONWARNING);
            delete er;
            delete result;
            closesocket (alpha);
            WSACleanup();
            return 6;
        }

        clients++;                 //великий и могучий с++

        for (int i = 0; i < 16; i++)
        {
            if (connections [i] == false)
            {
                client_number = i;
                connections [i] = true;
            }
        }

        recv (client_socket, buffer, sizeof (buffer), NULL);
        strcpy (clients_names [client_number], buffer);
        strcpy (buffer, "");

        cout << "Получение данных хоста клиента..." << endl;
        HOSTENT *hst;
        hst = gethostbyaddr((char *) &client_addr.sin_addr.s_addr, 4, AF_INET);

        strcpy (hostent_data1 [client_number], (hst)? hst -> h_name:"");
        strcpy (hostent_data2 [client_number], inet_ntoa (client_addr.sin_addr));

        strcpy (buffer, "+");
        strcat (buffer, hostent_data1 [client_number]);
        strcat (buffer, " [");
        strcat (buffer, hostent_data2 [client_number]);
        strcat (buffer, "] new connect! Name: ");
        strcat (buffer, clients_names [client_number]);
        strcat (buffer, "\n");
        printf("%s", buffer);
        //получение и вывод сведений о клиенте

        client [client_number] = client_socket;

        DWORD thID;

        for (int i = 0; i < 16; i++)
        {
            if (client[i] != INVALID_SOCKET) send (client [i], buffer, sizeof (buffer), NULL);
        }

        CreateThread (NULL, NULL, Work_With_Client, &client_number, NULL, &thID);
    }

    delete result;
    closesocket (alpha);
    closesocket (client_socket);
    closesocket (client [0]);
    closesocket (client [1]);
    closesocket (client [2]);
    closesocket (client [3]);
    closesocket (client [4]);
    closesocket (client [5]);
    closesocket (client [6]);
    closesocket (client [7]);
    closesocket (client [8]);
    closesocket (client [9]);
    closesocket (client [10]);
    closesocket (client [11]);
    closesocket (client [12]);
    closesocket (client [13]);
    closesocket (client [14]);
    closesocket (client [15]);
    WSACleanup ();
}
//}
//---------------------------------------------------------

//---------------------------------------------------------
//{         Work with client
//---------------------------------------------------------
DWORD WINAPI Work_With_Client (LPVOID client_data)
{
    cout << "work_with_client" << endl;
    char buffer [1024] = "";
    char main_buffer [1024] = "";

    int my_number;
    my_number = ((int *) client_data)[0];

    while (int bytes_recv = recv (client [my_number], buffer, sizeof (buffer), NULL)
           && bytes_recv != SOCKET_ERROR)
    {
        strcpy (main_buffer, clients_names [my_number]);
        strcat (main_buffer, " ");
        strcat (main_buffer, buffer);
        for (int i = 0; i < 16; i++)
        {
            if (connections [i] != false && i != my_number)
            {
                send (client [i], main_buffer, sizeof (main_buffer), NULL);
            }
        }
    }

    strcpy (buffer, "+");
    strcat (buffer, hostent_data1 [my_number]);
    strcat (buffer, " [");
    strcat (buffer, hostent_data2 [my_number]);
    strcat (buffer, "] disconnected from server! Name: ");
    strcat (buffer, clients_names [my_number]);
    strcat (buffer, "\n");
    printf("%s", buffer);

    for (int i = 0; i < 16; i++)
    {
        if (connections [i] != false)
        {
            send (client [i], buffer, sizeof (buffer), NULL);
        }
    }

    connections [my_number] = false;
    client [my_number] = INVALID_SOCKET;
    strcpy (clients_names [my_number], "");
}
//}
//---------------------------------------------------------
