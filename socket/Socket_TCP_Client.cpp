// Socket_TCP_Client.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#pragma  comment(lib, "ws2_32.lib")
using namespace std;

#define  LOW_VERSION  2
#define  HIGH_VERSION 2

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	SOCKET sock;
	const char* pServerIP = "127.0.0.1";
	unsigned short usServerPort = 9901;


	if (0 != WSAStartup(MAKEWORD(LOW_VERSION, HIGH_VERSION), &wsaData))
	{
		cerr << endl << "WSAStartup Error" << endl;
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sock)
	{
		WSACleanup();
		cerr << endl << "socket Error" << endl;
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////


	const int BUFFER_LENGTH = 1024;
	char buffer[BUFFER_LENGTH] = {0};
	
	struct sockaddr_in serverAddr;
	int lenOfOutAddr = sizeof(struct sockaddr_in);

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(usServerPort);
	serverAddr.sin_addr.S_un.S_addr = inet_addr(pServerIP);

	//////////////////////////////////////////////////////////////////////////
	char bufQuit[] = "quit";

	cout << endl << "----------- I am connecting to server "
		<< inet_ntoa(serverAddr.sin_addr)
		<< " : "
		<< ntohs(serverAddr.sin_port)
		<< " -----------"
		<< endl;

	int nRet = connect(sock, (const sockaddr*)&serverAddr, lenOfOutAddr);
	if (0 != nRet)
	{
		cerr << endl << "************connect " << pServerIP << " : " \
			 << usServerPort << "   failed ! **************"         \
			 << endl << endl;
	}
	else
	{
		cerr << endl << "---------- connect to server"
			<< inet_ntoa(serverAddr.sin_addr)
			<< " : "
			<< ntohs(serverAddr.sin_port)
			<< " succussful !*************"
			<< endl;



		while (true)
		{
			cout << endl << "-Please input information to send" << endl << ">>";
			cin.getline(buffer, BUFFER_LENGTH - 1, '\n');
			if (1 == strlen(buffer)  &&  'q' == buffer[0] )
			{
				send(sock, bufQuit, sizeof(bufQuit), 0);
				cout << endl << "************* Client will shutdown *************" << endl;
				break;
			}

			nRet = send(sock, buffer, sizeof(buffer), 0);
			if (SOCKET_ERROR != nRet)
			{
				cout << endl << "----------- Send to " << pServerIP << " : " << usServerPort << "  success  ! --------" << endl;
			}
			else
			{
				cerr << endl << "************ Send to " << pServerIP << " : " << usServerPort << " failed  ! **********" << endl;
				int nErrCode = WSAGetLastError();
				cout << "Last Error is " << nErrCode << endl << endl;
			} //else

			memset(buffer, 0, sizeof(buffer));
			cout << endl;

		} //while
		
	} //else
	


	closesocket(sock);
	WSACleanup();	

	system("pause");

	return 0;
}

















