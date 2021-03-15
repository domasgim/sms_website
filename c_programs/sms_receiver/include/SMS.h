//
// Created by hitmoon on 15-12-9.
//

#ifndef SMS_SMS_H
#define SMS_SMS_H

#include <string.h>
#include <stdint.h>
#include "utf.h"

#define MAX_SMS_NR 32

typedef int bool;

#define true 1
#define false 0

enum EnumDCS {
    BIT7 = 0,            // GSM 字符集
    BIT8 = 1,            // ASCII字符集
    UCS2 = 2             // Unicode 字符集
};

enum EnumUDL {
    BIT7UDL = 160,
    BIT8UDL = 140,
    UCS2UDL = 70
};

enum EnumCSMIEI {
    BIT8MIEI = 0,
    BIT16MIEI = 8
};

struct PDUUDH {
    unsigned int count;    // Information element data bytes 
    char IEI;           // Information element identification 
    char *IED;          // Information element data 
};

// User data header 
struct UDHS {
    int count;
    struct PDUUDH *UDH;
};

// User data array, used to split SMS
struct UDS {
    unsigned int total;
    char **Data;
};

// SMS after encoding 
struct PDUS {
    unsigned int count;
    char **PDU;
};

struct ByteArray {
    char *array;
    unsigned int len;
};

struct SMS_Struct {
    char *SCA;         // Service center address
    char *OA;          // Sender number
    char *SCTS;        // Timestamp
    struct UDHS *UDH;     // UDH
    char *UD;          // Message contents

    bool RP;              // Response path
    bool UDHI;            // UDHI
    bool SRI;             // Status report indication
    bool MMS;             // Send more info
    int MTI;              // Information type indication

    char PID;          // PID

    enum EnumDCS DCS;      // Data coding scheme
    bool TC;              // Text compression indication 0：uncompressed 1：compressed
    int MC;               // Message type -1: None 1: Mobile device specific type 2: SIM specific type 3: Terminal device specific type 

    uint8_t udhLen;									/* User Data Header Length */
	uint8_t udhInfoType;								/* Type of User Data Header */
	uint8_t udhInfoLen;								/* User Data Header information length */
	uint8_t concateMsgRefNo; 							/* Concatenated Message Reference Number */
	uint8_t concateTotalParts;						/* Maximum Number of concatenated messages */
	uint8_t concateCurntPart;							/* Sequence Number of concatenated messages */
	uint8_t isConcatenatedMsg;

    char *BIT7Data;
};


// SMS decoding 
struct SMS_Struct PDUDecoding(const char *data);

// SMS encoding, automatically determine the encoding scheme 
struct PDUS *PDUEncoding(char *DA, char *UDC, struct UDHS *udhs);

// 短信编码真正的工作
/// 发送方PDU格式（SMS-SUBMIT-PDU）
/// SCA（Service Center Adress）：短信中心，长度1-12
/// PDU-Type（Protocol Data Unit Type）：协议数据单元类型，长度1
/// MR（Message Reference）：消息参考值，为0～255。长度1
/// DA（Destination Adress）：接收方SME的地址，长度2-12
/// PID（Protocol Identifier）：协议标识，长度1
/// DCS（Data Coding Scheme）：编码方案，长度1
/// VP（Validity Period）：有效期，长度为1（相对）或者7（绝对或增强）
/// UDL（User Data Length）：用户数据段长度，长度1
/// UD（User Data）：用户数据，长度0-140

struct PDUS *PDUDoEncoding(char *SCA, char *DA, char *UDC, struct UDHS *udhs, enum EnumDCS DCS);


// 服务中心地址解码
char *SCADecoding(const char *data, int *EndIndex);

// 原始地址解码
char *OADecoding(const char *data, int index, int *EndIndex);

// 服务中心时间戳解码
char *SCTSDecoding(const char *data, int index);

// BCD 解码
int BCDDecoding(const char *data, int index, bool isMSB);

// 用户数据头解码
struct UDHS *UDHDecoding(const char *data, int index);

char * UserDataDecoding_BIT7(const char *data, int index, bool UDHI, enum EnumDCS dcs);

// 用户数据解码
char *UserDataDecoding(const char *data, int index, bool UDHI, enum EnumDCS dcs, char *BIT7Data);

// 7-Bit编码解压缩
char *BIT7Unpack(const char *data, int index, int Septets, int FillBits);

// 转换GSM字符编码到Unicode编码
char *BIT7Decoding(char *BIT7Data, unsigned int size);

// 7-Bit序列和Unicode编码是否相同
int isBIT7Same(u_int16_t UCS2);

// 判断是否是GSM字符串
int isGSMString(char *Data);

// User data split 
struct UDS *UDCSplit(char *UDC, struct UDHS *uhds, enum EnumDCS DCS);

// 获得用户数据头长度
int getUDHL(struct UDHS *udhs);

// 计算需要的7-Bit编码字节数
int SeptetsLength(char *source);

// 将7-Bit编码字节数换算成UCS2编码字符数
int SeptetsToChars(char *source, int index, int septets);

// 在用户数据头中增加长短信信息元素
struct UDHS *UpdateUDH(struct UDHS *udhs, int CSMMR, int total, int index);

//单条短信编码
char *SoloPDUEncoding(char *SCA, char *DA, char *UC, struct UDHS *udhs, enum EnumDCS DCS);

// SCA编码
char *SCAEncoding(char *SCA);

// PDUTYPE 编码
char *PDUTypeEncoding(bool UDH);

// MR,消息参考值
char *MREncoding();

//接收方SME地址
char *DAEncoding(char *DA);

// 协议标识
char *PIDEncoding();

// 编码方案
char *DCSEncoding(char *UD, enum EnumDCS DCS);

// 用户数据长度及内容
char *UDEncoding(char *UD, struct UDHS *udhs, enum EnumDCS DCS);

// 用户数据头编码
char *UDHEncoding(struct UDHS *udhs, int *UDHL);

// 用户数据内容编码
char *UDCEncoding(char *UDC, int *UDCL, int UDHL, enum EnumDCS DCS);

// 将UCS2编码字符串转换成7-Bit编码字节 序列
struct ByteArray *BIT7Encoding(char *UDC, int *Septets);

//  7-Bit编码压缩
char *BIT7Pack(struct ByteArray *Bit7Array, int UDHL);

void sms_init();

#endif //SMS_SMS_H
