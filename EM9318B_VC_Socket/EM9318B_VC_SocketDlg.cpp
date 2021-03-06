// EM9318B_VC_SocketDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "EM9318B_VC_Socket.h"
#include "EM9318B_VC_SocketDlg.h"
#include "IniRWer.h"

#ifdef _DEBUG

#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CEM9318B_VC_SocketDlg 对话框

string GetConfigFilePathAndNmae()
{
	TCHAR szFilePath[MAX_PATH + 1] = { 0 };
	GetModuleFileName(NULL, szFilePath, MAX_PATH);
	(_tcsrchr(szFilePath, _T('\\')))[1] = 0; //删除文件名，只获得路径

	string str = szFilePath;
	str += "config.ini";
	return str;
}

void CEM9318B_VC_SocketDlg::ReadPara()
{
	string str = GetConfigFilePathAndNmae();

	CIniReader iniReader( str.c_str() );
	_strIP = iniReader.ReadString( "网络设置", "IP", "192.168.1.126" );
	_cmdPort = iniReader.ReadInteger( "网络设置", "命令端口", 8000 );
	_dataPort = iniReader.ReadInteger( "网络设置", "数据端口", 8001 );
	_readInterval = iniReader.ReadInteger( "采集设置", "软件定时器间隔", 500 );
}

CEM9318B_VC_SocketDlg::CEM9318B_VC_SocketDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEM9318B_VC_SocketDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	ReadPara();
	_showTime = true;
}

void CEM9318B_VC_SocketDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, _strIP);
	DDX_Text(pDX, IDC_EDIT5, _cmdPort);
	DDX_Text(pDX, IDC_EDIT7, _readInterval);
	DDV_MinMaxInt(pDX, _readInterval, 1, 1000);
	DDX_Text(pDX, IDC_EDIT6, _dataPort);
	DDX_Control(pDX, IDC_TAB1, _tabCtrl);
}

BEGIN_MESSAGE_MAP(CEM9318B_VC_SocketDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CONNECT, &CEM9318B_VC_SocketDlg::OnBnClickedConnect)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CEM9318B_VC_SocketDlg::OnTcnSelchangeTab1)
	ON_BN_CLICKED(IDC_DISCONNECT, &CEM9318B_VC_SocketDlg::OnBnClickedDisconnect)
	ON_BN_CLICKED(IDC_BUTTON2, &CEM9318B_VC_SocketDlg::OnBnClickedButton2)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CEM9318B_VC_SocketDlg 消息处理程序

BOOL CEM9318B_VC_SocketDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	InitTabCtrl();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CEM9318B_VC_SocketDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CEM9318B_VC_SocketDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CEM9318B_VC_SocketDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CEM9318B_VC_SocketDlg::OnBnClickedConnect()
{
	UpdateData( TRUE );
	//创建设备，为设备分配必要的内存资源
	_hDev = EM9118_DeviceCreate();
	if( !_hDev )
	{
		ShowInfo( _T("建立设备错误，可能是内存不足！\n") );
		return;
	}
	//建立命令连接
	I32 ret = EM9118_CmdConnect( _hDev, _strIP.GetBuffer(0), _strIP.GetLength(), 8000 );
	if( ret < 0 )
	{
		ShowInfo( _T("无法建立命令连接！\n") );
		EM9118_DeviceClose( _hDev );
		return;
	}
	//建立数据连接，读取高速数据用
	ret = EM9118_DataConnect( _hDev, 8001 );
	if( ret < 0 )
	{
		ShowInfo( _T("无法建立数据连接！\n") );
		EM9118_CmdClose( _hDev );
		EM9118_DeviceClose( _hDev );
		return;
	}

	GetDlgItem( IDC_CONNECT )->EnableWindow(FALSE);
	GetDlgItem( IDC_DISCONNECT )->EnableWindow(TRUE);
	_tabCtrl.EnableWindow( TRUE );

}

enum{
	TAB_AD,
	TAB_IO,
	TAB_CT,
	TAB_EC,
	TAB_PWM,
	TAB_ITEM_COUNT,
};

CString g_strTab[] = { "AD采集", "IO采集", "CT采集", "EC采集", "PWM控制" };

