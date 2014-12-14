// IOCP_TCP_Server.cpp : 定义控制台应用程序的入口点。
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
using namespace std;

#include "H:\\JSONCPP\\jsoncpp-master\\include\\json\\json.h"
#pragma  comment(lib, "ws2_32.lib")
#pragma  comment(lib, "H:\\JSONCPP\\jsoncpp-master\\makefiles\\msvc2010\\Debug\\lib_json.lib")

#define  LOW_VERSION       2
#define  HIGHT_VERSION     2
#define  BUFFER_SIZE       1024 //buffer size of IO
#define  ZERO_PLACEHOLDER  0    //just a placeholder for function overload

HANDLE                 CreateNewIOCP(DWORD dwNumberOfConcurrentThreads = 0 /*default: amount of CPU */);
BOOL                   AssociateDeviceWithIOCP(HANDLE hIOCP, HANDLE hDevice, DWORD dwCompletionKey);
unsigned int __stdcall RecvThread(void* p);
void                   ShowMsg(SOCKET sock, struct sockaddr_in* pSockAddr, bool b = true);
void                   ShowMsg2(SOCKET sock, const char* pBuffer, struct sockaddr_in* pAddr);  


typedef struct _tagSockParam
{
	OVERLAPPED         ovp;
	SOCKET             sock;
	DWORD              dwBytesTransferred;
	DWORD              dwFlags;
	WSABUF             wsaBuf;
	struct sockaddr_in clientAddr;
	int                nLenOfSockAddr;
	static  struct sockaddr_in sockZeroAddr;

	/************************************************************************
	 * bOccupied default value is TRUE
	 * if bOccupied == false,  then a new client socket can use the wsaBuf 
	 * memory directly.
	 * ------------------
	 * A sapce for time.
	 ************************************************************************/
	bool               bOccupied;       


	struct _tagSockParam(SOCKET clientSock, struct sockaddr_in clientAddress)
	{
		try
		{
			wsaBuf.buf = new char[BUFFER_SIZE]();
			if (NULL == wsaBuf.buf)
			{
				throw(std::bad_alloc("alloc memory error"));
			}	


			Reset(ZERO_PLACEHOLDER, clientSock, clientAddress);			
		}
		catch (std::bad_alloc& e)
		{
			cerr << endl << __FILE__ 
				 << endl
				 << __LINE__
				 << endl
				 << e.what()
				 << endl;
		}//catch	
	}

	

	void Reset(bool b = true)
	{	
		ZeroMemory(&ovp, sizeof(OVERLAPPED));			
		memset(wsaBuf.buf, 0, wsaBuf.len);		
		dwBytesTransferred = 0;

		if (! b)
		{
			ZeroMemory(&clientAddr, sizeof(clientAddr));
		}

		bOccupied = b;
	}


	void ReUse(SOCKET sock, struct sockaddr_in clientAddress)
	{
		Reset(ZERO_PLACEHOLDER, sock, clientAddress);
	}
	


	~_tagSockParam()
	{
		if (NULL != wsaBuf.buf)
		{
			delete[] wsaBuf.buf;
			wsaBuf.buf = NULL;
		}
	}
	

private:
	void Reset(int nReserved /*just for function overload*/, SOCKET sockClient = 0, struct sockaddr_in clientAddress = sockZeroAddr)
	{
		ZeroMemory(&ovp, sizeof(OVERLAPPED));			
		ZeroMemory(&clientAddr, sizeof(clientAddr));
		memset(wsaBuf.buf, 0, sizeof(wsaBuf.len));		
		dwBytesTransferred = 0;
		bOccupied = true;

		wsaBuf.len = BUFFER_SIZE * sizeof(char);
		sock = sockClient;
		dwFlags = 0;
		nLenOfSockAddr = sizeof(clientAddr);
		clientAddr = clientAddress;
	}

	_tagSockParam(const _tagSockParam&){}
	_tagSockParam& operator=(const _tagSockParam&){}


}SockParam, *PSockParam;

struct sockaddr_in SockParam::sockZeroAddr = {0};



