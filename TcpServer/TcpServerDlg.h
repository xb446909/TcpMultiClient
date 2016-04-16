
// TcpServerDlg.h : header file
//

#pragma once

typedef struct _socketinfo
{
	SOCKET socket;
	sockaddr_in clientAddr;
}SocketInfo;

// CTcpServerDlg dialog
class CTcpServerDlg : public CDialogEx
{
// Construction
public:
	CTcpServerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TCPSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnBind();
	afx_msg void OnBnClickedBtnSend();
	afx_msg LRESULT OnSocketRecvMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateClientList(WPARAM wParam, LPARAM lParam);

	HANDLE hThreadListen;
	DWORD dwThreadListenID;
};