void CEM9318B_VC_SocketDlg::InitTabCtrl()
{
	CRect rsClient, rsLable;
	_tabCtrl.GetItemRect(0, &rsLable );
	_tabCtrl.GetClientRect( &rsClient );
	rsClient.left +=2;
	rsClient.top += rsLable.Height() + 5;
	rsClient.bottom -= 2;
	rsClient.right -= 2;
	//将对应选项卡对话框放到选项卡下面

	_dlgAd.Create( IDD_AD, GetDlgItem( IDC_TAB1) );
	_dlgAd._pF = this;
	_vTAbDlg.push_back( &_dlgAd );

	_dlgIo.Create( IDD_IO, GetDlgItem( IDC_TAB1) );
	_dlgIo._pF = this;
	_vTAbDlg.push_back( &_dlgIo );

	_dlgCt.Create( IDD_CT, GetDlgItem( IDC_TAB1) );
	_dlgCt._pF = this;
	_vTAbDlg.push_back( &_dlgCt );

	_dlgEc.Create( IDD_EC, GetDlgItem( IDC_TAB1) );
	_dlgEc._pF = this;
	_vTAbDlg.push_back( &_dlgEc );

	_dlgPwm.Create( IDD_PWM, GetDlgItem( IDC_TAB1) );
	_dlgPwm._pF = this;
	_vTAbDlg.push_back( &_dlgPwm );


	for( U32 i = 0; i < _vTAbDlg.size(); ++i )
	{
		_vTAbDlg[i]->MoveWindow( &rsClient );
		_tabCtrl.InsertItem( i, g_strTab[i] );
	}

	//设置当前有效选项卡
	_tabCtrl.SetCurSel( TAB_AD );
	_vTAbDlg[TAB_AD]->ShowWindow( TRUE );
}

void CEM9318B_VC_SocketDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	int curSel = _tabCtrl.GetCurSel();

	static int lastSel = TAB_AD;
	if( lastSel != curSel )
	{
		_vTAbDlg[curSel]->ShowWindow( TRUE );
		_vTAbDlg[lastSel]->ShowWindow( FALSE );
		lastSel = curSel;
	}
	*pResult = 0;
}

void CEM9318B_VC_SocketDlg::OnBnClickedDisconnect()
{
	if( _dlgAd._isDaqStart )//如果仍然在采集，先停止采集
		_dlgAd.OnBnClickedStopdaq();

	EM9118_DataClose( _hDev );//关闭数据连接
	EM9118_CmdClose( _hDev );//关闭命令连接
	EM9118_DeviceClose( _hDev );//释放设备资源
	_hDev = 0;
	GetDlgItem( IDC_CONNECT )->EnableWindow(TRUE);
	GetDlgItem( IDC_DISCONNECT )->EnableWindow(FALSE);
	_tabCtrl.EnableWindow( FALSE );
}

void CEM9318B_VC_SocketDlg::OnBnClickedButton2()
{
	ClearInfo();
}

void CEM9318B_VC_SocketDlg::ShowInfo( const char* szFormat, ... )
{
	CEdit* p = (CEdit*)GetDlgItem( IDC_EDIT3 );

	char temp[1024];
	int iReturn;
	va_list pArgs;
	va_start(pArgs, szFormat);
	iReturn = vsprintf_s(temp, szFormat, pArgs);
	va_end(pArgs);

	SYSTEMTIME st;
	GetLocalTime(&st);
//	CString strDate;
//	strDate.Format("%4d-%2d-%2d",st.wYear,st.wMonth,st.wDay);
	CString strTime;
	if( _showTime )
		strTime.Format("%02d:%02d:%02d.%03d: ",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds) ;
	strTime += temp;
	_strInfo += strTime;
	int nLength = p->SendMessage(WM_GETTEXTLENGTH);
	if( nLength + _strInfo.GetLength() > 20000 )
	{
		p->SetSel( 0, 10000 );
		p->ReplaceSel( "" );
		nLength -= 10000;
	}
	p->SetSel( nLength, nLength );
	p->ReplaceSel( strTime );

}

void CEM9318B_VC_SocketDlg::ClearInfo()
{
	_strInfo = "";
	GetDlgItem( IDC_EDIT3 )->SetWindowText( _strInfo );
}

void CEM9318B_VC_SocketDlg::WritePara()
{
	string str = GetConfigFilePathAndNmae();
	CIniWriter iniWriter( str.c_str() );
	iniWriter.WriteString( "网络设置", "IP", _strIP );
	iniWriter.WriteInteger( "网络设置", "命令端口", _cmdPort );
	iniWriter.WriteInteger( "网络设置", "数据端口", _dataPort );
	iniWriter.WriteInteger( "采集设置", "软件定时器间隔", _readInterval );
}

void CEM9318B_VC_SocketDlg::OnClose()
{
	UpdateData(TRUE);
	WritePara();

	CDialog::OnClose();
}
