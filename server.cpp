//=========================================================
//{      INFORMATION
//=========================================================
//!
//!     server.cpp
//!     ������ �� ������� ��� ��������� ����.
//!     ������������ ������������ �������� 16 �������!
//!
//!     Version 1.0.0
//!     Author: Polyakov Ivan <vk.com/programmer_c_forever>
//!                           <ivan.polyakov.01@gmail.com>
//!     Date:   23.05.2016  16:22
//!
//!     ������� ���������
//!         $ ������ 0.0.0
//!             !����������� ������������ ���-������
//!         $ ������ 0.0.1
//!             ! ����������� ��������������� (������������� ������������ ���������� ��������)
//!               ����� ���������� ������� ������� ������������ ������. ��� �������� ��� �����.
//!             ! ����������� ������������� �������� �� ���� �� �� ������������
//!         $ ������ 1.0.0
//!             ! ������ ��������� ��� ���� ���������� ����
//}
//=========================================================


//---------------------------------------------------------
//{         The Includes
//---------------------------------------------------------
#include "TXLib.h"
#include <Winsock2.h>                //���������� ��� ������ � ��������

#pragma comment (lib, c:\\windows\system32\ws2_32.dll)    //���������� ����������
//}
//---------------------------------------------------------

//---------------------------------------------------------
//{         Easy using some of std::
//---------------------------------------------------------
using std::endl;                     //��������� ���
using std::cin;
using std::cout;
using std::string;
//}
//---------------------------------------------------------

//---------------------------------------------------------
//{         Initialization
//---------------------------------------------------------
int clients = 0; //���������� ���������� - ���������� �������� �����������
char clients_names [16] [30];
SOCKET client [16];
bool connections [16];
char hostent_data1 [16] [30];
char hostent_data2 [16] [30];

DWORD WINAPI Work_With_Client (LPVOID client_data);//������ � �������� � ��������� ������
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
    cout << "TCP SERVER DEMO BY POLYAKOV IVAN\n";   //���������
    cout << "VERSION 2.0\n";                        //����� ������


    //-----------------------------------------------------
    //{         Preparations
    //-----------------------------------------------------
    //���������� ��� �������� ����������� ������ �������
    int *result = new int;

    cout << endl;
    cout << "����������� ws2_32..." << endl;
    WSADATA *w = new WSADATA {};                //����������� ���������
    *result = WSAStartup (0x0202, &*w);         //�������� ws2_32
                                                //������ ���������� 0x0202

    if (*result != 0)                             //�� ������� ���������� ����������
    {
        cout << "ERROR: " << WSAGetLastError () << endl; //�������� ��� ������
        char *er = new char [100];
        strcat (er, "�� ������� ������������ � ����������.\n");
        strcat (er, "��������� ��������� ����������.\n");
        strcat (er, "��������� ������������ ����������:\n");
        strcat (er, "C:\\windows\system32\ws2_32.dll");
        MessageBox (NULL, er, "error", MB_ICONWARNING);  //error message
        delete er;                                       //����������� ������
        return 1;                                        //��������� � README
    }

    cout << "�������� ������ �������..." << endl;
    if (w -> wVersion != 0x0202)     // �� �� ������ �������!
    {
        cout << "ERROR: " << WSAGetLastError () << endl;
        char *er = new char [100];
        strcat (er, "��� ���������� ������ ���������� ������ ������� 2x0202!\n");
        strcat (er, "�������� ������!\n");
        MessageBox (NULL, er, "error", MB_ICONWARNING);
        WSACleanup ();                     //��������� ����������
        delete er;
        return 2;
    }
    delete w;                              //������� ������ ����������


    cout << "�������� ���������� ������..." << endl;
    SOCKET alpha = socket (AF_INET, SOCK_STREAM, 0);
    //AF_INET - ��������� ���������� ��� ������ ����� ��������
    //SOCK_STREAM - ����� ���������
    //0 - TCP �������� �� ���������

    if (alpha == INVALID_SOCKET)    //����� ������ �� ����������
    {
        cout << "ERROR: " << WSAGetLastError () << endl;
        char *er = new char [100];
        strcpy (er, "�� ������� ������� �����!");
        MessageBox (NULL, er, "error", MB_ICONWARNING);
        delete er;
        WSACleanup ();
        return 3;
    }

    //���������, ����������� �����
    sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;                  //��������� ����������
    local_addr.sin_port   = htons (6666);             //����� �����, �� �������� ��� ������� ����
    local_addr.sin_addr.s_addr = inet_addr ("127.0.0.1");  //�� ���������! ��� IP ������ ��������

    cout << "������������ ������ � ���������� ������..." << endl;
    *result = bind(alpha, (sockaddr *) &local_addr, sizeof(local_addr));

    if (*result == SOCKET_ERROR)
    {
        cout << "ERROR: " << WSAGetLastError() << endl;
        char *er = new char [100];
        strcpy (er, "�� ������� ��������� ����� � ���������� ������!");
        MessageBox (NULL, er, "error", MB_ICONWARNING);
        delete er;
        delete result;
        closesocket (alpha);
        WSACleanup();
        return 4;
    }

    cout << "�������� �����������..." << endl;
    *result = listen (alpha, 0x10);         //������ ������������ ����� ����������� � ��������
                                            //�������������

    if (*result == SOCKET_ERROR)           //� ������ �������...
    {
        cout << "ERROR: " << WSAGetLastError() << endl;
        char *er = new char [100];
        strcpy (er, "������ �������� �����������\n");
        MessageBox (NULL, er, "error", MB_ICONWARNING);
        delete er;
        delete result;
        closesocket (alpha);
        WSACleanup();
        return 5;
    }
    //}
    //-----------------------------------------------------

    SOCKET client_socket = INVALID_SOCKET; //������������� ����� ��� ��������� �����������
    sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);
    int client_number = 0;

    cout << "��������� �����������..." << endl;

    for (int i = 0; i < 16; i++)
    {
        client [i] = INVALID_SOCKET;
        strcpy (clients_names [i], "");
        connections [i] = false;
    }

    for (; clients < 16; client_socket = INVALID_SOCKET)    //������������ �����������
    {
        //����������� � ��������
        client_socket = accept (alpha, (sockaddr *) &client_addr, &client_addr_size);

        if (client_socket == INVALID_SOCKET)  //�� ������� �����?!
        {
            cout << "ERROR: " << WSAGetLastError() << endl;
            char *er = new char [100];
            strcpy (er, "������ ���������� � ��������\n");
            MessageBox (NULL, er, "error", MB_ICONWARNING);
            delete er;
            delete result;
            closesocket (alpha);
            WSACleanup();
            return 6;
        }

        clients++;                 //������� � ������� �++

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

        cout << "��������� ������ ����� �������..." << endl;
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
        //��������� � ����� �������� � �������

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
