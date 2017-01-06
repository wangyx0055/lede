/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    rtmp_tkip.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Paul Wu     02-25-02        Initial
*/

#include    "MtConfig.h"

// Rotation functions on 32 bit values 
#define ROL32( A, n ) \
    ( ((A) << (n)) | ( ((A)>>(32-(n))) & ( (1UL << (n)) - 1 ) ) ) 
#define ROR32( A, n ) ROL32( (A), 32-(n) ) 

UINT Tkip_Sbox_Lower[256] = 
{ 
    0xA5,0x84,0x99,0x8D,0x0D,0xBD,0xB1,0x54, 
    0x50,0x03,0xA9,0x7D,0x19,0x62,0xE6,0x9A, 
    0x45,0x9D,0x40,0x87,0x15,0xEB,0xC9,0x0B, 
    0xEC,0x67,0xFD,0xEA,0xBF,0xF7,0x96,0x5B, 
    0xC2,0x1C,0xAE,0x6A,0x5A,0x41,0x02,0x4F, 
    0x5C,0xF4,0x34,0x08,0x93,0x73,0x53,0x3F, 
    0x0C,0x52,0x65,0x5E,0x28,0xA1,0x0F,0xB5, 
    0x09,0x36,0x9B,0x3D,0x26,0x69,0xCD,0x9F, 
    0x1B,0x9E,0x74,0x2E,0x2D,0xB2,0xEE,0xFB, 
    0xF6,0x4D,0x61,0xCE,0x7B,0x3E,0x71,0x97, 
    0xF5,0x68,0x00,0x2C,0x60,0x1F,0xC8,0xED, 
    0xBE,0x46,0xD9,0x4B,0xDE,0xD4,0xE8,0x4A, 
    0x6B,0x2A,0xE5,0x16,0xC5,0xD7,0x55,0x94, 
    0xCF,0x10,0x06,0x81,0xF0,0x44,0xBA,0xE3, 
    0xF3,0xFE,0xC0,0x8A,0xAD,0xBC,0x48,0x04, 
    0xDF,0xC1,0x75,0x63,0x30,0x1A,0x0E,0x6D, 
    0x4C,0x14,0x35,0x2F,0xE1,0xA2,0xCC,0x39, 
    0x57,0xF2,0x82,0x47,0xAC,0xE7,0x2B,0x95, 
    0xA0,0x98,0xD1,0x7F,0x66,0x7E,0xAB,0x83, 
    0xCA,0x29,0xD3,0x3C,0x79,0xE2,0x1D,0x76, 
    0x3B,0x56,0x4E,0x1E,0xDB,0x0A,0x6C,0xE4, 
    0x5D,0x6E,0xEF,0xA6,0xA8,0xA4,0x37,0x8B, 
    0x32,0x43,0x59,0xB7,0x8C,0x64,0xD2,0xE0, 
    0xB4,0xFA,0x07,0x25,0xAF,0x8E,0xE9,0x18, 
    0xD5,0x88,0x6F,0x72,0x24,0xF1,0xC7,0x51, 
    0x23,0x7C,0x9C,0x21,0xDD,0xDC,0x86,0x85, 
    0x90,0x42,0xC4,0xAA,0xD8,0x05,0x01,0x12, 
    0xA3,0x5F,0xF9,0xD0,0x91,0x58,0x27,0xB9, 
    0x38,0x13,0xB3,0x33,0xBB,0x70,0x89,0xA7, 
    0xB6,0x22,0x92,0x20,0x49,0xFF,0x78,0x7A, 
    0x8F,0xF8,0x80,0x17,0xDA,0x31,0xC6,0xB8, 
    0xC3,0xB0,0x77,0x11,0xCB,0xFC,0xD6,0x3A 
};

UINT Tkip_Sbox_Upper[256] = 
{ 
    0xC6,0xF8,0xEE,0xF6,0xFF,0xD6,0xDE,0x91, 
    0x60,0x02,0xCE,0x56,0xE7,0xB5,0x4D,0xEC, 
    0x8F,0x1F,0x89,0xFA,0xEF,0xB2,0x8E,0xFB, 
    0x41,0xB3,0x5F,0x45,0x23,0x53,0xE4,0x9B, 
    0x75,0xE1,0x3D,0x4C,0x6C,0x7E,0xF5,0x83, 
    0x68,0x51,0xD1,0xF9,0xE2,0xAB,0x62,0x2A, 
    0x08,0x95,0x46,0x9D,0x30,0x37,0x0A,0x2F, 
    0x0E,0x24,0x1B,0xDF,0xCD,0x4E,0x7F,0xEA, 
    0x12,0x1D,0x58,0x34,0x36,0xDC,0xB4,0x5B, 
    0xA4,0x76,0xB7,0x7D,0x52,0xDD,0x5E,0x13, 
    0xA6,0xB9,0x00,0xC1,0x40,0xE3,0x79,0xB6, 
    0xD4,0x8D,0x67,0x72,0x94,0x98,0xB0,0x85, 
    0xBB,0xC5,0x4F,0xED,0x86,0x9A,0x66,0x11, 
    0x8A,0xE9,0x04,0xFE,0xA0,0x78,0x25,0x4B, 
    0xA2,0x5D,0x80,0x05,0x3F,0x21,0x70,0xF1, 
    0x63,0x77,0xAF,0x42,0x20,0xE5,0xFD,0xBF, 
    0x81,0x18,0x26,0xC3,0xBE,0x35,0x88,0x2E, 
    0x93,0x55,0xFC,0x7A,0xC8,0xBA,0x32,0xE6, 
    0xC0,0x19,0x9E,0xA3,0x44,0x54,0x3B,0x0B, 
    0x8C,0xC7,0x6B,0x28,0xA7,0xBC,0x16,0xAD, 
    0xDB,0x64,0x74,0x14,0x92,0x0C,0x48,0xB8, 
    0x9F,0xBD,0x43,0xC4,0x39,0x31,0xD3,0xF2, 
    0xD5,0x8B,0x6E,0xDA,0x01,0xB1,0x9C,0x49, 
    0xD8,0xAC,0xF3,0xCF,0xCA,0xF4,0x47,0x10, 
    0x6F,0xF0,0x4A,0x5C,0x38,0x57,0x73,0x97, 
    0xCB,0xA1,0xE8,0x3E,0x96,0x61,0x0D,0x0F, 
    0xE0,0x7C,0x71,0xCC,0x90,0x06,0xF7,0x1C, 
    0xC2,0x6A,0xAE,0x69,0x17,0x99,0x3A,0x27, 
    0xD9,0xEB,0x2B,0x22,0xD2,0xA9,0x07,0x33, 
    0x2D,0x3C,0x15,0xC9,0x87,0xAA,0x50,0xA5, 
    0x03,0x59,0x09,0x1A,0x65,0xD7,0x84,0xD0, 
    0x82,0x29,0x5A,0x1E,0x7B,0xA8,0x6D,0x2C 
}; 

//
// Expanded IV for TKIP function.
//

typedef struct  _IV_CONTROL_
{
    union
    {
        struct 
        {
            UCHAR       rc0;
            UCHAR       rc1;
            UCHAR       rc2;

            union
            {
                struct
                {
                    UCHAR   Rsvd:5;
                    UCHAR   ExtIV:1;
                    UCHAR   KeyID:2;
                }   field;
                UCHAR       Byte;
            }   CONTROL;
        }   field;
        
        ULONG   word;
    }   IV16;
    
    ULONG   IV32;
}   TKIP_IV, *PTKIP_IV;

/*
    ========================================================================

    Routine Description:
        Convert from UCHAR[] to ULONG in a portable way 
        
    Arguments:
        pMICKey     pointer to MIC Key
        
    Return Value:
        None

    Note:
        
    ========================================================================
*/
ULONG   RTMPTkipGetUInt32(  
    IN  PUCHAR  pMICKey)
{   
    ULONG   res = 0; 
    int     i;
    
    for (i = 0; i < 4; i++) 
    { 
        res |= (*pMICKey++) << (8 * i); 
    }

    return res; 
} 

/*
    ========================================================================

    Routine Description:
        Convert from ULONG to UCHAR[] in a portable way 
        
    Arguments:
        pDst            pointer to destination for convert ULONG to UCHAR[]
        val         the value for convert

    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/
VOID    RTMPTkipPutUInt32(
    IN OUT  PUCHAR      pDst,
    IN      ULONG       val)
{
    int i;
    
    for(i = 0; i < 4; i++) 
    { 
        *pDst++ = (UCHAR) (val & 0xff); 
        val >>= 8; 
    } 
} 

/*
    ========================================================================

    Routine Description:
        Set the MIC Key.
        
    Arguments:
        pAd     Pointer to our adapter
        pMICKey     pointer to MIC Key
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/
VOID RTMPTkipSetMICKey(  
    IN  PTKIP_KEY_INFO  pTkip,  
    IN  PUCHAR          pMICKey)
{ 
    // Set the key 
    pTkip->K0 = RTMPTkipGetUInt32(pMICKey); 
    pTkip->K1 = RTMPTkipGetUInt32(pMICKey + 4); 
    // and reset the message 
    pTkip->L = pTkip->K0;
    pTkip->R = pTkip->K1;
    pTkip->nBytesInM = 0;
    pTkip->M = 0;
} 

/*
    ========================================================================

    Routine Description:
        Calculate the MIC Value.
        
    Arguments:
        pAd     Pointer to our adapter
        uChar           Append this uChar
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/
VOID    RTMPTkipAppendByte( 
    IN  PTKIP_KEY_INFO  pTkip,  
    IN  UCHAR           uChar)
{ 
    // Append the byte to our word-sized buffer 
    pTkip->M |= (uChar << (8* pTkip->nBytesInM)); 
    pTkip->nBytesInM++; 
    // Process the word if it is full. 
    if( pTkip->nBytesInM >= 4 ) 
    { 
        pTkip->L ^= pTkip->M; 
        pTkip->R ^= ROL32( pTkip->L, 17 ); 
        pTkip->L += pTkip->R; 
        pTkip->R ^= ((pTkip->L & 0xff00ff00) >> 8) | ((pTkip->L & 0x00ff00ff) << 8); 
        pTkip->L += pTkip->R; 
        pTkip->R ^= ROL32( pTkip->L, 3 ); 
        pTkip->L += pTkip->R; 
        pTkip->R ^= ROR32( pTkip->L, 2 ); 
        pTkip->L += pTkip->R; 
        // Clear the buffer 
        pTkip->M = 0; 
        pTkip->nBytesInM = 0; 
    } 
} 

/*
    ========================================================================

    Routine Description:
        Calculate the MIC Value.
        
    Arguments:
        pAd         Pointer to our adapter
        pSrc        Pointer to source data for Calculate MIC Value
        Len         Indicate the length of the source data
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/
VOID    RTMPTkipAppend( 
    IN  PTKIP_KEY_INFO  pTkip,  
    IN  PUCHAR          pSrc,
    IN  UINT            nBytes)
{ 
    // This is simple 
    while(nBytes > 0) 
    { 
        RTMPTkipAppendByte(pTkip, *pSrc++); 
        nBytes--; 
    } 
} 

/*
    ========================================================================

    Routine Description:
        Get the MIC Value.
        
    Arguments:
        pAd     Pointer to our adapter
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
        the MIC Value is store in pAd->pNicCfg->PrivateInfo.MIC
    ========================================================================
*/
VOID    RTMPTkipGetMIC( 
    IN  PTKIP_KEY_INFO  pTkip)
{ 
    // Append the minimum padding
    RTMPTkipAppendByte(pTkip, 0x5a );   
    RTMPTkipAppendByte(pTkip, 0 ); 
    RTMPTkipAppendByte(pTkip, 0 ); 
    RTMPTkipAppendByte(pTkip, 0 ); 
    RTMPTkipAppendByte(pTkip, 0 ); 
    // and then zeroes until the length is a multiple of 4 
    while( pTkip->nBytesInM != 0 ) 
    { 
        RTMPTkipAppendByte(pTkip, 0 ); 
    } 
    // The appendByte function has already computed the result. 
    RTMPTkipPutUInt32(pTkip->MIC, pTkip->L);
    RTMPTkipPutUInt32(pTkip->MIC + 4, pTkip->R);
} 

