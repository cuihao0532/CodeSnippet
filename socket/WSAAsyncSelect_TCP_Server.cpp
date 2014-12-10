
// WSAAsyncSelect_TCP_ServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "WSAAsyncSelect_TCP_Server.h"
#include "WSAAsyncSelect_TCP_ServerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

 
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

 



CWSAAsyncSelect_TCP_ServerDlg::CWSAAsyncSelect_TCP_ServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWSAAsyncSelect_TCP_ServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bInitSuccess = false;
    m_pServerIP = "127.0.0.1";
	m_usPort = 9901;
	
	m_sockAddr.sin_addr.S_un.S_addr = inet_addr(m_pServerIP);
	m_sockAddr.sin_family = AF_INET;
	m_sockAddr.sin_port = htons(m_usPort);
	m_nLenOfSockAddr = sizeof(m_sockAddr);

	m_sockListen = 0;

}



CWSAAsyncSelect_TCP_ServerDlg::~CWSAAsyncSelect_TCP_ServerDlg()
{
	if (! m_bInitSuccess)
		return;

	for (VEC_SOCKET::const_iterator iter = m_vecSockets.begin();
		iter != m_vecSockets.end() && true == iter->bEnable; 
		++ iter)
	{
		closesocket(iter->sock);
	}
}

 

BEGIN_MESSAGE_MAP(CWSAAsyncSelect_TCP_ServerDlg, CDialogEx) 
	ON_MESSAGE(WM_SOCK_MSG, &CWSAAsyncSelect_TCP_ServerDlg::OnSocket)
END_MESSAGE_MAP()


// CWSAAsyncSelect_TCP_ServerDlg 消息处理程序

BOOL CWSAAsyncSelect_TCP_ServerDlg::OnInitDialog()
{
	 //..........


	//Initialize WSAAysncSelect

	if (0 != WSAStartup(MAKEWORD(LOW_VERSION, HIGH_VERSIOIN), &m_wsaData))
	{
		return TRUE;
	}


	m_sockListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == m_sockListen)
	{
		WSACleanup();
		return TRUE;
	}


	if (0 != bind(m_sockListen, (const sockaddr*)&m_sockAddr, m_nLenOfSockAddr))
	{
		closesocket(m_sockListen);
		WSACleanup();
		return TRUE;
	}



	if (0 != listen(m_sockListen, 1))
	{
		closesocket(m_sockListen);
		WSACleanup();
		return TRUE;
	}


	SockParam sp;
	sp.sock = m_sockListen;
	sp.addr = m_sockAddr;
	m_vecSockets.push_back(sp);


	if (0 != WSAAsyncSelect(
		m_sockListen, m_hWnd, WM_SOCK_MSG, EVENT_SET))
	{
		int nErrCode = WSAGetLastError();
		afxDump << "\r\n---------- WSAAsyncSelect Error, "
			    << " ErrorCode is " 
				<< nErrCode 
				<< " --------\r\n";

		closesocket(m_sockListen);
		WSACleanup();

		return TRUE;
	}


	m_bInitSuccess = true;

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}
 
 




//socket message function
LRESULT CWSAAsyncSelect_TCP_ServerDlg::OnSocket( WPARAM wParam, LPARAM lParam )
{
	if (false == m_bInitSuccess)
		return 0;


	SOCKET sock = (SOCKET)(wParam);
	WORD wEvent = LOWORD(lParam);
	WORD wErrCode = HIWORD(lParam);
	TCHAR bufErrCode[200] = {0};
	char buffer[1024] = {0};

	if (0 != wErrCode)
	{
		_sntprintf_s(bufErrCode, sizeof(bufErrCode),
			L"OnSocket error, code is %d", wErrCode);

		AfxMessageBox(bufErrCode);
		memset(bufErrCode, 0, sizeof(bufErrCode));
	}

	int nRet = 0;
	struct sockaddr_in sockClientAddr;
	int nLenOfClientAddr = sizeof(sockClientAddr);

	if (wEvent == FD_ACCEPT)
	{
		if (m_sockListen != sock)
		{
			//the socket is NOT m_sockListen socket
			ShowMsg("FD_ACCEPT error");
			return 0;
		}

		SOCKET sockClient = accept(m_sockListen, (sockaddr*)&sockClientAddr, &nLenOfClientAddr);

		if (INVALID_SOCKET == sockClient)
		{
			int nErrCode = WSAGetLastError();
			ShowMsg("accept error");
		}
		else
		{
			SockParam sp;
			sp.sock = sockClient;
			sp.addr = sockClientAddr;

			VEC_SOCKET::iterator iter = std::find(m_vecSockets.begin(), m_vecSockets.end(), sp);

			//judge if the client socket exits in m_vecSockets
			if (m_vecSockets.end() != iter)
			{
				iter->bEnable = true;
			}
			else
			{
				m_vecSockets.push_back(sp);
			} //else

		} //else

	} //if (wEvent == FD_ACCEPT)
	else if (wEvent == FD_READ)
	{
		int nBytes = recv(sock, buffer, sizeof(buffer), 0);
		if (0 == nBytes)
		{
			//connection has closed
			int nErrCode = WSAGetLastError();
		}
		else if (SOCKET_ERROR == nBytes)
		{ 
			//getlasterror
			int nErrCode = WSAGetLastError();
		}
		else
		{
			SockParam sp;
			sp.sock = sock;

			VEC_SOCKET::const_iterator iter = std::find(m_vecSockets.begin(), m_vecSockets.end(), sp);
			if (iter != m_vecSockets.end())
			{
				ShowMsg(sock, &(iter->addr), buffer);
				memset(buffer, 0, sizeof(buffer));
			}

		}
	} //else if (wEvent == FD_READ)
	else if (wEvent == FD_CLOSE)
	{
		SockParam sp;
		sp.sock = sock;

		closesocket(sock);
		VEC_SOCKET::iterator iter = std::find(m_vecSockets.begin(), m_vecSockets.end(), sp);
		if (iter != m_vecSockets.end())
		{
			iter->bEnable = false;
			ShowMsg("FD_CLOSE comming");
		}

	} //else if (wEvent == FD_CLOSE) 
	else
	{
		//some event UNKNOWN
	}


	return 0;
}

void CWSAAsyncSelect_TCP_ServerDlg::ShowMsg(
	SOCKET sock, const struct sockaddr_in* pSockAddr, const char* pMsg ) const
{
	Json::Reader reader;
	Json::Value root;

	afxDump << "\r\n\r\n" 
		<< "---- sock = " << sock
		<< ", IP = " << inet_ntoa(pSockAddr->sin_addr)
		<< ":" 
		<< ntohs(pSockAddr->sin_port)
		<< " ----- "
		<< "\r\n"
		<< "--- message is ";

	if (reader.parse(pMsg, root))
	{
		std::string strDate = root[0]["date"].asString();
		std::string strTime = root[0]["time"].asString();
		std::string strContent = root[0]["content"].asString();

		afxDump << strDate.c_str() << ", " << strTime.c_str() << "\r\n"
			 << strContent.c_str() << "\r\n\r\n";
	}
	else
	{
		afxDump << pMsg
			 << " ------"
			 << "\r\n\r\n";
	}	
		
}

void CWSAAsyncSelect_TCP_ServerDlg::ShowMsg( const char* pMsg ) const
{
	afxDump << "\r\n\r\n"
		    << "-----"
			<< pMsg
			<< "-----"
			<< "\r\n\r\n";
}
