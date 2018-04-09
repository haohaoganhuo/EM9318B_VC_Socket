#ifndef EM9118_ERRORS_H
#define EM9118_ERRORS_H

//定义错误号
#define EM9118_SUCCESS                           0 //成功
#define EM9118_FAILURE                          -1 //失败
#define EM9118_TIMEOUT_ERROR                    -2 //超时错误
#define EM9118_CONNECT_ERROR                    -3 //网络连接错误，发生此错误后应该重新建立连接
#define EM9118_SEND_ERROR                       -4 //发送错误
#define EM9118_CMDVERIFY_ERROR                  -5 //发送命令校验错误
#define EM9118_RECVVERIFY_ERROR                 -6 //接收数据校验错误
#define EM9118_DATANOTREADY                     -7 //数据没有准备好
#define EM9118_PARA_ERROR                       -8 //参数错误
#define EM9118_FREQBASE_ERROR                   -9 //参数错误
#define EM9118_INITNET_ERROR                    -100 //初始化网络错误
#define EM9118_SOCKET_ERROR                     -101 //打开网络套接字错误
#define EM9118_FILEPATH_ERROR                   -201 //文件目录错误
#define EM9118_FILEWRITE_ERROR                  -202 //写文件错误
#define EM9118_FILEREAD_ERROR                   -203 //读文件错误
#define EM9118_FILESEEK_ERROR                   -204 //文件定位错误
#define EM9118_FILECLOSE_ERROR                  -205 //读文件错误

#endif
