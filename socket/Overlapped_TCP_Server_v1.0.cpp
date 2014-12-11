// Overlapped_TCP_Server.cpp : 定义控制台应用程序的入口点。

//this version maybe has some bugs
//
#include "stdafx.h"
#include <algorithm>
#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <string>
#include "H:\\JSONCPP\\jsoncpp-master\\include\\json\\json.h"
#pragma  comment(lib, "ws2_32.lib")
#pragma  comment(lib, "H:\\JSONCPP\\jsoncpp-master\\makefiles\\msvc2010\\Debug\\lib_json.lib")
#define  LOW_VERSION   2
#define  HIGHT_VERSION 2
#define  BUFFER_SIZE   1024

using namespace std;

typedef struct _tagSockParam
{
	SOCKET sock;
	struct sockaddr_in addr;
	bool bEnable;
	struct _tagSockParam() {bEnable = true;}

	bool operator==(const struct _tagSockParam& obj)
	{
		return (this->sock == obj.sock);
	}

}SockParam, *PSockParam;




typedef struct _tagOverlappedParam
{
	LPWSABUF        pWSABuf[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT        wsaEvents[WSA_MAXIMUM_WAIT_EVENTS];
	LPWSAOVERLAPPED pWSAOverlapped[WSA_MAXIMUM_WAIT_EVENTS];
	SOCKET          sockets[WSA_MAXIMUM_WAIT_EVENTS];
	size_t          nCount;

	struct _tagOverlappedParam() 
	{
		memset(pWSABuf, 0, WSA_MAXIMUM_WAIT_EVENTS * sizeof(pWSABuf[0]));
		memset(wsaEvents, 0, WSA_MAXIMUM_WAIT_EVENTS * sizeof(wsaEvents[0]));
		memset(pWSAOverlapped, 0, WSA_MAXIMUM_WAIT_EVENTS * sizeof(pWSAOverlapped[0]));
		memset(sockets, 0, WSA_MAXIMUM_WAIT_EVENTS * sockets[0]);
		nCount = 0;
	}
}Overlappedparam, *POverlappedParam;

Overlappedparam ovp;

bool g_bFlag = false;

unsigned int __stdcall WorkThread(void* p);
unsigned int __stdcall RecvThread(void* p);


int _tmain(int argc, _TCHAR* argv[])
{

#pragma  region 变量
	WSADATA wsaData;
	const char* pServerIP = "127.0.0.1";
	unsigned short usPort = 9901;
	int nRet = 0;
	SOCKET skListen;
	SOCKET skAccept;
	struct sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.S_un.S_addr = inet_addr(pServerIP);
	sockAddr.sin_port = htons(usPort);
	int nLenOfAddr = sizeof(struct sockaddr_in);

	DWORD dwFlags = 0;


	int nIndex = 0;
	typedef vector<SockParam> VEC_SOCKET;
	VEC_SOCKET vecSockets;

#pragma  endregion


#pragma region  初始化

	if (0 != WSAStartup(MAKEWORD(LOW_VERSION, HIGHT_VERSION), &wsaData))
	{
		cerr << endl << "WSAStartup error" << endl;
		return -1;
	}



	/*skListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == skListen)
	{
	WSACleanup();
	cerr << endl << "socket error" << endl;
	return -1;
	}*/


	skListen = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == skListen)
	{
		WSACleanup();
		cerr << endl << "socket error" << endl;
		return -1;
	}

	


	if (0 != bind(skListen, (const sockaddr*)&sockAddr, nLenOfAddr))
	{
		cerr << endl << "bind error" << endl;
		closesocket(skListen);
		WSACleanup();
		return -1;
	}


	int nListenState = 0;
	int nOptVal = 0;
	int nLenOfListenState = sizeof(nOptVal);
	nListenState = getsockopt(skListen, SOL_SOCKET, SO_ACCEPTCONN, (char*)&nOptVal, &nLenOfListenState);
	if (0 == nListenState)
	{
		cout << endl << "************* The listen state of socket "
			<< skListen << " is succss! --"
			<< nOptVal << endl;
	}
	else
	{
		cout << endl << "************* Tne listen state of socket "
			<< skListen << " is failed! ---- " << nOptVal << endl;
	}






	if (0 != listen(skListen, 1))
	{
		cerr << endl << "listen error" << endl;
		closesocket(skListen);
		WSACleanup();
		return -1;
	}



	cout << endl << endl;



	nListenState = getsockopt(skListen, SOL_SOCKET, SO_ACCEPTCONN, (char*)&nOptVal, &nLenOfListenState);
	if (0 == nListenState)
	{
		cout << endl << "************* The listen state of socket "<< skListen << " is succss! --"
			<< nOptVal << endl;
	}
	else
	{
		cout << endl << "************* Tne listen state of socket " 
			<< skListen << " is failed! ---- " << nOptVal << endl;
	}




#pragma  endregion





	while(true)
	{

		SOCKET sockAccept = accept(skListen, (sockaddr*)&sockAddr, &nLenOfAddr);
		if (INVALID_SOCKET == sockAccept)
		{
			int nErrCode = WSAGetLastError();
			continue;
		}

		ovp.sockets[ovp.nCount] = sockAccept;

		//////////////////////////////////////////////////////////////////////////

		LPWSABUF pWSABuf = new WSABUF();
		ZeroMemory(pWSABuf, sizeof(WSABUF));
		char* pBuffer = new char[BUFFER_SIZE]();
		pWSABuf->len = BUFFER_SIZE * sizeof(char);
		pWSABuf->buf = pBuffer;

		ovp.pWSABuf[ovp.nCount] = pWSABuf;

		//////////////////////////////////////////////////////////////////////////

		LPWSAOVERLAPPED pWSAOverlapped = new WSAOVERLAPPED();
		ZeroMemory(pWSAOverlapped, sizeof(WSAOVERLAPPED));
		WSAEVENT hEvent = WSACreateEvent();
		pWSAOverlapped->hEvent = hEvent;		

		ovp.pWSAOverlapped[ovp.nCount] = pWSAOverlapped;
		ovp.wsaEvents[nIndex] = hEvent;

		
		//////////////////////////////////////////////////////////////////////////


		//////////////////////////////////////////////////////////////////////////
		//int nRet = WSARecv(sockAccept, pWSABuf, 1, NULL, NULL, pWSAOverlapped, NULL);
		int nRet = WSARecv(
			ovp.sockets[nIndex],
			ovp.pWSABuf[nIndex], 
			1, 
			NULL, 
			&dwFlags, 
			ovp.pWSAOverlapped[ovp.nCount],
			NULL);

		++ (ovp.nCount);

		


		if (! g_bFlag)
		{
			g_bFlag = true;

			/*HANDLE hRecv = (HANDLE)_beginthreadex(
			NULL,
			0,
			RecvThread,
			NULL,
			0,
			NULL);

			if (INVALID_HANDLE_VALUE == hRecv || NULL == hRecv)
			{
			break;
			}*/



			HANDLE hThread = (HANDLE)_beginthreadex(
				NULL,
				0,
				WorkThread,
				NULL,
				0,
				NULL);

			if (INVALID_HANDLE_VALUE == hThread || NULL == hThread)
			{
				break;
			}
		}



		if (0 != nRet || 
			(SOCKET_ERROR == nRet && WSA_IO_PENDING != WSAGetLastError())
			)
		{
			//error
			continue;
		}


		

	} //while(true)


	closesocket(skListen);
	WSACleanup();

	return 0;
}



