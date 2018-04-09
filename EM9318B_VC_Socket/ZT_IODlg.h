#pragma once
#include "EM9118.h"

// CZT_IODlg 对话框
class CEM9318B_VC_SocketDlg;

class CZT_IODlg : public CDialog
{
	DECLARE_DYNAMIC(CZT_IODlg)

public:
	CZT_IODlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CZT_IODlg();

// 对话框数据
	enum { IDD = IDD_IO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEM9318B_VC_SocketDlg* _pF;
	BOOL _isDi;
	BOOL _isIoInFifo;
	I8 _do[EM9318_MAXIOCHCNT];
	afx_msg void OnBnClickedIsdi();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnIo(UINT uiID);
	afx_msg void OnIoMode(UINT uiID);
	virtual BOOL OnInitDialog();
};