/*
    ========================================================================

    Routine Description:
        Init Tkip function. 
        
    Arguments:
        pAd         Pointer to our adapter
        pTKey       Pointer to the Temporal Key (TK), TK shall be 128bits.
        KeyId       TK Key ID
        pTA         Pointer to transmitter address
        pMICKey     pointer to MIC Key
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
    
    ========================================================================
*/
VOID    RTMPInitTkipEngine(
    IN  PMP_ADAPTER   pAd,    
    IN  PUCHAR          pKey,
    IN  UCHAR           KeyId,
    IN  PUCHAR          pTA,
    IN  PUCHAR          pMICKey,
    IN  PUCHAR          pTSC,
    OUT PULONG          pIV16,
    OUT PULONG          pIV32)
{
    TKIP_IV tkipIv;

    // Prepare 8 bytes TKIP encapsulation for MPDU
    PlatformZeroMemory(&tkipIv, sizeof(TKIP_IV));   
    tkipIv.IV16.field.rc0 = *(pTSC + 1);    
    tkipIv.IV16.field.rc1 = (tkipIv.IV16.field.rc0 | 0x20) & 0x7f;  
    tkipIv.IV16.field.rc2 = *pTSC;  
    tkipIv.IV16.field.CONTROL.field.ExtIV = 1;  // 0: non-extended IV, 1: an extended IV
    tkipIv.IV16.field.CONTROL.field.KeyID = KeyId;
    tkipIv.IV32 = *(PULONG)(pTSC + 2);

    *pIV16 = tkipIv.IV16.word;
    *pIV32 = tkipIv.IV32;
}


/*
    ========================================================================

    Routine Description:
        Init MIC Value calculation function which include set MIC key & 
        calculate first 16 bytes (DA + SA + priority +  0)
        
    Arguments:
        pAd         Pointer to our adapter
        pTKey       Pointer to the Temporal Key (TK), TK shall be 128bits.
        pDA         Pointer to DA address
        pSA         Pointer to SA address
        UserPriority Pointer to Priority
        pMICKey     pointer to MIC Key
        
    Return Value:
        None

    Note:
    
    ========================================================================
*/
VOID    RTMPInitMICEngine(
    IN  PMP_ADAPTER   pAd,    
    IN  PUCHAR          pKey,
    IN  PUCHAR          pDA,
    IN  PUCHAR          pSA,
    IN  UCHAR           UserPriority,
    IN  PUCHAR          pMICKey)
{
    //
    // One octet priority field and 3 octet reserved (0).
    //
    ULONG Priority = UserPriority;

    // Init MIC value calculation
    RTMPTkipSetMICKey(&pAd->pNicCfg->PrivateInfo.Tx, pMICKey);
    // DA
    RTMPTkipAppend(&pAd->pNicCfg->PrivateInfo.Tx, pDA, MAC_ADDR_LEN);
    // SA
    RTMPTkipAppend(&pAd->pNicCfg->PrivateInfo.Tx, pSA, MAC_ADDR_LEN);
    // Priority + 3 bytes of 0
    RTMPTkipAppend(&pAd->pNicCfg->PrivateInfo.Tx, (PUCHAR)&Priority, 4);
}

/*
    ========================================================================

    Routine Description:
        Compare MIC value of received MSDU
        
    Arguments:
        pAd Pointer to our adapter
        pSrc        Pointer to the received Plain text data
        pDA         Pointer to DA address
        pSA         Pointer to SA address
        pMICKey     pointer to MIC Key
        Len         the length of the received plain text data exclude MIC value
        
    Return Value:
        TRUE        MIC value matched
        FALSE       MIC value mismatched
        
    IRQL = DISPATCH_LEVEL
    
    Note:
    
    ========================================================================
*/
BOOLEAN RTMPTkipCompareMICValue(
    IN  PMP_ADAPTER   pAd,    
    IN  PUCHAR          pSrc,
    IN  PUCHAR          pDA,
    IN  PUCHAR          pSA,
    IN  PUCHAR          pMICKey,
    IN  UINT            Len,
    IN  UCHAR           UP)
{
    UCHAR   OldMic[8];
    // management frame have to use 0xFF
    //ULONG Priority = UP&0x7;
    ULONG   Priority = UP;
    INT     i;

    // Init MIC value calculation
    RTMPTkipSetMICKey(&pAd->pNicCfg->PrivateInfo.Rx, pMICKey);
    // DA
    RTMPTkipAppend(&pAd->pNicCfg->PrivateInfo.Rx, pDA, MAC_ADDR_LEN);
    // SA
    RTMPTkipAppend(&pAd->pNicCfg->PrivateInfo.Rx, pSA, MAC_ADDR_LEN);
    // Priority + 3 bytes of 0
    RTMPTkipAppend(&pAd->pNicCfg->PrivateInfo.Rx, (PUCHAR)&Priority, 4);
    
    // Calculate MIC value from plain text data
    RTMPTkipAppend(&pAd->pNicCfg->PrivateInfo.Rx, pSrc, Len);

    // Get MIC valude from received frame
    PlatformMoveMemory(OldMic, pSrc + Len, 8);
    
    // Get MIC value from decrypted plain data
    RTMPTkipGetMIC(&pAd->pNicCfg->PrivateInfo.Rx);
        
    // Move MIC value from MSDU, this steps should move to data path.
    // Since the MIC value might cross MPDUs.
    if(!PlatformEqualMemory(pAd->pNicCfg->PrivateInfo.Rx.MIC, OldMic, 8))
    {
        DBGPRINT_RAW(RT_DEBUG_ERROR, ("! TKIP MIC Error !\n"));  //MIC error.
        DBGPRINT_RAW(RT_DEBUG_INFO, ("Orig MIC value ="));  //MIC error.
        for (i = 0; i < 8; i++)
        {
            DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x:", OldMic[i]));  //MIC error.
        }
        DBGPRINT_RAW(RT_DEBUG_INFO, ("\n"));  //MIC error.
        DBGPRINT_RAW(RT_DEBUG_INFO, ("Calculated MIC value ="));  //MIC error.
        for (i = 0; i < 8; i++)
        {
            DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x:", pAd->pNicCfg->PrivateInfo.Rx.MIC[i]));  //MIC error.
        }
        DBGPRINT_RAW(RT_DEBUG_INFO, ("\n"));  //MIC error.
        return (FALSE);
    }
    return (TRUE);
}


/*
    ========================================================================

    Routine Description:
        Compare MIC value of received MSDU
        
    Arguments:
        pAd Pointer to our adapter
        pLLC        LLC header
        pSrc        Pointer to the received Plain text data
        pDA         Pointer to DA address
        pSA         Pointer to SA address
        pMICKey     pointer to MIC Key
        Len         the length of the received plain text data exclude MIC value
        
    Return Value:
        TRUE        MIC value matched
        FALSE       MIC value mismatched
        
    IRQL = DISPATCH_LEVEL
    
    Note:
    
    ========================================================================
*/
BOOLEAN RTMPTkipCompareMICValueWithLLC(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pLLC,
    IN  PUCHAR          pSrc,
    IN  PUCHAR          pDA,
    IN  PUCHAR          pSA,
    IN  PUCHAR          pMICKey,
    IN  UINT            Len)
{
    UCHAR   OldMic[8];
    ULONG   Priority = 0;
    INT     i;

    // Init MIC value calculation
    RTMPTkipSetMICKey(&pAd->pNicCfg->PrivateInfo.Rx, pMICKey);
    // DA
    RTMPTkipAppend(&pAd->pNicCfg->PrivateInfo.Rx, pDA, MAC_ADDR_LEN);
    // SA
    RTMPTkipAppend(&pAd->pNicCfg->PrivateInfo.Rx, pSA, MAC_ADDR_LEN);
    // Priority + 3 bytes of 0
    RTMPTkipAppend(&pAd->pNicCfg->PrivateInfo.Rx, (PUCHAR)&Priority, 4);
    
    // Start with LLC header
    RTMPTkipAppend(&pAd->pNicCfg->PrivateInfo.Rx, pLLC, 8);

    // Calculate MIC value from plain text data
    RTMPTkipAppend(&pAd->pNicCfg->PrivateInfo.Rx, pSrc, Len);

    // Get MIC valude from received frame
    PlatformMoveMemory(OldMic, pSrc + Len, 8);
    
    // Get MIC value from decrypted plain data
    RTMPTkipGetMIC(&pAd->pNicCfg->PrivateInfo.Rx);
        
    // Move MIC value from MSDU, this steps should move to data path.
    // Since the MIC value might cross MPDUs.
    if(!PlatformEqualMemory(pAd->pNicCfg->PrivateInfo.Rx.MIC, OldMic, 8))
    {
        DBGPRINT_RAW(RT_DEBUG_ERROR, ("! TKIP MIC Error !\n"));  //MIC error.
        DBGPRINT_RAW(RT_DEBUG_INFO, ("Orig MIC value ="));  //MIC error.
        for (i = 0; i < 8; i++)
        {
            DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x:", OldMic[i]));  //MIC error.
        }
        DBGPRINT_RAW(RT_DEBUG_INFO, ("\n"));  //MIC error.
        DBGPRINT_RAW(RT_DEBUG_INFO, ("Calculated MIC value ="));  //MIC error.
        for (i = 0; i < 8; i++)
        {
            DBGPRINT_RAW(RT_DEBUG_INFO, ("%02x:", pAd->pNicCfg->PrivateInfo.Rx.MIC[i]));  //MIC error.
        }
        DBGPRINT_RAW(RT_DEBUG_INFO, ("\n"));  //MIC error.
        return (FALSE);
    }
    return (TRUE);
}
/*
    ========================================================================

    Routine Description:
        Copy frame from waiting queue into relative ring buffer and set 
    appropriate ASIC register to kick hardware transmit function
        
    Arguments:
        pAd     Pointer to our adapter
        PNDIS_PACKET    Pointer to Ndis Packet for MIC calculation
        pEncap          Pointer to LLC encap data
        LenEncap        Total encap length, might be 0 which indicates no encap
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
    
    ========================================================================
*/
VOID    
RTMPCalculateMICValue(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pDA,
    IN  PUCHAR          pSA,
    IN  PMT_XMIT_CTRL_UNIT pXcu,
    IN  PCIPHER_KEY     pKey,
    IN  UCHAR           UserPriority
    )
{
    USHORT  i=0;
    //
    // Gather information of the input Net Buffer
    //

    //make sure the first MDL size is correct
    if (pXcu->NumOfBuf>1)
    {
        // Start calculating MIC value
        RTMPInitMICEngine(
            pAd,
            pKey->Key,
            pDA,
            pSA,
            UserPriority,
            pKey->TxMic);

        for(i =1; i<pXcu->NumOfBuf; i++)
        {
            RTMPTkipAppend(&pAd->pNicCfg->PrivateInfo.Tx, pXcu->BufferList[i].VirtualAddr, pXcu->BufferList[i].Length);
        }

        // Compute the final MIC Value
        RTMPTkipGetMIC(&pAd->pNicCfg->PrivateInfo.Tx);
    }
}

