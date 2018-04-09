// ZT_ECDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "EM9318B_VC_Socket.h"
#include "ZT_ECDlg.h"
#include "EM9318B_VC_SocketDlg.h"


// CZT_ECDlg 对话框

IMPLEMENT_DYNAMIC(CZT_ECDlg, CDialog)

CZT_ECDlg::CZT_ECDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CZT_ECDlg::IDD, pParent)
	, _isEc(FALSE)
{
	I32 i = 0;
	for( i = 0; i < EM9118_MAXECCHCNT; ++i )
	{
		_isEcInFifo[i] = 0;
	}
}

CZT_ECDlg::~CZT_ECDlg()
{
}

void CZT_ECDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_ISEC, _isEc);
	I32 i = 0;
	for( i = IDC_CHKEC1; i <= IDC_CHKEC2; ++i )
		DDX_Check(pDX, i, _isEcInFifo[i-IDC_CHKEC1]);
}


BEGIN_MESSAGE_MAP(CZT_ECDlg, CDialog)
	ON_BN_CLICKED(IDC_ECCLEAR1, &CZT_ECDlg::OnBnClickedEcclear1)
	ON_BN_CLICKED(IDC_ECCLEAR2, &CZT_ECDlg::OnBnClickedEcclear2)
	ON_BN_CLICKED(IDC_ISEC, &CZT_ECDlg::OnBnClickedIsec)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CZT_ECDlg 消息处理程序

void CZT_ECDlg::OnBnClickedEcclear1()
{
	//编码器1清零
	I32 chClear[] = {1, 0};
	I32 ret = EM9318_EcClear( _pF->_hDev, chClear );
	if( ret < 0 )
		_pF->ShowInfo( "EM9318_EcClear：%d\n", ret );
}

void CZT_ECDlg::OnBnClickedEcclear2()
{
	//编码器2清零
	I32 chClear[] = {0, 1};
	I32 ret = EM9318_EcClear( _pF->_hDev, chClear );
	if( ret < 0 )
		_pF->ShowInfo( "EM9318_EcClear：%d\n", ret );
}

void CZT_ECDlg::OnBnClickedIsec()
{
	UpdateData( TRUE );
	if( _isEc )
	{
		SetTimer( 1, _pF->_readInterval, 0 );
	}else
	{
		KillTimer( 1 );
	}
}

void CZT_ECDlg::ShowEcValue( I32 chInx, I32 ecCode )
{
	ASSERT( 0 <= chInx && chInx < EM9318_MAXECCHCNT );

	CString str;
	str.Format( "%8d", ecCode );
	GetDlgItem( IDC_EC1 + chInx )->SetWindowText( str );
}

void CZT_ECDlg::OnTimer(UINT_PTR nIDEvent)
{
	I32 ecValue[EM9318_MAXECCHCNT];
	I32 ret = EM9318_EcReadCodeAll( _pF->_hDev, ecValue, 0 );
	if( ret < 0 )
		_pF->ShowInfo( "EM9318_EcReadCodeAll：%d\n", ret );
	for( int i = 0; i < EM9118_MAXECCHCNT; ++i )
	{
		ShowEcValue( i, ecValue[i] );
	}

	CDialog::OnTimer(nIDEvent);
}
