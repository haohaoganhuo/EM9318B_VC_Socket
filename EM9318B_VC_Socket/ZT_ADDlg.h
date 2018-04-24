#pragma once

#include "TimeInterval.h"
#include "ZCD_Lock.h"

// CZT_ADDlg 对话框

class CEM9318B_VC_SocketDlg;

class CZT_ADDlg : public CDialog
{
	DECLARE_DYNAMIC(CZT_ADDlg)

public:
	CZT_ADDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CZT_ADDlg();

// 对话框数据
	enum { IDD = IDD_AD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEM9318B_VC_SocketDlg* _pF;
	CString	_ctrlDaqFreq;
	int _adRange;
	int _clkSrc;
	int _triSrc;
	int _edgeLevel;
	int _upDown;
	I32 _groupBC;//每组数据字节数
	BOOL _isAdInFifo[EM9118_MAXADCHCNT];
	I32 _adInx2No[EM9118_MAXADCHCNT];//AD在FIFO中排列的数据顺序对应实际的通道号
	I32 _adCnt;
	I32 _ctCnt;
	I32 _ecCnt;
	I32 _groupCntNeed;
	V_F64 _adValue[EM9118_MAXADCHCNT];
	V_F64 _ctValue[EM9118_MAXCTCHCNT];
	V_F64 _ecValue[EM9118_MAXECCHCNT];
	V_U16 _ioValue;
	I32 _isDaqStart;
	HANDLE _readThreadExit;//用于判断读数据线程是否退出的信号量
	int _dispMode;
	BOOL _isSave;
	F64 _allReadBC;
	CTimeInterval _tvAll;
	I32 _processGroupCnt;

	I32 InitAD();
	F64 GetDaqFreq();
	void ReadThread();
	void ProcessData( V_I8& codeBuffer, I32 bcGroup );
	I32 StopAD();
	void ShowSpeed( F64 tvS, I32 onceBC, I32 bcGroup );
	afx_msg void OnBnClickedStartdaq();
	afx_msg void OnBnClickedStopdaq();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void ShowAdValue( I32 chInx, const V_F64& adValue );
	void ShowCtValue( I32 chInx, F64 ctCode );
	void ShowEcValue( I32 chInx, I32 ecCode );
	void ShowIoValue( U16 ioCode );
	void OpenWriteFile();
	BOOL _isProcess;
	int _adAverageNumber;
	CZCD_Lock _lockValue;
	I32 _readTimes;
	DWORD _triCount;
	afx_msg void OnBnClickedReadadonce();
};