/************************************************************************/
/* Record the set of SockParam pointers
 * in order to save time, which no need to free memory and alloc memory 
 * multi time if there still memory usable.
/************************************************************************/
typedef std::vector<PSockParam>  VEC_SOCKET;
VEC_SOCKET g_vecSock;






/************************************************************************/
/* find a pointer to a SockParam object which [bOccupied] is false
/************************************************************************/
PSockParam FirstAvailable(const VEC_SOCKET& vecSock);




int _tmain(int argc, _TCHAR* argv[])
{
	
#pragma  region 变量

	SYSTEM_INFO sysInfo = {0};	
	WSADATA wsaData;
	const char* pServerIP = "127.0.0.1";
	unsigned short usPort = 9901;
	int nRet = 0;
	SOCKET skListen;
	struct sockaddr_in clientAddr;
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr.S_un.S_addr = inet_addr(pServerIP);
	clientAddr.sin_port = htons(usPort);
	int nLenOfAddr = sizeof(struct sockaddr_in);
	HANDLE hIOCP = NULL;
	DWORD dwNumOfCPU = 0;

	DWORD dwFlags = 0;
	int nIndex = 0;
	bool bFlag = false;

#pragma  endregion


#pragma region  初始化SOCKET

	GetSystemInfo(&sysInfo);

	if (0 != WSAStartup(MAKEWORD(LOW_VERSION, HIGHT_VERSION), &wsaData))
	{
		cerr << endl << "WSAStartup error" << endl;
		return -1;
	}


	skListen = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == skListen)
	{
		WSACleanup();
		cerr << endl << "socket error" << endl;
		return -1;
	}

	


	if (0 != bind(skListen, (const sockaddr*)&clientAddr, nLenOfAddr))
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


	hIOCP = CreateNewIOCP();
	if (NULL == hIOCP)
	{
		closesocket(skListen);
		WSACleanup();
		return -1;
	}

	GetSystemInfo(&sysInfo);
	dwNumOfCPU = sysInfo.dwNumberOfProcessors;

	




	while(true)
	{
		//circle accept connect of clients
		SOCKET sockAccept = accept(skListen, (sockaddr*)&clientAddr, &nLenOfAddr);
		if (INVALID_SOCKET == sockAccept)
		{
			int nErrCode = WSAGetLastError();
			continue;
		}


		ShowMsg(sockAccept, &clientAddr);

		BOOL bRet = AssociateDeviceWithIOCP(hIOCP, (HANDLE)(sockAccept), (DWORD)(sockAccept));
		if (FALSE == bRet)
		{
			closesocket(sockAccept);
			ShowMsg(sockAccept, &clientAddr, false);
			continue;			
		}

		//find a SockParam object which [bOccupied] is false
		PSockParam pSockTemp = FirstAvailable(g_vecSock);
		PSockParam pSockParam = NULL;

		//no avaiable 
		if (NULL == pSockTemp)
		{
			pSockParam = new SockParam(sockAccept, clientAddr);
			g_vecSock.push_back(pSockParam);
		}
		else
		{
			pSockParam = pSockTemp;
			pSockParam->ReUse(sockAccept, clientAddr);
		}


		

		int nRet = WSARecv(
			pSockParam->sock,	
			&(pSockParam->wsaBuf),
			1,
			&(pSockParam->dwBytesTransferred),
			&(pSockParam->dwFlags),
			(LPWSAOVERLAPPED)(pSockParam),
			NULL);

		if (! bFlag)
		{
			bFlag = true;

			for (DWORD i = 0; i < dwNumOfCPU * 2; ++ i)
			{
				HANDLE hThread = (HANDLE)_beginthreadex(
					NULL,
					0,
					RecvThread,
					(void*)(hIOCP),
					0,
					NULL);

				if (INVALID_HANDLE_VALUE == hThread || NULL == hThread)
				{
					//error
					break;
				}

			} //for

		} //if (! bFlag)
		
	} //while(true)


	/*for (VEC_SOCKET::const_iterator iter = g_vecSock.begin();
	iter != g_vecSock.end();
	++ iter)
	{
	if (true == (*iter)->bOccupied)
	{
	closesocket((*iter)->sock);
	delete (*iter);
	}
	}*/


	closesocket(skListen);
	WSACleanup();

	return 0;
}





