// Overlapped_TCP_Server.cpp : 定义控制台应用程序的入口点。
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
	bool            bOccupied[WSA_MAXIMUM_WAIT_EVENTS];          //当前位置是否被占用
	bool            bHasMemory[WSA_MAXIMUM_WAIT_EVENTS];         //当前位置是否分配过内存
	int             nCount;

	struct _tagOverlappedParam() 
	{
		memset(pWSABuf, 0, WSA_MAXIMUM_WAIT_EVENTS * sizeof(pWSABuf[0]));
		memset(wsaEvents, 0, WSA_MAXIMUM_WAIT_EVENTS * sizeof(wsaEvents[0]));
		memset(pWSAOverlapped, 0, WSA_MAXIMUM_WAIT_EVENTS * sizeof(pWSAOverlapped[0]));
		memset(sockets, 0, WSA_MAXIMUM_WAIT_EVENTS * sockets[0]);
		memset(bOccupied, 0, WSA_MAXIMUM_WAIT_EVENTS * sizeof(bOccupied[0]));
		memset(bHasMemory, 0, WSA_MAXIMUM_WAIT_EVENTS * sizeof(bHasMemory[0]));

		nCount = 0;
	}
}Overlappedparam, *POverlappedParam;

Overlappedparam g_ovp;

bool g_bFlag = false;
SRWLOCK g_srwLock;

