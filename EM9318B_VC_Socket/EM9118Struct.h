// EM9118_Server.h: interface for the CEM9118 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EM9118_Server_H__D1413467_66A2_475B_8398_F579D95B3FB0__INCLUDED_)
#define AFX_EM9118_Server_H__D1413467_66A2_475B_8398_F579D95B3FB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <winsock2.h>
#include <vector>

const I32 ADBC = 2;//一个AD原码所占宽度
const I32 CTBC = 4;//一个CT原码所占宽度
const I32 ECBC = 4;//一个EC原码所占宽度
const I32 IOBC = 2;//一个IO原码所占宽度

inline F64 AdCodeToValue( I16 iCode, I16 zeroCode, I16 fullCode, F64 zeroValue, F64 fullValue );

typedef struct EM9118_ADPARA
{//AD相关参数
	I8 isInFifo[EM9118_MAXADCHCNT];//终止通道号
	I16 fullCode[EM9118_MAXADCHCNT];
	I16 zeroCode[EM9118_MAXADCHCNT];
	F64 fullValue[EM9118_MAXADCHCNT];
	F64 zeroValue[EM9118_MAXADCHCNT];
	I32 rangeInx;//采集范围，请参见“AD采集范围”宏设置
}EM9118_AdPara;

typedef struct EM9118_IOPARA
{//开关量相关参数
	I8 doMod[2];//2路开关量功能
}EM9118_IoPara;

typedef struct EM9118_CTPARA
{//计数器相关参数
	I8 mode[EM9118_MAXCTCHCNT];//计数方式，具体请看“计数方式宏定义”
	double freqBase[EM9118_MAXCTCHCNT];//分频系数时基，以ms为单位，在测量高低频率时含义不同，计算方法请看示例程序
}EM9118_CtPara;

typedef struct EM9118_COMMONPARA
{
	I32 adChCount;
	I32 ctChCount;
	I32 ecChCount;
	I32 ioInFifo;
}EM9118_CommonPara;

typedef struct EM9118_DEVPARA 
{//设备共用参数
	EM9118_AdPara adPara;
	EM9118_IoPara ioPara;
	EM9118_CtPara ctPara;
	I8 ctInFifo[EM9118_MAXCTCHCNT];
	I8 ecInFifo[EM9118_MAXECCHCNT];
	I8 diInFifo;//这个只是为了占位用，目前没有作用
	EM9118_CommonPara commonPara;
	I32 groupBC;//每组数据的字节数
	F64 groupFreq;//组切换频率

	I32 isPick;//是否需要挑选数据
	F64 realGroupFreq;//当groupFreq设置过低时，为了保证下位机可以在1s内返回数据，将实际频率设快，然后挑选数据仿真低频采集
	I32 pickCodeCnt ;//当groupFreq设置过低时，每隔这个数目挑选一个数据

	I32 isHcStart;
	I32 clkSrc;//时钟源
	I32 triSrc;//触发源
	I32 edgeOrLevel;
	I32 upOrDown;
	I8 strIP[256];//存放IP地址
	HANDLE hCmd;//命令端口操作句柄
	SOCKET dataFd;//数据端口
	CRITICAL_SECTION sectCmd;
	I32 TimeOutCnt;//超时次数，用于判断网络是否中断 //2014-04-24
	bool enableWtd;//是否使能看门狗 //2014-04-24
	string dataPathName;
	FILE* dfwF;
	FILE* csvF;
	I32 dfwPos;
	F64 csvLineInx;//当前写入的索引
	I32 singleFileSize;
	I32 csvLinesCount;//最大行数
	U32 triCount;
}EM9118_DevPara;

typedef struct DAQTIME//采集时间段 
{
	//开始时间
	I8 startS;//秒
	I8 startM;//分
	I8 startH;//时
	//结束时间
	I8 endS;//秒
	I8 endM;//分
	I8 endH;//时
}DaqTime;

