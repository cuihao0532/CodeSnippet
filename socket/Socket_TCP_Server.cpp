//Socket1.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#pragma  comment(lib, "ws2_32.lib")

using namespace std;

#define  LOW_VERSION   2
#define  HIGHT_VERSION 2

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData = {0};
	SOCKET sock = 0;
	const char* pServerIP = "127.0.0.1";
	unsigned short usPort = 9901;

	if (0 != WSAStartup(MAKEWORD(LOW_VERSION, HIGHT_VERSION), &wsaData))
	{
		cerr << endl << "-----------Startup Error----------" << endl;
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sock)
	{
		cerr << endl << "---------- socket Error ---------" << endl;
		WSACleanup();
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////
	int len = sizeof(struct sockaddr_in);
	struct sockaddr_in skaddr;
	skaddr.sin_family = AF_INET;
	skaddr.sin_port = htons(usPort);
	//skaddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	skaddr.sin_addr.S_un.S_addr = inet_addr(pServerIP);
	

	if(0 != bind(sock, (const struct sockaddr*)&skaddr, len))
	{
		closesocket(sock);
		WSACleanup();
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////
	//检查监听状态
	int nOptVal = 0;
	int nLenOfOptVal = sizeof(nOptVal);
	int nListenState = getsockopt(sock, SOL_SOCKET, SO_ACCEPTCONN, (char*)&nOptVal, &nLenOfOptVal);
	if (0 == nListenState)
	{
		cout << endl << "************* Listen State of socket " << sock << " is " << nOptVal << " ************" << endl;
	}
	else
	{
		cerr << endl << "************* Get Listen State of socket " << sock << " failed ! **********" << endl;
	}

	
	//////////////////////////////////////////////////////////////////////////

	if (0 != listen(sock, 1))
	{
		closesocket(sock);
		WSACleanup();
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////
	//再次检查监听状态
	nListenState = getsockopt(sock, SOL_SOCKET, SO_ACCEPTCONN, (char*)&nOptVal, &nLenOfOptVal);
	if (0 == nListenState)
	{
		cout << endl << "************* Listen State of socket " << sock << " is " << nOptVal << " ************" << endl;
	}
	else
	{
		cerr << endl << "************* Get Listen State of socket " << sock << " failed ! **********" << endl;
	}

	//////////////////////////////////////////////////////////////////////////

	struct sockaddr_in inAddr;
	int lenOfInAddr = sizeof(struct sockaddr_in);
	SOCKET sockIn = 0;
	const int BUFFER_LENGTH = 1024;
	const int BUFFER_WIDE_LENGTH = BUFFER_LENGTH * 2 + 2;
	char buffer[BUFFER_LENGTH] = {0};
	wchar_t bufWide[BUFFER_WIDE_LENGTH] = {0};
	int sizeOfBuffer = sizeof(char) * BUFFER_LENGTH;

	cout << endl << "--------- I am waiting for client -----------" << endl;

	sockIn = accept(sock, (sockaddr*)&inAddr, &lenOfInAddr);
	if (INVALID_SOCKET == sockIn)
	{
		cerr << endl << "********** accpet failed ! **************" << endl << endl;
	}
	cout << endl << "---------Client " << inet_ntoa(inAddr.sin_addr) \
		 << " : " << ntohs(inAddr.sin_port)                          \
		 << "  is coming --------------" << endl; 

	char bufQuit[] = "quit";
	while(true)
	{
		int nRev = recv(sockIn, buffer, sizeof(buffer), 0);
		if (0 == nRev ||  SOCKET_ERROR == nRev)
		{
			int nErrCode = WSAGetLastError();
			cerr << endl << "*************** Last Error is " << nErrCode << " ******************" << endl << endl;
			break;
		}	

		if (0 == strcmp(bufQuit, buffer)) 
			break;

		//MultiByteToWideChar(CP_ACP, 0, buffer, sizeof(buffer), bufWide, BUFFER_WIDE_LENGTH);
		//wcout << endl << bufWide << endl;
		//memset(bufWide, 0, sizeof(bufWide));


		cout << endl;
		unsigned short  usPort = ntohs(inAddr.sin_port);
		ADDRESS_FAMILY addrFamily = inAddr.sin_family;
		char* pAddrIP = inet_ntoa(inAddr.sin_addr);
		cout << endl << "--------- Receive From ----------" \
			<< endl                                         \
			<< "IP : " << pAddrIP << ", Port : " << usPort  \
			<< ", AddressFamily : " << addrFamily           \
			<< endl                                         \
			<< "content length is " << strlen(buffer)       \
			<< endl                                         \
			<< "content is : " << buffer                    \
			<< endl                                         \
			<< "------------*************--------------" << endl << endl << endl;

		memset(buffer, 0, sizeof(buffer));
	}

	cout << endl << "************* server will shutdown *************" << endl;

	closesocket(sockIn);
	closesocket(sock);
	WSACleanup();

	system("pause");
	return 0;
}