/************************************************************/ 
/* tkip_sbox()                                                              */ 
/* Returns a 16 bit value from a 64K entry table. The Table */ 
/* is synthesized from two 256 entry byte wide tables.      */ 
/************************************************************/ 

UINT tkip_sbox(UINT index) 
{ 
    UINT index_low; 
    UINT index_high; 
    UINT left, right; 

    index_low = (index % 256); 
    index_high = ((index >> 8) % 256); 

    left = Tkip_Sbox_Lower[index_low] + (Tkip_Sbox_Upper[index_low] * 256); 
    right = Tkip_Sbox_Upper[index_high] + (Tkip_Sbox_Lower[index_high] * 256); 

    return (left ^ right); 
}

UINT rotr1(UINT a) 
{ 
    unsigned int b; 

    if ((a & 0x01) == 0x01) 
    { 
        b = (a >> 1) | 0x8000; 
    } 
    else 
    { 
        b = (a >> 1) & 0x7fff; 
    } 
    b = b % 65536; 
    return b; 
} 

VOID RTMPTkipMixKey(
    UCHAR   *key, 
    UCHAR   *ta, 
    ULONG   pnl, /* Least significant 16 bits of PN */
    ULONG   pnh, /* Most significant 32 bits of PN */ 
    UCHAR   *rc4key, 
    UINT    *p1k)
{

    UINT tsc0; 
    UINT tsc1;
    UINT tsc2; 

    UINT ppk0; 
    UINT ppk1; 
    UINT ppk2; 
    UINT ppk3; 
    UINT ppk4; 
    UINT ppk5; 

    INT i; 
    INT j; 

    tsc0 = (unsigned int)((pnh >> 16) % 65536); /* msb */ 
    tsc1 = (unsigned int)(pnh % 65536); 
    tsc2 = (unsigned int)(pnl % 65536); /* lsb */ 

    /* Phase 1, step 1 */ 
    p1k[0] = tsc1; 
    p1k[1] = tsc0; 
    p1k[2] = (UINT)(ta[0] + (ta[1]*256)); 
    p1k[3] = (UINT)(ta[2] + (ta[3]*256)); 
    p1k[4] = (UINT)(ta[4] + (ta[5]*256)); 

    /* Phase 1, step 2 */ 
    for (i = 0; i < 8; i++) 
    { 
        j = 2*(i & 1); 
        p1k[0] = (p1k[0] + tkip_sbox( (p1k[4] ^ ((256*key[1+j]) + key[j])) % 65536 )) % 65536; 
        p1k[1] = (p1k[1] + tkip_sbox( (p1k[0] ^ ((256*key[5+j]) + key[4+j])) % 65536 )) % 65536; 
        p1k[2] = (p1k[2] + tkip_sbox( (p1k[1] ^ ((256*key[9+j]) + key[8+j])) % 65536 )) % 65536; 
        p1k[3] = (p1k[3] + tkip_sbox( (p1k[2] ^ ((256*key[13+j]) + key[12+j])) % 65536 )) % 65536; 
        p1k[4] = (p1k[4] + tkip_sbox( (p1k[3] ^ (((256*key[1+j]) + key[j]))) % 65536 )) % 65536; 
        p1k[4] = (p1k[4] + i) % 65536; 
    } 

    /* Phase 2, Step 1 */ 
    ppk0 = p1k[0]; 
    ppk1 = p1k[1]; 
    ppk2 = p1k[2]; 
    ppk3 = p1k[3]; 
    ppk4 = p1k[4]; 
    ppk5 = (p1k[4] + tsc2) % 65536; 

    /* Phase2, Step 2 */ 
    ppk0 = ppk0 + tkip_sbox( (ppk5 ^ ((256*key[1]) + key[0])) % 65536); 
    ppk1 = ppk1 + tkip_sbox( (ppk0 ^ ((256*key[3]) + key[2])) % 65536); 
    ppk2 = ppk2 + tkip_sbox( (ppk1 ^ ((256*key[5]) + key[4])) % 65536); 
    ppk3 = ppk3 + tkip_sbox( (ppk2 ^ ((256*key[7]) + key[6])) % 65536); 
    ppk4 = ppk4 + tkip_sbox( (ppk3 ^ ((256*key[9]) + key[8])) % 65536); 
    ppk5 = ppk5 + tkip_sbox( (ppk4 ^ ((256*key[11]) + key[10])) % 65536); 

    ppk0 = ppk0 + rotr1(ppk5 ^ ((256*key[13]) + key[12])); 
    ppk1 = ppk1 + rotr1(ppk0 ^ ((256*key[15]) + key[14])); 
    ppk2 = ppk2 + rotr1(ppk1); 
    ppk3 = ppk3 + rotr1(ppk2); 
    ppk4 = ppk4 + rotr1(ppk3); 
    ppk5 = ppk5 + rotr1(ppk4); 

    /* Phase 2, Step 3 */ 
    /* Phase 2, Step 3 */

    tsc0 = (unsigned int)((pnh >> 16) % 65536); /* msb */ 
    tsc1 = (unsigned int)(pnh % 65536); 
    tsc2 = (unsigned int)(pnl % 65536); /* lsb */ 

    rc4key[0] = (tsc2 >> 8) % 256; 
    rc4key[1] = (((tsc2 >> 8) % 256) | 0x20) & 0x7f; 
    rc4key[2] = tsc2 % 256; 
    rc4key[3] = ((ppk5 ^ ((256*key[1]) + key[0])) >> 1) % 256; 

    rc4key[4] = ppk0 % 256; 
    rc4key[5] = (ppk0 >> 8) % 256; 

    rc4key[6] = ppk1 % 256; 
    rc4key[7] = (ppk1 >> 8) % 256; 

    rc4key[8] = ppk2 % 256; 
    rc4key[9] = (ppk2 >> 8) % 256; 

    rc4key[10] = ppk3 % 256; 
    rc4key[11] = (ppk3 >> 8) % 256; 

    rc4key[12] = ppk4 % 256; 
    rc4key[13] = (ppk4 >> 8) % 256; 

    rc4key[14] = ppk5 % 256; 
    rc4key[15] = (ppk5 >> 8) % 256; 
}


/************************************************/
/* construct_mic_header1()                      */
/* Builds the first MIC header block from       */
/* header fields.                               */
/************************************************/

void construct_mic_header1(
    unsigned char *mic_header1,
    int header_length,
    unsigned char *mpdu,
    int Mgmt)
{
    mic_header1[0] = (unsigned char)((header_length - 2) / 256);
    mic_header1[1] = (unsigned char)((header_length - 2) % 256);

    if (Mgmt == 0)  
    mic_header1[2] = mpdu[0] & 0xcf;    /* Mute CF poll & CF ack bits */

    mic_header1[3] = mpdu[1] & 0xc7;    /* Mute retry, more data and pwr mgt bits */
    mic_header1[4] = mpdu[4];       /* A1 */
    mic_header1[5] = mpdu[5];
    mic_header1[6] = mpdu[6];
    mic_header1[7] = mpdu[7];
    mic_header1[8] = mpdu[8];
    mic_header1[9] = mpdu[9];
    mic_header1[10] = mpdu[10];     /* A2 */
    mic_header1[11] = mpdu[11];
    mic_header1[12] = mpdu[12];
    mic_header1[13] = mpdu[13];
    mic_header1[14] = mpdu[14];
    mic_header1[15] = mpdu[15];
}

/************************************************/
/* construct_mic_header2()                      */
/* Builds the last MIC header block from        */
/* header fields.                               */
/************************************************/

void construct_mic_header2(
    unsigned char *mic_header2,
    unsigned char *mpdu,
    int a4_exists,
    int qc_exists)
{
    int i;

    for (i = 0; i<16; i++) mic_header2[i]=0x00;

    mic_header2[0] = mpdu[16];    /* A3 */
    mic_header2[1] = mpdu[17];
    mic_header2[2] = mpdu[18];
    mic_header2[3] = mpdu[19];
    mic_header2[4] = mpdu[20];
    mic_header2[5] = mpdu[21];

    mic_header2[6] = mpdu[22] & 0x0f;   /* SC */
    mic_header2[7] = 0x00; /* mpdu[23]; */

    if ((!qc_exists) && a4_exists)
    {
        for (i=0;i<6;i++) mic_header2[8+i] = mpdu[24+i];   /* A4 */
    }

    if (qc_exists && (!a4_exists))
    {
        mic_header2[8] = mpdu[24] & 0x0f; /* mute bits 15 - 4 */
        mic_header2[9] = mpdu[25] & 0x00;
    }

    if (qc_exists && a4_exists)
    {
        for (i=0;i<6;i++) mic_header2[8+i] = mpdu[24+i];   /* A4 */

        mic_header2[14] = mpdu[30] & 0x0f;  /* mute bits 15 - 4 */
        mic_header2[15] = mpdu[31] & 0x00;
    }
}


/************************************************/
/* construct_mic_iv()                           */
/* Builds the MIC IV from header fields and PN  */
/************************************************/

void construct_mic_iv(
    unsigned char   *mic_iv,
    int             qc_exists,
    int             a4_exists,
    unsigned char   *mpdu,
    unsigned int    payload_length,
    unsigned char *pn_vector,
    int Mgmt)
{
    int i;

    mic_iv[0] = 0x59;
    if (Mgmt == 1) {
        mic_iv[1] = 0xff;
    }
    else {
        if (qc_exists && a4_exists) 
            mic_iv[1] = mpdu[30] & 0x0f;    /* QoS_TC           */
            if (qc_exists && (!a4_exists)) 
            mic_iv[1] = mpdu[24] & 0x0f;   /* mute bits 7-4    */
        if (!qc_exists) 
            mic_iv[1] = 0x00;
    }   
    for (i = 2; i < 8; i++)
        mic_iv[i] = mpdu[i + 8];                    /* mic_iv[2:7] = A2[0:5] = mpdu[10:15] */
#ifdef CONSISTENT_PN_ORDER
    for (i = 8; i < 14; i++)
        mic_iv[i] = pn_vector[i - 8];           /* mic_iv[8:13] = PN[0:5] */
#else
    for (i = 8; i < 14; i++)
        mic_iv[i] = pn_vector[13 - i];          /* mic_iv[8:13] = PN[5:0] */
#endif

    mic_iv[14] = (unsigned char) (payload_length / 256);
    mic_iv[15] = (unsigned char) (payload_length % 256);
}



/************************************/
/* bitwise_xor()                    */
/* A 128 bit, bitwise exclusive or  */
/************************************/

void bitwise_xor(unsigned char *ina, unsigned char *inb, unsigned char *out)
{
    int i;
    for (i = 0; i < 16; i++)
    {
        out[i] = ina[i] ^ inb[i];
    }
}


void aes128k128d(unsigned char *key, unsigned char *data, unsigned char *ciphertext)
{
    int             round;
    int             i;
    unsigned char   intermediatea[16];
    unsigned char   intermediateb[16];
    unsigned char   round_key[16];

    for(i = 0; i<16; i++) round_key[i] = key[i];

    for (round = 0; round < 11; round++)
    {
        if (round == 0)
        {
            xor_128(round_key, data, ciphertext);
            next_key(round_key, round);         
        }
        else if (round == 10)
        {
            byte_sub(ciphertext, intermediatea);
            shift_row(intermediatea, intermediateb);
            xor_128(intermediateb, round_key, ciphertext);
        }
        else    /* 1 - 9 */
        {
            byte_sub(ciphertext, intermediatea);
            shift_row(intermediatea, intermediateb);
            mix_column(&intermediateb[0], &intermediatea[0]);
            mix_column(&intermediateb[4], &intermediatea[4]);
            mix_column(&intermediateb[8], &intermediatea[8]);
            mix_column(&intermediateb[12], &intermediatea[12]);
            xor_128(intermediatea, round_key, ciphertext);
            next_key(round_key, round);
        }
    }
}

