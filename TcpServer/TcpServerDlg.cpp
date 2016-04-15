
// TcpServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TcpServer.h"
#include "TcpServerDlg.h"
#include "afxdialogex.h"
#include <vector>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define UM_SOCK_ASYNCRECVMSG	(WM_USER + 1)
#define UM_SOCK_ACCEPT			(WM_USER + 2)

vector<SOCKET> socketList;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CTcpServerDlg dialog

DWORD WINAPI TcpListenThread( LPVOID lpParam );


CTcpServerDlg::CTcpServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTcpServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTcpServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTcpServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_BIND, &CTcpServerDlg::OnBnClickedBtnBind)
	ON_BN_CLICKED(IDC_BTN_SEND, &CTcpServerDlg::OnBnClickedBtnSend)
	ON_MESSAGE(UM_SOCK_ASYNCRECVMSG, &CTcpServerDlg::OnSocketRecvMsg)
	ON_MESSAGE(UM_SOCK_ACCEPT, &CTcpServerDlg::OnSocketAccpet)
END_MESSAGE_MAP()


// CTcpServerDlg message handlers

BOOL CTcpServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTcpServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTcpServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTcpServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTcpServerDlg::OnBnClickedBtnBind()
{
	// TODO: Add your control notification handler code here
	hThreadListen = CreateThread(NULL, 0, TcpListenThread, m_hWnd, 0, &dwThreadListenID);
}

DWORD WINAPI TcpListenThread( LPVOID lpParam )
{
	HWND hWnd = (HWND)lpParam;

	WSADATA wsaData;
	int iResult = 0;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		wprintf(L"WSAStartup() failed with error: %d\n", iResult);
		return 1;
	}

	SOCKET listenSock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	UINT port = GetDlgItemInt(hWnd, IDC_EDIT_PORT, NULL, FALSE);
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	int nRet = bind( listenSock, (sockaddr*)&sin, (int)(sizeof(sin)));
	if ( nRet == SOCKET_ERROR )
	{
		DWORD errCode = GetLastError();
		return 0;
	}

	listen( listenSock, SOMAXCONN);

	sockaddr_in clientAddr;
	int nameLen = sizeof( clientAddr );

	char buffer[32] = {0};

	while( socketList.size() < FD_SETSIZE )
	{
		SOCKET clientSock = accept( listenSock, (sockaddr*)&clientAddr, &nameLen );
		socketList.push_back(clientSock);
		sprintf_s(buffer, "%s:%d", inet_ntoa(clientAddr.sin_addr), clientAddr.sin_port);
		::SendMessage(hWnd, UM_SOCK_ACCEPT, 0, (LPARAM)buffer);
		WSAAsyncSelect( clientSock, hWnd, UM_SOCK_ASYNCRECVMSG, FD_READ | FD_CLOSE );
	}
	return 0;
}

LRESULT CTcpServerDlg::OnSocketRecvMsg(WPARAM wParam, LPARAM lParam)
{
	SOCKET clientSock = (SOCKET)wParam;
	if ( WSAGETSELECTERROR( lParam ) )
	{
		closesocket( clientSock );
		return 0;
	}

	switch ( WSAGETSELECTEVENT( lParam ) )
	{
	case FD_READ:
		{
			char recvBuffer[10240] = {'\0'};
			int nRet = recv( clientSock, recvBuffer, 10240, 0 );
			if ( nRet > 0 )
			{
				//szRecvMsg.AppendFormat(_T("Client %d Say:%s\r\n"), clientSock, recvBuffer );
			}
			else
			{
				//client disconnect
				//szRecvMsg.AppendFormat(_T("Client %d Disconnect!\r\n"), clientSock );
			}
		}                              

		break;

	case FD_CLOSE:
		{
			closesocket( clientSock );
			//szRecvMsg.AppendFormat(_T("Client %d Disconnect!\r\n"), clientSock );
		}

		break;
	}
	return 0;
}

LRESULT CTcpServerDlg::OnSocketAccpet(WPARAM wParam, LPARAM lParam)
{
	char* buffer = (char*)lParam;
	((CListBox*)GetDlgItem(IDC_LIST_CLIENT))->AddString(CA2W(buffer));
	return 0;
}

void CTcpServerDlg::OnBnClickedBtnSend()
{
	// TODO: Add your control notification handler code here
}