//create a new IOCP with a default param 
HANDLE CreateNewIOCP(DWORD dwNumberOfConcurrentThreads /* = 0 , default: amount of CPU */)
{
	return CreateIoCompletionPort(
		INVALID_HANDLE_VALUE,         //hFile
		NULL,                         //existing IOCP
		0,                            //complete key
		dwNumberOfConcurrentThreads   //number of can be runable threads in thread pool
		);
}




//associate a device with an existing IOCP 
BOOL AssociateDeviceWithIOCP(HANDLE hIOCP, HANDLE hDevice, DWORD dwCompletionKey)
{
	return (hIOCP == CreateIoCompletionPort(hDevice, hIOCP, dwCompletionKey, 0));
}



//receive thread
unsigned int __stdcall RecvThread(void* p)
{
	if (NULL == p)
		return 0;

	HANDLE hIOCP = (HANDLE)(p);
	DWORD dwBytesOfTransferred = 0;
	DWORD dwCompletionKey = 0;
	PSockParam pSockParam = NULL;
	SOCKET sock = 0;
	DWORD dwErrCode = 0;

	while(true)
	{
		BOOL bRet = GetQueuedCompletionStatus(
			hIOCP,
			&dwBytesOfTransferred,
			&dwCompletionKey,
			(LPOVERLAPPED *)(& pSockParam),
			INFINITE
			);

		sock = (SOCKET)(dwCompletionKey);
		struct sockaddr_in& clientAddr = pSockParam->clientAddr;
		char* pBuf = pSockParam->wsaBuf.buf;

		dwErrCode = GetLastError();

		if (FALSE == bRet)
		{
			if (WAIT_TIMEOUT == dwErrCode)
			{
				continue;
			}
			else //if ()
			{
				ShowMsg(sock, &clientAddr, false);
				pSockParam->Reset(false);
				closesocket(sock);
				continue;
			}

		} //if

		else
		{
			ShowMsg2(sock, pBuf, &clientAddr);

			pSockParam->Reset(); //default param is TURE

			int nRet = WSARecv(
				sock,
				&(pSockParam->wsaBuf),
				1,
				&(pSockParam->dwBytesTransferred),
				&(pSockParam->dwFlags),
				(LPWSAOVERLAPPED)(pSockParam),
				NULL);


		} //else

	} //while(true)


	return 0;
}


void ShowMsg(SOCKET sock, struct sockaddr_in* pSockAddr, bool b /* = true*/)
{
	if (true == b)
	{
		cout << endl << "-------- The Client socket = " 
			<< sock << ",  "
			<< inet_ntoa(pSockAddr->sin_addr) 
			<< ":" << ntohs(pSockAddr->sin_port)
			<< "   is comming! --------" 
			<< endl << endl;

	}
	else
	{
		cout << endl << "-------- The Client socket = " 
			<< sock << ",  "
			<< inet_ntoa(pSockAddr->sin_addr) 
			<< ":" << ntohs(pSockAddr->sin_port)
			<< "   has been closed !" 
			<< endl << endl;
	}

}



void ShowMsg2(SOCKET sock, const char* pBuffer, struct sockaddr_in* pAddr)  
{
	Json::Reader reader;
	Json::Value root;

	cout << endl << endl
		<< "------- From "
		<< " socket "
		<< sock << ",   "
		<< inet_ntoa(pAddr->sin_addr)
		<< ":"
		<< ntohs(pAddr->sin_port)
		<< " -------"
		<< endl;

	if (reader.parse(pBuffer, root))
	{
		std::string strDate = root[0]["date"].asString();
		std::string strTime = root[0]["time"].asString();
		std::string strMsg = root[0]["content"].asString();

		cout << strDate << ", "
			<< strTime << endl
			<< strMsg << endl << endl;
	}
	else
	{
		cout << pBuffer << endl << endl;
	}


}


PSockParam FirstAvailable(const VEC_SOCKET& vecSock)
{
	PSockParam pSockParam = NULL;

	for (VEC_SOCKET::const_iterator iter = vecSock.begin();
		iter != vecSock.end();
		++ iter)
	{
		if (false == (*iter)->bOccupied)
		{
			return (pSockParam = *iter);
		}
	}

	return pSockParam;
}