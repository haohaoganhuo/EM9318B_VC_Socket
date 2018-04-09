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

const I32 ADBC = 2;//һ��ADԭ����ռ���
const I32 CTBC = 4;//һ��CTԭ����ռ���
const I32 ECBC = 4;//һ��ECԭ����ռ���
const I32 IOBC = 2;//һ��IOԭ����ռ���

inline F64 AdCodeToValue( I16 iCode, I16 zeroCode, I16 fullCode, F64 zeroValue, F64 fullValue );

typedef struct EM9118_ADPARA
{//AD��ز���
	I8 isInFifo[EM9118_MAXADCHCNT];//��ֹͨ����
	I16 fullCode[EM9118_MAXADCHCNT];
	I16 zeroCode[EM9118_MAXADCHCNT];
	F64 fullValue[EM9118_MAXADCHCNT];
	F64 zeroValue[EM9118_MAXADCHCNT];
	I32 rangeInx;//�ɼ���Χ����μ���AD�ɼ���Χ��������
}EM9118_AdPara;

typedef struct EM9118_IOPARA
{//��������ز���
	I8 doMod[2];//2·����������
}EM9118_IoPara;

typedef struct EM9118_CTPARA
{//��������ز���
	I8 mode[EM9118_MAXCTCHCNT];//������ʽ�������뿴��������ʽ�궨�塱
	double freqBase[EM9118_MAXCTCHCNT];//��Ƶϵ��ʱ������msΪ��λ���ڲ����ߵ�Ƶ��ʱ���岻ͬ�����㷽���뿴ʾ������
}EM9118_CtPara;

typedef struct EM9118_COMMONPARA
{
	I32 adChCount;
	I32 ctChCount;
	I32 ecChCount;
	I32 ioInFifo;
}EM9118_CommonPara;

typedef struct EM9118_DEVPARA 
{//�豸���ò���
	EM9118_AdPara adPara;
	EM9118_IoPara ioPara;
	EM9118_CtPara ctPara;
	I8 ctInFifo[EM9118_MAXCTCHCNT];
	I8 ecInFifo[EM9118_MAXECCHCNT];
	I8 diInFifo;//���ֻ��Ϊ��ռλ�ã�Ŀǰû������
	EM9118_CommonPara commonPara;
	I32 groupBC;//ÿ�����ݵ��ֽ���
	F64 groupFreq;//���л�Ƶ��

	I32 isPick;//�Ƿ���Ҫ��ѡ����
	F64 realGroupFreq;//��groupFreq���ù���ʱ��Ϊ�˱�֤��λ��������1s�ڷ������ݣ���ʵ��Ƶ����죬Ȼ����ѡ���ݷ����Ƶ�ɼ�
	I32 pickCodeCnt ;//��groupFreq���ù���ʱ��ÿ�������Ŀ��ѡһ������

	I32 isHcStart;
	I32 clkSrc;//ʱ��Դ
	I32 triSrc;//����Դ
	I32 edgeOrLevel;
	I32 upOrDown;
	I8 strIP[256];//���IP��ַ
	HANDLE hCmd;//����˿ڲ������
	SOCKET dataFd;//���ݶ˿�
	CRITICAL_SECTION sectCmd;
	I32 TimeOutCnt;//��ʱ�����������ж������Ƿ��ж� //2014-04-24
	bool enableWtd;//�Ƿ�ʹ�ܿ��Ź� //2014-04-24
	string dataPathName;
	FILE* dfwF;
	FILE* csvF;
	I32 dfwPos;
	F64 csvLineInx;//��ǰд�������
	I32 singleFileSize;
	I32 csvLinesCount;//�������
	U32 triCount;
}EM9118_DevPara;

typedef struct DAQTIME//�ɼ�ʱ��� 
{
	//��ʼʱ��
	I8 startS;//��
	I8 startM;//��
	I8 startH;//ʱ
	//����ʱ��
	I8 endS;//��
	I8 endM;//��
	I8 endH;//ʱ
}DaqTime;

