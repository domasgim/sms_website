
//###########################################################################
// @INCLUDES
//###########################################################################
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "pdu.h"

//###########################################################################
// @DEFINES
//###########################################################################
#define ESC_CHR							0x1B
#define TIME_STAMP_LEN					 7
#define MSG_CLASS0						0x00
#define MSG_CLASS1						0x01

//###########################################################################
// @ENUMERATOR
//###########################################################################
/* Data Coding Scheme Groups */
enum
{
	GROUP1_WITH_MSG_CLASS = 0x10,
	GROUP1_WITH_NO_MSG_CLASS = 0x00,
	GROUP2_WITH_MSG_CLASS = 0xF0
};

//###########################################################################
// @GLOBAL VARIABLE
//###########################################################################

//###########################################################################
// @FUNCTIONS
//###########################################################################
static uint8_t i_Hex2Ascii(uint8_t hexNibble);
static uint8_t i_Ascii2Hex(uint8_t asciiChar);
static uint16_t i_HexBuf2AsciiBuf(uint8_t *hexBuf, uint8_t hexBufLen, char *asciiStrng);
static uint8_t i_AsciiBuf2HexBuf(char *asciiStrng, uint8_t *hexBuf);
static uint8_t i_DecSemiOctet2Ascii(char *decSemiOctetBuf, char *asciiStrng);
static uint8_t i_Ascii2DecSemiOctet(char *asciiStrng, char *decSemiOctetBuf);

static void i_Gsm7BitDfltChrToUtf8Chr(char cIn, uint8_t *utf8Char, uint8_t *utf8CharLen);
static void i_GsmExt7BitChrToUtf8Chr(char cIn, uint8_t *utf8Char, uint8_t *utf8CharLen);
static uint16_t i_GsmStrToUtf8Str(uint8_t *pStrInGsm, uint8_t strInGsmLen, uint8_t *pStrOutUtf);
static void i_Utf8StrToGsmStr(uint8_t *cIn, uint16_t cInLen, uint8_t *gsmOut, uint16_t *gsmLen);

static uint8_t i_Text2Pdu(char *pAsciiBuf, uint8_t asciiLen, uint8_t *pPduBuf);
static uint8_t i_Pdu2Text(uint8_t *pPduBuf, uint8_t pduLen, char *pAsciiBuf);

//***************************************************************************
// @NAME        : Hex2Ascii
// @PARAM       : hexNibble
// @RETURNS     : Ascii Character or FALSE
// @DESCRIPTION : This function converts hex nibble to ascii character,
//				  and if fails than return FALSE.
//***************************************************************************
static uint8_t i_Hex2Ascii(uint8_t hexNibble)
{
	if (hexNibble <= 0x09)
	{
		return (hexNibble + 0x30);
	}
	else  if ((hexNibble >= 0x0A) && (hexNibble <= 0x0F))
	{
		return (hexNibble + 0x37);
	}
	
	return (FALSE);
}

//***************************************************************************
// @NAME        : i_Ascii2Hex
// @PARAM       : uint8_t asciiChar
// @RETURNS     : hex nibble or FALSE
// @DESCRIPTION : This function converts ascii pack to hex nibble.
//***************************************************************************
static uint8_t i_Ascii2Hex(uint8_t asciiChar)
{		
	if ((asciiChar >= '0') && (asciiChar <= '9'))
	{
		return (asciiChar - 0x30);
	}
	else  if ((asciiChar >= 'A') && (asciiChar <= 'F'))
	{
		return (asciiChar - 0x37);
	}
	
	return (FALSE);
}

//***************************************************************************
// @NAME        : HexBuf2AsciiBuf
// @PARAM       : hexBuf - Pointer to buffer containg hex data
//				  hexBufLen - length of hexBuf
// @RETURNS     : Length of Ascii string, if fails in between than returns FALSE.
// @DESCRIPTION : This function converts series of hex data in to ascii string.
//***************************************************************************
static uint16_t i_HexBuf2AsciiBuf(uint8_t *pHexBuf, uint8_t hexBufLen, char *pAsciiStrng)
{
	uint8_t idx = 0;
	uint8_t hexData = 0;
	uint8_t higherNibble = 0;
	uint8_t lowerNibble = 0;
	uint8_t asciiChar = 0;
	uint16_t asciiIndex = 0;
	
	for (idx =0; idx < hexBufLen; idx++)
	{
		hexData = pHexBuf[idx];
		higherNibble = hexData >> 4;
		lowerNibble = hexData & 0x0F;
		
		asciiChar = i_Hex2Ascii(higherNibble);
		if (asciiChar != FALSE)
		{
			pAsciiStrng[asciiIndex++] = asciiChar;
		}
		else
		{
			return (FALSE);
		}
		
		asciiChar = i_Hex2Ascii(lowerNibble);
		if (asciiChar != FALSE)
		{
			pAsciiStrng[asciiIndex++] = asciiChar;
		}
		else
		{
			return (FALSE);
		}
	}
	pAsciiStrng[asciiIndex] = '\0';
	
	return (asciiIndex);
}

//***************************************************************************
// @NAME        : AsciiBuf2HexBuf
// @PARAM       : asciiStrng - Pointer to buffer string buffer.
//				  hexBuf - Pointer to hex buffer.
// @RETURNS     : Length of hex buffer, if fails in between than returns FALSE.
// @DESCRIPTION : This function converts ascii string in to siries of hex data.
//***************************************************************************
static uint8_t i_AsciiBuf2HexBuf(char *pAsciiStrng, uint8_t *pHexBuf)
{
	uint8_t idx = 0;
	uint8_t hexData = 0;
	uint8_t higherNibble = 0;
	uint8_t lowerNibble = 0;
	uint8_t asciiChar = 0;
	uint8_t hexIndex = 0;
	uint16_t asciiLen= 0;
	
	asciiLen = strlen (pAsciiStrng);
	asciiLen = asciiLen >> 1;
	
	for (idx =0; idx < asciiLen; idx++)
	{
		/* Process higher nibble */
		asciiChar = pAsciiStrng[idx*2];
		higherNibble = i_Ascii2Hex(asciiChar);
		higherNibble = higherNibble << 4;
		
		/* Process lower nibble */
		asciiChar = pAsciiStrng[(idx*2)+1];
		lowerNibble = i_Ascii2Hex(asciiChar);
		
		/* Prepare complete hex byte */
		hexData = higherNibble | lowerNibble;
		pHexBuf[hexIndex++] = hexData;
	}
	
	return (hexIndex);
}

