# EM9318B_VC_Socket

本示例程序演示了使用VC2008软件,通过Socket操作EM9318B编程方法
关于硬件的操作，都放在了EM9118.cpp文件中
用户可以看EM9118.h中的注释来了解具体函数使用方法

## 网络连接相关操作：

初始化设备参数以及建立网络连接
void CEM9318B_VC_SocketDlg::OnBnClickedConnect()

关闭网络连接以及释放设备资源
void CEM9318B_VC_SocketDlg::OnBnClickedDisconnect()


## 高速采集相关函数：

初始化并启动采集
void CZT_ADDlg::OnBnClickedStartdaq()
停止采集
void CZT_ADDlg::OnBnClickedStopdaq()
读取、拆分处理数据
void CZT_ADDlg::ReadThread()

## 计数器相关操作

初始化计数器
void CZT_CTDlg::OnBnClickedSetcnt()
读取并处理计数器数据
void CZT_CTDlg::OnTimer(UINT_PTR nIDEvent)

## 编码器相关操作

编码器清零
void CZT_ECDlg::OnBnClickedEcclear1()
void CZT_ECDlg::OnBnClickedEcclear2()
读取编码器数值
void CZT_ECDlg::OnTimer(UINT_PTR nIDEvent)

## 开关量相关操作

读取开关量数据
void CZT_IODlg::OnTimer(UINT_PTR nIDEvent)
设置开关量端口状态
void CZT_IODlg::OnIo(UINT uiID)
设置开关量管脚功能
void CZT_IODlg::OnIoMode(UINT uiID)

## PWM输出操作

设置PWM参数并且启动输出
void CZT_PWMDlg::OnBnClickedPwmstart()
停止PWM输出
void CZT_PWMDlg::OnBnClickedPwmstop()
读取PWM输出状态
void CZT_PWMDlg::OnTimer(UINT_PTR nIDEvent)