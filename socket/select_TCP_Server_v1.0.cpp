/************************************************************************/
/* 基于select模型的TCP服务器, TCP Server based on SELECT Model
/************************************************************************/

#include "stdafx.h"
#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include "H:\\JSONCPP\\jsoncpp-master\\include\\json\\json.h"
#pragma  comment(lib, "ws2_32.lib")
#pragma  comment(lib, "H:\\JSONCPP\\jsoncpp-master\\makefiles\\msvc2010\\Debug\\lib_json.lib")

using namespace std;

#define  LOW_VERSION   2
#define  HIGHT_VERSION 2

void ShowMsg(SOCKET sock, struct sockaddr_in* pSockAddr, bool b = true);
void ShowMsg2(SOCKET sock, const char* pBuffer, struct sockaddr_in* pAddr);  



int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	const char* pServerIP = "127.0.0.1";
	unsigned short usPort = 9901;
	int nRet = 0;
	SOCKET skListen;
	struct sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.S_un.S_addr = inet_addr(pServerIP);
	sockAddr.sin_port = htons(usPort);
	int nLenOfAddr = sizeof(struct sockaddr_in);

	fd_set fdRead;
	fd_set fdWrite;
	fd_set fdExcepts;
	FD_ZERO(&fdRead);
	FD_ZERO(&fdWrite);
	FD_ZERO(&fdExcepts);

	int nIndex = 0;
	SOCKET sockArray[FD_SETSIZE] = {0};


#pragma region  初始化

	if (0 != WSAStartup(MAKEWORD(LOW_VERSION, HIGHT_VERSION), &wsaData))
	{
		cerr << endl << "WSAStartup error" << endl;
		return -1;
	}



	skListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == skListen)
	{
		WSACleanup();
		cerr << endl << "socket error" << endl;
		return -1;
	}


	unsigned long ulArg = 1;
	nRet = ioctlsocket(skListen, FIONBIO, &ulArg);
	if (SOCKET_ERROR == nRet)
	{
		cerr << endl << "ioctlsocket eror" << endl;
		closesocket(skListen);
		WSACleanup();
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


	sockArray[nIndex] = skListen;

	//Initial count of sock array
	size_t nCountOfSocket = 1;
	char buffer[1024] = {0};


	while(true)
	{
		FD_ZERO(&fdRead);
		FD_ZERO(&fdExcepts);
		for (size_t i = 0; i < nCountOfSocket; ++i)
		{
			FD_SET(sockArray[i], &fdRead);
			FD_SET(sockArray[i], &fdExcepts);
		}
		

		int nRet = select(
			0,           //reserved
			&fdRead,     //
			NULL,        //
			&fdExcepts,  //
			NULL         //
			);

		//error happened
		if (SOCKET_ERROR == nRet)
		{
			cerr << endl << "----------- some error " 
				 << endl << "Line Num is "<< __LINE__
				 << endl;
			cerr << "GetLastError is " << WSAGetLastError() << endl;
			break;
		} //if
		else if (0 == nRet)
		{
			continue;
		}




		//handle all the sockets
		for (size_t i = 0; i < nCountOfSocket; ++ i)
		{
			//if the sock belongs to the READ set
			if (FD_ISSET(sockArray[i], &fdRead))
			{
				if (skListen == sockArray[i])
				{
					SOCKET skClient = accept(skListen, (sockaddr*)&sockAddr, &nLenOfAddr);

					//if reach the limit of socket amount, close it
					if (nCountOfSocket >= FD_SETSIZE)
					{
						closesocket(skClient);
						ShowMsg(skClient, &sockAddr, false);
						continue;
					}

					ShowMsg(skClient, &sockAddr);

					//set  skClient as NON-Blocked socket
					//if fail to set, close the skClient
					nRet = ioctlsocket(skClient, FIONBIO, &ulArg); 
					if (SOCKET_ERROR == nRet)
					{
						ShowMsg(skClient, &sockAddr, false);
						closesocket(skClient);
					}
					else
					{
						//add a socket to socket array
						sockArray[nCountOfSocket++] = skClient;	
					}
				} //(skListen == sockArray[i])

				else 
				{
					nRet = recv(sockArray[i], buffer, sizeof(buffer), 0);
					if (SOCKET_ERROR == nRet)
					{
						int nErrCode = WSAGetLastError();

						//have data can be read 
						if (WSAEWOULDBLOCK == nErrCode)
							continue;

						//or connection has been broken/reset/ended
						else
						{
						    /************************************************************************/
						    /* socket error. maybe the socket has been closed
							 * The server will close the socket and delete it.
						    /************************************************************************/
							closesocket(sockArray[i]);
							cerr << endl << "--------- recv error -----------" << endl << endl;
						}


					} //if

					else //receive data successfully and show message.
					{
						ShowMsg2(sockArray[i], buffer, NULL);
						memset(buffer, 0, sizeof(buffer));
					} //else


				} //else
				

			} //if (FD_ISSET(sockArray[i], &fdRead))
		}

		



	} //while(true)





	cout << endl << endl 
		 << "The server will shutdown" 
		 << endl;

	return 0;
}

void ShowMsg(SOCKET sock, struct sockaddr_in* pSockAddr, bool b)
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

	if (! reader.parse(pBuffer, root))
		return;

	std::string strDate = root[0]["date"].asString();
	std::string strTime = root[0]["time"].asString();
	std::string strMsg = root[0]["content"].asString();

	cout << endl << endl
		 << "------- From "
		 << " socket "
		 << sock << ",   "
		 //<< inet_ntoa(pAddr->sin_addr)
		 << ":"
		 //<< ntohs(pAddr->sin_port)
		 << " -------"
		 << endl
		 << strDate << ", "
		 << strTime << endl
		 << strMsg << endl << endl;
}