//***************************************************************************
// @NAME        : i_Utf8StrToGsmStr
// @PARAM       : uint8_t *cIn - the pointer to buffer containing UTF8 characters.
//				  uint16_t cInLen - length of data in cIn buffer.
//				  uint8_t *gsmOut - The pointer to buffer which carries converetd
//								  string in Gsm character set.
//				  uint8_t *gsmLen - The pointer to length of converetd data with
//								  Gsm character set.
// @RETURNS     : void
// @DESCRIPTION : This function converts string in UTF8 character set to 
//				  Gsm cgaracter set.
//***************************************************************************
static void i_Utf8StrToGsmStr(uint8_t *cIn, uint16_t cInLen, uint8_t *gsmOut, uint16_t *gsmLen)
{
	uint16_t gsmIdx = 0;
	uint16_t cInidx = 0;
	uint8_t charVar = 0;
	
	
	while (cInidx < cInLen)
	{
		if (gsmIdx >= LONG_SMS_TEXT_MAX_LEN)
		{
			gsmIdx = LONG_SMS_TEXT_MAX_LEN;
			break;
		}
		
		charVar = cIn[cInidx++];
		switch(charVar)
		{
			// Characters not listed here are equal to those in the
			// UTF8 charset OR not present in it.

			case 0x40: 
				gsmOut[gsmIdx++] = 0; 
				break;
				
			case 0x24: 
				gsmOut[gsmIdx++] = 2; 
				break;			
				
			case 0x5F: 
				gsmOut[gsmIdx++] = 17; 
				break;						
				
			case 0xc2:
				switch (cIn[cInidx++])
				{
					case 0xA3:
						gsmOut[gsmIdx++] = 1; 
						break;
						
					case 0xA5:
						gsmOut[gsmIdx++] = 3; 
						break;					
						
					case 0xA4:
						gsmOut[gsmIdx++] = 36; 
						break;										
						
					case 0xA1:
						gsmOut[gsmIdx++] = 64; 
						break;															
						
					case 0xA7:
						gsmOut[gsmIdx++] = 95; 
						break;	
						
					case 0xBF:
						gsmOut[gsmIdx++] = 96; 
						break;				
						
					default:
						gsmOut[gsmIdx++] = ' '; 
						break;					
				}
				break;
				
			case 0xc3:
				switch (cIn[cInidx++])
				{
					case 0xA8:
						gsmOut[gsmIdx++] = 4; 
						break;
						
					case 0xA9:
						gsmOut[gsmIdx++] = 5; 
						break;					
						
					case 0xB9:
						gsmOut[gsmIdx++] = 6; 
						break;										
						
					case 0xAC:
						gsmOut[gsmIdx++] = 7; 
						break;															
						
					case 0xB2:
						gsmOut[gsmIdx++] = 8; 
						break;								
						
					case 0x87:
						gsmOut[gsmIdx++] = 9; 
						break;																				
						
					case 0x98:
						gsmOut[gsmIdx++] = 11; 
						break;	
						
					case 0xB8:
						gsmOut[gsmIdx++] = 12; 
						break;
						
					case 0x85:
						gsmOut[gsmIdx++] = 14; 
						break;	
						
					case 0xA5:
						gsmOut[gsmIdx++] = 15; 
						break;	
						
					case 0x86:
						gsmOut[gsmIdx++] = 28; 
						break;	
						
					case 0xA6:
						gsmOut[gsmIdx++] = 29; 
						break;					
						
					case 0x9F:
						gsmOut[gsmIdx++] = 30; 
						break;					
						
					case 0x89:
						gsmOut[gsmIdx++] = 31; 
						break;					
						
					case 0x84:
						gsmOut[gsmIdx++] = 91; 
						break;
						
					case 0x96:
						gsmOut[gsmIdx++] = 92; 
						break;										
						
					case 0x91:
						gsmOut[gsmIdx++] = 93; 
						break;										
						
					case 0x9C:
						gsmOut[gsmIdx++] = 94; 
						break;
						
					case 0xA4:
						gsmOut[gsmIdx++] = 123; 
						break;
						
					case 0xB6:
						gsmOut[gsmIdx++] = 124; 
						break;
						
					case 0xB1:
						gsmOut[gsmIdx++] = 125; 
						break;
						
					case 0xBC:
						gsmOut[gsmIdx++] = 126; 
						break;
						
					case 0xA0:
						gsmOut[gsmIdx++] = 127; 
						break;
						
					default:
						gsmOut[gsmIdx++] = ' '; 
						break;
				}
				break;	
				
				
			case 0xce:
				switch (cIn[cInidx++])
				{
					case 0x94:
						gsmOut[gsmIdx++] = 16;
						break;
						
					case 0xA6:
						gsmOut[gsmIdx++] = 18;
						break;
						
					case 0x9B:
						gsmOut[gsmIdx++] = 20;
						break;
						
					case 0xA9:
						gsmOut[gsmIdx++] = 21;
						break;
						
					case 0xA0:
						gsmOut[gsmIdx++] = 22;
						break;												
						
					case 0xA8:
						gsmOut[gsmIdx++] = 23;
						break;
						
					case 0xA3:
						gsmOut[gsmIdx++] = 24;
						break;
						
					case 0x98:
						gsmOut[gsmIdx++] = 25;
						break;
						
					case 0x9E:
						gsmOut[gsmIdx++] = 26;
						break;	
						
					default:
						gsmOut[gsmIdx++] = ' ';
						break;
				}
				break;
				
			case 0xe2:
				if ((cIn[cInidx++] == 0x82) /*&& (cIn[cInidx++] == 0x03)*/)
				{
						switch (cIn[cInidx++])
						{	
							case 0xAC:
								gsmOut[gsmIdx++] = ESC_CHR; 
								gsmOut[gsmIdx++] = 0x65; 
								break;
								
							default:
								gsmOut[gsmIdx++] = ' '; 
								break;							
						}
				}
				break;

			// extension table
			case '\f': 
				gsmOut[gsmIdx++] = ESC_CHR;
				gsmOut[gsmIdx++] = 10; 
				break; // form feed, 0x0C
				
			case '^': 
				gsmOut[gsmIdx++] = ESC_CHR;
				gsmOut[gsmIdx++] = 20; 
				break;
				
			case '{': 
				gsmOut[gsmIdx++] = ESC_CHR;		
				gsmOut[gsmIdx++] = 40;
				break;
				
			case '}': 
				gsmOut[gsmIdx++] = ESC_CHR;		
				gsmOut[gsmIdx++] = 41;
				break;
				
			case '\\':
				gsmOut[gsmIdx++] = ESC_CHR;		
				gsmOut[gsmIdx++] = 47; 
				break;
				
			case '[': 
				gsmOut[gsmIdx++] = ESC_CHR;		
				gsmOut[gsmIdx++] = 60; 
				break;
				
			case '~': 
				gsmOut[gsmIdx++] = ESC_CHR;		
				gsmOut[gsmIdx++] = 61; 
				break;
				
			case ']': 
				gsmOut[gsmIdx++] = ESC_CHR;		
				gsmOut[gsmIdx++] = 62; 
				break;
				
			case '|': 
				gsmOut[gsmIdx++] = ESC_CHR;	
				gsmOut[gsmIdx++] = 64; 
				break;

			default: 
				gsmOut[gsmIdx++] = charVar;
				break;
		}
	}
	
	*gsmLen = gsmIdx;
}

