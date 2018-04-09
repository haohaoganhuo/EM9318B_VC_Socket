#pragma once

#include "EM9118.h"
// CZT_ECDlg 对话框
class CEM9318B_VC_SocketDlg;

class CZT_ECDlg : public CDialog
{
	DECLARE_DYNAMIC(CZT_ECDlg)

public:
	CZT_ECDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CZT_ECDlg();

// 对话框数据
	enum { IDD = IDD_EC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEM9318B_VC_SocketDlg* _pF;
	BOOL _isEc;
	BOOL _isEcInFifo[EM9318_MAXECCHCNT];
	
	afx_msg void OnBnClickedEcclear1();
	afx_msg void OnBnClickedEcclear2();
	afx_msg void OnBnClickedIsec();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void ShowEcValue( I32 chInx, I32 ecCode );
};