typedef struct OFFLINESET
{//������������������1ע������ṹ���Ѿ�����λ���̶��õģ���������Ķ���������������������
	short version;//�汾��
	//�ɼ�ʱ��������
	I16 intervalMS;//����
	I8 intervalS;//��
	I8 intervalM;//��
	I8 intervalH;//Сʱ
	I8 daqStyle; //daqStyle���ɼ���ʽ��
	//0��λ���ϵ������ɼ���
	//1��λ��������ͣ
	I8 startMode;//������ʽ 0 �������ADת�������ٲɼ��� 1 ��ʱ������ADת�� 2 ��ʱ������ADת�� 3 �ⲿ�ߵ�ƽ����AD�ɼ�
	I32 groupCount;//�ڸ��ٲɼ�ʱ����fifo�ж�ȡ���������ݶ�ȡһ�β�����fifo�����ݣ�io��ct��ec��
	I32 isPSync;//0��αͬ����1αͬ��
	I32 chFreq;//ͨ���л��ɼ�Ƶ��
	I32 groupFreq;//��ɼ�Ƶ�ʣ�αͬ����Ч
	I8 adBeginCh;//AD��ʼͨ��
	I8 adEndCh;//AD����ͨ��
	I8 adRange;//AD�ɼ���Χ
	I8 adGain;//AD ����
	I8 adEndMode;//AD��˫��
	I8 ioInFifo;//io1~io16�Ƿ����fifo
	I8 ioDir[4];//����������
	I8 ctMode[6];//��������ʽ
	I32 ctGateFreq[6];//�ſ��ź�Ƶ��
	I8 ad[EM9118_MAXADCHCNT];//��Ӧadͨ���Ƿ���̣�0��ֹ��1����
	I8 io[4];//��Ӧ��Ŀ������Ƿ���̣�0��ֹ��1����
	I8 ct[4];//������ͨ���Ƿ���̣�0��ֹ��1����
	I8 ec[2];//�����������Ƿ���̣�0��ֹ��1����
	I16 zCode[EM9118_MAXADCHCNT];//ȫ��18��ͨ�������ֵ
	I16 fCode[EM9118_MAXADCHCNT];//ȫ��18��ͨ��������ֵ
	F64 zValue[EM9118_MAXADCHCNT];//ȫ��18��ͨ�����������
	F64 fValue[EM9118_MAXADCHCNT];//ȫ��18��ͨ������������
	I8 daqTimeCnt;//�ɼ�ʱ��θ���
	U32 savelength; //��������������
	DaqTime* daqTime; //ʱ�����ֹʱ��
	U32 saveCtrlDport;//���̿����źŶ˿�(����)
	U32 saveCtrlDvalue;//���̿����ź�ֵ(����)
	U32 saveCtrlAport;//���̿����źŶ˿�(ģ��)
	I32 saveCtrlAvalue;//���̿����ź�ֵ(ģ��)
}OfflineSet;

typedef struct OFFLINEFILE{
	FILE* f;
	I32 dataBeginPos;//��ǰλ��
	I32 dataSize;
	OfflineSet os;
	I8 ctLogicToRealChInx[4];//��ȡ����ʱ�����ڼ�������ʹ��ͨ�������������ã�������������ͨ��˳����ܲ��������˴���Ϊ�˼�¼��һ��CT�����ǵڼ�ͨ����CT����
	EM9118_CommonPara commonPara;
}OfflineFile;

/*I32 _stdcall EM9118_Init();
//�������ܣ�����һЩ��Ҫ�ĳ�ʼ�����ã���ÿһ��AD�ɼ���ʼ��
//          ��һ�ε���EM9118_ExtractADCodeǰҪ���ô˺������Ժ󲻱��ٵ��á�
//����ֵ��0��ʾ�ɹ�

I32 _stdcall EM9118_ExtractADCode( U8* enCh, U16* chDiv, I16* codeBuffer, U32 codeCount, U32* chCodeCount );
//�������ܣ������ݰ������ò�ֳ�18·��
//��ڲ�����
//            enCh��ͨ��ʹ�����ã�����Ϊ18��8λ�޷������飬0��ʾͨ����ֹ��1��ʾͨ��ʹ��
//           chDiv��ͨ����Ƶ���ã�����Ϊ18��16λ�޷������飬ÿ��Ԫ��ֵΪ1~255
//      codeBuffer���洢��ADԭ��Ļ�����
//       codeCount��ԭ�뻺������С����16λ��Ϊ��λ
//���ڲ�����
//      hCodeCount����ֺ�ÿͨ��ԭ���������16λ��Ϊ��λ�����ڷ�Ƶ���Ӧͨ�������ݻ�����
//����ֵ��0��ʾ�ɹ���-1��ʾ�в�������

I32 _stdcall EM9118_GetADChCode( I32 chNo, I16* chCodeBuffer );
//�������ܣ���ȡָ��ͨ�������ݡ�
//��ڲ�����
//            chNo��ͨ���ţ�1~18
//���ڲ�����
//    chCodeBuffer�����ݻ����������û����䣬���С��EM9118_ExtractADCode��chCodeCount��������
//����ֵ��0��ʾ�ɹ�*/


#endif // !defined(AFX_EM9118_Server_H__D1413467_66A2_475B_8398_F579D95B3FB0__INCLUDED_)
