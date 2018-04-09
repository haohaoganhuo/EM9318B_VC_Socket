#ifndef ZT_TYPE_H
#define ZT_TYPE_H

using namespace std;
#include <vector>

typedef char			I8;
typedef unsigned char	U8;
typedef short			I16;
typedef unsigned short	U16;
typedef long			I32;
typedef unsigned long	U32;
typedef double			F64;
typedef LONG64			I64;

typedef std::vector<I8> V_I8;
typedef std::vector<I16> V_I16;
typedef std::vector<U8> V_U8;
typedef V_U8::iterator IT_U8;
typedef V_U8::const_iterator CIT_U8;
typedef std::vector<U16> V_U16;
typedef V_U16::iterator IT_U16;
typedef std::vector<U32> V_U32;
typedef V_U16::const_iterator CIT_U16;
typedef std::vector<I32> V_I32;
typedef V_I32::iterator IT_I32;
typedef V_I32::const_iterator CIT_I32;
typedef std::vector<F64> V_F64;
typedef V_F64::iterator IT_F64;
typedef V_F64::const_iterator CIT_F64;
typedef pair<F64,F64> F64F64;
typedef std::vector<F64F64> V_F64F64;
typedef V_F64F64::iterator IT_F64F64;
typedef V_F64F64::const_iterator CIT_F64F64;
typedef std::vector<string> V_STR;
#endif