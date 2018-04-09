#pragma once

#include "EM9118.h"
// CZT_CTDlg 对话框

class CEM9318B_VC_SocketDlg;

class CZT_CTDlg : public CDialog
{
	DECLARE_DYNAMIC(CZT_CTDlg)

public:
	CZT_CTDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CZT_CTDlg();

// 对话框数据
	enum { IDD = IDD_CT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEM9318B_VC_SocketDlg* _pF;
	int		_cntMode;
	F64 _ctBaseMS[EM9118_MAXCTCHCNT];
	BOOL _isCtInFifo[EM9118_MAXCTCHCNT];
	BOOL _isCt;
	afx_msg void OnBnClickedSetcnt();
	afx_msg void OnBnClickedIsct();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void ShowCtValue( I32 chInx, F64 ctCode );
	afx_msg void OnBnClickedCtclear();
};