//***************************************************************************
// @NAME        : i_Gsm7BitDfltChrToUtf8Chr
// @PARAM       : char cIn - Gsm7 bit default Character.
//				  uint8_t *utf8Char - The pointer to converted UTF character.
//				  uint8_t *utf8CharLen - The pointer to length of converted UTF
//				 					   character.
// @RETURNS     : void
// @DESCRIPTION : This function converts Gsm 7-bit default character to UTF 
//				  characetr set.
//***************************************************************************
static void i_Gsm7BitDfltChrToUtf8Chr(char cIn, uint8_t *utf8Char, uint8_t *utf8CharLen)
{
	uint8_t idx = 0;
	
	switch(cIn)
	{
		// Characters not listed here are equal to those in the
		// UTF8 charset OR not present in it.

		case 0: 
			utf8Char[idx++] = 0x40; 
			break;
			
		case 1: 
			utf8Char[idx++] = 0xc2;
			utf8Char[idx++] = 0xA3;
			break;
			
		case 2:
			utf8Char[idx++] = 0x24; 
			break;
			
		case 3: 
			utf8Char[idx++] = 0xc2;	
			utf8Char[idx++] = 0xA5; 
			break;
			
		case 4: 
			utf8Char[idx++] = 0xc3;
			utf8Char[idx++] = 0xA8;
			break;
			
		case 5: 
			utf8Char[idx++] = 0xc3;
			utf8Char[idx++] = 0xA9; 
			break;
			
		case 6: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xB9; 
			break;
			
		case 7: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xAC; 
			break;
			
		case 8: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xB2; 
			break;
			
		case 9: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0x87; 
			break;
			
		case 11: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0x98; 
			break;
			
		case 12: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xB8; 
			break;
			
		case 14: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0x85; 
			break;
			
		case 15: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xA5; 
			break;		
		
		case 16: 
			utf8Char[idx++] = 0xce; 
			utf8Char[idx++] = 0x94; 
			break;	/* Added */	
		
		case 17: 
			utf8Char[idx++] = 0x5F; 
			break;
		
		case 18: 
			utf8Char[idx++] = 0xce;
			utf8Char[idx++] = 0xA6; 
			break;	/* Added */	
			
		case 19: 
			/* Pending */
			break;	/* Added */	
			
		case 20: 
			utf8Char[idx++] = 0xce; 	
			utf8Char[idx++] = 0x9B;
			break;	/* Added */	
			
		case 21: 
			utf8Char[idx++] = 0xce;
			utf8Char[idx++] = 0xA9; 
			break;	/* Added */	
			
		case 22: 
			utf8Char[idx++] = 0xce;
			utf8Char[idx++] = 0xA0; 
			break;	/* Added */	
			
		case 23: 
			utf8Char[idx++] = 0xce; 
			utf8Char[idx++] = 0xA8; 
			break;	/* Added */	
			
		case 24: 
			utf8Char[idx++] = 0xce; 
			utf8Char[idx++] = 0xA3; 
			break;	/* Added */	
			
		case 25: 
			utf8Char[idx++] = 0xce;
			utf8Char[idx++] = 0x98; 
			break;	/* Added */	
		case 26: 
			utf8Char[idx++] = 0xce; 
			utf8Char[idx++] = 0x9E; 
			break;	/* Added */	
		
		case 28: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0x86; 			
			break;
			
		case 29: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xA6; 			
			break;
			
		case 30: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0x9F; 			
			break;
			
		case 31: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0x89; 			
			break;
			
		case 36: 
			utf8Char[idx++] = 0xc2; 
			utf8Char[idx++] = 0xA4; 
			break; // 164 in UTF8
			
		case 64: 
			utf8Char[idx++] = 0xc2; 
			utf8Char[idx++] = 0xA1; 
			break;
			
		// 65-90 capital letters
		case 91: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0x84; 
			break;
			
		case 92: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0x96; 			
			break;
			
		case 93: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0x91; 						
			break;
			
		case 94: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0x9C; 						
			break;
			
		case 95: 
			utf8Char[idx++] = 0xc2; 
			utf8Char[idx++] = 0xA7; 						
			break;
			
		case 96:
			utf8Char[idx++] = 0xc2; 
			utf8Char[idx++] = 0xBF; 									
			break;
			
		// 97-122 small letters
		case 123: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xA4;
			break;
			
		case 124: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xB6;			
			break;
			
		case 125: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xB1;				
			break;
			
		case 126: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xBC;							
			break;
			
		case 127: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xA0;
			break;

		default: 
			utf8Char[idx++] = cIn; 
			break;
	}
	
	*utf8CharLen = idx;
}

//***************************************************************************
// @NAME        : i_GsmExt7BitChrToUtf8Chr
// @PARAM       : char cIn - Gsm extended 7-bit default Character.
//				  uint8_t *utf8Char - The pointer to converted UTF character.
//				  uint8_t *utf8CharLen - The pointer to length of converted UTF
//				 					   character.
// @RETURNS     : void
// @DESCRIPTION : This function converts Gsm extended 7-bit character to UTF 
//				  characetr set.
//***************************************************************************
static void i_GsmExt7BitChrToUtf8Chr(char cIn, uint8_t *utf8Char, uint8_t *utf8CharLen)
{
	uint8_t idx = 0;
	
	switch(cIn)
	{
		// Characters not listed here are equal to those in the
		// UTF8 charset OR not present in it.

		// extension table
		case 10: 
			utf8Char[idx++] = '\f'; 
			break; // form feed, 0x0C
			
		case 20: 
			utf8Char[idx++] = '^';  
			break;
			
		case 40: 
			utf8Char[idx++] = '{';  
			break;
			
		case 41: 
			utf8Char[idx++] = '}';  
			break;
			
		case 47:
			utf8Char[idx++] = '\\';  
			break;
			
		case 60: 
			utf8Char[idx++] = '[';  
			break;
			
		case 61: 
			utf8Char[idx++] = '~';  
			break;
			
		case 62: 
			utf8Char[idx++] = ']';  
			break;
			
		case 64: 
			utf8Char[idx++] = '|';  
			break;
			
		case 101: 
			utf8Char[idx++] = 0xe2;  
			utf8Char[idx++] = 0x82;  
			utf8Char[idx++] = 0xAC;  
			break; // 164 in UTF8

		default: 
			utf8Char[idx++] = cIn; 
			break;
	}
	
	*utf8CharLen = idx;
}

//***************************************************************************
// @NAME        : i_GsmStrToUtf8Str
// @PARAM       : uint8_t *pStrInGsm - The pointer to string with Gsm character set.
//				  uint8_t strInGsmLen - length of string with gsm character set..
//				  uint8_t *pStrOutUtf - The pointer to buffer containing converted
//									  UTF8 character set.
// @RETURNS     : void
// @DESCRIPTION : This function converts Gsm 7-bit characters to UTF8 characters.
//***************************************************************************
static uint16_t i_GsmStrToUtf8Str(uint8_t *pStrInGsm, uint8_t strInGsmLen, uint8_t *pStrOutUtf)
{
	uint8_t index = 0;
	uint8_t utf8CharLen = 0;
	uint16_t cnvrtdStrIndex = 0;
	
	for (index = 0; index < strInGsmLen; index++)
	{
		if (pStrInGsm[index] == ESC_CHR)
		{
			index++;
			i_GsmExt7BitChrToUtf8Chr(pStrInGsm[index], &pStrOutUtf[cnvrtdStrIndex], &utf8CharLen);
			cnvrtdStrIndex += utf8CharLen;
		}
		else
		{
			i_Gsm7BitDfltChrToUtf8Chr(pStrInGsm[index], &pStrOutUtf[cnvrtdStrIndex], &utf8CharLen);
			cnvrtdStrIndex += utf8CharLen;
		}
	}
	
	if (cnvrtdStrIndex > 550)
	{
		cnvrtdStrIndex = 550;
	}
	pStrOutUtf[cnvrtdStrIndex] = '\0';	
	
	return (cnvrtdStrIndex);
}

