
// WSAAsyncSelect_TCP_ServerDlg.h : 头文件
//

#pragma once

/************************************************************************/

#define  WM_FD_READ   (WM_USER + 100)
#define  WM_FD_WRITE  (WM_USER + 101)
#define  WM_FD_ACCPET (WM_USER + 102)
#define  WM_FD_CLOSE  (WM_USER + 103)

#define  WM_SOCK_MSG  (WM_USER + 200)
#define  EVENT_SET    (FD_READ | FD_ACCEPT | FD_CLOSE)

#define  LOW_VERSION     2
#define  HIGH_VERSIOIN   2


#include <WinSock2.h>
#include <vector>
#include <algorithm>
#include "H:\\JSONCPP\\jsoncpp-master\\include\\json\\json.h"
#pragma  comment(lib, "ws2_32.lib")
#pragma  comment(lib, "H:\\JSONCPP\\jsoncpp-master\\makefiles\\msvc2010\\Debug\\lib_json.lib")


using std::vector;

typedef struct _tagSockParam
{
	SOCKET sock;
	struct sockaddr_in addr;
	bool bEnable;
	struct _tagSockParam() : bEnable(true){}
	bool operator==(const struct _tagSockParam& obj)
	{
		return (sock == obj.sock);
	}

}SockParam, *PSockParam;
/************************************************************************/





// CWSAAsyncSelect_TCP_ServerDlg 对话框
class CWSAAsyncSelect_TCP_ServerDlg : public CDialogEx
{
// 构造
public:
	CWSAAsyncSelect_TCP_ServerDlg(CWnd* pParent = NULL);	// 标准构造函数
	virtual ~CWSAAsyncSelect_TCP_ServerDlg();

// 对话框数据
	enum { IDD = IDD_WSAASYNCSELECT_TCP_SERVER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持



	/************************************************************************/
	typedef std::vector<SockParam> VEC_SOCKET;
	/************************************************************************/

	


// 实现
protected:

	/************************************************************************/
	HICON m_hIcon;
	VEC_SOCKET  m_vecSockets;
	bool m_bInitSuccess;

	const char* m_pServerIP;
	unsigned short m_usPort;
	struct sockaddr_in m_sockAddr;
	int m_nLenOfSockAddr;
	SOCKET m_sockListen;
	WSADATA m_wsaData;
	/************************************************************************/



	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();



	/************************************************************************/
	LRESULT OnSocket(WPARAM wParam, LPARAM lParam);
	void ShowMsg(SOCKET sock, const struct sockaddr_in* pSockAddr, const char* pMsg) const;
	void ShowMsg(const char* pMsg) const;
	/************************************************************************/



};
