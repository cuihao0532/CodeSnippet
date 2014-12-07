// Ioctlsocket_TCP_Client.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <time.h>
#include "H:\\JSONCPP\\jsoncpp-master\\include\\json\\json.h"
#pragma  comment(lib, "ws2_32.lib")
#pragma  comment(lib, "H:\\JSONCPP\\jsoncpp-master\\makefiles\\msvc2010\\Debug\\lib_json.lib")

using namespace std;

#define  LOW_VERSION   2
#define  HIGHT_VERSION 2

bool Send2(const SOCKET& sock, const char* buf, int len, int flags);
string GeneGUID();


int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	int nRet = 0;
	unsigned short usPort = 9901;
	const char* pServerIP = "127.0.0.1";
	SOCKET sockConn = 0;

	struct sockaddr_in serverAddr;
	serverAddr.sin_port = htons(usPort);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = inet_addr(pServerIP);
	int lenOfServerAddr = sizeof(struct sockaddr_in);

	//////////////////////////////////////////////////////////////////////////

	nRet = WSAStartup(MAKEWORD(LOW_VERSION, HIGHT_VERSION), &wsaData);
	if (0 != nRet)
	{
		cerr << endl << "------------- WSAStartup error -------------" << endl;
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////

	sockConn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sockConn)
	{
		cerr << endl << "--------------- socket error ------------" << endl;
		WSACleanup();
		return -1;
	}

	//设置非阻塞模式socke
	unsigned long ulArg = 1UL;
	ioctlsocket(sockConn, FIONBIO, &ulArg);


	//////////////////////////////////////////////////////////////////////////
	const int BUFFER_SIZE = 1024;
	char buffer[BUFFER_SIZE] = {0};
	char bufQuit[] = "quit";

	cout << endl << "----------- I am connecting to server "
		 << inet_ntoa(serverAddr.sin_addr)
		 << " : "
		 << ntohs(serverAddr.sin_port)
		<< " -----------"
		<< endl;

	//循环等待connect到服务器
	while(true)
	{
		nRet = connect(sockConn, (const sockaddr*)&serverAddr, lenOfServerAddr);
		if (SOCKET_ERROR == nRet)
		{
			int nErrCode = WSAGetLastError();
			if (WSAEWOULDBLOCK == nErrCode 
				|| WSAEALREADY == nErrCode   //在一个非阻止性套接字上尝试了一个已经在进行的操作
				|| WSAEINVAL   == nErrCode
				)
			{
				Sleep(10);
				continue;
			}
			else if(WSAEISCONN == nErrCode)   //在一个已经连接的套接字上做了一个连接请求
			{
				break;
			}
			else
			{
				cerr << endl << "---------- connect to server"
					<< inet_ntoa(serverAddr.sin_addr)
					<< " : "
					<< ntohs(serverAddr.sin_port)
					<< " Error *************"
					<< endl;

				closesocket(sockConn);
				WSACleanup();
				return -1;

			} //else

		} //if

		break;

	} //while

	cerr << endl << "---------- connect to server"
		<< inet_ntoa(serverAddr.sin_addr)
		<< " : "
		<< ntohs(serverAddr.sin_port)
		<< " succussful !*************"
		<< endl;
	
	//////////////////////////////////////////////////////////////////////////
	//ID of Client
	string strClientID = GeneGUID();

	while(true)
	{
		cout << endl << "Please input message " << endl << ">>";
			
		cin.getline(buffer, sizeof(buffer), '\n');
		if (0 == strlen(buffer))
			continue;

		if (strlen(buffer) >= (  ( sizeof(buffer) / sizeof(buffer[0]) ) - 4)  )  
		{
			cerr << endl << "-- too long message --" << endl;
			continue;
		}

		/************************************************************************/
		/* 构建发送消息格式的json对象, Create the JSON object to send
		/************************************************************************/
		SYSTEMTIME st = {0};
		GetLocalTime(&st);
		char bufDate[20] = {0};
		char bufTime[10] = {0};
		_snprintf(bufDate, sizeof(bufDate), "%d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
		_snprintf(bufTime, sizeof(bufTime), "%d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);

		Json::Value root;
		Json::FastWriter writer;
		Json::Value msg;

		msg["date"] = bufDate;
		msg["time"] = bufTime;
		const char* pMsg = NULL;


		/************************************************************************/


		//判断是否为退出操作
		if (1 == strlen(buffer) && 'q' == buffer[0])
		{
			msg["content"] = bufQuit;
			root.append(msg);

			std::string strMsg = writer.write(root);
		    pMsg = strMsg.c_str();

			if (false == Send2(sockConn, pMsg, (strlen(pMsg) + 1) * sizeof(char), 0))
			{
				cerr << endl <<"********** send quit msg to server failed ! ***********" << endl << endl;
			}
			cout << endl << "************* The client will shutdown *************" << endl << endl;
			break;
		}



		msg["content"] = buffer;
		root.append(msg);

		std::string strMsg = writer.write(root);
		pMsg = strMsg.c_str();

		if(false == Send2(sockConn, pMsg, (strlen(pMsg) + 1) * sizeof(char), 0))
		{
			cerr << endl << endl
			     <<"************"
				 << " send error, maybe the connection has broken "
				 << " ****************"
				 << endl << endl;

			break;
		}		

	} //while
	
	closesocket(sockConn);
	WSACleanup();
	
	system("pause");
	return 0;
}


bool Send2(const SOCKET& sock, const char* buf, int len, int flags)
{
	while(true)
	{
		int nRet = send(sock, buf, len, flags);
		if (SOCKET_ERROR == nRet)
		{
			int nErrCode = WSAGetLastError();
			if (WSAEWOULDBLOCK == nErrCode)
			{
				Sleep(10);
				continue;
			}
			else
			{
				return false;
			} //else
		}//if

		return true;
	} //while

}


string GeneGUID()
{
	char buffer[128] = {0};
	GUID guid;
	CoInitialize(NULL);
	if (S_OK == ::CoCreateGuid(&guid))
	{
		_snprintf(buffer, sizeof(buffer),
			"%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
			guid.Data1,
			guid.Data2,
			guid.Data3,
			guid.Data4[0], guid.Data4[1],
			guid.Data4[2], guid.Data4[3],
			guid.Data4[4], guid.Data4[5], 
			guid.Data4[6], guid.Data4[7]);

		CoUninitialize();

		return string(buffer);
	}

	return string("null");
}