//***************************************************************************
// @NAME        : i_DecSemiOctet2Ascii
// @PARAM       : decSemiOctetBuf - Pointer to decimal semi octet buffer.
//				  asciiStrng - Pointer to ascii buffer.
// @RETURNS     : void.
// @DESCRIPTION : This function converts decimal semi octet in to ascii.
//***************************************************************************
static uint8_t i_DecSemiOctet2Ascii(char *pDecSemiOctetBuf, char *pAsciiStrng)
{
	uint8_t idx = 0;
	uint8_t asciiChar = 0;
	uint8_t asciiNextChar = 0;
	uint8_t decSemiOctetLen= 0;
	uint8_t loopCnt = 0;
	
	decSemiOctetLen = strlen (pDecSemiOctetBuf);
	loopCnt = decSemiOctetLen >> 1;
	
	for (idx = 0; idx < loopCnt; idx++)
	{
		asciiChar = pDecSemiOctetBuf[idx*2];
		asciiNextChar = pDecSemiOctetBuf[(idx*2) + 1];
		
		pAsciiStrng[idx*2] = asciiNextChar;
		if (idx == (loopCnt - 1))
		{
			if (asciiChar == 'F')
			{
				pAsciiStrng[(idx*2) + 1] = '\0';
				return (decSemiOctetLen - 1);
			}
			else
			{
				pAsciiStrng[(idx*2) + 1] = asciiChar;
			}
		}
		else
		{
			pAsciiStrng[(idx*2) + 1] = asciiChar;
		}
	}
	
	pAsciiStrng[idx*2] = '\0';
	
	return (decSemiOctetLen);
}


//***************************************************************************
// @NAME        : i_Ascii2DecSemiOctet
// @PARAM       : asciiStrng - Pointer to ascii buffer.
//				  decSemiOctetBuf - Pointer to decimal semi octet buffer.
// @RETURNS     : void.
// @DESCRIPTION : This function converts ascii in to decimal semi octet.
//***************************************************************************
static uint8_t i_Ascii2DecSemiOctet(char *pAsciiStrng, char *pDecSemiOctetBuf)
{
	uint8_t idx = 0;
	uint8_t asciiChar = 0;
	uint8_t asciiNextChar = 0;
	uint8_t asciiLen= 0;
	uint8_t loopCnt = 0;
	uint8_t isToAddF = 0;
	
	asciiLen = strlen (pAsciiStrng);
	if ((asciiLen % 2) != 0)
	{
		asciiLen++;
		isToAddF = TRUE;
	}
	loopCnt = asciiLen >> 1;
	
	for (idx = 0; idx < loopCnt; idx++)
	{
		asciiChar = pAsciiStrng[idx*2];
		if (idx == (loopCnt - 1))
		{
			if (isToAddF == TRUE)
			{
				asciiNextChar = 'F';
			}
			else
			{
				asciiNextChar = pAsciiStrng[(idx*2) + 1];
			}
		}
		else
		{
			asciiNextChar = pAsciiStrng[(idx*2) + 1];
		}
		
		pDecSemiOctetBuf[idx*2] = asciiNextChar;
		pDecSemiOctetBuf[(idx*2) + 1] = asciiChar;
	}
	pDecSemiOctetBuf[idx*2] = '\0';
	
	return (asciiLen);
}


//***************************************************************************
// @NAME        : i_TextToPdu
// @PARAM       : asciiBuf- Pointer to ascii buffer containing text data.
//				  pduBuf - Pointer to pdu buffer(for converted pdu data).
// @RETURNS     : uint8_t,pdu data length.
// @DESCRIPTION : This function converts Text data(7bit) to pdu data(8bit).
//***************************************************************************
static uint8_t i_Text2Pdu(char *pAsciiBuf, uint8_t asciiLen, uint8_t *pPduBuf)
{
	uint8_t idx = 0;
	uint8_t index = 0;
	uint8_t pduIndex = 0;
	uint8_t procChar = 0;
	uint8_t procNextChar = 0;
	uint8_t procVariable = 0;
	uint8_t tempVariable;
	uint8_t asciiChar = 0;
	
	for (idx = 0; idx < asciiLen; idx++)
	{
		asciiChar = pAsciiBuf[idx];
		procChar = asciiChar >> index;
		
		if (index != 7)
		{
			procNextChar = pAsciiBuf[idx + 1];
			tempVariable = procNextChar;
			procVariable = 0xFF << (index + 1);
			tempVariable = tempVariable & (~procVariable);
			tempVariable = tempVariable << (7 - index);
			pPduBuf[pduIndex++] = procChar | tempVariable;
		}

		index++;
		if (index > 7)	index = 0;
	}
	
	return (pduIndex);
}

//***************************************************************************
// @NAME        : i_PduToText
// @PARAM       : pduBuf - Pointer to pdu buffer(for converted pdu data).
//				: pduLen - length of pdu data.
//				  asciiBuf- Pointer to ascii buffer containing text data.
// @RETURNS     : uint8_t,pdu data length.
// @DESCRIPTION : This function converts pdu data(8bit) to text data(7bit).
//***************************************************************************
static uint8_t i_Pdu2Text(uint8_t *pPduBuf, uint8_t pduLen, char *pAsciiBuf)
{
	uint8_t idx = 0;
	uint8_t index = 0;
	uint8_t asciiIndex = 0;
	uint8_t procByte = 0;
	uint8_t procPrevByte = 0;
	uint8_t procVariable = 0;
	uint8_t tempVariable = 0;
	uint8_t pduByte = 0;
	
	
	for (idx = 0; idx < pduLen; idx++)
	{
		pduByte = pPduBuf[idx];
		procByte = pduByte & (0xFF >> (index + 1));
		procByte = procByte << index;
		if (idx > 0)
		{
			procPrevByte = pPduBuf[idx - 1];
			tempVariable = procPrevByte;
			procVariable = 0xFF >> index;
			tempVariable = tempVariable & (~procVariable);
			tempVariable = tempVariable >> (7 - (index - 1));
		}
		else
		{
			tempVariable = 0;
		}
		
		pAsciiBuf[asciiIndex++] = procByte | tempVariable;
		if (index == 6)
		{
			if (idx == (pduLen - 1))
			{
				if ((pduByte >> 1) != 0)
				{
					pAsciiBuf[asciiIndex++] = pduByte >> 1;
				}
			}
			else
			{
				pAsciiBuf[asciiIndex++] = pduByte >> 1;
			}
		}
		
		index++;
		if (index  > 6)		index = 0;
	}
	
	pAsciiBuf[asciiIndex] = '\0';
	
	return (asciiIndex);
}

