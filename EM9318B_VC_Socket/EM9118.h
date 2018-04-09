//本DLL主要提供辅助函数处理EM9118数据
/*拆分数据时调用方法：
在采集开始后第一次调用EM9118_ExtractADCode之前，需要调用EM9118_Init函数，然后在停止采集之前不可以再次调用EM9118_Init
每次调用EM9118_ExtractADCode都会根据其参数设置为每个通道生成一个原码数组，这个数组可以通过EM9118_GetADChCode读出。
如果在调用EM9118_ExtractADCode后没有通过EM9118_GetADChCode读出的数据，在下一次调用EM9118_ExtractADCode后将会被覆盖
*/

#ifndef EM9118_H
#define EM9118_H

#define EM9118_TIMOUT_MS                            1000
#define EM9118_USB_MAX_COUNT                        16 //最大可支持USB模块个数
#define EM9118_BASE_FREQ                            36000000.0//基础时钟，36M
#define EM9118_PWM_BASE_FREQ                        4000000.0//PWM基准分频系数
#define EM9118_MAXADCHCNT                           18 //AD最大通道数
#define EM9118_MAXDOCHCNT                           2 //DO最大通道数
#define EM9118_MAXDICHCNT                           4 //DI最大通道数
#define EM9118_MAXCTCHCNT                           4 //计数器最大通道数
#define EM9118_MAXECCHCNT                           2 //编码器最大通道数
#define EM9118_MAXPWMCHCNT                          2 //PWM最大通道数
#define EM9106_MAXIOCHCNT							24//EM9106的IO最大通道数
#define EM9318_MAXIOCHCNT							8 //EM9318的IO最大通道数
#define EM9318_MAXPWMCHCNT                          4 //EM9318的PWM最大通道数
#define EM9318_MAXECCHCNT                           2 //EM9318的编码器最大通道数
#define EM9318_PWM_BASE_FREQ                        36000000.0//EM9318的PWM时钟频率，36M

//启动方式
#define EM9118_CLKSRC_IN                            0 //内时钟启动采集
#define EM9118_CLKSRC_EX                            1 //外时钟启动采集
#define EM9118_TRISRC_IN                            0 //内触发启动采集
#define EM9118_TRISRC_EX                            1 //外触发启动采集
#define EM9118_TRI_EDGE                             0 //外触发时边沿启动采集
#define EM9118_TRI_LEVEL                            1 //外触发时电平启动采集
#define EM9118_TRI_UP                               0 //外触发时上升沿（高电平）启动采集
#define EM9118_TRI_DOWN                             1 //外触发时下降沿（低电平）启动采集


//AD采集范围
#define EM9118_AD_RANGE_N10_10V                      0L //±10V
#define	EM9118_AD_RANGE_N5_5V                        1L //±5V
#define	EM9118_AD_RANGE_0_20MA                       2L //0~20mA

//计数方式
#define EM9118_CT_MODE_COUNT                         0 //计数
#define EM9118_CT_MODE_HFREQ                         1 //测高频
#define EM9118_CT_MODE_LFREQ                         3 //测低频

//开关量输出管脚功能
#define EM9118_DO_MODE_DO                            0 //开关量输出
#define EM9118_DO_MODE_PWM                           1 //PWM
#define EM9118_DO_MODE_OCLK                          2 //OCLK，采集时钟输出

#include "EM9118Struct.h"

EM9118_DevPara* _stdcall EM9118_DeviceCreate();
//函数功能：创建设备
//入口参数：
//      无
//返回值：0，创建设备失败
//        其它值为设备句柄，在以后的设备相关操作中均要使用其返回值

void _stdcall EM9118_DeviceClose( EM9118_DevPara* pDev );
//函数功能：关闭设备，同时释放设备所占用资源
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//返回值：无

