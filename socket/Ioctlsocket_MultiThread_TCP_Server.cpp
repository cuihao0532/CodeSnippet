/************************************************************************/
/* 基于非阻塞模式的多线程服务器应用程序编程实例                                                                     */
/************************************************************************/
#include "stdafx.h"
#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#pragma  comment(lib, "ws2_32.lib")

using namespace std;

#define  LOW_VERSION   2
#define  HIGHT_VERSION 2

unsigned int __stdcall WorkThread(void* p);

typedef struct _tagParam
{
	SOCKET sock;
	struct sockaddr_in skAddr;

}Param;

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	SOCKET skServer;
	SOCKET skConnect;
	const char* pServerIP = "127.0.0.1";
	unsigned short usPort = 9901;
	struct sockaddr_in  serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(usPort);
	serverAddr.sin_addr.S_un.S_addr = inet_addr(pServerIP);
	int nLenOfServerAddr = sizeof(struct sockaddr_in);

	struct sockaddr_in clientAddr;
	int nLenOfClientAddr = sizeof(struct sockaddr_in);

	const int THREAD_COUNT = 100;
	HANDLE hThreads[THREAD_COUNT] = {INVALID_HANDLE_VALUE};  //线程数组
	long nCount = 0;

	//////////////////////////////////////////////////////////////////////////

	if (0 != WSAStartup(MAKEWORD(LOW_VERSION, HIGHT_VERSION), &wsaData))
	{
		cerr << endl << "WSAStartup Error" << endl;
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////

	skServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == skServer)
	{
		cerr << endl << "socket error !" << endl;
		WSACleanup();
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////

	unsigned long ulArgp = 1UL;
	int nRet = ioctlsocket(skServer, FIONBIO, &ulArgp);
	if (SOCKET_ERROR == nRet)
	{
		cerr << endl << "ioctlsocket error " << endl;
		closesocket(skServer);
		WSACleanup();
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////

	if (0 != bind(skServer, (const struct sockaddr*)&serverAddr, nLenOfServerAddr))
	{
		cerr << endl << "bind error" << endl;
		closesocket(skServer);
		WSACleanup();
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////
	//检查监听状态
	int nListenState = 0;
	int nOptVal = 0;
	int nLenOfListenState = sizeof(nOptVal);
	nListenState = getsockopt(skServer, SOL_SOCKET, SO_ACCEPTCONN, (char*)&nOptVal, &nLenOfListenState);
	if (0 == nListenState)
	{
		cout << endl << "*********** The Listen state of socket " << skServer << " is " << nOptVal << " ***************" << endl;
	}
	else
	{
		cerr << endl << "**********Get The listen state of socket " << skServer << " failed ! *************" << endl;
	}

	//////////////////////////////////////////////////////////////////////////
	if (0 != listen(skServer, 1))
	{
		cerr << endl << "listen error" << endl;
		closesocket(skServer);
		WSACleanup();
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////
	//再次检查监听状态
	nListenState = getsockopt(skServer, SOL_SOCKET, SO_ACCEPTCONN, (char*)&nOptVal, &nLenOfListenState);
	if (0 == nListenState)
	{
		cout << endl << "*********** The Listen state of socket " << skServer << " is " << nOptVal << " ***************" << endl;
	}
	else
	{
		cerr << endl << "**********Get The listen state of socket " << skServer << " failed ! *************" << endl;
	}

	//////////////////////////////////////////////////////////////////////////

	cout << endl << "---------- I am waiting for a client ---------" << endl;
	while(true)
	{
		skConnect = accept(skServer, (struct sockaddr*)&clientAddr, &nLenOfClientAddr);
		if (INVALID_SOCKET == skConnect)
		{
			int nErrCode = WSAGetLastError();
			if (WSAEWOULDBLOCK == nErrCode)
			{
				Sleep(10);
				continue;
			} //if
			
		} //if

		cout << endl << "-------- Client IP " << inet_ntoa(clientAddr.sin_addr) \
			<< ", port " << ntohs(clientAddr.sin_port)                       \
			<< " is coming ---------" << endl << endl;


		if (nCount >= THREAD_COUNT)
		{
			closesocket(skConnect);
			continue;
		}
		
		//线程参数
		Param param;
		param.sock = skConnect;

		struct  sockaddr_in skAddr;
		memcpy_s(&param.skAddr, sizeof(param.skAddr), &clientAddr, sizeof(clientAddr));

		hThreads[nCount] = (HANDLE)_beginthreadex(
			NULL,
			0,
			WorkThread,
			(void*)(&param),	
			0,
			NULL);

		cerr << endl << "************ Create thread for client "
			<< inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port);

		//创建线程失败
		if (NULL == hThreads[nCount] || INVALID_HANDLE_VALUE == hThreads[nCount])
		{
			closesocket(skConnect);
			cerr << " Error******************";
		}
		else
		{
			cerr << " success !******************";
			InterlockedExchangeAdd(&nCount, 1L);
		}

		cout << endl << endl;

	} //while

	

	//////////////////////////////////////////////////////////////////////////

	return 0;
}




unsigned int __stdcall WorkThread(void* p)
{
	const int BUFFER_LENGTH = 1024;
	char buffer[BUFFER_LENGTH] = {0};
	char bufQuit[] = "quit";
	
	Param param;
	memcpy_s(&param, sizeof(param), ((Param*)p), sizeof(Param));

	SOCKET skConnect = param.sock;
	struct sockaddr_in clientAddr = param.skAddr;

	unsigned short  usPort = ntohs(clientAddr.sin_port);
	ADDRESS_FAMILY addrFamily = clientAddr.sin_family;
	char* pAddrIP = inet_ntoa(clientAddr.sin_addr);

	while(true)
	{
		int nRecv = recv(skConnect, buffer, sizeof(buffer), 0);
		if (SOCKET_ERROR == nRecv || 0 == nRecv)
		{
			int nErrCode = WSAGetLastError();
			if (WSAEWOULDBLOCK == nErrCode)
			{
				Sleep(10);
				continue;
			} //if
			else //if(WSAETIMEDOUT == nErrCode || WSAENETDOWN == nErrCode || WSAECONNRESET == nErrCode)
			{
				cerr << endl << "*********** recv error , maybe the connection has broken **********"
					 << endl 
					 << "---maybe client " << pAddrIP << ":" << usPort << " offline --------"
					 << endl << endl;

				break;	
			} //else

		} //if

		if (0 == strcmp(buffer, bufQuit))
		{
			break;			
		}

		cout << endl;

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

	} //while

	closesocket(skConnect);


	return 0;
}