void construct_ctr_preload(
    unsigned char   *ctr_preload,
    int             a4_exists,
    int             qc_exists,
    unsigned char   *mpdu,
    unsigned char   *pn_vector,
    int c,
    int Mgmt)
{

    int i = 0;
    for (i = 0; i < 16; i++) ctr_preload[i] = 0x00;
    i = 0;

    ctr_preload[0] = 0x01;                                  /* flag */

    if (Mgmt == 1) {
        ctr_preload[1] = 0xff;
    }   
    else {
        if (qc_exists && a4_exists) ctr_preload[1] = mpdu[30] & 0x0f;   /* QoC_Control  */
            if (qc_exists && (!a4_exists)) ctr_preload[1] = mpdu[24] & 0x0f;
    }

    for (i = 2; i < 8; i++)
        ctr_preload[i] = mpdu[i + 8];                       /* ctr_preload[2:7] = A2[0:5] = mpdu[10:15] */
    #ifdef CONSISTENT_PN_ORDER
    for (i = 8; i < 14; i++)
        ctr_preload[i] =    pn_vector[i - 8];           /* ctr_preload[8:13] = PN[0:5] */
    #else
    for (i = 8; i < 14; i++)
        ctr_preload[i] =    pn_vector[13 - i];          /* ctr_preload[8:13] = PN[5:0] */
    #endif
    ctr_preload[14] =  (unsigned char) (c / 256); // Ctr 
    ctr_preload[15] =  (unsigned char) (c % 256);

}



VOID GetMacHdrLength(
    IN  PUCHAR  pData,
    OUT UINT    *pHeaderLen,
    OUT UINT    *pQcExists,
    OUT UINT    *pAddr4Exists)
{
    UCHAR           Fc0;
    UCHAR           Fc1;
    USHORT          Fc;
    UINT            FrameType;
    UINT            FrameSubtype;
    UINT            FromDs;
    UINT            ToDs;

    *pHeaderLen = 24;

    Fc0 = *pData;
    Fc1 = *(pData + 1);

    Fc = *((PUSHORT)pData); 
    
    FrameType = ((Fc0 >> 2) & 0x03);
    FrameSubtype = ((Fc0 >> 4) & 0x0f); 

    FromDs = (Fc1 & 0x2) >> 1;
    ToDs = (Fc1 & 0x1);

    *pAddr4Exists = (FromDs & ToDs);
    *pQcExists = (FrameType == 0x02 && // add for 11w or disassoc frame will dectrypt fail
                  ((FrameSubtype == 0x08) ||    /* Assumed QoS subtypes */
                  (FrameSubtype == 0x09) ||   /* Likely to change.    */
                  (FrameSubtype == 0x0a) || 
                  (FrameSubtype == 0x0b))
                 );

    if ((*pQcExists == 1) && (*pAddr4Exists == 0))  
        *pHeaderLen += 2;
    else if ((*pQcExists == 0) && (*pAddr4Exists == 1)) 
        *pHeaderLen += 6;
    else if ((*pQcExists == 1) && (*pAddr4Exists == 1))
        *pHeaderLen += 8;
    
}

//
// TRUE: Success!
// FALSE: Encrypt Error!
//
BOOLEAN RTMPSoftEncryptTKIP(    
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN PUCHAR   pDot11Hdr,
    IN PUCHAR   pData,
    IN ULONG    DataByteCnt, 
    IN UCHAR    UserPriority,
    IN UCHAR    KeyID,
    IN PCIPHER_KEY  pWpaKey,
    IN BOOLEAN  bMgmt)
{
    PHEADER_802_11  pHeader;
    UINT            HeaderLen = 24;
    UCHAR           fc0;
    UCHAR           fc1;
    USHORT          fc;
    UINT            from_ds;
    UINT            to_ds;
    INT             a4_exists = 0;
    INT             qc_exists = 0;
    UCHAR           TA[MAC_ADDR_LEN];
    UCHAR           DA[MAC_ADDR_LEN];
    UCHAR           SA[MAC_ADDR_LEN];
    UCHAR           RC4Key[16];
    INT             payload_len;
    UINT            p1k[5]; //for mix_key;
    ULONG           pnl;/* Least significant 16 bits of PN */
    ULONG           pnh;/* Most significant 32 bits of PN */    
    ULONG           crc32 = 0;
    ULONG           LeastLen;
    PMHDRIE         MhdrIe = NULL;
    PUCHAR          pSrc;
    ULONG           Iv16;
    ULONG           Iv32;
    UCHAR           tkip_header[8] = {0};
    ULONG           i;
    
    //
    // Point to 802.11 data header.
    //
    pHeader = (PHEADER_802_11)pDot11Hdr;

    if (pWpaKey[KeyID].KeyLen == 0)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("RTMPSoftEncryptTKIP failed! (KeyID[%d] Length can not be 0) \n", KeyID));
        return FALSE;
    }


    //
    // Data must contain MAC header, 4-byte IV/KeyID, (4-byte EIV), at least 1-byte of encrypted PDU, followed by
    // 8-byte MIC and 4-byte ICV.
    //

    //
    // Check the data length. It has to be bigger than MAC header, 8 bytes IV/EIV, 8 bytes MIC and 4 bytes ICV.
    //
    GetMacHdrLength(pDot11Hdr, (UINT *)&HeaderLen,  (UINT *)&qc_exists, (UINT *)&a4_exists);
    
    LeastLen = bMgmt ? (8 + 8 + 4 + 14) : (8 + 8 + 4);
    if (DataByteCnt < LeastLen)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("RTMPSoftEncryptTKIP! (Incorrect data length=%d) \n", DataByteCnt));
        return FALSE;
    }

    
    // find DA, SA, TA
    fc0 = *pDot11Hdr;
    fc1 = *(pDot11Hdr + 1);
    fc = *((PUSHORT)pDot11Hdr); 
    from_ds = (fc1 & 0x2) >> 1;
    to_ds = (fc1 & 0x1);
    
    if (to_ds == 0 && from_ds == 1)
    {
        PlatformMoveMemory(DA, pDot11Hdr+4, MAC_ADDR_LEN);
        PlatformMoveMemory(SA, pDot11Hdr+16, MAC_ADDR_LEN);
        PlatformMoveMemory(TA, pDot11Hdr+10, MAC_ADDR_LEN);  //BSSID
    }   
    else if (to_ds == 0 && from_ds == 0 )
    {
        PlatformMoveMemory(TA, pDot11Hdr+10, MAC_ADDR_LEN);
        PlatformMoveMemory(DA, pDot11Hdr+4, MAC_ADDR_LEN);
        PlatformMoveMemory(SA, pDot11Hdr+10, MAC_ADDR_LEN);
    }
    else if (to_ds == 1 && from_ds == 0)
    {
        PlatformMoveMemory(SA, pDot11Hdr+10, MAC_ADDR_LEN);
        PlatformMoveMemory(TA, pDot11Hdr+10, MAC_ADDR_LEN);
        PlatformMoveMemory(DA, pDot11Hdr+16, MAC_ADDR_LEN);
    }
    else if (to_ds == 1 && from_ds == 1)
    {
        PlatformMoveMemory(TA, pDot11Hdr+10, MAC_ADDR_LEN);
        PlatformMoveMemory(DA, pDot11Hdr+16, MAC_ADDR_LEN);
        PlatformMoveMemory(SA, pDot11Hdr+22, MAC_ADDR_LEN);
    }

    payload_len = DataByteCnt - 8 - 8 - 4; // including MHDRIE if exists.
    
    // Prepare MHDRIE as part of payload
    if (bMgmt)
    {
        // Attach MHDRIE if MFP with TKIP.(total length=14 bytes)
        DBGPRINT(RT_DEBUG_TRACE, ("CCX-MFP: Append MHDRIE behind the frame body in TKIP\n"));

        MhdrIe  = (PMHDRIE) (pData + payload_len - 14);
        MhdrIe->Eid= 0xDD;
        MhdrIe->Length= 0x0C;
        MhdrIe->AironetOui[0]  = 0x00;
        MhdrIe->AironetOui[1]  = 0x40;
        MhdrIe->AironetOui[2]  = 0x96;
        MhdrIe->ID = 0x10;
        PlatformMoveMemory(&MhdrIe->FC, &pHeader->FC, 2);
        PlatformMoveMemory(MhdrIe->Bssid, pPort->PortCfg.Bssid, MAC_ADDR_LEN);
    }

    // Start to fill output

    pSrc = pData;

    // Calculate MIC value
    RTMPInitMICEngine(
        pAd, 
        pWpaKey[KeyID].Key, 
        DA, 
        SA, 
        (bMgmt ? 0xff : UserPriority),
        pWpaKey[KeyID].TxMic);

    RTMPTkipAppend(&pAd->pNicCfg->PrivateInfo.Tx, pSrc, payload_len);
    RTMPTkipGetMIC(&pAd->pNicCfg->PrivateInfo.Tx);

    RTMPInitTkipEngine(
            pAd,
            pWpaKey[KeyID].Key,
            KeyID,
            pAd->HwCfg.CurrentAddress,
            pWpaKey[KeyID].TxMic,
            pWpaKey[KeyID].TxTsc,
            &Iv16,
            &Iv32);

    pnl = (ULONG) *(pWpaKey[KeyID].TxTsc) + (((ULONG) *(pWpaKey[KeyID].TxTsc + 1)) << 8);   //TSC[0]:TSC[1]
    pnh = *(PULONG)(pWpaKey[KeyID].TxTsc + 2);

    pAd->pNicCfg->PrivateInfo.FCSCRC32 = PPPINITFCS32;   //Init crc32.
    RTMPTkipMixKey(pWpaKey[KeyID].Key, pAd->HwCfg.CurrentAddress, pnl, pnh, RC4Key, p1k);

    // Copy 8 bytes encapsulation into Tx ring
    PlatformMoveMemory(&tkip_header[0], &Iv16, 4);
    PlatformMoveMemory(&tkip_header[4], &Iv32, 4);

    // Init RC4 encyption engine
    ARCFOUR_INIT(&pAd->pNicCfg->PrivateInfo.WEPCONTEXT, RC4Key, 16);

    // Increase TxTsc value for next transmission
    INC_TX_TSC(pWpaKey[KeyID].TxTsc);
            
    RTMPEncryptData(pAd, pSrc, pSrc, payload_len);

    RTMPEncryptData(pAd, pAd->pNicCfg->PrivateInfo.Tx.MIC, pSrc + payload_len, 8);   // Generate MIC
    RTMPSetICV(pAd, pSrc + payload_len + 8);    // Generate ICV

    // Go-back-shift Source Data with 8 bytes for TKIP header
    for (i = DataByteCnt - 1; i >= 8; i--)
        (UCHAR)(*(pSrc + i)) = (UCHAR)(*(pSrc + i - 8));
    // Append TKIP header
    PlatformMoveMemory(pSrc, tkip_header, 8);

    DBGPRINT(RT_DEBUG_INFO, ("RTMPSoftEncryptTKIP Encrypt done !!\n"));
    return TRUE;
    
}