//***************************************************************************
// @NAME        : DecodePduData
// @PARAM       : pGsmPduStr-Reference To PDU String,
//				  gsmPduStrLen-Length of PDU String,
//				  PDU_DECODE_DESC-Object Pointer
// @RETURNS     : TRUE/FALSE
// @DESCRIPTION : This function extracts Pdu String data & fills relevant parameters in Descriptor
//				  if fails then return FALSE.
//***************************************************************************
uint8_t DecodePduData(char *pGsmPduStr, PDU_DESC *pPduDecodeDesc, uint8_t *pError)
{
	 uint8_t idx = 0;
	 uint8_t length = 0;
	 uint8_t addrLen = 0;
	 uint8_t asciiLen = 0;
	 uint8_t npi = 0;
	 uint8_t udl = 0;
	 uint8_t hexBuf[SMS_PDU_MAX_LEN];
	 uint8_t oct = 0;
	 uint8_t grpId = 0;
	 
	 /** Converting whole Ascii String to Hex String */
	 i_AsciiBuf2HexBuf(pGsmPduStr, hexBuf);
	 
	 /** Service center Number Length */
	 pPduDecodeDesc->smscAddrLen = hexBuf[idx++];
	
	 /** Service Center Type of Address (Eg: 91 , 81) */
	 pPduDecodeDesc->smscTypeOfAddr = hexBuf[idx++];
	 /** Numbering Plan Identification */
	 npi = pPduDecodeDesc->smscTypeOfAddr & 0x0F; 
	 /** Type of Number */
	 pPduDecodeDesc->smscTypeOfAddr = (pPduDecodeDesc->smscTypeOfAddr & 0x70) >> 4;

	 /** Service Center Number */
	 addrLen = pPduDecodeDesc->smscAddrLen - 1; // Subtracting Type of Addr octet length
	 i_HexBuf2AsciiBuf(&hexBuf[idx], addrLen, pPduDecodeDesc->smscAddr);
	 pPduDecodeDesc->smscAddrLen = i_DecSemiOctet2Ascii(pPduDecodeDesc->smscAddr, pPduDecodeDesc->smscAddr); // Internal Swapping
	 idx = idx + addrLen;
		
	 /** First Octet of SMS_DELIVER PDU */
	 pPduDecodeDesc->firstOct = hexBuf[idx++];
	 if ((pPduDecodeDesc->firstOct & 0x40) == USER_DATA_HEADER_INDICATION)
	 {
		pPduDecodeDesc->isHeaderPrsnt = TRUE;
	 }

	 /** Message Type Indicator */
	 switch (pPduDecodeDesc->firstOct & 0x03)
	 {
		 case MSG_TYPE_SMS_DELIVER:
			pPduDecodeDesc->msgType = MSG_TYPE_SMS_DELIVER;
			break;
			
		 case MSG_TYPE_SMS_STATUS_REPORT:
			pPduDecodeDesc->msgType = MSG_TYPE_SMS_STATUS_REPORT;
			/** Message Reference Number TP-MR of SMS_STATUS_REPORT PDU */
			pPduDecodeDesc->msgRefNo = hexBuf[idx++];
			break;
		
		 default:
			*pError = ERR_MSG_TYPE;
			return (FALSE);
			break;
	 }
	 
	 /** Phone Number Length */
	 pPduDecodeDesc->phoneAddrLen = hexBuf[idx++];
		 
	 /** Phone Number Type of Address (Eg: 91 , 81) */
	 pPduDecodeDesc->phoneTypeOfAddr = hexBuf[idx++];
	 /** Numbering Plan Identification */
	 npi = pPduDecodeDesc->phoneTypeOfAddr & 0x0F;
	 /** Type of Number */
	 pPduDecodeDesc->phoneTypeOfAddr = (pPduDecodeDesc->phoneTypeOfAddr & 0x70) >> 4;
	 
	 /** Check type of number */
	 switch (pPduDecodeDesc->phoneTypeOfAddr)
	 {
		 case NUM_TYPE_UNKNOWN:
		 case NUM_TYPE_INTERNATIONAL:
		 case NUM_TYPE_NATIONAL:
				/** for Alphanumeric type of Address, Numbering plan is fix 0x00 */
				if (npi != NUM_PLAN_ISDN)
				{
					*pError = ERR_PHONE_NUM_PLAN;
					 return (FALSE);	 
				}
				/** Phone Number (Source Address) */
				if (((addrLen = pPduDecodeDesc->phoneAddrLen) % 2) != 0)
				{
					/** Eg: For "46708251358" Number Length will be 11 ("6407281553F8") */
					/** Eg: For "9898907249" Number Length will be 12 ("8989092794") */
					addrLen++;
				}
				addrLen = addrLen >> 1; // addrLen / 2
				i_HexBuf2AsciiBuf(&hexBuf[idx], addrLen, pPduDecodeDesc->phoneAddr);
				i_DecSemiOctet2Ascii(pPduDecodeDesc->phoneAddr, pPduDecodeDesc->phoneAddr); // Internal Swapping
				idx = idx + addrLen;
				break;
				
		 case NUM_TYPE_ALPHANUMERIC:
				addrLen = pPduDecodeDesc->phoneAddrLen; // length is in terms of Ascii characters
				addrLen = addrLen >> 1; // addrLen / 2
				pPduDecodeDesc->phoneAddrLen = i_Pdu2Text(&hexBuf[idx], addrLen, pPduDecodeDesc->phoneAddr);	
				break;
		
		 default:
			*pError = ERR_PHONE_TYPE_OF_ADDR;	
			return (FALSE);
			break;
	 }
	 	 
	 if (pPduDecodeDesc->msgType == MSG_TYPE_SMS_DELIVER)
	 {
		/** Protocol Identifier */
	    pPduDecodeDesc->protocolId = hexBuf[idx++];
		if (pPduDecodeDesc->protocolId != 0x00)
		{
			 *pError = ERR_PROTOCOL_ID;
			 return (FALSE);
		}
		
		/** Data Coding Scheme */
		pPduDecodeDesc->dataCodeScheme = hexBuf[idx++];
		grpId = pPduDecodeDesc->dataCodeScheme & 0xF0;
		
		switch (grpId)
		{
			case GROUP1_WITH_MSG_CLASS:
			case GROUP1_WITH_NO_MSG_CLASS:
				/** Check Character Set */
				switch ((pPduDecodeDesc->dataCodeScheme & 0x0C) >> 2)
				{
					case GSM_7BIT:
						pPduDecodeDesc->usrDataFormat = GSM_7BIT;
						break;
				
					case ANSI_8BIT:
						pPduDecodeDesc->usrDataFormat = ANSI_8BIT;
						break;
			
					case UCS2_16BIT:
						pPduDecodeDesc->usrDataFormat = UCS2_16BIT;
						break;
			
					default:
						*pError = ERR_CHAR_SET;
						return (FALSE);
						break;
				 }	
				
				 if (grpId == GROUP1_WITH_MSG_CLASS)
				 {		
					 /** Special case consideration Flash Messsage */
					 if ((pPduDecodeDesc->dataCodeScheme & 0x03) == MSG_CLASS0)
					 {
						 pPduDecodeDesc->isFlashMsg = TRUE;	
					 }
				 }
				 break;
				
			case GROUP2_WITH_MSG_CLASS:
				/** Special case consideration Flash Messsage */
				if ((pPduDecodeDesc->dataCodeScheme & 0x03) == MSG_CLASS0)
				{
					 pPduDecodeDesc->isFlashMsg = TRUE;	
				}
				switch ((pPduDecodeDesc->dataCodeScheme & 0x04) >> 2)
				{
					case GSM_7BIT:
						pPduDecodeDesc->usrDataFormat = GSM_7BIT;
						break;
				
					case ANSI_8BIT:
						pPduDecodeDesc->usrDataFormat = ANSI_8BIT;
						/** Special case consideration WAP_PUSH Messsage */
						if ((pPduDecodeDesc->dataCodeScheme & 0x03) == MSG_CLASS1)
						{
							pPduDecodeDesc->isWapPushMsg = TRUE;
						}
						break;
				 }	
				 break;
			
			default:
				*pError = ERR_DATA_CODE_SCHEME;
				return (FALSE);
				break;
		}
			
	 }
	 
	 /** Service Center Time Stamp */
	 i_HexBuf2AsciiBuf(&hexBuf[idx], TIME_STAMP_LEN, pPduDecodeDesc->timeStamp);
	 i_DecSemiOctet2Ascii(pPduDecodeDesc->timeStamp, pPduDecodeDesc->timeStamp); // Internal Swapping

	 pPduDecodeDesc->date.year = (pPduDecodeDesc->timeStamp[0] - '0') * 10 + (pPduDecodeDesc->timeStamp[1] - '0');
	 pPduDecodeDesc->date.month = (pPduDecodeDesc->timeStamp[2] - '0') * 10 + (pPduDecodeDesc->timeStamp[3] - '0');
	 pPduDecodeDesc->date.day = (pPduDecodeDesc->timeStamp[4] - '0') * 10 + (pPduDecodeDesc->timeStamp[5] - '0');

	 pPduDecodeDesc->time.hour = (pPduDecodeDesc->timeStamp[6] - '0') * 10 + (pPduDecodeDesc->timeStamp[7] - '0');
	 pPduDecodeDesc->time.minute = (pPduDecodeDesc->timeStamp[8] - '0') * 10 + (pPduDecodeDesc->timeStamp[9] - '0');
	 pPduDecodeDesc->time.second = (pPduDecodeDesc->timeStamp[10] - '0') * 10 + (pPduDecodeDesc->timeStamp[11] - '0');

	 idx = idx + TIME_STAMP_LEN;
		 
 	 if (pPduDecodeDesc->msgType == MSG_TYPE_SMS_STATUS_REPORT)
	 {
		 /** Discharge Time Stamp */
		 i_HexBuf2AsciiBuf(&hexBuf[idx], TIME_STAMP_LEN, pPduDecodeDesc->dischrgTimeStamp);
		 i_DecSemiOctet2Ascii(pPduDecodeDesc->dischrgTimeStamp, pPduDecodeDesc->dischrgTimeStamp); // Internal Swapping
		 idx = idx + TIME_STAMP_LEN;
			 
		 /** Status of SMS */
		 pPduDecodeDesc->smsSts = hexBuf[idx++];
		 if (pPduDecodeDesc->smsSts == 0x00)
	 	 {
			pPduDecodeDesc->smsSts = MSG_DELIVERY_SUCCESS;
		 }
		 else
		 {
			pPduDecodeDesc->smsSts = MSG_DELIVERY_FAIL; 
		 }
		 return (TRUE);
	 }
		 
	 /* User Data Length */
	 pPduDecodeDesc->usrDataLen = hexBuf[idx++];
	 udl = pPduDecodeDesc->usrDataLen;
	 
	 /* User Data */
	 
	 /*****************************************************************************
	 * Below section of code process user data header information
	 *****************************************************************************/
	 
	 if (pPduDecodeDesc->isHeaderPrsnt) 		// Check whether Header Present
	 {
		 pPduDecodeDesc->udhLen = hexBuf[idx++];
		 
		 for (length = idx; length < (idx + pPduDecodeDesc->udhLen); length += idx)
		 {
				pPduDecodeDesc->udhInfoType = hexBuf[idx++];
				pPduDecodeDesc->udhInfoLen = hexBuf[idx++];
				
				if (pPduDecodeDesc->udhInfoType == IE_CONCATENATED_MSG) // whether Concatenated Message
				{
					pPduDecodeDesc->isConcatenatedMsg = TRUE;
					pPduDecodeDesc->concateMsgRefNo = hexBuf[idx++];
					pPduDecodeDesc->concateTotalParts = hexBuf[idx++];	 
					pPduDecodeDesc->concateCurntPart = hexBuf[idx++];	 
				}
				else if (pPduDecodeDesc->udhInfoType == IE_PORT_ADDR_8BIT) // Port Address 8bit
				{
					pPduDecodeDesc->srcPortAddr = hexBuf[idx++];
					pPduDecodeDesc->destPortAddr = hexBuf[idx++];
				}
				else if (pPduDecodeDesc->udhInfoType == IE_PORT_ADDR_16BIT) // Port Address 16bit
				{
					pPduDecodeDesc->srcPortAddr = hexBuf[idx++];
					pPduDecodeDesc->srcPortAddr = pPduDecodeDesc->srcPortAddr << 8;
					pPduDecodeDesc->srcPortAddr |= hexBuf[idx++];
					
					pPduDecodeDesc->destPortAddr = hexBuf[idx++];
					pPduDecodeDesc->destPortAddr = pPduDecodeDesc->destPortAddr << 8;
					pPduDecodeDesc->destPortAddr |= hexBuf[idx++];
				}
				else // Ignoring other Header Information
				{
					idx = idx + pPduDecodeDesc->udhInfoLen;
				}
		 }
			
		 if (pPduDecodeDesc->usrDataFormat == GSM_7BIT)
		 {
			uint8_t fillBitsNum = 0;
			uint8_t udhSeptet = 0;
		
			/* Derive number of octet to be filled */
			fillBitsNum = ((1 + pPduDecodeDesc->udhLen) * 8) % 7;
			fillBitsNum = 7 - fillBitsNum;
		
			udhSeptet = (((1 + pPduDecodeDesc->udhLen) * 8) + fillBitsNum) / 7;
			udl -= udhSeptet;
		 }
	 }
	 
	 /* Extract user data */
	 if (pPduDecodeDesc->usrDataFormat == GSM_7BIT)
	 {
		oct = (udl*7) / 8;
		udl = (udl*7) % 8;
		if (udl != 0)
		{
			oct++;	 // Deriving no. of octes from septets
		}
		udl = oct;
		asciiLen = i_Pdu2Text(&hexBuf[idx], udl, (char *)pPduDecodeDesc->usrData);
		pPduDecodeDesc->usrDataLen = i_GsmStrToUtf8Str(pPduDecodeDesc->usrData, asciiLen, pPduDecodeDesc->usrData);
	 }
	 else 	// for 8/16bit data
	 {	
	 	 memcpy(pPduDecodeDesc->usrData, &hexBuf[idx], pPduDecodeDesc->usrDataLen);
		 pPduDecodeDesc->usrData[pPduDecodeDesc->usrDataLen] = '\0';
	 }
	 
	 return (TRUE);		 		 
}