unsigned int __stdcall WorkThread(void* p)
{
	WSAOVERLAPPED wsaOverlapped;
	ZeroMemory(&wsaOverlapped, sizeof(wsaOverlapped));
	DWORD dwTransfer = 0;
	DWORD dwFlags = 0;

	while(true)
	{
		size_t nCount = ovp.nCount;

		DWORD dwIndex = WSAWaitForMultipleEvents(
			ovp.nCount,
			ovp.wsaEvents,
			FALSE,
			WSA_INFINITE,
			FALSE);

		if (WSA_WAIT_TIMEOUT == dwIndex || WSA_WAIT_FAILED == dwIndex)
		{
			//error
			continue;
		}

		dwIndex = dwIndex - WSA_WAIT_EVENT_0;

		BOOL bRet = WSAGetOverlappedResult(
			ovp.sockets[dwIndex],
			&wsaOverlapped,
			&dwTransfer,
			FALSE,
			&dwFlags );

		if (TRUE == bRet)
		{
			cout << endl
				 << ovp.pWSABuf[dwIndex]->buf << endl << endl;			
		}
		else
		{
			cout << endl << endl
				 << "----- WSAGetOverlappedResult error ----"
				 << endl << endl;
		}

		WSAResetEvent(ovp.wsaEvents[dwIndex]);
		
		
	} //while(true)


	


	return 0;
}


unsigned int __stdcall RecvThread(void* p)
{
	DWORD dwFlags = 0;

	while(true)
	{
		size_t nCount = ovp.nCount;

		for (size_t i = 0; i < nCount; ++ i)
		{
			int nRet = WSARecv(
				ovp.sockets[i],
				ovp.pWSABuf[i], 
				1, 
				NULL, 
				&dwFlags, 
				ovp.pWSAOverlapped[ovp.nCount],
				NULL);

		} //for

		Sleep(10);

	} //while

	
}