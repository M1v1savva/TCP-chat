//=========================================================
//{      INFORMATION
//=========================================================
//!
//!     client.cpp
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
//!             ! ���������������� �������� � �������� ��������� � �������.
//!         $ ������ 0.0.1
//!             ! ����������� ������������� �������� �� ���� �� �� ������������
//!         $ ������ 1.0.0
//!             ! ��������� ������ ��� �������� � �������� ���������.
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
    //���������� ��� �������� ����������� ������ �������
    int *result = new int;
    cout << "����������� ws2_32...";
    WSADATA *w = new WSADATA {};                //����������� ���������
    *result = WSAStartup (0x0202, &*w);         //���������� ws2_32
                                                //������ ���������� 0x0202
    if (*result != 0)                             //�� ������� ���������� ����������
    {
        cout << "ERROR: " << WSAGetLastError () << endl;
        char *er = new char [100];
        strcpy (er, "�� ������� ������������ � ����������.\n");
        strcpy (er, "��������� ��������� ����������.\n");
        strcpy (er, "��������� ������������ ����������:\n");
        strcpy (er, "C:\\windows\system32\ws2_32.dll");
        MessageBox (NULL, er, "error", MB_ICONWARNING);
        delete er;
        return 1;
    }

    cout << "�������� ������ �������..." << endl;
    if (w -> wVersion != 0x0202)     // �� �� ������ �������!
    {
        cout << "ERROR: " << WSAGetLastError () << endl;
        char *er = new char [100];
        strcpy (er, "��� ���������� ������ ���������� ������ ������� 2x0202!\n");
        strcpy (er, "�������� ������!\n");
        MessageBox (NULL, er, "error", MB_ICONWARNING);
        WSACleanup ();
        delete er;
        return 2;
    }
    delete w;

    cout << "�������� ������" << endl;
    SOCKET alpha = socket (AF_INET, SOCK_STREAM, 0);

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

    sockaddr_in my_addr;

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(6666);
    my_addr.sin_addr.s_addr = inet_addr ("127.0.0.1");
    HOSTENT *hst;

                                // ����� ������� ������� � �������� ����������
                                // ����������

    cout << "�����������..." << endl;
    if (connect (alpha, (LPSOCKADDR) &my_addr, sizeof(my_addr)) != NULL)
    {
        cout << "ERROR: " << WSAGetLastError () << endl;
        char *er = new char [100];
        strcpy (er, "�� ������� ������������ � �������!");
        MessageBox (NULL, er, "error", MB_ICONWARNING);
        delete er;
        WSACleanup ();
        return 4;
    }
    //}
    //---------------------------------------------------------

    cout << "���������� �����������" << endl;
    cout << "����� ����������!!!" << endl;
    cout << "������� ���� ���(�������� 30 ��������): ";

    scanf ("%s", buffer);
    send (alpha, buffer, sizeof (buffer), NULL);
    strcpy (buffer, "");

    cout << "��� ������ ������� \"quit\"" << endl;

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