//
// TRUE: Success!
// FALSE: Decrypt Error!
//
BOOLEAN RTMPSoftDecryptTKIP(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort,
    IN PUCHAR   pDot11Hdr,
    IN PUCHAR           pData,
    IN ULONG            DataByteCnt, 
    IN UCHAR            UserPriority,
    IN PCIPHER_KEY  pWpaKey,
    IN BOOLEAN  bMgmt)
{
    PHEADER_802_11  pHeader;
    UCHAR           KeyID;
    UINT            HeaderLen;
    UCHAR           fc0;
    UCHAR           fc1;
    USHORT          fc;
    UINT            from_ds;
    UINT            to_ds;
    INT             a4_exists;
    INT             qc_exists;
    UCHAR           TA[MAC_ADDR_LEN];
    UCHAR           DA[MAC_ADDR_LEN];
    UCHAR           SA[MAC_ADDR_LEN];
    UCHAR           RC4Key[16];
    UINT            p1k[5]; //for mix_key;
    ULONG           pnl;/* Least significant 16 bits of PN */
    ULONG           pnh;/* Most significant 32 bits of PN */ 
    UINT            payload_len;
    ARCFOURCONTEXT  ArcFourContext;
    ULONG           crc32 = 0;
    ULONG           trailfcs = 0;
    ULONG           LeastLen;

    //
    // Point to 802.11 data header.
    //
    pHeader = (PHEADER_802_11)pDot11Hdr;

    //
    // Data must contain MAC header, 4-byte IV/KeyID, (4-byte EIV), at least 1-byte of encrypted PDU, followed by
    // 8-byte MIC and 4-byte ICV.
    //
    
    //
    // Check the data length. It has to be bigger than MAC header, 8 bytes IV/EIV, 8 bytes MIC and 4 bytes ICV.
    //
    GetMacHdrLength(pDot11Hdr,  (UINT *)&HeaderLen,  (UINT *)&qc_exists, (UINT *)&a4_exists);   

    LeastLen = bMgmt ? (8 + 8 + 4 + 14) : ( 8 + 8 + 4);
    if (DataByteCnt < LeastLen)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("RTMPSoftDecryptTKIP failed! (Incorrect data length=%d) \n", DataByteCnt));
        return FALSE;
    }


    KeyID = *((PUCHAR)(pData + 3)); 
    KeyID = KeyID >> 6;
    if (pWpaKey[KeyID].KeyLen == 0)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("RTMPSoftDecryptTKIP failed! (KeyID[%d] Length can not be 0) \n", KeyID));
        return FALSE;
    }

    // find DA, SA, TA
    fc0 = *pDot11Hdr;
    fc1 = *(pDot11Hdr + 1);
    fc = *((PUSHORT)pDot11Hdr); 
    from_ds = (fc1 & 0x2) >> 1;
    to_ds = (fc1 & 0x1);
    
    if (to_ds == 0 && from_ds == 1)
    {
        PlatformMoveMemory(DA, pDot11Hdr+4, MAC_ADDR_LEN);
        PlatformMoveMemory(SA, pDot11Hdr+16, MAC_ADDR_LEN);
        PlatformMoveMemory(TA, pDot11Hdr+10, MAC_ADDR_LEN);  //BSSID
    }   
    else if (to_ds == 0 && from_ds == 0 )
    {
        PlatformMoveMemory(TA, pDot11Hdr+10, MAC_ADDR_LEN);
        PlatformMoveMemory(DA, pDot11Hdr+4, MAC_ADDR_LEN);
        PlatformMoveMemory(SA, pDot11Hdr+10, MAC_ADDR_LEN);
    }
    else if (to_ds == 1 && from_ds == 0)
    {
        PlatformMoveMemory(SA, pDot11Hdr+10, MAC_ADDR_LEN);
        PlatformMoveMemory(TA, pDot11Hdr+10, MAC_ADDR_LEN);
        PlatformMoveMemory(DA, pDot11Hdr+16, MAC_ADDR_LEN);
    }
    else if (to_ds == 1 && from_ds == 1)
    {
        PlatformMoveMemory(TA, pDot11Hdr+10, MAC_ADDR_LEN);
        PlatformMoveMemory(DA, pDot11Hdr+16, MAC_ADDR_LEN);
        PlatformMoveMemory(SA, pDot11Hdr+22, MAC_ADDR_LEN);
    }


    payload_len = (DataByteCnt - 8 - 8 - 4);

    pnl = (*pData) * 256 + *(pData + 2);    
    pnh = *((PULONG)(pData + 4));
    RTMPTkipMixKey(pWpaKey[KeyID].Key, TA, pnl, pnh, RC4Key, p1k);

    ARCFOUR_INIT(&ArcFourContext, RC4Key, 16); 

    ARCFOUR_DECRYPT(
        &ArcFourContext, 
        pData,      /* (plaintext || ICV )*/
        pData + 8,  /* ciphertext */ 
        DataByteCnt - 8);
    
    PlatformMoveMemory(&trailfcs, pData + DataByteCnt - 8 - 4, 4);
    crc32 = RTMP_CALC_FCS32(PPPINITFCS32, pData, DataByteCnt - 8 - 4);  //Skip MIC and ICV
    crc32 ^= 0xffffffff;             /* complement */

    if(crc32 != trailfcs)
    {
        DBGPRINT_RAW(RT_DEBUG_ERROR, ("RTMPSoftDecryptTKIP failed! (WEP Data ICV Error) \n"));   //ICV error.
        return (FALSE);
    }   

    if (RTMPTkipCompareMICValue(
                                pAd, 
                                pData, 
                                DA, 
                                SA, 
                                pWpaKey[KeyID].RxMic, 
                                payload_len,
                                (bMgmt ? 0xff : UserPriority)) == FALSE)
    {
        //
        // Report MicError only on port secured.
        //
        if (pAd->StaCfg.PortSecured == WPA_802_1X_PORT_SECURED)
        {
            N6PlatformIndicateMICFailure(pAd, pHeader->Addr2, KeyID, (KeyID == 0) ? FALSE : TRUE);
            return (FALSE);     
        }
    }

    // 
    // Validate MHDRIE
    // 
    if (bMgmt)
    {
        MHDRIE  *MhdrIe = (MHDRIE *)(pData + DataByteCnt - 20 - 14);
        FRAME_CONTROL   FC;

        // Retry Bit might be changed by hardware, so ignore it.
        PlatformMoveMemory(&FC, &pHeader->FC, 2);
        FC.Retry = 0;

        if ((MhdrIe->Eid != 0xDD)   ||
            (MhdrIe->Length != 0x0C) ||
            (MhdrIe->AironetOui[0] != 0x00) ||
            (MhdrIe->AironetOui[1] != 0x40) ||
            (MhdrIe->AironetOui[2] != 0x96) ||
            (MhdrIe->ID != 0x10) ||
            (!PlatformEqualMemory(&MhdrIe->FC, &FC, 2)) ||
            (!PlatformEqualMemory(MhdrIe->Bssid, pPort->PortCfg.Bssid, MAC_ADDR_LEN)))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("RTMPSoftDecryptTKIP, invalid MHDRIE !\n"));
            return (FALSE);
        }
    }   

    DBGPRINT(RT_DEBUG_INFO, ("RTMPSoftDecryptTKIP Decrypt done!!\n"));
    return TRUE;
}


//
// TRUE: Success!
// FALSE: Encrypt Error!
//
BOOLEAN RTMPSoftEncryptAES(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN PUCHAR   pDot11Hdr,
    IN PUCHAR   pData,
    IN ULONG    DataByteCnt,
    IN UCHAR    KeyID,
    IN PCIPHER_KEY  pWpaKey,
    IN BOOLEAN  bMgmt)
{
    PHEADER_802_11          pHeader;
    ULONG                   HeaderLen = 24;
    ULONG                   payload_len;
    INT                     a4_exists = 0;
    INT                     qc_exists = 0;
    PUCHAR                  pSrc;
    ULONG                   Iv16;
    ULONG                   Iv32;
    PUCHAR                  pTmp;
    /* PN */
    UCHAR                   pn_vector[6];
    /* MIC working variables */
    INT                     i;
    INT                     num_blocks;
    INT                     payload_remainder;
    INT                     payload_index;
    /* Initialization Blocks */
    UCHAR                   mic_iv[16];
    UCHAR                   mic_header1[16];
    UCHAR                   mic_header2[16];
    UCHAR                   ctr_preload[16];
    /* Intermediate Buffers */
    UCHAR                   chain_buffer[16];
    UCHAR                   aes_out[16];
    UCHAR                   padded_buffer[16];
    /* MIC */
    UCHAR                   mic[8];
    /* CCMP header*/
    UCHAR                   ccmp_header[8] = {0};
    
    //
    // Point to 802.11 data header.
    //
    pHeader = (PHEADER_802_11)pDot11Hdr;


    if (pWpaKey[KeyID].KeyLen == 0)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("RTMPSoftEncryptAES failed! (KeyID[%d] Length can not be 0) \n", KeyID));
        return FALSE;
    }

    //
    // Data must contain 8-byte CCMP header, at least 1-byte of encrypted PDU, followed by
    // 8-byte MIC.
    //

    //
    // Check the data length. It has to be bigger than MAC header, 8 bytes CCMP header and 8 bytes MIC.
    //
    GetMacHdrLength(pDot11Hdr,  (UINT *)&HeaderLen,  (UINT *)&qc_exists, (UINT *)&a4_exists);

    if (DataByteCnt < ( 8 + 8))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("RTMPSoftEncryptAES! (Incorrect data length=%d) \n", DataByteCnt));
        return FALSE;
    }


    //
    // Start to fill output
    //
    
    pSrc = pData;

    //
    // Construct CCMP header
    //
    pTmp        = (PUCHAR) &Iv16;
    *pTmp       = pWpaKey[KeyID].TxTsc[0];
    *(pTmp + 1) = pWpaKey[KeyID].TxTsc[1];
    *(pTmp + 2) = 0;
    *(pTmp + 3) = (pPort->PortCfg.DefaultKeyId << 6) | 0x20;
    Iv32 = *(PULONG)(&pWpaKey[KeyID].TxTsc[2]);
                    
    // Increase TxTsc for next transmission
    INC_TX_TSC(pWpaKey[KeyID].TxTsc);

    PlatformMoveMemory(&ccmp_header[0], &Iv16, 4);  // Copy IV
    PlatformMoveMemory(&ccmp_header[4], &Iv32, 4);  // Copy EIV


    payload_len = (DataByteCnt - 8 - 8);

    // PN vector
    pn_vector[0] = ccmp_header[0];  /* point to the first byte of IV */
    pn_vector[1] = ccmp_header[1];
    pn_vector[2] = ccmp_header[4];
    pn_vector[3] = ccmp_header[5];
    pn_vector[4] = ccmp_header[6];
    pn_vector[5] = ccmp_header[7];


    // Calculate MIC
        //
    construct_mic_iv(
                        mic_iv,
                        qc_exists,
                        a4_exists,
                        pDot11Hdr,  /* point to the 802.11 header */
                        payload_len,
                        pn_vector,
                        bMgmt);
    construct_mic_header1(
                        mic_header1,
                        HeaderLen,
                        pDot11Hdr,  /* point to the 802.11 header */
                        bMgmt);
    construct_mic_header2(
                        mic_header2,
                        pDot11Hdr,  /* point to the 802.11 header */
                        a4_exists,
                        qc_exists);


    // Calculate number of 16 byte blocks in MPDU
    payload_remainder = (payload_len) % 16;
    num_blocks = (payload_len) / 16;

    // Find start of payload
    payload_index = 0;

    aes128k128d(pWpaKey[KeyID].Key, mic_iv, aes_out);
    bitwise_xor(aes_out, mic_header1, chain_buffer);
    aes128k128d(pWpaKey[KeyID].Key, chain_buffer, aes_out);
    bitwise_xor(aes_out, mic_header2, chain_buffer);
    aes128k128d(pWpaKey[KeyID].Key, chain_buffer, aes_out);

    // iterate through each 16 byte payload block
    for (i = 0; i < num_blocks; i++)     
    {
        bitwise_xor(aes_out, pSrc + payload_index, chain_buffer);
        aes128k128d(pWpaKey[KeyID].Key, chain_buffer, aes_out);
        payload_index += 16;
    }

    // Add on the final payload block if it needs padding
    if (payload_remainder > 0)
    {
        PlatformZeroMemory(padded_buffer, 16);
        PlatformMoveMemory(padded_buffer, pSrc + payload_index, payload_remainder);
        payload_index += payload_remainder;
        bitwise_xor(aes_out, padded_buffer, chain_buffer);
        aes128k128d(pWpaKey[KeyID].Key, chain_buffer, aes_out);
    }

    /* aes_out contains padded mic, discard most significant    */
    /* 8 bytes to generate 64 bit MIC                           */
    PlatformMoveMemory(mic, aes_out, 8);


    // Insert MIC into payload
    PlatformMoveMemory(pSrc + payload_len, mic, 8);


    /**** Encrypt the payload and MIC ****/
    payload_index = 0;  
    for (i=0; i< num_blocks; i++)
    {
        construct_ctr_preload(
                            ctr_preload,
                            a4_exists,
                            qc_exists,
                            pDot11Hdr,  /* point to the 802.11 header */
                            pn_vector,
                            i + 1,
                            bMgmt);

        aes128k128d(pWpaKey[KeyID].Key, ctr_preload, aes_out);
        bitwise_xor(aes_out, pSrc + payload_index, chain_buffer);
        PlatformMoveMemory(pSrc + payload_index, chain_buffer, 16);
        payload_index += 16;
    }

    if (payload_remainder > 0)          /* If there is a short final block, then pad it,*/
    {                                   /* encrypt it and copy the unpadded part back   */
        construct_ctr_preload(
                            ctr_preload,
                            a4_exists,
                            qc_exists,
                            pDot11Hdr,  /* point to the 802.11 header */
                            pn_vector,
                            num_blocks + 1,
                            bMgmt);

        PlatformZeroMemory(padded_buffer, 16);
        PlatformMoveMemory(padded_buffer, pSrc + payload_index, payload_remainder);
        aes128k128d(pWpaKey[KeyID].Key, ctr_preload, aes_out);
        bitwise_xor(aes_out, padded_buffer, chain_buffer);
        PlatformMoveMemory(pSrc + payload_index, chain_buffer, payload_remainder);
        payload_index += payload_remainder;
    }

    // Encrypt the MIC
    construct_ctr_preload(
                        ctr_preload,
                        a4_exists,
                        qc_exists,
                        pDot11Hdr,  /* point to the 802.11 header */
                        pn_vector,
                        0,
                        bMgmt);

    PlatformZeroMemory(padded_buffer, 16);
    PlatformMoveMemory(padded_buffer, mic, 8);
    aes128k128d(pWpaKey[KeyID].Key, ctr_preload, aes_out);
    bitwise_xor(aes_out, padded_buffer, chain_buffer);
    PlatformMoveMemory(pSrc + payload_index, chain_buffer, 8);

    
    // Go-back-shift Source Data with 8 bytes for CCMP header
    for (i = DataByteCnt - 1; i >= 8; i--)
        (UCHAR)(*(pSrc + i)) = (UCHAR)(*(pSrc + i - 8));
    // Append CCMP header
    PlatformMoveMemory(pSrc, ccmp_header, 8);
    
    DBGPRINT(RT_DEBUG_INFO, ("RTMPSoftEncryptAES Encrypt done!!\n"));
    return TRUE;

}