unsigned int __stdcall WorkThread(void* p);
unsigned int __stdcall RecvThread(void* p);
int ExitsSocket(const Overlappedparam&  ovp,  SOCKET sock);
int AvailableCounts(const Overlappedparam& ovp, size_t& nFirstAvailable, bool& bHasMemory);
int FirstAvailable(const Overlappedparam& ovp);



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



	InitializeSRWLock(&g_srwLock);


	while(true)
	{

		SOCKET sockAccept = accept(skListen, (sockaddr*)&sockAddr, &nLenOfAddr);
		if (INVALID_SOCKET == sockAccept )
		{
			int nErrCode = WSAGetLastError();
			continue;
		}
		

		AcquireSRWLockExclusive(&g_srwLock);



		int nFirstAvailable  = 0;     //first available pos
		nFirstAvailable = FirstAvailable(g_ovp);

		//no available 
		if (nFirstAvailable < 0)
		{
			closesocket(sockAccept);
			ReleaseSRWLockExclusive(&g_srwLock);
			continue;
		}


		/************************************************************************/
		/* 判断该第一个可用的位置是已经内分配过内存
		 * Judge if the first pos has memory once ever
		/************************************************************************/

		//has not memory
		if (false == g_ovp.bHasMemory[nFirstAvailable])
		{

			g_ovp.sockets[nFirstAvailable] = sockAccept;

			//////////////////////////////////////////////////////////////////////////

			LPWSABUF pWSABuf = new WSABUF();
			ZeroMemory(pWSABuf, sizeof(WSABUF));
			char* pBuffer = new char[BUFFER_SIZE]();
			pWSABuf->len = BUFFER_SIZE * sizeof(char);
			pWSABuf->buf = pBuffer;

			g_ovp.pWSABuf[nFirstAvailable] = pWSABuf;

			//////////////////////////////////////////////////////////////////////////

			LPWSAOVERLAPPED pWSAOverlapped = new WSAOVERLAPPED();
			ZeroMemory(pWSAOverlapped, sizeof(WSAOVERLAPPED));
			WSAEVENT hEvent = WSACreateEvent();
			pWSAOverlapped->hEvent = hEvent;		

			g_ovp.pWSAOverlapped[nFirstAvailable] = pWSAOverlapped;
			g_ovp.wsaEvents[nFirstAvailable] = hEvent;

			//////////////////////////////////////////////////////////////////////////

			g_ovp.bOccupied[nFirstAvailable] = true;  //current pos occupied
			g_ovp.bHasMemory[nFirstAvailable] = true;    //current pos has memory
			
		}

		//has memory
		else
		{
			ZeroMemory(g_ovp.pWSABuf[nFirstAvailable]->buf, g_ovp.pWSABuf[nFirstAvailable]->len);

			{
				ZeroMemory(g_ovp.pWSAOverlapped[nFirstAvailable], sizeof(WSAOVERLAPPED));
				g_ovp.pWSAOverlapped[nFirstAvailable]->hEvent = g_ovp.wsaEvents[nFirstAvailable];

				WSAResetEvent(g_ovp.wsaEvents[nFirstAvailable]);
			}

			g_ovp.sockets[nFirstAvailable] = sockAccept;

			g_ovp.bOccupied[nFirstAvailable] = true; // the pos has been ocuppyied
		}


		//////////////////////////////////////////////////////////////////////////

		ReleaseSRWLockExclusive(&g_srwLock);




		int nRet = WSARecv(
			g_ovp.sockets[nFirstAvailable],
			g_ovp.pWSABuf[nFirstAvailable], 
			1, 
			NULL, 
			&dwFlags, 
			g_ovp.pWSAOverlapped[nFirstAvailable],
			NULL);



		if (! g_bFlag)
		{
			g_bFlag = true;


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
	DWORD dwTransfer = 0;
	DWORD dwFlags = 0;

	while(true)
	{
		size_t nCount = 0;


		AcquireSRWLockExclusive(&g_srwLock);

		WSAEVENT wsaEvents[WSA_MAXIMUM_WAIT_EVENTS] = {0};
		for (size_t i = 0; i < WSA_MAXIMUM_WAIT_EVENTS; ++ i)
		{
			if (true == g_ovp.bOccupied[i])
			{
				wsaEvents[i] = g_ovp.wsaEvents[i];
				++nCount;
			}
		}

		//DWORD dwIndex = WSAWaitForMultipleEvents(
		//	2, //WSA_MAXIMUM_WAIT_EVENTS, //g_ovp.nCount,
		//	g_ovp.wsaEvents,
		//	FALSE,
		//	WSA_INFINITE,
		//	FALSE);

		DWORD dwIndex = WSAWaitForMultipleEvents(
			nCount,
			wsaEvents,
			FALSE,
			0, //WSA_INFINITE,
			FALSE);


		ReleaseSRWLockExclusive(&g_srwLock);



		if (WSA_WAIT_TIMEOUT == dwIndex || WSA_WAIT_FAILED == dwIndex)
		{
			//error
			continue;
		}

		dwIndex = dwIndex - WSA_WAIT_EVENT_0;

		BOOL bRet = WSAGetOverlappedResult(
			g_ovp.sockets[dwIndex],
			g_ovp.pWSAOverlapped[dwIndex],
			&dwTransfer,
			FALSE,
			&dwFlags );


		WSAResetEvent(g_ovp.wsaEvents[dwIndex]);


		if (TRUE == bRet)
		{
			cout << endl
				 << g_ovp.pWSABuf[dwIndex]->buf << endl << endl;			
		
			int nRet = WSARecv(
				g_ovp.sockets[dwIndex],
				g_ovp.pWSABuf[dwIndex], 
				1, 
				NULL, 
				&dwFlags, 
				g_ovp.pWSAOverlapped[dwIndex],
				NULL);
		}


		else
		{
			int nErrCode = WSAGetLastError();
			cout << endl << endl
				 << "----- WSAGetOverlappedResult error ----"
				 << endl;
			if (WSAECONNRESET == nErrCode 
				|| WSAECONNABORTED == nErrCode || 0 == dwTransfer)
			{
				cout << "socket broken " << endl << endl;
			}

			g_ovp.bOccupied[dwIndex] = false;
		}

		dwFlags = 0;
		ZeroMemory(g_ovp.pWSABuf[dwIndex]->buf, g_ovp.pWSABuf[dwIndex]->len);

		{
			ZeroMemory(g_ovp.pWSAOverlapped[dwIndex], sizeof(WSAOVERLAPPED));
			g_ovp.pWSAOverlapped[dwIndex]->hEvent = g_ovp.wsaEvents[dwIndex]; 
		}


		
	} //while(true)


	


	return 0;
}



int ExitsSocket(const Overlappedparam&  ovp,  SOCKET sock)
{
	for (size_t i = 0; i < ovp.nCount; ++ i)
	{
		if (sock == ovp.sockets[i])
		{
			return i;
		}
	}

	return -1;
}


/************************************************************************/
/* nFirstAvaiable : the first available pos in array
 * bHasMemory     : does the first available pos has memory once ever

 * return         : total available pos in array
/************************************************************************/
int  AvailableCounts(const Overlappedparam& ovp, int& nFirstAvailable, bool& bHasMemory)
{
	int nCount = 0;
	bool bFlag = false;

	for (int i = 0; i < WSA_MAXIMUM_WAIT_EVENTS; ++ i)
	{
		if (false == ovp.bOccupied[i])
		{
			++ nCount;
			if (! bFlag)
			{
				bFlag = true;
				nFirstAvailable = i;

				bHasMemory = ovp.bHasMemory[i];
			}
		}
	}

	return nCount;
}



 int FirstAvailable(const Overlappedparam& ovp)
{
	for (int i = 0; i < WSA_MAXIMUM_WAIT_EVENTS; ++ i)
	{
		if (false == ovp.bOccupied[i])
			return i;
	}
	
	return -1;
}