typedef struct OFFLINESET
{//！！！！！！！！！1注意这个结构是已经是下位机固定好的，不能随意改动！！！！！！！！！！
	short version;//版本号
	//采集时间间隔设置
	I16 intervalMS;//毫秒
	I8 intervalS;//秒
	I8 intervalM;//分
	I8 intervalH;//小时
	I8 daqStyle; //daqStyle：采集方式：
	//0下位机上电启动采集；
	//1上位机控制启停
	I8 startMode;//启动方式 0 软件启动AD转换（慢速采集） 1 内时钟启动AD转换 2 外时钟启动AD转换 3 外部高电平启动AD采集
	I32 groupCount;//在高速采集时，从fifo中读取多少组数据读取一次不进入fifo的数据（io、ct、ec）
	I32 isPSync;//0非伪同步，1伪同步
	I32 chFreq;//通道切换采集频率
	I32 groupFreq;//组采集频率，伪同步有效
	I8 adBeginCh;//AD起始通道
	I8 adEndCh;//AD结束通道
	I8 adRange;//AD采集范围
	I8 adGain;//AD 增益
	I8 adEndMode;//AD单双端
	I8 ioInFifo;//io1~io16是否进入fifo
	I8 ioDir[4];//开关量方向
	I8 ctMode[6];//计数器方式
	I32 ctGateFreq[6];//门控信号频率
	I8 ad[EM9118_MAXADCHCNT];//对应ad通道是否存盘，0禁止，1允许
	I8 io[4];//对应组的开关量是否存盘，0禁止，1允许
	I8 ct[4];//计数器通道是否存盘，0禁止，1允许
	I8 ec[2];//编码器数据是否存盘，0禁止，1允许
	I16 zCode[EM9118_MAXADCHCNT];//全部18个通道的零点值
	I16 fCode[EM9118_MAXADCHCNT];//全部18个通道的满度值
	F64 zValue[EM9118_MAXADCHCNT];//全部18个通道的零点物理
	F64 fValue[EM9118_MAXADCHCNT];//全部18个通道的满度物理
	I8 daqTimeCnt;//采集时间段个数
	U32 savelength; //存盘数据量控制
	DaqTime* daqTime; //时间段起止时间
	U32 saveCtrlDport;//存盘控制信号端口(数字)
	U32 saveCtrlDvalue;//存盘控制信号值(数字)
	U32 saveCtrlAport;//存盘控制信号端口(模拟)
	I32 saveCtrlAvalue;//存盘控制信号值(模拟)
}OfflineSet;

typedef struct OFFLINEFILE{
	FILE* f;
	I32 dataBeginPos;//当前位置
	I32 dataSize;
	OfflineSet os;
	I8 ctLogicToRealChInx[4];//读取数据时，由于计数器的使能通道可以任意设置，因此数据里面的通道顺序可能不连续，此处是为了记录第一个CT数据是第几通道的CT数据
	EM9118_CommonPara commonPara;
}OfflineFile;

/*I32 _stdcall EM9118_Init();
//函数功能：进行一些必要的初始化设置，在每一次AD采集开始后，
//          第一次调用EM9118_ExtractADCode前要调用此函数，以后不必再调用。
//返回值：0表示成功

I32 _stdcall EM9118_ExtractADCode( U8* enCh, U16* chDiv, I16* codeBuffer, U32 codeCount, U32* chCodeCount );
//函数功能：将数据按照设置拆分成18路。
//入口参数：
//            enCh：通道使能设置，长度为18的8位无符号数组，0表示通道禁止，1表示通道使能
//           chDiv：通道分频设置，长度为18的16位无符号数组，每个元素值为1~255
//      codeBuffer：存储了AD原码的缓冲区
//       codeCount：原码缓冲区大小，以16位字为单位
//出口参数：
//      hCodeCount：拆分后每通道原码个数，以16位字为单位，用于分频相对应通道的数据缓冲区
//返回值：0表示成功，-1表示有参数错误

I32 _stdcall EM9118_GetADChCode( I32 chNo, I16* chCodeBuffer );
//函数功能：获取指定通道的数据。
//入口参数：
//            chNo：通道号，1~18
//出口参数：
//    chCodeBuffer：数据缓冲区，由用户分配，其大小由EM9118_ExtractADCode的chCodeCount参数返回
//返回值：0表示成功*/


#endif // !defined(AFX_EM9118_Server_H__D1413467_66A2_475B_8398_F579D95B3FB0__INCLUDED_)