//***********************************************************************************************
// @NAME        : EncodePduData
// @PARAM       : pGsmPduStr-Reference To PDU String
//				  gsmPduStrLen-Length of PDU String,
//				  PDU_ENCODE_DESC-Object Pointer
// @RETURNS     : TRUE/FALSE
// @DESCRIPTION : This function extracts PDU data from Descriptor & Prepares PDU String
//				  if fails then return FALSE.
//***********************************udl= udl - udhSeptet;****************************************
uint8_t EncodePduData(PDU_DESC *pPduEncodeDesc, uint8_t *pGsmPduStr, uint16_t gsmPduStrLen)
{
	 uint16_t idx = 0;
	 uint8_t tidx = 0;
	 uint8_t addrLen = 0;
	 uint8_t asciiLen = 0;
	 uint8_t fillBits = 0;
	 uint16_t gsmLen = 0;
	 uint8_t hexBuf[SMS_PDU_MAX_LEN + 1];
	
	 if (pPduEncodeDesc->smscAddrLen != 0)  // Check whether Service Centre Present
	 {
		/* Service center Number Length */
		if ((pPduEncodeDesc->smscAddrLen % 2) != 0)
		{
			pPduEncodeDesc->smscAddrLen++;
		}
		pPduEncodeDesc->smscAddrLen = 1 + (pPduEncodeDesc->smscAddrLen / 2);  // Adding length of Type of Addr
		hexBuf[idx++] = pPduEncodeDesc->smscAddrLen;
		
		/* Service Center Type of Address (Eg: 91 , 81) */
		if (pPduEncodeDesc->smscTypeOfAddr == NUM_TYPE_INTERNATIONAL)
		{
			hexBuf[idx++] = 0x91;
		}
		else if (pPduEncodeDesc->smscTypeOfAddr == NUM_TYPE_NATIONAL)
		{
			hexBuf[idx++] = 0xA1;
		}	
		else // Unknown
		{
			hexBuf[idx++] = 0x81;
		}
		
		/* Service Center Number */
		i_Ascii2DecSemiOctet(pPduEncodeDesc->smscAddr, pPduEncodeDesc->smscAddr); // Internal Swapping
		addrLen = i_AsciiBuf2HexBuf(pPduEncodeDesc->smscAddr, &hexBuf[idx]);
		idx = idx + addrLen;
	 }
	 else
	 {
		hexBuf[idx++] = 0x00; // SMSC stored on phone is used
	 }
	 
	 /* First Octet of SMS_SUBMIT PDU */
	 pPduEncodeDesc->firstOct |= MSG_TYPE_SMS_SUBMIT;
	 pPduEncodeDesc->firstOct |= pPduEncodeDesc->vldtPrdFrmt;
	 
	 if (pPduEncodeDesc->isConcatenatedMsg)
	 {
		pPduEncodeDesc->firstOct |= USER_DATA_HEADER_INDICATION;	// Indicate that UDH is present
		/* Is last part to send? */
		if (pPduEncodeDesc->concateTotalParts == pPduEncodeDesc->concateCurntPart) // Yes
		{
			/* Indicate that delivery report is require */
			if(pPduEncodeDesc->isDeliveryReq)
			{	
				pPduEncodeDesc->firstOct |= STATUS_REPORT_INDICATOR;
			}
		}
	 }
	 /* Set status report indication bit */
	 else
	 {
		 if(pPduEncodeDesc->isDeliveryReq)
		 {	
			 pPduEncodeDesc->firstOct |= STATUS_REPORT_INDICATOR;	// Indicate that delivery report is require
		 }
	 }
	
	 hexBuf[idx++] = pPduEncodeDesc->firstOct;
	
	 /* Allow Mobile to set Message Reference No. */
	 hexBuf[idx++] = MSG_REF_NO_DEFAULT; 
	 
	 /* Phone Number Length */
	 hexBuf[idx++] = pPduEncodeDesc->phoneAddrLen;
		 
	 /* Phone Number Type of Address (Eg: 91 , 81) */
	 if (pPduEncodeDesc->phoneTypeOfAddr == NUM_TYPE_INTERNATIONAL)
	 {
		hexBuf[idx++] = 0x91;
	 }
	 else if (pPduEncodeDesc->phoneTypeOfAddr == NUM_TYPE_NATIONAL)
	 {
		hexBuf[idx++] = 0xA1;
	 }	
	 else
	 {
		hexBuf[idx++] = 0x81;
	 }
	 
	 /* Phone Number (Source Address) */
	 i_Ascii2DecSemiOctet(pPduEncodeDesc->phoneAddr, pPduEncodeDesc->phoneAddr); // Internal Swapping
	 addrLen = i_AsciiBuf2HexBuf(pPduEncodeDesc->phoneAddr, &hexBuf[idx]);
	 idx = idx + addrLen;
	 
	 /* Protocol Identifier */
	 hexBuf[idx++] = 0x00;
			 
	 /* Data Coding Scheme */
	 pPduEncodeDesc->dataCodeScheme |= (pPduEncodeDesc->usrDataFormat << 2); // Character Set
	 	
	 /* Special case considerations WAP-PUSH & Flash Messsage */
	 if (pPduEncodeDesc->isFlashMsg)
	 {
		pPduEncodeDesc->dataCodeScheme |= 0x10;
	 }
	 else if (pPduEncodeDesc->isWapPushMsg)
	 {
		pPduEncodeDesc->dataCodeScheme = 0xF5;
	 }
		
	 hexBuf[idx++] = pPduEncodeDesc->dataCodeScheme;
	 
	 switch (pPduEncodeDesc->vldtPrdFrmt)
	 {
		 case VLDTY_PERIOD_DEFAULT: //Validity Period not preset
			  break;
			
		 case VLDTY_PERIOD_RELATIVE: // One octet
			  hexBuf[idx++] = pPduEncodeDesc->vldtPrd;
			  break;
			
		 default:
			  /* Reserved */
			  break;
	 }
	
	 /* User Data Length */
	 pPduEncodeDesc->usrDataLen = strlen(pPduEncodeDesc->usrData); // for 8/16bit data

	 /* User Data */
	
	 /* Check whether length is sufficient */
	 if (pPduEncodeDesc->usrDataFormat == GSM_7BIT)
	 {
			gsmLen = strlen(pPduEncodeDesc->usrData);	// for GSM_7bit data		
			tidx = idx;
			hexBuf[idx++] = gsmLen;		// TP-UDL
			
			if (gsmLen > SMS_GSM7BIT_MAX_LEN)
			{
					gsmLen = TRUNCATED_GSM_DATA_LEN;
			}
	 }
	 else // for 8bit & 16bit Data
	 {
			hexBuf[idx++] = pPduEncodeDesc->usrDataLen;		// TP-UDL
			
			if (pPduEncodeDesc->usrDataLen > SMS_PDU_USER_DATA_MAX_LEN)
			{
					pPduEncodeDesc->usrDataLen = TRUNCATED_PDU_DATA_LEN;
			}		
	 }
	
	 /*****************************************************************************
	 * Below section of code process user data header information
	 *****************************************************************************/
	 
	 if (pPduEncodeDesc->isConcatenatedMsg) // Check for Concatenated Message
	 {
		 if (pPduEncodeDesc->usrDataFormat == GSM_7BIT)
		 {	
			uint8_t fillBitsNum = 0;
			uint8_t udhSeptet = 0;
		
			/* Derive number of octet to be filled */
			fillBitsNum = ((1 + UDH_CONCATENATED_MSG_LEN) * 8) % 7;
			fillBitsNum = 7 - fillBitsNum;
			
			/* Derive User Data Length in septets */
			udhSeptet = (((1 + UDH_CONCATENATED_MSG_LEN) * 8) + fillBitsNum) / 7;
			hexBuf[tidx] = gsmLen + udhSeptet; // Updating TP-UDL
			tidx = idx; 					  
			
			hexBuf[idx++] = UDH_CONCATENATED_MSG_LEN;
			hexBuf[idx++] = IE_CONCATENATED_MSG;
			hexBuf[idx++] = IE_CONCATENATED_MSG_LEN;
			hexBuf[idx++] = pPduEncodeDesc->concateMsgRefNo;
			hexBuf[idx++] = pPduEncodeDesc->concateTotalParts; 
			hexBuf[idx++] = pPduEncodeDesc->concateCurntPart;
			
			/* Copy 7bit text to buffer */
			i_Utf8StrToGsmStr(pPduEncodeDesc->usrData, gsmLen, pPduEncodeDesc->usrData, &gsmLen);
			tidx = tidx + udhSeptet;  // for septet boundary of user data header
			i_Text2Pdu(pPduEncodeDesc->usrData, gsmLen + udhSeptet, &hexBuf[tidx]);
		}
		else
		{
			hexBuf[idx++] = UDH_CONCATENATED_MSG_LEN;
			hexBuf[idx++] = IE_CONCATENATED_MSG;
			hexBuf[idx++] = IE_CONCATENATED_MSG_LEN;
			hexBuf[idx++] = pPduEncodeDesc->concateMsgRefNo;
			hexBuf[idx++] = pPduEncodeDesc->concateTotalParts; 
			hexBuf[idx++] = pPduEncodeDesc->concateCurntPart;
			
			/* Copy 8/16bit text to buffer */
			memcpy(&hexBuf[idx], pPduEncodeDesc->usrData, pPduEncodeDesc->usrDataLen);
			hexBuf[pPduEncodeDesc->usrDataLen] = '\0';
		}
		
	 }	
	 else
	 {
		 if (pPduEncodeDesc->usrDataFormat == GSM_7BIT)
		 {		
			 /* Copy 7bit text to buffer */
			 i_Utf8StrToGsmStr(pPduEncodeDesc->usrData, gsmLen, pPduEncodeDesc->usrData, &gsmLen);
			 i_Text2Pdu(pPduEncodeDesc->usrData, gsmLen, &hexBuf[idx]);
		 }
		 else
		 {
			 /* Copy 8/16bit text to buffer */
			 memcpy(&hexBuf[idx], pPduEncodeDesc->usrData, pPduEncodeDesc->usrDataLen);
			 hexBuf[pPduEncodeDesc->usrDataLen] = '\0';
		 }
	 }
		
	/* Deriving length & String to be sent in AT command */
	gsmPduStrLen = idx;
	memcpy(pGsmPduStr, hexBuf, gsmPduStrLen);
	
	return (TRUE);
}