//
// TRUE: Success!
// FALSE: Decrypt Error!
//
BOOLEAN RTMPSoftDecryptAES(
    IN PMP_ADAPTER    pAd,
    IN PUCHAR   pDot11Hdr,
    IN PUCHAR           pData,
    IN ULONG            DataByteCnt, 
    IN PCIPHER_KEY      pWpaKey,
    IN BOOLEAN  bMgmt)
{
    PHEADER_802_11          pHeader;
    ULONG                   HeaderLen = 24;
    UCHAR           KeyID;
    INT                     a4_exists = 0;
    INT                     qc_exists = 0;
    UINT            payload_len;    
    PUCHAR                  pSrc;
    /* PN */
    UCHAR                   pn_vector[6];
    /* MIC working variables */
    INT                     i;
    INT                     num_blocks;
    INT                     payload_remainder;
    INT                     payload_index;
    /* Initialization Blocks */
    UCHAR                   mic_iv[16];
    UCHAR                   mic_header1[16];
    UCHAR                   mic_header2[16];
    UCHAR           ctr_preload[16];
    /* Intermediate Buffers */
    UCHAR           chain_buffer[16];
    UCHAR                   aes_out[16];
    UCHAR           padded_buffer[16];
    /* MIC */
    UCHAR                   mic[8], TrailMIC[8];
    /* CCMP header*/
    UCHAR                   ccmp_header[8] = {0};

    //
    // Point to 802.11 data header.
    //
    pHeader = (PHEADER_802_11)pDot11Hdr;    

    //
    // Data must contain 8-byte CCMP header, at least 1-byte of encrypted PDU, followed by
    // 8-byte MIC.
    //

    //
    // Check the data length. It has to be bigger than MAC header, 8 bytes CCMP header and 8 bytes MIC.
    //
    GetMacHdrLength(pDot11Hdr,  (UINT *)&HeaderLen,  (UINT *)&qc_exists, (UINT *)&a4_exists);

    if (DataByteCnt < (8 + 8))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("RTMPSoftDecryptAES failed! (Incorrect data length=%d) \n", DataByteCnt));
        return FALSE;
    }
    

    KeyID = *((PUCHAR)(pData + 3)); 
    KeyID = KeyID >> 6;
    if (pWpaKey[KeyID].KeyLen == 0)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("RTMPSoftDecryptAES failed!(KeyID[%d] Length can not be 0)\n", KeyID));
        return FALSE;
    }
    

    // Validate CCMP header. 
    // 1. Reserved field (3rd byte) must be 0.
    // 2. In 4th byte of CCMP header, ExtIV must be 1 and rsvd must be 0
    if ((*((PUCHAR)(pData + 2))) != 0)
        return FALSE;

    if ((( *((PUCHAR)(pData + 3))) & 0x3f) != 0x20)
        return FALSE;


    //
    // Start to decrypt output
    //

    PlatformMoveMemory(ccmp_header, pData, 8);  
    pn_vector[0] = ccmp_header[0];  
    pn_vector[1] = ccmp_header[1];
    pn_vector[2] = ccmp_header[4];
    pn_vector[3] = ccmp_header[5];
    pn_vector[4] = ccmp_header[6];
    pn_vector[5] = ccmp_header[7];

    pSrc = pData + 8;   /* pSrc point to the first byte of ciphertext */

    payload_len = DataByteCnt - 8 - 8;
    payload_remainder = (payload_len) % 16;
    num_blocks = (payload_len) / 16; 

    // Find start of payload
    payload_index = 0;
    for (i = 0; i < num_blocks; i++)    
    {
        construct_ctr_preload(ctr_preload,
                                a4_exists,
                                qc_exists,
                                pDot11Hdr,  /* point to the 802.11 header */
                                pn_vector,
                                i+1,
                                bMgmt);

        aes128k128d(pWpaKey[KeyID].Key, ctr_preload, aes_out);
        bitwise_xor(aes_out, pSrc + payload_index, chain_buffer);
        PlatformMoveMemory(pSrc + payload_index, chain_buffer, 16);
        payload_index += 16;
    }

    //
    // If there is a short final block, then pad it
    // encrypt it and copy the unpadded part back 
    //
    if (payload_remainder > 0)
    {
        construct_ctr_preload(ctr_preload,
                                a4_exists,
                                qc_exists,
                                pDot11Hdr,  /* point to the 802.11 header */
                                pn_vector,
                                num_blocks + 1,
                                bMgmt);

        PlatformZeroMemory(padded_buffer, 16);
        PlatformMoveMemory(padded_buffer, pSrc + payload_index, payload_remainder);

        aes128k128d(pWpaKey[KeyID].Key, ctr_preload, aes_out);
        bitwise_xor(aes_out, padded_buffer, chain_buffer);
        PlatformMoveMemory(pSrc + payload_index, chain_buffer, payload_remainder);
        payload_index += payload_remainder;
    }

    //
    // Deccrypt the MIC
    // 
    construct_ctr_preload(ctr_preload,
                            a4_exists,
                            qc_exists,
                            pDot11Hdr,  /* point to the 802.11 header */
                            pn_vector,
                            0,
                            bMgmt);
    PlatformZeroMemory(padded_buffer, 16);
    PlatformMoveMemory(padded_buffer, pSrc + payload_len, 8); 

    aes128k128d(pWpaKey[KeyID].Key, ctr_preload, aes_out);
    bitwise_xor(aes_out, padded_buffer, chain_buffer);  

    PlatformMoveMemory(TrailMIC, chain_buffer, 8);


    //
    // Calculate MIC
    //

    payload_index = 0;  
    construct_mic_iv(
                    mic_iv,
                    qc_exists,
                    a4_exists,
                    pDot11Hdr,  /* point to the 802.11 header */
                    payload_len,
                    pn_vector,
                    bMgmt);

    construct_mic_header1(
                        mic_header1,
                        HeaderLen,
                        pDot11Hdr,  /* point to the 802.11 header */
                        bMgmt);

    construct_mic_header2(
                        mic_header2,
                        pDot11Hdr,  /* point to the 802.11 header */
                        a4_exists,
                        qc_exists);

    aes128k128d(pWpaKey[KeyID].Key, mic_iv, aes_out);
    bitwise_xor(aes_out, mic_header1, chain_buffer);
    aes128k128d(pWpaKey[KeyID].Key, chain_buffer, aes_out);
    bitwise_xor(aes_out, mic_header2, chain_buffer);
    aes128k128d(pWpaKey[KeyID].Key, chain_buffer, aes_out);

    // iterate through each 16 byte payload block
    for (i = 0; i < num_blocks; i++)     
    {
        bitwise_xor(aes_out, pSrc + payload_index, chain_buffer);
        aes128k128d(pWpaKey[KeyID].Key, chain_buffer, aes_out);
        payload_index += 16;    
    }

    // Add on the final payload block if it needs padding
    if (payload_remainder > 0)
    {
        PlatformZeroMemory(padded_buffer, 16);
        PlatformMoveMemory(padded_buffer, pSrc + payload_index, payload_remainder);
        bitwise_xor(aes_out, padded_buffer, chain_buffer);
        aes128k128d(pWpaKey[KeyID].Key, chain_buffer, aes_out);     
        payload_index += payload_remainder;
    }

    // aes_out contains padded mic, discard most significant
    // 8 bytes to generate 64 bit MIC
    PlatformMoveMemory(mic, aes_out, 8);


    if (!PlatformEqualMemory(mic, TrailMIC, 8))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("RTMPSoftDecryptAES, WEP Data MIC Error !\n"));    //MIC error.
        return FALSE;
    }


    // Go-ahead-shift Source Data with 8 bytes for CCMP header
    PlatformMoveMemory(pData, pSrc, payload_len);

    DBGPRINT(RT_DEBUG_INFO, ("RTMPSoftDecryptAES Decrypt done!!\n"));
    return TRUE;
}


// =============================================================================
// These function codes are implemented on Mgmt encrypted/decrypted frames 
//                          (Cisco Library)
// =============================================================================

// Pack struct to align at byte
#pragma pack(1)
typedef struct CCMP_NONCE {
    UCHAR   Priority: 4;
    UCHAR   Mgmt: 1;
    UCHAR   Rsvd: 3;
    UCHAR   Addr2[MAC_ADDR_LEN];
    UCHAR   PN[6];
} CCMP_NONCE, * PCCMP_NONCE;

