/************************************************************************/
/* ���ڷ�����ģʽ�Ķ��̷߳�����Ӧ�ó�����ʵ��         
*  Non-block socket of TCP Server based on Multi-threads.
/************************************************************************/
#include "stdafx.h"
#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#include <stdlib.h>
#include <string>
#include "H:\\JSONCPP\\jsoncpp-master\\include\\json\\json.h"
#pragma  comment(lib, "ws2_32.lib")
#pragma  comment(lib, "H:\\JSONCPP\\jsoncpp-master\\makefiles\\msvc2010\\Debug\\lib_json.lib")

using namespace std;

#define  LOW_VERSION   2
#define  HIGHT_VERSION 2

unsigned int __stdcall WorkThread(void* p);

typedef struct _tagParam
{
	struct _tagParam() {}
	SOCKET sock;                  //client socket
	struct sockaddr_in skAddr;    //client address
}Param;

long g_nCountOfCurrentConns = 0;  //current connections of client
SRWLOCK g_srwLock;


int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	SOCKET skServer;
	SOCKET skConnect;
	const char* pServerIP = "127.0.0.1";                  //IP
	unsigned short usPort = 9901;                         //PORT

	struct sockaddr_in  serverAddr;                       //Server Address
	serverAddr.sin_family = AF_INET;                      //Address Family
	serverAddr.sin_port = htons(usPort);                  //Port, host -> net
	serverAddr.sin_addr.S_un.S_addr = inet_addr(pServerIP); 
	int nLenOfServerAddr = sizeof(struct sockaddr_in);    

	struct sockaddr_in clientAddr;                        //client address
	int nLenOfClientAddr = sizeof(struct sockaddr_in);

	const int THREAD_COUNT = 3;                            //The limit of client
	HANDLE hThreads[THREAD_COUNT] = {INVALID_HANDLE_VALUE};  //Thread handle array



	/************************************************************************/
	/* ��ʼ��SOCKET, Initial SOCKET
	/************************************************************************/

	if (0 != WSAStartup(MAKEWORD(LOW_VERSION, HIGHT_VERSION), &wsaData))
	{
		cerr << endl << "WSAStartup Error" << endl;
		return -1;
	}




	/************************************************************************/
	/* ����SOCKET, Create SOCKET
	/************************************************************************/

	skServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == skServer)
	{
		cerr << endl << "socket error !" << endl;
		WSACleanup();
		return -1;
	}



	/************************************************************************/
	/* ����SOCKETΪ������, Set socket as non-block
	/************************************************************************/

	unsigned long ulArgp = 1UL;
	int nRet = ioctlsocket(skServer, FIONBIO, &ulArgp);
	if (SOCKET_ERROR == nRet)
	{
		cerr << endl << "ioctlsocket error " << endl;
		closesocket(skServer);
		WSACleanup();
		return -1;
	}



	/************************************************************************/
	/* ��SOCKET�����ص�ַ, Bind SOCKET to local address
	/************************************************************************/

	if (0 != bind(skServer, (const struct sockaddr*)&serverAddr, nLenOfServerAddr))
	{
		cerr << endl << "bind error" << endl;
		closesocket(skServer);
		WSACleanup();
		return -1;
	}

	
	/************************************************************************/
	/* ��鵱ǰ����״̬, Check current listen state
	/************************************************************************/
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




	/************************************************************************/
	/* ����Ϊ����״̬, Set listen state
	/************************************************************************/
	if (0 != listen(skServer, 1))
	{
		cerr << endl << "listen error" << endl;
		closesocket(skServer);
		WSACleanup();
		return -1;
	}




	/************************************************************************/
	/* �ٴμ�����״̬, Check the listen state again.
	/************************************************************************/
	nListenState = getsockopt(skServer, SOL_SOCKET, SO_ACCEPTCONN, (char*)&nOptVal, &nLenOfListenState);
	if (0 == nListenState)
	{
		cout << endl << "*********** The Listen state of socket " << skServer << " is " << nOptVal << " ***************" << endl;
	}
	else
	{
		cerr << endl << "**********Get The listen state of socket " << skServer << " failed ! *************" << endl;
	}





	/************************************************************************/
	/* ��ʼ����д��, Initialize Slim Read/Write lock                                                                     */
	/************************************************************************/
	InitializeSRWLock(&g_srwLock);



	/************************************************************************/
	/* ѭ���ȴ��ͻ������ӣ� Waiting for client to connect circly.
	/************************************************************************/
	cout << endl << "---------- I am waiting for a client ---------" << endl;
	while(true)
	{
		skConnect = accept(skServer, (struct sockaddr*)&clientAddr, &nLenOfClientAddr);
		if (INVALID_SOCKET == skConnect)
		{
			int nErrCode = WSAGetLastError();

			//non-block accect has not completed, go on waiting.
			if (WSAEWOULDBLOCK == nErrCode)
			{
				Sleep(10);
				continue;
			} //if
			
		} //if
		






		/************************************************************************/
		/* �����ʹ��g_nCountOfCurrentConns
		 * Use g_nCountOfCurrentConns with thread safe
		/************************************************************************/
		AcquireSRWLockExclusive(&g_srwLock);
		{
			//Show client message and the current connections
			cout << endl << "-------- Client IP " << inet_ntoa(clientAddr.sin_addr) \
				<< ", port " << ntohs(clientAddr.sin_port)                       \
				<< " is coming ---------"
				<< endl
				<< "------- Current Connections are " << g_nCountOfCurrentConns + 1<< " ---------" << endl << endl;



			//Check current mounts of current connections, if has reach the limite.
			if (g_nCountOfCurrentConns >= THREAD_COUNT)
			{
				cout << endl << "-----------Reach the limit of connections " << THREAD_COUNT << " -------------" << endl << endl;
				closesocket(skConnect);
				ReleaseSRWLockExclusive(&g_srwLock);
				continue;
			}




			//Thread param to pass to the thread function
			Param param;
			param.sock = skConnect;
			memcpy_s(&param.skAddr, sizeof(param.skAddr), &clientAddr, sizeof(clientAddr));


			//Create work thread
			hThreads[g_nCountOfCurrentConns] = (HANDLE)_beginthreadex(
				NULL,
				0,
				WorkThread,
				(void*)(&param),	
				0,
				NULL);

			cerr << endl << "************ Create thread for client "
				<< inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port);



			//If fail to create thread, show message
			if (NULL == hThreads[g_nCountOfCurrentConns] || INVALID_HANDLE_VALUE == hThreads[g_nCountOfCurrentConns])
			{
				closesocket(skConnect);
				cerr << " Error******************";
			}
			else
			{
				cerr << " success !******************";
				++g_nCountOfCurrentConns;
			}
		}
		ReleaseSRWLockExclusive(&g_srwLock);
		/************************************************************************/
		/* �����ʹ��g_nCountOfCurrentConns
		 * Use g_nCountOfCurrentConns with thread safe
		/************************************************************************/




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
	memcpy_s(&param, sizeof(Param), (Param*)(p), sizeof(Param));
	SOCKET skConnect = param.sock;
	struct sockaddr_in& clientAddr = param.skAddr;

	
	unsigned short  usPort = ntohs(clientAddr.sin_port);
	ADDRESS_FAMILY addrFamily = clientAddr.sin_family;
	char* pAddrIP = inet_ntoa(clientAddr.sin_addr);

	Json::Reader reader;
	Json::Value root;

	

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

		
		if (! reader.parse(buffer, root))
			continue;

		
		/********************�������յ����ַ�����json����****************************/
		std::string strDate = root[0]["date"].asString();
		std::string strTime = root[0]["time"].asString();
		std::string strContent = root[0]["content"].asString();

		const char* pContent = strContent.c_str();

		if (0 == strcmp(pContent, bufQuit))
		{
			break;
		}

		cout << endl;

		cout << endl << "--------- Receive From ----------" \
			<< endl                                         \
			<< "IP : " << pAddrIP << ", Port : " << usPort  \
			<< ", AddressFamily : " << addrFamily           \
			<< endl                                         \
			<< "content length is " << strContent.size()    \
			<< endl                                         \
			<< strDate << "  " << strTime                   \
			<< endl
			<< "content is : " << strContent                \
			<< endl                                         \
			<< "------------*************--------------" << endl << endl << endl;

		memset(buffer, 0, sizeof(buffer));

	} //while

	closesocket(skConnect);

	//Decrement the amount of CURRENT CONNECTION after closing socket.
	AcquireSRWLockExclusive(&g_srwLock);
	-- g_nCountOfCurrentConns;
	ReleaseSRWLockExclusive(&g_srwLock);

	return 0;
}

