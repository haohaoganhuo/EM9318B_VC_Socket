#ifndef INIREADER_H
#define INIREADER_H
//#include <windows.h>
class CIniReader
{
public:
	CIniReader(LPCTSTR szFileName);
	int ReadInteger(LPCTSTR szSection, LPCTSTR szKey, int iDefaultValue);	
	float ReadFloat(LPCTSTR szSection, LPCTSTR szKey, float fltDefaultValue);
	bool ReadBoolean(LPCTSTR szSection, LPCTSTR szKey, bool bolDefaultValue);
	LPTSTR ReadString(LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szDefaultValue);
private:
	TCHAR m_szFileName[255];
	TCHAR _szReadString[255];
};

class CIniWriter
{
public:
	CIniWriter(LPCTSTR szFileName); 
	 void WriteInteger(LPCTSTR szSection, LPCTSTR szKey, int iValue);
	 void WriteFloat(LPCTSTR szSection, LPCTSTR szKey, float fltValue);
	 void WriteBoolean(LPCTSTR szSection, LPCTSTR szKey, bool bolValue);
	 void WriteString(LPCTSTR szSection, LPCTSTR szKey, LPCTSTR szValue);
private:
	TCHAR m_szFileName[255];
};
#endif //INIREADER_H