typedef struct DOT11_AAD_ALL_FIELDS {
    FRAME_CONTROL   FC;
    UCHAR       Addr1[MAC_ADDR_LEN];
    UCHAR       Addr2[MAC_ADDR_LEN];
    UCHAR       Addr3[MAC_ADDR_LEN];
    USHORT      Frag:4;
    USHORT      Sequence:12;

    union {
        struct {
            UCHAR   Addr4[MAC_ADDR_LEN];
            UCHAR   A4QoSControl[2];
        };

        UCHAR A3QoSControl[2];
    };
} DOT11_AAD_ALL_FIELDS, *PDOT11_AAD_ALL_FIELDS;

typedef union CCMP_COUNTER {
    ULONGLONG dummy[2];
    struct {
        UCHAR flag;
        UCHAR nonce[13];
        USHORT bno;
    };
} CCMP_COUNTER, * PCCMP_COUNTER;

typedef union CCMP_DATA {
    ULONGLONG val64[2];
    UCHAR val8[16];
} CCMP_DATA, * PCCMP_DATA;

typedef struct CCMP128_STATE {
    CCMP_DATA MAC;  // Message Authentication Code
    CCMP_DATA S;
    CCMP_COUNTER cnr;
    AES_KEY *ktab;
    USHORT bno;
#if DBG
    USHORT len;
#endif  
    USHORT cur_loc;
} CCMP128_STATE, *PCCMP128_STATE;
#pragma pack()


void
ccmp128_init(
    AES_KEY * ktab,
    PUCHAR nonce,
    USHORT aad_len,
    PUCHAR aad,
    USHORT len,
    PCCMP128_STATE pState)
{
    unsigned char block[16];
    ULONGLONG * p;
    USHORT i;
    
    if (aad_len) {
        // L = 2, M = 8, AAD present
        block[0] = 0x59;
    } else {
        // L = 2, M = 8, AAD not present
        block[0] = 0x19;
    }

    memcpy(block + 1, nonce, 13);
    pState->cnr.flag = 1;
    memcpy(pState->cnr.nonce, nonce, 13);
    pState->bno = 0;
    pState->cur_loc = 0;
    pState->ktab = ktab;

#if DBG
    pState->len = len;
#endif
    *((USHORT*)(block + 14)) = be2cpu16(len);
    
    // prepare for authentication
    AES_encrypt(block, (u8 *)&pState->MAC, pState->ktab);
    if (!aad_len) {
        return;
    }
    
    if (aad_len < 14) {
        *((USHORT *)block) = be2cpu16((USHORT)aad_len);
        memcpy(block + 2, aad, aad_len);
        memset(block + 2 + aad_len, 0, 14 - aad_len);
        aad += aad_len;
        aad_len = 0;
    } else if (aad_len < 65536 - 256) {
        *((USHORT *)block) = be2cpu16((USHORT)aad_len);
        memcpy(block + 2, aad, 14);
        aad += 14;
        aad_len -= 14;
    } else {
    *((USHORT *)block) = be2cpu16(0xfffe);
    *((ULONG *)(block + 2)) = be2cpu32(aad_len);
    memcpy(block + 6, aad, 10);
    aad += 10;
    aad_len -= 10;
    }
    
    p = (ULONGLONG*)block;
    pState->MAC.val64[0] ^= p[0];
    pState->MAC.val64[1] ^= p[1];
    AES_encrypt((u8 *)&pState->MAC, (u8 *)&pState->MAC, pState->ktab);

    for (i = 0; i < (int)(aad_len & (~(16 - 1))); i += 16) {
    p = (ULONGLONG *)(aad + i);
    pState->MAC.val64[0] ^= p[0];
    pState->MAC.val64[1] ^= p[1];
    AES_encrypt((u8 *)&pState->MAC, (u8 *)&pState->MAC, pState->ktab);
    }
    
    if (i < aad_len) {
        aad_len = aad_len - i;
        memcpy(block, aad + i, aad_len);
        memset(block + aad_len, 0, 16 - aad_len);
        
        p = (ULONGLONG *)block;
        pState->MAC.val64[0] ^= p[0];
        pState->MAC.val64[1] ^= p[1];
        AES_encrypt((u8 *)&pState->MAC, (u8 *)&pState->MAC, pState->ktab);
    }
}

void
ccmp128_encrypt(
    PCCMP128_STATE pState,
    USHORT len,
    UCHAR * buf)
{
    ULONGLONG * p;
    USHORT i;
    USHORT tmp;
    
    if (!len) {
        return;
    }

    tmp = (-pState->cur_loc) & (16 - 1);
    if (tmp) {
        int block_end = (tmp <= len);
        
        if (!block_end) {
            tmp = len;
        }
        
        len = len - tmp;
        i = pState->cur_loc & (16 - 1);
        pState->cur_loc = pState->cur_loc + tmp;
        while(tmp) {
            pState->MAC.val8[i] ^= *buf;
            *buf ^= pState->S.val8[i];
            buf++;
            tmp--;
            i++;
        }
        
        if (block_end) {
            AES_encrypt((u8 *)&pState->MAC, (u8 *)&pState->MAC, pState->ktab);
        }
    }

    p = (ULONGLONG*)buf;
    for (i = 0; i < (len & (~(16 - 1))); i += 16) {
        pState->bno++;
        pState->cnr.bno = be2cpu16(pState->bno);
        AES_encrypt((u8 *)&pState->cnr, (u8 *)&pState->S, pState->ktab);
        
        pState->MAC.val64[0] ^= p[0];
        pState->MAC.val64[1] ^= p[1];
        p[0] ^= pState->S.val64[0];
        p[1] ^= pState->S.val64[1];
        
        AES_encrypt((u8 *)&pState->MAC, (u8 *)&pState->MAC, pState->ktab);
        p += 2;
    }
    
    pState->cur_loc = pState->cur_loc + i;
    buf = (UCHAR *)p;
    len = len - i;
    
    if (len) {
        pState->bno++;
        pState->cnr.bno = be2cpu16(pState->bno);
        AES_encrypt((u8 *)&pState->cnr, (u8 *)&pState->S, pState->ktab);
        
        pState->cur_loc = pState->cur_loc + len;
        for(i = 0; i < len; i++) {
            pState->MAC.val8[i] ^= *buf;
            *buf ^= pState->S.val8[i];
            buf++;
        }
    }
}

void
ccmp128_decrypt(
    PCCMP128_STATE pState,
    USHORT len,
    UCHAR * buf)
{
    ULONGLONG * p;
    USHORT i;
    USHORT tmp;
    
    if (!len) {
        return;
    }
    
    tmp = (-pState->cur_loc) & (16 - 1);
    if (tmp) {
        int block_end = (tmp <= len);
        
        if (!block_end) {
            tmp = len;
        }
        
        len = len - tmp;
        i = pState->cur_loc & (16 - 1);
        pState->cur_loc = pState->cur_loc + tmp;
        while(tmp) {
            *buf ^= pState->S.val8[i];
            pState->MAC.val8[i] ^= *buf;
            tmp--;
            buf++;
            i++;
        }
        
        if (block_end) {
            AES_encrypt((u8 *)&pState->MAC, (u8 *)&pState->MAC, pState->ktab);
        }
    }
    
    p = (ULONGLONG*)buf;
    for (i = 0; i < (len & (~(16 - 1))); i += 16) {
        pState->bno++;
        pState->cnr.bno = be2cpu16(pState->bno);
        AES_encrypt((u8 *)&pState->cnr, (u8 *)&pState->S, pState->ktab);
        
        p[0] ^= pState->S.val64[0];
        p[1] ^= pState->S.val64[1];
        pState->MAC.val64[0] ^= p[0];
        pState->MAC.val64[1] ^= p[1];
        
        AES_encrypt((u8 *)&pState->MAC, (u8 *)&pState->MAC, pState->ktab);
        p += 2;
    }
    
    pState->cur_loc = pState->cur_loc + i;
    buf = (UCHAR *)p;
    len = len - i;
    
    if (len) {
        pState->bno++;
        pState->cnr.bno = be2cpu16(pState->bno);
        AES_encrypt((u8 *)&pState->cnr, (u8 *)&pState->S, pState->ktab);
        
        pState->cur_loc = pState->cur_loc + len;
        for(i = 0; i < len; i++) {
            *buf ^= pState->S.val8[i];
            pState->MAC.val8[i] ^= *buf;
            buf++;
        }
    }
    
    ASSERT(pState->cur_loc <= pState->len);
    return;
}

void
ccmp128_finalize(
    PCCMP128_STATE pState,
    void * MIC)
{
    ULONGLONG m;
    if (pState->cur_loc & (16 - 1)) {
        AES_encrypt((u8 *)&pState->MAC, (u8 *)&pState->MAC, pState->ktab);
    }
    
    pState->cnr.bno = 0;
    AES_encrypt((u8 *)&pState->cnr, (u8 *)&pState->S, pState->ktab);
    m = pState->MAC.val64[0] ^ pState->S.val64[0];
    memcpy(MIC, &m, 8);
    
    ASSERT(pState->cur_loc == pState->len);
}

//
// TRUE: Success!
// FALSE: Encrypt Error!
//
BOOLEAN RTMPSoftEncryptMgmtAES(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN PUCHAR   pDot11Hdr,
    IN PUCHAR   pData,
    IN ULONG    DataByteCnt,
    IN UCHAR    KeyID,
    IN PCIPHER_KEY  pWpaKey)
{
    PHEADER_802_11          pHeader;
    ULONG                   HeaderLen = 24;
    CCMP_NONCE              nonce;
    DOT11_AAD_ALL_FIELDS    aad;
    USHORT                  aadLength;
    AES_KEY                 AESKeyTable;
    CCMP128_STATE           state;
    ULONG                   payload_len;
    INT                     a4_exists = 0;
    INT                     qc_exists = 0;
    INT                     status;
    PUCHAR                  pSrc;
    ULONG                   Iv16;
    ULONG                   Iv32;
    PUCHAR                  pTmp;
    
    BOOLEAN                 bMgmt = TRUE;
    /* CCMP header*/
    UCHAR                   ccmp_header[8] = {0};
    ULONG                   i;

    //
    // Point to 802.11 data header.
    //
    pHeader = (PHEADER_802_11)pDot11Hdr;
    

    if (pWpaKey[KeyID].KeyLen == 0)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("RTMPSoftEncryptMgmtAES failed! (KeyID[%d] Length can not be 0) \n", KeyID));
        return FALSE;
    }

    //
    // Data must contain 8-byte CCMP header, at least 1-byte of encrypted PDU, followed by
    // 8-byte MIC.
    //

    //
    // Check the data length. It has to be bigger than MAC header, 8 bytes CCMP header and 8 bytes MIC.
    //
    GetMacHdrLength(pDot11Hdr,  (UINT *)&HeaderLen,  (UINT *)&qc_exists, (UINT *)&a4_exists);

    if (DataByteCnt < (8 + 8))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("RTMPSoftEncryptMgmtAES! (Incorrect data length) \n"));
        return FALSE;
    }


    // init aad length, excluding duration
    aadLength = 22; 
    if ((qc_exists == 1) && (a4_exists == 0))   
        aadLength += 2;
    else if ((qc_exists == 0) && (a4_exists == 1)) 
        aadLength += 6;
    else if ((qc_exists == 1) && (a4_exists == 1))
        aadLength += 8;


    //
    // Get the AES128 key table.
    //
    status = AES_set_encrypt_key(pWpaKey[KeyID].Key, 128, &AESKeyTable);
    if (status < 0)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("Set AES key failed !\n"));
        return FALSE;
    }


    //
    // Start to fill output
    //
    
    pSrc = pData;

    //
    // Construct CCMP header
    //
    pTmp        = (PUCHAR) &Iv16;
    *pTmp       = pWpaKey[KeyID].TxTsc[0];
    *(pTmp + 1) = pWpaKey[KeyID].TxTsc[1];
    *(pTmp + 2) = 0;
    *(pTmp + 3) = (pPort->PortCfg.DefaultKeyId << 6) | 0x20;
    Iv32 = *(PULONG)(&pWpaKey[KeyID].TxTsc[2]);
                    
    // Increase TxTsc for next transmission
    INC_TX_TSC(pWpaKey[KeyID].TxTsc);
    
    PlatformMoveMemory(&ccmp_header[0], &Iv16, 4);  // Copy IV
    PlatformMoveMemory(&ccmp_header[4], &Iv32, 4);  // Copy EIV

    
    //
    // Construct Nonce and AAD
    //
    PlatformZeroMemory(&nonce, sizeof(nonce));
    PlatformZeroMemory(&aad, sizeof(aad));

    // 1. AAD
    PlatformMoveMemory(&aad.FC, &pHeader->FC, 2);
    PlatformMoveMemory(aad.Addr1, pHeader->Addr1, aadLength - 2);

    // -- these field must be muted
    if (!bMgmt)
        aad.FC.SubType = 0;
    aad.FC.Retry    = 0;
    aad.FC.PwrMgmt  = 0;
    aad.FC.MoreData = 0;
    aad.Sequence    = 0;
    // -- always turn on wep bit
    aad.FC.Wep      = 1;

    //  QC TID bit 4 to bit 15 are set to zero
    if ((qc_exists == 1) && (a4_exists == 0))
    {
        aad.A3QoSControl[0] &= 0x0f;
        aad.A3QoSControl[1] = 0;
    }
    else if ((qc_exists == 1) && (a4_exists == 1))
    {
        aad.A4QoSControl[0] &= 0x0f;
        aad.A4QoSControl[1] = 0;
    }

