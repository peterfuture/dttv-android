/************************************************************************
   T h e   O p e n   W i n d o w s   P r o j e c t
 ------------------------------------------------------------------------
   Filename   : IniFile.h
   Author(s)  : Carsten Breuer
 ------------------------------------------------------------------------
 Copyright (c) 2000 by Carsten Breuer (C.Breuer@openwin.de)
 */
/************************************************************************/

#ifndef INIFILE_H
#define INIFILE_H

#include <stdbool.h>

#define _T(x) (x)
#define MAX_NET_PWD_LEN 64
#define MAX_PACKET_SIZE 260*4
#define MAX_PATH 260
#define MAX_ID_LEN 64
#define MAX_AREA_NUM 256
#define MAX_FILE_NUM_PER_AREA 512
#define CONF_MAX_PATH 512
typedef unsigned char BYTE;

typedef enum {
    NET_PWD_METHOD_DISABLE = 0,
    NET_PWD_METHOD_WO,
    NET_PWD_METHOD_WS,
    NET_PWD_METHOD_WPA,
    NET_PWD_METHOD_WPA2,

    NET_PWD_METHOD_NUM,
} Net_Pwd_Method;
#define NET_PWD_METHOD_DISABLE_STR  _T("禁用加密")
#define NET_PWD_METHOD_WO_STR       _T("WEP Open")
#define NET_PWD_METHOD_WS_STR           _T("WEP Shared Key")
#define NET_PWD_METHOD_WPA_STR          _T("WPA")
#define NET_PWD_METHOD_WPA2_STR         _T("WPA2")

typedef enum {
    NET_PWD1_TYPE_TKIP = 0,
    NET_PWD1_TYPE_AES,
    NET_PWD1_TYPE_NUM,
} Net_Pwd1_Type;
#define NET_PWD1_TYPE_TKIP_STR  _T("TKIP")
#define NET_PWD1_TYPE_AES_STR   _T("AES")

typedef enum {
    NET_PWD2_TYPE_ASCII = 0,
    NET_PWD2_TYPE_HEX,
    NET_PWD2_TYPE_NUM,
} Net_Pwd2_Type;
#define NET_PWD2_TYPE_ASCII_STR  _T("ASCII")
#define NET_PWD2_TYPE_HEX_STR   _T("HEX")

#define LINUX 1
#ifdef LINUX                    /* Remove CR, on unix systems. */
#define INI_REMOVE_CR
#define DONT_HAVE_STRUPR
#endif

#ifndef CCHR_H
#define CCHR_H
typedef const char cchr;
#endif

#ifndef __cplusplus
//typedef char bool;
#define true  1
#define TRUE  1
#define false 0
#define FALSE 0
#endif

#define tpNULL       0
#define tpSECTION    1
#define tpKEYVALUE   2
#define tpCOMMENT    3

typedef struct ENTRY {
    char Type;
    char *Text;
    struct ENTRY *pPrev;
    struct ENTRY *pNext;
} ENTRY;

typedef struct {
    struct ENTRY *pSec;
    struct ENTRY *pKey;
    char KeyText[128];
    char ValText[128];
    char Comment[255];
} EFIND;

/* Macros */
#define ArePtrValid(Sec,Key,Val) ((Sec!=NULL)&&(Key!=NULL)&&(Val!=NULL))

/* Connectors of this file (Prototypes) */

bool OpenIniFile(cchr * FileName);

bool ReadBool(cchr * Section, cchr * Key, bool Default);
int ReadInt(cchr * Section, cchr * Key, int Default);
double ReadDouble(cchr * Section, cchr * Key, double Default);
cchr *ReadString(cchr * Section, cchr * Key, cchr * Default);

void WriteBool(cchr * Section, cchr * Key, bool Value);
void WriteInt(cchr * Section, cchr * Key, int Value);
void WriteDouble(cchr * Section, cchr * Key, double Value);
void WriteString(cchr * Section, cchr * Key, cchr * Value);

bool DeleteKey(cchr * Section, cchr * Key);

void CloseIniFile();
bool WriteIniFile(cchr * FileName);
void CloseTypeFile();
//ssg add
int GetPrivateProfileString(char *appNam, char *keyNam, char *keyVal, char *fileNam);
int WritePrivateProfileString(char *appNam, char *keyNam, char *keyVal, char *filNam);
int OpenTypeFile(char *filNam);
int GetTypeKeyVal(char *appNam, char *keyNam, char *keyVal);
int SetTypeKeyVal(char *appNam, char *keyNam, char *keyVal);
void CloseWriteFile(char *filNam);

/*API ADDED BY DTSOFT*/
int GetEnv(char *appNam, char *keyNam, char *keyVal);

#endif