//***************************************************************************
// @NAME        : print_decoded_pdu
// @PARAM       : pPduDecodeDesc- Pointer to pdu desc
// @RETURNS     : void
// @DESCRIPTION : This prints pdu descriptor.
//***************************************************************************
void print_decoded_pdu(PDU_DESC *pPduDecodeDesc)
{
	printf("smscAddrLen: %d\n", pPduDecodeDesc->smscAddrLen);
	printf("smscNpi: %d\n", pPduDecodeDesc->smscNpi);
	printf("smscTypeOfAddr: %d\n", pPduDecodeDesc->smscTypeOfAddr);
	printf("smscAddr: %s\n", pPduDecodeDesc->smscAddr);
	printf("firstOct: %d\n", pPduDecodeDesc->firstOct);
	printf("isHeaderPrsnt: %d\n", pPduDecodeDesc->isHeaderPrsnt);
	printf("msgRefNo: %d\n", pPduDecodeDesc->msgRefNo);
	printf("phoneAddrLen: %d\n", pPduDecodeDesc->phoneAddrLen);
	printf("phoneAddr: %s\n", pPduDecodeDesc->phoneAddr);
	printf("protocolId: %d\n", pPduDecodeDesc->protocolId);
	printf("dataCodeScheme: %d\n", pPduDecodeDesc->dataCodeScheme);
	printf("msgType: %d\n", pPduDecodeDesc->msgType);
	printf("isWapPushMsg: %d\n", pPduDecodeDesc->isWapPushMsg);
	printf("isFlashMsg: %d\n", pPduDecodeDesc->isFlashMsg);
	printf("isStsReportReq: %d\n", pPduDecodeDesc->isStsReportReq);
	printf("isMsgWait: %d\n", pPduDecodeDesc->isMsgWait);

	printf("usrDataFormat: %d\n", pPduDecodeDesc->usrDataFormat);
	printf("timeStamp: %s\n", pPduDecodeDesc->timeStamp);
	printf("dischrgTimeStamp: %s\n", pPduDecodeDesc->dischrgTimeStamp);
	printf("vldtPrd: %d\n", pPduDecodeDesc->vldtPrd);
	printf("vldtPrdFrmt: %d\n", pPduDecodeDesc->vldtPrdFrmt);

	printf("usrDataLen: %d\n", pPduDecodeDesc->usrDataLen);
	printf("usrData: %s\n", pPduDecodeDesc->usrData);
	printf("udhLen: %d\n", pPduDecodeDesc->udhLen);
	printf("udhInfoType: %d\n", pPduDecodeDesc->udhInfoType);

	printf("udhInfoLen: %d\n", pPduDecodeDesc->udhInfoLen);
	printf("concateMsgRefNo: %d\n", pPduDecodeDesc->concateMsgRefNo);
	printf("concateTotalParts: %d\n", pPduDecodeDesc->concateTotalParts);
	printf("concateCurntPart: %d\n", pPduDecodeDesc->concateCurntPart);
	printf("isConcatenatedMsg: %d\n", pPduDecodeDesc->isConcatenatedMsg);

	printf("smsSts: %d\n", pPduDecodeDesc->smsSts);
	printf("srcPortAddr: %d\n", pPduDecodeDesc->srcPortAddr);
	printf("destPortAddr: %d\n", pPduDecodeDesc->destPortAddr);

	printf("isDeliveryReq: %d\n", pPduDecodeDesc->isDeliveryReq);

	printf("Date: %02d/%02d/%02d\n", pPduDecodeDesc->date.day, pPduDecodeDesc->date.month, pPduDecodeDesc->date.year);
	printf("Time: %02d:%02d:%02d\n", pPduDecodeDesc->time.hour, pPduDecodeDesc->time.minute, pPduDecodeDesc->time.second);
}