#if 0 //only for debug
    if(pHeader->FC.SubType == SUBTYPE_DISASSOC)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("MFP: Dump DISASSOC AAD !(size:%d)\n",sizeof(DOT11_AAD_ALL_FIELDS)));
        DumpFrame((PUCHAR)&aad, sizeof(DOT11_AAD_ALL_FIELDS));
    }

    if(pHeader->FC.SubType == SUBTYPE_DEAUTH)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("MFP: Dump DEAUTH AAD !(size:%d)\n",sizeof(DOT11_AAD_ALL_FIELDS)));
        DumpFrame((PUCHAR)&aad, sizeof(DOT11_AAD_ALL_FIELDS));
    }
#endif

    // 2. Nonce- Priority
    if (bMgmt)
    {
        if(MFP_ON(pAd, pPort))
        {
            nonce.Priority  = 0x00;
            nonce.Mgmt      = 0x01;
            nonce.Rsvd      = 0x00;
        }
        else
        {
            // nonce for CCX MFP
            // mgmt frames always do not include Qos
        nonce.Priority  = 0x0f;
            nonce.Mgmt      = 0x01;
            nonce.Rsvd      = 0x07;
        }
    }
    else if (qc_exists)
    {   
        //  QC TID bit 4 to bit 7 are set to zero
        if (a4_exists == 0)
            nonce.Priority = aad.A3QoSControl[0];
        else
            nonce.Priority = aad.A4QoSControl[0];
    }
    else
    {
        nonce.Priority  = 0x00;
    }
    
    // 3. Nonce- PN
    nonce.PN[0] = ccmp_header[7];   
    nonce.PN[1] = ccmp_header[6];
    nonce.PN[2] = ccmp_header[5];
    nonce.PN[3] = ccmp_header[4];
    nonce.PN[4] = ccmp_header[1];
    nonce.PN[5] = ccmp_header[0];   // point to the first byte of IV
    
    // 4. Nonce- Address2
    PlatformMoveMemory(nonce.Addr2,  pHeader->Addr2, MAC_ADDR_LEN);


    //
    // Initialize CCMP decryptor.
    //
    payload_len = (DataByteCnt - 8 - 8);
    ccmp128_init(&AESKeyTable,
                (PUCHAR)&nonce,
                aadLength,
                (PUCHAR)&aad,
                (USHORT)payload_len,
                &state);
        
    //
    // Encrypt the payload 
    //
    ccmp128_encrypt(&state, (USHORT)payload_len, pSrc);
    
    //
    // Calculate MIC
    //
    ccmp128_finalize(&state, pSrc + payload_len);


    // Go-back-shift Source Data with 8 bytes for CCMP header
    for (i = DataByteCnt - 1; i >= 8; i--)
        (UCHAR)(*(pSrc + i)) = (UCHAR)(*(pSrc + i - 8));
    // Append CCMP header
    PlatformMoveMemory(pSrc, ccmp_header, 8);

    DBGPRINT(RT_DEBUG_TRACE, ("RTMPSoftEncryptMgmtAES Encrypt done!!\n"));
    return TRUE;


}

//
// TRUE: Success!
// FALSE: Decrypt Error!
//
BOOLEAN RTMPSoftDecryptMgmtAES(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN PUCHAR   pDot11Hdr,
    IN PUCHAR   pData,
    IN ULONG    DataByteCnt, 
    IN PCIPHER_KEY  pWpaKey)
{
    PHEADER_802_11          pHeader;
    ULONG                   HeaderLen = 24;
    UCHAR                   KeyID;
    INT                     a4_exists = 0;
    INT                     qc_exists = 0;
    PUCHAR                  payload;
    UINT                    payload_len;
    UCHAR                   MIC[8];
    UCHAR                   TrailMIC[8];
    CCMP_NONCE              nonce;
    DOT11_AAD_ALL_FIELDS    aad;
    USHORT                  aadLength;
    AES_KEY                 AESKeyTable;
    CCMP128_STATE           state;
    INT                     status;
    PUCHAR                  pSrc;

    BOOLEAN                 bMgmt = TRUE;
    /* CCMP header*/
    UCHAR                   ccmp_header[8] = {0};
    
    //
    // Point to 802.11 data header.
    //
    pHeader = (PHEADER_802_11)pDot11Hdr;


    //
    // Data must contain 8-byte CCMP header, at least 1-byte of encrypted PDU, followed by
    // 8-byte MIC.
    //

    //
    // Check the data length. It has to be bigger than MAC header, 8 bytes CCMP header and 8 bytes MIC.
    //
    GetMacHdrLength(pDot11Hdr,  (UINT *)&HeaderLen,  (UINT *)&qc_exists, (UINT *)&a4_exists);

    if (DataByteCnt < (8 + 8))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("RTMPSoftDecryptMgmtAES failed! (Incorrect data length) \n"));
        return FALSE;
    }


    // init aad length, excluding duration
    aadLength = 22; 
    if ((qc_exists == 1) && (a4_exists == 0))   
        aadLength += 2;
    else if ((qc_exists == 0) && (a4_exists == 1)) 
        aadLength += 6;
    else if ((qc_exists == 1) && (a4_exists == 1))
        aadLength += 8;


    KeyID = *((PUCHAR)(pData + 3)); 
    KeyID = KeyID >> 6;
    if (pWpaKey[KeyID].KeyLen == 0)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("RTMPSoftDecryptMgmtAES failed!(KeyID[%d] Length can not be 0)\n", KeyID));
        return FALSE;
    }


    // Validate CCMP header. 
    // 1. Reserved field (3rd byte) must be 0.
    // 2. In 4th byte of CCMP header, ExtIV must be 1 and rsvd must be 0
    if ((*((PUCHAR)(pData + 2))) != 0)
        return FALSE;

    if ((( *((PUCHAR)(pData + 3))) & 0x3f) != 0x20)
        return FALSE;


    //
    // Construct Nonce and AAD
    //
    PlatformZeroMemory(&nonce, sizeof(nonce));
    PlatformZeroMemory(&aad, sizeof(aad));

    // 1. AAD
    PlatformMoveMemory(&aad.FC, &pHeader->FC, 2);
    PlatformMoveMemory(aad.Addr1, pHeader->Addr1, aadLength - 2);
    // -- these field must be muted
    if (!bMgmt)
        aad.FC.SubType = 0;
    aad.FC.Retry    = 0;
    aad.FC.PwrMgmt  = 0;
    aad.FC.MoreData = 0;
    aad.Sequence    = 0;
    // -- always turn on wep bit
    aad.FC.Wep      = 1;

    //  QC TID bit 4 to bit 15 are set to zero
    if ((qc_exists == 1) && (a4_exists == 0))
    {
        aad.A3QoSControl[0] &= 0x0f;
        aad.A3QoSControl[1] = 0;
    }
    else if ((qc_exists == 1) && (a4_exists == 1))
    {
        aad.A4QoSControl[0] &= 0x0f;
        aad.A4QoSControl[1] = 0;
    }


    // 2. Nonce- Priority
    if (bMgmt)
    {
        if(MFP_ON(pAd, pPort))
        {
            nonce.Priority  = 0x00;
            nonce.Mgmt      = 0x01;
            nonce.Rsvd      = 0x00;
        }
        else
        {
            // nonce for CCX MFP
            // mgmt frames always do not include Qos
        nonce.Priority  = 0x0f;
            nonce.Mgmt      = 0x01;
            nonce.Rsvd      = 0x07;
        }
    }
    else if (qc_exists)
    {   
        //  QC TID bit 4 to bit 7 are set to zero
        if (a4_exists == 0)
            nonce.Priority = aad.A3QoSControl[0];
        else
            nonce.Priority = aad.A4QoSControl[0];
    }
    else
    {
        nonce.Priority  = 0x00;
    }
    
    // 3. Nonce- PN
    PlatformMoveMemory(ccmp_header, pData, 8);  
    nonce.PN[0] = ccmp_header[7];
    nonce.PN[1] = ccmp_header[6];
    nonce.PN[2] = ccmp_header[5];
    nonce.PN[3] = ccmp_header[4];
    nonce.PN[4] = ccmp_header[1];
    nonce.PN[5] = ccmp_header[0];
    
    // 4. Nonce- Address2
    PlatformMoveMemory(nonce.Addr2,  pHeader->Addr2, MAC_ADDR_LEN);
    

    //
    // Get the AES128 key table.
    //
    status = AES_set_encrypt_key(pWpaKey[KeyID].Key, 128, &AESKeyTable);
    if (status < 0)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("Set AES key failed !\n"));
        return FALSE;
    }
    

    //
    // Start to fill output
    //

    pSrc = pData + 8;   /* pSrc point to the first byte of ciphertext */

    
    //
    // Initialize CCMP decryptor.
    //
    
    // Find start of payload
    payload = pSrc;
    payload_len = (DataByteCnt - 8 - 8);
    PlatformMoveMemory(TrailMIC, payload + payload_len, 8); // backup MIC from received data
    
    ccmp128_init(&AESKeyTable,
                (PUCHAR)&nonce,
                aadLength,
                (PUCHAR)&aad,
                (USHORT)payload_len,
                &state);

    //
    // Decrypt the payload 
    //
    ccmp128_decrypt(&state, (USHORT)payload_len, payload);

    //
    // Verify MIC
    //
    ccmp128_finalize(&state, &MIC);

    if (!PlatformEqualMemory(MIC, TrailMIC, 8))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("RTMPSoftDecryptMgmtAES, WEP Data MIC Error !\n"));    //MIC error.   
        return FALSE;
    }

    // fill pliantext
    // Go-ahead-shift Source Data with 8 bytes for CCMP header
    PlatformMoveMemory(pData, pSrc, payload_len);

    DBGPRINT(RT_DEBUG_TRACE, ("RTMPSoftDecryptMgmtAES Decrypt done!!\n"));
    return TRUE;
}