I32 _stdcall EM9118_CmdConnect( EM9118_DevPara* pDev, char* strIP, I32 ipBC, I32 cmdPort, I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：连接命令端口
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//             strIP：设备IP
//              ipBC：strIP的长度，此参数为0时表示strIP是以0为结束的字符串。
//           cmdPort：命令端口号。
//         timeOutMS：超时毫秒数。
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_DataConnect( EM9118_DevPara* pDev, I32 dataPort, I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：连接数据端口，调用此函数之前必须先调用EM9118_CmdConnect
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//          dataPort：数据端口号。
//返回值：0表示成功，<0表示失败

void _stdcall EM9118_CmdClose( EM9118_DevPara* pDev, I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：关闭命令端口
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//返回值：0表示成功，<0表示失败

void _stdcall EM9118_DataClose( EM9118_DevPara* pDev, I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：关闭数据端口
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_AdChIsInFifo( EM9118_DevPara* pDev, I32 isInFifo[18], I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：设置AD通道数据是否进入缓冲区，只有指定的通道才能够进入缓冲区
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//          isInFifo：18字节数组，每一个元素对应一路AD，0表示对应通道不进入FIFO,1表示进入FIFO
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_AdSetRange( EM9118_DevPara* pDev, I32 rangeInx, I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：设置AD采集范围，注意要与设备跳线一致
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//          rangeInx：采集范围
//                    0，±10V
//                    1，±5V
//                    2，0~20mA
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_AdReadZeroFullCode( EM9118_DevPara* pDev, I16 zeroCode[18], I16 fullCode[18], I32 timeOutMS = EM9118_TIMOUT_MS );
//读取AD零点满度，本命令一般用户使用不到

I32 _stdcall EM9118_AdWriteZero( EM9118_DevPara* pDev, I32 chNo, I16 zeroCode, I32 timeOutMS = EM9118_TIMOUT_MS );
//写入AD零点，本命令一般用户使用不到

I32 _stdcall EM9118_AdWriteFull( EM9118_DevPara* pDev, I32 chNo, I16 fullCode, I32 timeOutMS = EM9118_TIMOUT_MS );
//写入AD满度，本命令一般用户使用不到

I32 _stdcall EM9118_AdChGetCode( EM9118_DevPara* pDev, I32 chInx, U32 bytesCount, I8* codeBuffer, I16* adCode, U32* adCodeCount );
//函数功能：将从缓冲区中获得指定通道的AD原码值，缓冲区数据调用EM9118_HcReadData得到
//入口参数：
//         pDev：设备句柄，EM9118_DeviceCreate函数返回值
//           chInx：通道索引，0~实际使能通道数-1，这里的通道索引是指AD使能通道的序号，比如使能了AD5和AD6两个通道，则chInx=0时表示要取得AD5的值，chInx=1时表示要取得AD6的值
//      codeBuffer：数据缓冲区，和EM9118_HcReadData应该是相同的
//出口参数：
//            adCode：AD原码值，需要用户分配空间
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_AdChCodeToValue( EM9118_DevPara* pDev, I32 chNo, I16 iCode, F64* adValue );
//函数功能：将AD原码值转换成物理值
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//              chNo：通道号，1~18
//             iCode：原码值
//出口参数：
//           adValue：AD物理值转换结果，电压值，需要用户分配空间
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_AdChBatchCodeToValue( EM9118_DevPara* pDev, I32 chNo, I16* pCode, I32 codeCount, F64* adValue );
//函数功能：将AD一批原码值转换成物理值
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//              chNo：通道号，1~18
//             pCode：原码值数组，16位有符号整形，一般调用EM9118_AdChGetCode得到
//         codeCount：pCode数组所包含元素个数
//出口参数：
//           adValue：AD物理值转换结果，电压值，需要用户分配空间，其元素个数应该是codeCount
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_HcSetGroupFreq( EM9118_DevPara* pDev,  F64 groupFreq, I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：设置组频率
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//         groupFreq：组频率，1~450000Hz
//出口参数：
//                无
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_HcSetTriCount( HANDLE pDev, U32 triCount, I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：设置外触发组数，此函数只有外触发且边沿触发时才有效
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//          triCount：触发组数，1～4294967295
//返回值：0表示成功，<0表示失败


I32 _stdcall EM9118_HcStart( EM9118_DevPara* pDev, I32 clkSrc, I32 triSrc, I32 edgeOrLevel, I32 upOrDown, I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：启动硬件控制采集
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//            clkSrc：时钟源，0内时钟，1外时钟
//            triSrc：触发源，0内部，1外部
//       edgeOrLevel：0，外触发时边沿启动采集；1，外触发时电平启动采集
//          upOrDown：0，外触发时上升沿（高电平）启动采集；1，外触发时下降沿（低电平）启动采集
//出口参数：
//                无
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_HcStop( EM9118_DevPara* pDev, I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：停止硬件控制采集
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//出口参数：
//                无
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_HcReadData( EM9118_DevPara* pDev, I32 byteCount, I8* codeBuffer, I32* realByteCount, I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：在硬件控制采集启动后，使用此函数读回采集结果AD原码值
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//         byteCount：要读取的字节数。
//出口参数：
//        codeBuffer：数据缓冲区数组，需要用户分配，元素个数至少为byteCount
//     realByteCount：实际读回数据字节数，需要用户分配
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_CtChGetCode( EM9118_DevPara* pDev, I32 chInx, U32 bytesCount, I8* codeBuffer, U32* ctCode, U32* ctCodeCount );
//函数功能：将从缓冲区中获得指定通道的计数器原码值，缓冲区数据调用EM9118_HcReadData得到
//入口参数：
//         pDev：设备句柄，EM9118_DeviceCreate函数返回值
//           chInx：通道索引，0~实际使能通道数-1，这里的通道索引是值离线文件使能通道的序号，比如使能了CT2和CT3两个通道，则chInx=0时表示要取得CT2的值，chInx=1时表示要取得CT3的值
//      codeBuffer：数据缓冲区，和EM9118_HcReadData应该是相同的
//出口参数：
//          ctCode：计数器原码值，需要用户分配空间
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_CtChBatchCodeToValue( EM9118_DevPara* pDev, I32 chNo, U32* pCode, I32 codeCount, F64* ctValue );
//函数功能：将计数器一批原码值转换成物理值，计数时转换结果就是计数值，测频转换结果是频率值
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//              chNo：通道号，1~4
//             pCode：原码值数组，32位无符号整形，一般调用EM9118_AdChGetCode得到
//         codeCount：pCode数组所包含元素个数
//出口参数：
//           ctValue：计数时是计数值，测频时是频率值，需要用户分配空间，其元素个数应该是codeCount
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_EcChGetCode( EM9118_DevPara* pDev, I32 chInx, U32 bytesCount, I8* codeBuffer, I32* ecCode, U32* ecCodeCount );
//函数功能：将从缓冲区中获得指定通道的编码器原码值，缓冲区数据调用EM9118_HcReadData得到
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//           chInx：通道索引，0~实际使能通道数-1，这里的通道索引是值离线文件使能通道的序号
//        codeBuffer：数据缓冲区，和EM9118_HcReadData应该是相同的
//出口参数：
//            ecCode：编码器原码值，需要用户分配空间
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_DoSetMode( EM9118_DevPara* pDev, I8 doMode1, I8 doMode2, I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：设置DO管脚功能
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//           doMode1：DO1功能，0开关量输出;1,PWM1;2,OCLK;
//           doMode2：DO2功能，0开关量输出;1,PWM2;
//出口参数：
//                  无
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_DiAll( EM9118_DevPara* pDev, I8 iStatus[4], I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：得到所有通道开关量值
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//出口参数：
//           iStatus：4字节数组，iStatus[0]~iStatus[3]对应DI1~DI4
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_DoAll( EM9118_DevPara* pDev, I8 oStatus[2], I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：设置所有通道开关量值，如果对应开关量方向设置为输入，则对应开关量的设置无效
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//           oStatus：2字节数组，oStatus[0]~oStatus[1]对应DO1~DO2
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_EcClear( EM9118_DevPara* pDev, I32 chClear[EM9118_MAXECCHCNT], I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：编码器计数值清零
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//           chClear：2元素数组，每一个元素对应一路编码器，1表示对应通道清零,0表示对应通道不清零
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_EcReadCodeAll( EM9118_DevPara* pDev, I32 abCode[2], I32 zCode[2], I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：读取编码器返回值
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//出口参数：
//            abCode：编码器AB值，32位有符号数组，其元素个数为2，需要用户分配空间。
//             zCode：编码器过零值，设备暂时不支持此功能
//
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_CtChIsInFifo( EM9118_DevPara* pDev, I32 isInFifo[EM9118_MAXCTCHCNT], I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：设置计数器通道是否进入FIFO
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//          isInFifo：4元素数组，每一个元素对应一路计数器，0表示对应通道不进入FIFO,1表示进入FIFO
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_CtSetFreqBase( EM9118_DevPara* pDev, I32 chNo, F64 freqBase_ms, F64* real_ms, I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：设置指定通道的计数器测频基准脉冲周期，以毫秒为单位
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//              chNo：通道号，1~4
//       freqBase_ms：测频基准周期，以毫秒为单位,0.001~1000
//出口参数：
//           real_ms：实际测频基准，大多数情况和freqBase_ms一致
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_CtSetModeAll( EM9118_DevPara* pDev, I8 ctMode[EM9118_MAXCTCHCNT], I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：设置所有路计数器的工作方式
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//            ctMode：计数器工作方式，4元素整形数组
//                     0，计数
//                     1，测高频
//                     2，清零，设成此值后需要再设回相应功能设备才能正确工作
//                     3，测低频
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_CtClear( EM9118_DevPara* pDev, I32 chNo, I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：指定通道计数器清零，本函数只有在计数方式下才有意义，因此执行后会将相应通道设置为计数工作方式	
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//              chNo：通道号，1~4
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_CtReadCodeAll( EM9118_DevPara* pDev, U32 ctCode[EM9118_MAXCTCHCNT], I32 timeOutMS );
//函数功能：读取所有路计数器的原码值
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//出口参数：
//            ctCode：计数器原码值，如果是计数方式就是计数值，如果是测频则需要计算才能转化成频率值
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_EcChIsInFifo( EM9118_DevPara* pDev, I32 isInFifo[EM9118_MAXECCHCNT], I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：设置编码器通道是否进入FIFO
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//          isInFifo：2元素数组，每一个元素对应一路编码器，0表示对应通道不进入FIFO,1表示进入FIFO
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_GetFifoAdChCount( EM9118_DevPara* pDev );
I32 _stdcall EM9118_GetFifoCtChCount( EM9118_DevPara* pDev );
I32 _stdcall EM9118_GetFifoEcChCount( EM9118_DevPara* pDev );

I32 _stdcall EM9118_GetFifoGroupByteCount( EM9118_DevPara* pDev, I32* byteCount );
//函数功能：根据设置，得到FIFO中每组数据的字节数
//         调用此函数后如果调用下面的函数，每组字节数可能会发生变化
//         EM9118_AdChIsInFifo
//         EM9118_CtChIsInFifo
//         EM9118_EcChIsInFifo
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//出口参数：
//         byteCount：返回每组字节数
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_PwmStartAll( EM9118_DevPara* pDev, I8 startOrStop[EM9118_MAXPWMCHCNT], I8 is90[EM9118_MAXPWMCHCNT], I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：设定所有通道的PWM输出开始或者停止
//入口参数：
//                pDev：设备句柄，EM9118_DeviceCreate函数返回值
//            startOrStop：启动设置数组，数组元素0~1对应PWM1~PWM2，以PWM1为例：
//                         startOrStop[0]=0，停止输出
//                         startOrStop[0]=1，启动输出
//                   is90：相位设置数组，数组元素0~1对应PWM1~PWM2，以PWM1为例：
//                         is90[0]=0，输出滞后90度
//                         is90[0]=1，输出不滞后90度
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_PwmSetPulse( EM9118_DevPara* pDev, I32 chNo, double freq, double dutyCycle, double* realFreq, double* realDutyCycle, I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：设定指定通道的PWM输出各项参数
//入口参数：
//                pDev：设备句柄，EM9118_DeviceCreate函数返回值
//                   chNo：通道号，1~2
//                   freq：输出频率。160Hz~100KHz，在某些频率下会有误差。
//              dutyCycle：占空比。0~1，具体调节档位取决于输出频率，输出频率越低，可调节的档位越多。
//出口参数：
//               realFreq：真实频率值，由于数字量化误差，设定频率和真实频率之间会有些误差
//          realDutyCycle：真实占空比值，由于数字量化误差，设定占空比和真实占空比之间会有些误差
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_PwmIsOver( EM9118_DevPara* pDev, I8 isOver[EM9118_MAXPWMCHCNT], I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：在制定脉冲数出个数的情况下，判断PWM输出是否完成
//入口参数：
//                pDev：设备句柄，EM9118_DeviceCreate函数返回值
//                 isOver：是否完成，数组元素0~1对应PWM1~2，以PWM1为例：
//                           isOver[0]=0，输出没有完成
//                           isOver[0]=1，输出已经完成
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_PwmSetCount( EM9118_DevPara* pDev, I32 chNo, U32 setCount, I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：设定输出脉冲个数
//入口参数：
//                pDev：设备句柄，EM9118_DeviceCreate函数返回值
//                   chNo：通道号，1~2
//               setCount：脉冲个数，如果设为，则表示连续输出
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_CSVOpen( EM9118_DevPara* pDev, char* dirPath, I32 pathLen );
//函数功能：初始化CVS历史数据写入操作函数。
//入口参数：
//          devNo：设备号，从1开始
//        dirPath：存放历史文件的路径，其最大长度不能超过256
//        pathLen：路径名长度，如果为0表示disPath是以0结尾的字符串。
//                程序将会在指定目录下自动建立“yy-mm-dd-hh-MM-ss-1.cvs”文件，依次是年月日时分秒,-1表示第一个文件，每个文件512MB左右。超过512MB的文件将会顺次增加-2，-3……
//出口参数：
//                无
//返回值：0表示没有错误

I32 _stdcall EM9118_CSVClose( EM9118_DevPara* pDev );
//函数功能：关闭历史数据文件
//入口参数：
//          devNo：设备号，从1开始
//出口参数：
//                无
//返回值：0表示没有错误

I32 _stdcall EM9118_CSVWriteValue( EM9118_DevPara* pDev, I8* pCode, U32 codeBC );
//函数功能：将指定的采集原码值转换成物理值后写入到数据文件中，要先调用EM9118_CVSOpen，否则数据将无法保存。
//入口参数：
//          devNo：设备号，从1开始
//          pCode：装有原码值的数组
//         codeBC：数组字节数
//出口参数：
//                无
//返回值：0表示没有错误

I32 _stdcall EM9118_DFWOpen( EM9118_DevPara* pDev, char* dirPath, I32 pathLen );
//函数功能：初始化历史数据写入操作函数。
//入口参数：
//          devNo：设备号，从1开始
//        dirPath：存放历史文件的路径，其最大长度不能超过256
//        pathLen：路径名长度，如果为0表示disPath是以0结尾的字符串。
//                程序将会在指定目录下自动建立“yy-mm-dd-hh-MM-ss-1.dat”文件，依次是年月日时分秒,-1表示第一个文件，每个文件512MB左右。超过512MB的文件将会顺次增加-2，-3……
//出口参数：
//                无
//返回值：0表示没有错误

I32 _stdcall EM9118_DFWClose( EM9118_DevPara* pDev );
//函数功能：关闭历史数据文件
//入口参数：
//          devNo：设备号，从1开始
//出口参数：
//                无
//返回值：0表示没有错误

I32 _stdcall EM9118_DFWrite( EM9118_DevPara* pDev, I8* pCode, U32 codeBC );
//函数功能：将指定的采集原码值写入到数据文件中，要先调用EM9118_DFWOpen，否则数据将无法保存。
//入口参数：
//          devNo：设备号，从1开始
//          pCode：装有原码值的数组
//         codeBC：数组字节数
//出口参数：
//                无
//返回值：0表示没有错误

I32 _stdcall EM9118_DFROpen( char* filePathName, I32 pathLen );
//函数功能：初始化历史数据读取操作函数
//入口参数：
//        dirPath：包含路径名的历史文件名
//出口参数：
//                无
//返回值：非0值表示文件句柄
//        0表示打开文件失败。

I32 _stdcall EM9118_DFRClose( I32 hDFR );
//函数功能：初始化历史数据读取操作函数
//入口参数：
//           hDFR：历史文件句柄
//出口参数：
//                无
//返回值：无

I32 _stdcall EM9118_DFRCodeBufferSize( I32 hDFR, U32 groupCount, U32* bufferSize );
//函数功能：初始化历史数据读取操作函数
//入口参数：
//           hDFR：历史文件句柄
//     groupCount：要读取的每通道数据个数。
//出口参数：
//     bufferSize：缓冲区元素个数，每个元素是16位整形，可以使用这个值给数据缓冲区分配空间。
//返回值：0值成功
//        <0表示失败，具体请参考EM9118_Error.H。

I32 _stdcall EM9118_DFRReadCodeByGroupInx( I32 hDFR, I8* codeBuf, U32 groupCodeCount, F64 codeBeginInx, U32* realReadCount );
//函数功能：从历史文件指定位置读出数据
//入口参数：
//              hDFR：历史文件句柄
//           codeBuf：数据缓冲区，需要用户分配空间，其大小可以调用EM9118_DFRCodeBufferSize得到
//    groupCodeCount：要读取的每通道数据个数。
//      codeBeginInx：读取开始的位置。如果为-1，则表示从上一次读取的结束开始读取。
//出口参数：
//     realReadCount：实际读取的每通道数据个数。
//返回值：0值成功
//        <0表示失败，具体请参考EM9118_Error.H。

I32 _stdcall EM9118_DFRAdChGetCode( I32 hDFR, I32 chInx, U32 bytesCount, I8* originalCode, I16* adCode, U32* adCodeCount );
//函数功能：将从缓冲区中获得指定通道的AD原码值，缓冲区数据调用EM9118_DFRReadCodeByGroupInx得到
//入口参数：
//            hDFR：历史文件句柄
//           chInx：通道索引，0~实际使能通道数-1，这里的通道索引是指AD使能通道的序号，比如使能了AD5和AD6两个通道，则chInx=0时表示要取得AD5的值，chInx=1时表示要取得AD6的值
//      codeBuffer：数据缓冲区，和EM9118_DFRReadCodeByGroupInx应该是相同的
//出口参数：
//            adCode：AD原码值，需要用户分配空间
//       adCodeCount：AD原码值个数，需要用户分配空间
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_DFRAdCodeToValue( I32 hDFR, I32 chNo, I16 adCode, F64* adValue );
//函数功能：AD原码转换成物理值
//入口参数：
//              hDFR：历史文件句柄
//              chNo：通道号，1~18
//            adCode：AD原码值
//出口参数：
//           adValue：转换结果，电压以V为单位，电流以mA为单位
//返回值：0值成功
//        <0表示失败，具体请参考EM9118_Error.H。

I32 _stdcall EM9118_DFRAdBatchCodeToValue( I32 hDFR, I32 chNo, I16* pCode, I32 codeCount, F64* adValue );
//函数功能：将一批AD原码转换成物理值
//入口参数：
//              hDFR：历史文件句柄
//              chNo：通道号，1~18
//             pCode：AD原码值缓冲区地址
//         codeCount：缓冲区数据个数
//出口参数：
//           adValue：转换结果，电压以V为单位，电流以mA为单位
//返回值：0值成功
//        <0表示失败，具体请参考EM9118_Error.H。

I32 _stdcall EM9118_DFRGetAdChValue( I32 hDFR, I32 chNo, I8* codeBuf, U32 groupCodeCount, F64* adValue );
//此函数只是为了保持和老程序的兼容性，一遍用户无需使用

I32 _stdcall EM9118_DFRCtChGetCode( I32 hDFR, I32 chInx, U32 bytesCount, I8* originalCode, U32* ctCode, U32* ctCodeCount );
//函数功能：将从缓冲区中获得指定通道的AD原码值，缓冲区数据调用EM9118_DFRReadCodeByGroupInx得到
//入口参数：
//            hDFR：历史文件句柄
//           chInx：通道索引，0~实际使能通道数-1，这里的通道索引是指CT使能通道的序号，比如使能了CT2和CT3两个通道，则chInx=0时表示要取得CT2的值，chInx=1时表示要取得CT3的值
//      codeBuffer：数据缓冲区，和EM9118_DFRReadCodeByGroupInx应该是相同的
//出口参数：
//          ctCode：CT原码值，需要用户分配空间
//     ctCodeCount：CT原码值个数，需要用户分配空间
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_DFRCtBatchCodeToValue( I32 hDFR, I32 chNo, U32* pCode, I32 codeCount, F64* ctValue );
//函数功能：CT原码转换成物理值
//入口参数：
//              hDFR：历史文件句柄
//              chNo：通道号，1~4
//            ctCode：CT原码值
//出口参数：
//           ctValue：转换结果，计数无单位，测频以Hz为单位
//返回值：0值成功
//        <0表示失败，具体请参考EM9118_Error.H。

I32 _stdcall EM9118_DFREcChGetCode( I32 hDFR, I32 chInx, U32 bytesCount, I8* originalCode, I32* ctCode, U32* ctCodeCount );
//函数功能：将从缓冲区中获得指定通道的AD原码值，缓冲区数据调用EM9118_DFRReadCodeByGroupInx得到
//入口参数：
//            hDFR：历史文件句柄
//           chInx：通道索引，0~实际使能通道数-1，这里的通道索引是指CT使能通道的序号，比如使能了CT2和CT3两个通道，则chInx=0时表示要取得CT2的值，chInx=1时表示要取得CT3的值
//      codeBuffer：数据缓冲区，和EM9118_DFRReadCodeByGroupInx应该是相同的
//出口参数：
//          ctCode：CT原码值，需要用户分配空间
//     ctCodeCount：CT原码值个数，需要用户分配空间
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9318_DFRIoGetCode( I32 hDFR, U32 bytesCount, I8* originalCode, U16* ioCode, U32* ctCodeCount );
//函数功能：将从缓冲区中获得指定通道的IO原码值，缓冲区数据调用EM9118_DFRReadCodeByGroupInx得到
//入口参数：
//            hDFR：历史文件句柄
//      codeBuffer：数据缓冲区，和EM9118_DFRReadCodeByGroupInx应该是相同的
//出口参数：
//          ioCode：IO原码值，需要用户分配空间
//     ctCodeCount：IO原码值个数，需要用户分配空间
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_DFRGetFreq( I32 hDFR, F64* daqFreq );
//函数功能：得到历史文件的采集频率
//入口参数：
//              hDFR：历史文件句柄
//出口参数：
//           daqFreq：采集频率，需要用户分配空间
//返回值：0值成功
//        <0表示失败，具体请参考EM9118_Error.H。

I32 _stdcall EM9118_DFRGetGroupTotal( I32 hDFR, F64* groupTotal );
//函数功能：得到历史文件所包含的数据总组数，使用这个值可以和采集频率计算出历史数据时长
//入口参数：
//              hDFR：历史文件句柄
//出口参数：
//        groupTotal：总组数
//返回值：0值成功
//        <0表示失败，具体请参考EM9118_Error.H。

I32 _stdcall EM9118_DFRGetAdIsInfifo( I32 hDFR, I8* isInFifo );
//函数功能：得到历史文件所包含的使能AD通道
//入口参数：
//              hDFR：历史文件句柄
//出口参数：
//     isInFifo：18字节数组，0~17元素对应AD1~AD18
//               0表示相应通道禁止，1表示相应通道使能
//返回值：0值成功
//        <0表示失败，具体请参考EM9118_Error.H。

I32 _stdcall EM9118_DFRGetCtIsInfifo( I32 hDFR, I8* isInFifo );
//函数功能：得到历史文件所包含的使能计数器通道
//入口参数：
//              hDFR：历史文件句柄
//出口参数：
//     isInFifo：4字节数组，0~3元素对应计数器1~计数器18
//               0表示相应通道禁止，1表示相应通道使能
//返回值：0值成功
//        <0表示失败，具体请参考EM9118_Error.H。

I32 _stdcall EM9118_DFRGetEcIsInfifo( I32 hDFR, I8* isInFifo );
//函数功能：得到历史文件所包含的使能计数器通道
//入口参数：
//              hDFR：历史文件句柄
//出口参数：
//     isInFifo：2字节数组，0~1元素对应编码器1~编码器18
//               0表示相应通道禁止，1表示相应通道使能
//返回值：0值成功
//        <0表示失败，具体请参考EM9118_Error.H。

I32 _stdcall EM9118_DFRGetClkSource( I32 hDFR, I32* clkSource );
//函数功能：得到历史文件所包含的时钟源
//入口参数：
//              hDFR：历史文件句柄
//出口参数：
//         clkSource：时钟源
//返回值：0值成功
//        <0表示失败，具体请参考EM9118_Error.H。

I32 _stdcall EM9118_DFRGetEtrSource( I32 hDFR, I32* etrSource );
//函数功能：得到历史文件所包含的时钟源
//入口参数：
//              hDFR：历史文件句柄
//出口参数：
//         etrSource：触发源
//返回值：0值成功
//        <0表示失败，具体请参考EM9118_Error.H。

I32 _stdcall EM9118_OlFileOpen( I8* filePathName );
//函数功能：打开数据文件
//入口参数：
//      filePathName：文件路径
//返回值：-1,打开文件失败
//        其它值为文件句柄

I32 _stdcall EM9118_OlFileClose( I32 fileHandle );
//函数功能：关闭数据文件
//入口参数：
//      fileHandle：文件句柄，EM9118_OlFileOpen的返回值
//返回值：-1,文件句柄无效
//        0表示关闭成功

I32 _stdcall EM9118_OlGetDataBytesCount( I32 fileHandle );
//函数功能：从离线数据文件中得到总数据量，以字节位单位
//入口参数：
//      fileHandle：文件句柄，EM9118_OlFileOpen的返回值
//返回值：>0,总数据量
//        <0,表示读取失败

I32 _stdcall EM9118_OlGetAdEnCh( I32 fileHandle, I8 enCh[EM9118_MAXADCHCNT] );
//函数功能：从离线数据文件中得到AD使能通道数
//入口参数：
//      fileHandle：文件句柄，EM9118_OlFileOpen的返回值
//出口参数：
//            enCh：返回AD的使能通道，长度为18的字节数组
//返回值：>=0,返回使能通道数量
//        <0,表示读取失败

I32 _stdcall EM9118_OlGetCtEnCh( I32 fileHandle, I8 enCh[EM9118_MAXCTCHCNT] );
//函数功能：从离线数据文件中得到计数器使能通道数
//入口参数：
//      fileHandle：文件句柄，EM9118_OlFileOpen的返回值
//出口参数：
//            enCh：返回AD的使能通道，长度为18的字节数组
//返回值：>=0,返回使能通道数量
//        <0,表示读取失败

I32 _stdcall EM9118_OlGetEcEnCh( I32 fileHandle, I8 enCh[EM9118_MAXCTCHCNT] );
//函数功能：从离线数据文件中得到编码器使能通道数
//入口参数：
//      fileHandle：文件句柄，EM9118_OlFileOpen的返回值
//出口参数：
//            enCh：返回AD的使能通道，长度为18的字节数组
//返回值：>=0,返回使能通道数量
//        <0,表示读取失败

I32 _stdcall EM9118_OlGetGroupBytesCount( I32 fileHandle );
//函数功能：从离线数据文件中得到每组字节数，也就是每次采样所获得的数据字节数
//入口参数：
//      fileHandle：文件句柄，EM9118_OlFileOpen的返回值
//返回值：>0,每组字节数
//        <0,表示读取失败

I32 _stdcall EM9118_OlGetGroupFreq( I32 fileHandle, F64* groupFreq );
//函数功能：从离线数据文件中得到组频率值
//入口参数：
//      fileHandle：文件句柄，EM9118_OlFileOpen的返回值
//出口参数：
//       groupFreq：以Hz为单位的采集频率
//返回值：=0,表示成功

I32 _stdcall EM9118_OlGetOriginalCode( I32 fileHandle, I32 readPos, U32 bytesCount, U8* originalCode, U32* realBC );
//函数功能：从离线数据文件中获取原始数据
//入口参数：
//      fileHandle：文件句柄，EM9118_OlFileOpen的返回值
//         readPos：读取位置，第一个采集通道第一个采集点的读取位置为0，如果为-1表示从当前位置读取
//      bytesCount：要读取的字节数。
//出口参数：
//    originalCode：数据缓冲区，需要用户分配，其大小为bytesCount
//          realBC：实际读取的字节数
//返回值：>0,返回文件当前指向的位置，可以用于确定下一次读取的位置
//        <0,表示读取失败

I32 _stdcall EM9118_OlDirOpen( I8* path );
//函数功能：打开数据文件目录，此函数将符合格式的目录打开，准备按照存储日期顺序分析文件
//入口参数：
//            path：目录的路径，此目录内部结构必须和离线文件存储盘的结构一致。
//                  离线文件存储盘目录结构：根目录是日期文件夹，每个日期文件夹内是数据文件。
//返回值：-1,打开文件失败
//        其它值为文件句柄

I32 _stdcall EM9118_OlGetAdChCode( I32 fileHandle, I32 chInx, U32 bytesCount, U8* originalCode, I16* adChCode, U32* adCodeCount );
//函数功能：从离线数据文件中获取AD原始数据
//入口参数：
//      fileHandle：文件句柄，EM9118_OlFileOpen的返回值
//           chInx：通道索引，0~实际使能通道数-1，这里的通道索引是值离线文件使能通道的序号，比如使能了AD5和AD6两个通道，则chInx=0时表示要取得AD5的值，chInx=1时表示要取得AD6的值
//      bytesCount：originalCode所包含数据的字节数。
//    originalCode：原始码缓冲区
//出口参数：
//        adChCode：指定通道的数据接收数组，需要用户分配空间，类型为16位有符号整型，数组大小为bytesCount/每组字节数，每组字节数可以由EM9118_OlGetGroupBytesCount函数得到，用户也可以分配bytesCount个数，然后再根据adCodeCount来截取数组。
//     adCodeCount：adChCode返回的有效数据个数，=0则表示不返回。
//返回值：<0,表示操作失败

I32 _stdcall EM9118_OlGetAdChValue( I32 fileHandle, I32 chInx, U32 bytesCount, U8* originalCode, F64* adChValue, U32* adValueCount );
//函数功能：从离线数据文件中获取AD物理值
//入口参数：
//      fileHandle：文件句柄，EM9118_OlFileOpen的返回值
//           chInx：通道索引，0~实际使能通道数-1，这里的通道索引是值离线文件使能通道的序号，比如使能了AD5和AD6两个通道，则chInx=0时表示要取得AD5的值，chInx=1时表示要取得AD6的值
//      bytesCount：originalCode所包含数据的字节数。
//    originalCode：原始码缓冲区
//出口参数：
//        adChValue：指定通道的数据接收数组，需要用户分配空间，类型为64位双精度浮点，数组大小为bytesCount/每组字节数，每组字节数可以由EM9118_OlGetGroupBytesCount函数得到，用户也可以分配bytesCount个数，然后再根据adCodeCount来截取数组。
//     adValueCount：adChValue返回的有效数据个数，=0则表示不返回。
//返回值：<0,表示操作失败

I32 _stdcall EM9118_OlGetCtChCode( I32 fileHandle, I32 chInx, U32 bytesCount, U8* originalCode, U32* ctChCode, U32* ctCodeCount );
//函数功能：从离线数据文件中获取计数器原始数据
//入口参数：
//      fileHandle：文件句柄，EM9118_OlFileOpen的返回值
//           chInx：通道索引，0~实际使能通道数-1，这里的通道索引是值离线文件使能通道的序号，比如使能了CT2和3两个通道，则chInx=0时表示要取得CT2的值，chInx=1时表示要取得CT3的值
//      bytesCount：originalCode所包含数据的字节数。
//    originalCode：原始码缓冲区
//出口参数：
//        ctChCode：指定通道的数据接收数组，需要用户分配空间，类型为32位无符号整型，数组大小为bytesCount/每组字节数，每组字节数可以由EM9118_OlGetGroupBytesCount函数得到，用户也可以分配bytesCount个数，然后再根据adCodeCount来截取数组。
//     ctCodeCount：ctChCode返回的有效数据个数，=0则表示不返回。
//返回值：<0,表示操作失败

I32 _stdcall EM9118_OlGetCtChValue( I32 fileHandle, I32 chInx, U32 bytesCount, U8* originalCode, F64* ctChValue, U32* ctValueCount );
//函数功能：从离线数据文件中获取计数器物理值，如果是计数器工作方式则是原码值，测频方式则是频率值
//入口参数：
//      fileHandle：文件句柄，EM9118_OlFileOpen的返回值
//           chInx：通道索引，0~实际使能通道数-1，这里的通道索引是值离线文件使能通道的序号，比如使能了CT2和3两个通道，则chInx=0时表示要取得CT2的值，chInx=1时表示要取得CT3的值
//      bytesCount：originalCode所包含数据的字节数。
//    originalCode：原始码缓冲区
//出口参数：
//       ctChValue：指定通道的数据接收数组，需要用户分配空间，类型为64位双精度浮点型，数组大小为bytesCount/每组字节数，每组字节数可以由EM9118_OlGetGroupBytesCount函数得到，用户也可以分配bytesCount个数，然后再根据ctValueCount来截取数组。
//    ctValueCount：ctChValue返回的有效数据个数，=0则表示不返回。
//返回值：<0,表示操作失败

I32 _stdcall EM9118_OlGetEcChCode( I32 fileHandle, I32 chInx, U32 bytesCount, U8* originalCode, I32* ecChCode, U32* ecCodeCount );
//函数功能：从离线数据文件中获取编码器原始数据
//入口参数：
//      fileHandle：文件句柄，EM9118_OlFileOpen的返回值
//           chInx：通道索引，0~实际使能通道数-1，这里的通道索引是值离线文件使能通道的序号
//      bytesCount：originalCode所包含数据的字节数。
//    originalCode：原始码缓冲区
//出口参数：
//        ecChCode：指定通道的数据接收数组，需要用户分配空间，类型为16位有符号整型，数组大小为bytesCount/每组字节数，每组字节数可以由EM9118_OlGetGroupBytesCount函数得到，用户也可以分配bytesCount个数，然后再根据adCodeCount来截取数组。
//     ecCodeCount：ecChCode返回的有效数据个数，=0则表示不返回。
//返回值：<0,表示操作失败

//////////////////////////////////和EM9106有关系的函数////////////////////////////////////////////////////
I32 _stdcall EM9106_IoSetDirAndMode( EM9118_DevPara* pDev, I8 ioDir[3], I8 doMode[4], I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：本命令针对EM9106B，设置IO输入输出方向和输出时复用管脚功能
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//             ioDir：开关量方向，3元素数组
//                    ioDir[0]对应IO1～IO8
//                    ioDir[1]对应IO9～IO16
//                    ioDir[2]对应IO17～IO24
//            doMode：IO管脚功能，4元素数组
//                    doMode[0]对应IO17，0表示用于开关量，1表示PWM1
//                    doMode[1]对应IO18，0表示用于开关量，1表示PWM2
//                    doMode[2]对应IO19，0表示用于开关量，1表示OCLK
//                    doMode[3]对应IO20，0表示用于开关量，1表示OTR
//出口参数：
//                  无
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9106_IoSetAll( EM9118_DevPara* pDev, I8 oStatus[EM9106_MAXIOCHCNT], I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：设置所有通道开关量值，如果对应开关量方向设置为输入，则对应开关量的设置无效
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//           oStatus：24字节数组，oStatus[0]~oStatus[23]对应IO1~IO24
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9106_IoGetAll( EM9118_DevPara* pDev, I8 iStatus[EM9106_MAXIOCHCNT], I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：得到所有通道开关量值
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//出口参数：
//           iStatus：24字节数组，iStatus[0]~iStatus[23]对应IO1~IO24
//返回值：0表示成功，<0表示失败

//////////////////////////////////和EM9106有关系的函数////////////////////////////////////////////////////
I32 _stdcall EM9318_IoSetMode( EM9118_DevPara* pDev, I32 chNo, I8 ioMode, I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：本命令针对EM9318，设置IO输入输出方向和输出时复用管脚功能
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//              chNo：通道号，1～8
//            ioMode：IO管脚功能
//                    0，开关量输入，1，开关量输出
//                    当设成开关量输入时，IO1～IO4同时可以接计数信号用于计数、测频
//                                        IO5～IO8同时可以接编码器信号
//                    当设成开关量输出时，IO5～IO8在PWM使能后将输出PWM信号，开关量控制无效
//出口参数：
//                  无
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9318_IoSetAll( EM9118_DevPara* pDev, I8 oStatus[EM9318_MAXIOCHCNT], I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：本命令针对EM9318，设置IO输出状态
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//           oStatus：8字节数组，oStatus[0]~oStatus[7]对应IO1~IO8
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9318_IoGetAll( EM9118_DevPara* pDev, I8 iStatus[EM9318_MAXIOCHCNT], I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：本命令针对EM9318，得到所有通道开关量值
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//出口参数：
//           iStatus：8字节数组，iStatus[0]~iStatus[8]对应IO1~IO8
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9318_PwmStartAll( EM9118_DevPara* pDev, I8 startOrStop[EM9318_MAXPWMCHCNT], I8 is90[EM9318_MAXPWMCHCNT], I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：本命令针对EM9318，设定所有通道的PWM输出开始或者停止
//入口参数：
//                pDev：设备句柄，EM9118_DeviceCreate函数返回值
//            startOrStop：启动设置数组，数组元素0~3对应PWM1~PWM4，以PWM1为例：
//                         startOrStop[0]=0，停止输出
//                         startOrStop[0]=1，启动输出
//                   is90：相位设置数组，数组元素0~3对应PWM1~PWM4，以PWM1为例：
//                         is90[0]=0，输出滞后90度
//                         is90[0]=1，输出不滞后90度
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9318_PwmSetPulse( EM9118_DevPara* pDev, I32 chNo, double freq, double dutyCycle, double* realFreq, double* realDutyCycle, I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：本命令针对EM9318，设定指定通道的PWM输出各项参数
//入口参数：
//                pDev：设备句柄，EM9118_DeviceCreate函数返回值
//                   chNo：通道号，1~4
//                   freq：输出频率。1Hz~500KHz，在某些频率下会有误差。
//              dutyCycle：占空比。0~1，具体调节档位取决于输出频率，输出频率越低，可调节的档位越多。
//出口参数：
//               realFreq：真实频率值，由于数字量化误差，设定频率和真实频率之间会有些误差
//          realDutyCycle：真实占空比值，由于数字量化误差，设定占空比和真实占空比之间会有些误差
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9318_PwmIsOver( EM9118_DevPara* pDev, I8 isOver[EM9318_MAXPWMCHCNT], I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：本命令针对EM9318，在制定脉冲数出个数的情况下，判断PWM输出是否完成
//入口参数：
//                pDev：设备句柄，EM9118_DeviceCreate函数返回值
//                 isOver：是否完成，数组元素0~3对应PWM1~4，以PWM1为例：
//                           isOver[0]=0，输出没有完成
//                           isOver[0]=1，输出已经完成
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9318_PwmSetCount( EM9118_DevPara* pDev, I32 chNo, U32 setCount, I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：本命令针对EM9318，设定输出脉冲个数
//入口参数：
//                pDev：设备句柄，EM9118_DeviceCreate函数返回值
//                   chNo：通道号，1~4
//               setCount：脉冲个数，如果设为，则表示连续输出
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9318_EcClear( EM9118_DevPara* pDev, I32 chClear[EM9318_MAXECCHCNT], I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：本命令针对EM9318，编码器计数值清零
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//           chClear：4元素数组，每一个元素对应一路编码器，1表示对应通道清零,0表示对应通道不清零
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9318_EcReadCodeAll( EM9118_DevPara* pDev, I32 abCode[EM9318_MAXECCHCNT], I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：本命令针对EM9318，读取编码器返回值
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//出口参数：
//            abCode：编码器AB值，4元素32位有符号数组，每个元素对应一路编码器，需要用户分配空间。
//
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9118_DiIsInFifo( EM9118_DevPara* pDev, I32 isInFifo, I32 timeOutMS = EM9118_TIMOUT_MS );
//函数功能：本命令针对EM9318和EM9118定制产品，设置开关量输入进入FIFO
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//          isInFifo：0表示不进入FIFO，1表示进入FIFO
//
//返回值：0表示成功，<0表示失败

I32 _stdcall EM9318_IoGetCode( EM9118_DevPara* pDev, U32 bytesCount, I8* originalCode, U16* diCode, U32* diCodeCount );
//函数功能：本命令针对EM9318和EM9118定制产品,将从缓冲区中获得指定通道的开关量值，缓冲区数据调用EM9118_HcReadData得到
//入口参数：
//           pDev：设备句柄，EM9118_DeviceCreate函数返回值
//        codeBuffer：数据缓冲区，和EM9118_HcReadData应该是相同的
//出口参数：
//            diCode：开关量返回值，需要用户分配空间
//            diCodeCount：数据个数
//返回值：0表示成功，<0表示失败


#endif