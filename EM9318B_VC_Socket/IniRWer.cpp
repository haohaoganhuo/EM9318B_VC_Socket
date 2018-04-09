#include "stdafx.h"
#include "IniRWer.h"
#include <iostream>
//#include <windows.h>
CIniReader::CIniReader(LPCTSTR szFileName)
{
	memset(m_szFileName, 0x00, sizeof(m_szFileName));
	memcpy(m_szFileName, szFileName, _tcslen(szFileName)*sizeof(TCHAR));
}
int CIniReader::ReadInteger(LPCTSTR szSection, LPCTSTR szKey, int iDefaultValue)
{
	int iResult = GetPrivateProfileInt(szSection, szKey, iDefaultValue, m_szFileName); 
	return iResult;
}
float CIniReader::ReadFloat(LPCTSTR szSection, LPCTSTR szKey, float fltDefaultValue)
{
	TCHAR szResult[255];
	TCHAR szDefault[255];
	float fltResult;
	_stprintf_s(szDefault, 255, TEXT("%f"),fltDefaultValue);
	GetPrivateProfileString(szSection, szKey, szDefault, szResult, 255, m_szFileName);
	fltResult = (float)_tstof(szResult);
	return fltResult;
}
bool CIniReader::ReadBoolean(LPCTSTR szSection, LPCTSTR szKey, bool bolDefaultValue)
{
	TCHAR szResult[255];
	TCHAR szDefault[255];
	bool bolResult;
	_stprintf_s(szDefault, 255, TEXT("%s"), bolDefaultValue? TEXT("True") : TEXT("False"));
	GetPrivateProfileString(szSection, szKey, szDefault, szResult, 255, m_szFileName); 
	bolResult = (_tcscmp(szResult, TEXT("True")) == 0 || 
		_tcscmp(szResult, TEXT("true")) == 0) ? true : false;
	return bolResult;
}
LPTSTR CIniReader::ReadString(LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szDefaultValue)
{
	memset(_szReadString, 0x00, sizeof(_szReadString));
	GetPrivateProfileString(szSection, szKey, szDefaultValue, _szReadString, 255, m_szFileName); 
	return _szReadString;
}

CIniWriter::CIniWriter(LPCTSTR szFileName)
{
	memset(m_szFileName, 0x00, sizeof(m_szFileName));
	memcpy(m_szFileName, szFileName, _tcslen(szFileName)*sizeof(TCHAR));
}
void CIniWriter::WriteInteger(LPCTSTR szSection, LPCTSTR szKey, int iValue)
{
	TCHAR szValue[255];
	_stprintf_s(szValue, 255, TEXT("%d"), iValue);
	DWORD a = WritePrivateProfileString(szSection, szKey, szValue, m_szFileName);
	if( !a )
	{
		a = GetLastError();
	}
}
void CIniWriter::WriteFloat(LPCTSTR szSection, LPCTSTR szKey, float fltValue)
{
	TCHAR szValue[255];
	_stprintf_s(szValue, 255, TEXT("%f"), fltValue);
	WritePrivateProfileString(szSection, szKey, szValue, m_szFileName); 
}
void CIniWriter::WriteBoolean(LPCTSTR szSection, LPCTSTR szKey, bool bolValue)
{
	TCHAR szValue[255];
	_stprintf_s(szValue, 255, TEXT("%s"), bolValue ? TEXT("True") : TEXT("False"));
	WritePrivateProfileString(szSection, szKey, szValue, m_szFileName); 
}
void CIniWriter::WriteString(LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szValue)
{
	WritePrivateProfileString(szSection, szKey, szValue, m_szFileName);
}