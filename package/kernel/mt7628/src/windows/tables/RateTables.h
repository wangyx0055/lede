/*
    ***************************************************************************
    * Ralink Technology, Corp.
    * 5F., No. 5, Tai-Yuan 1st St., Jhubei City, Hsinchu County 30265, Taiwan
    *
    * (c) Copyright 2002-2011, Ralink Technology, Corp.
    *
    * All rights reserved. Ralink's source code is an unpublished work and the use of a copyright notice 
    * does not imply otherwise. This source code contains confidential trade secret material of 
    * Ralink Technology, Corp. Any attempt or participation in deciphering, decoding, reverse engineering 
    * or in any way altering the source code is stricitly prohibited, unless the prior written consent of 
    * Ralink Technology, Corp. is obtained.
    ***************************************************************************

    Module Name: 
    RT6X9X.c

    Abstract: 
    RT6X9X features (RT6590/RT6592/MT7601)

    Revision History:
    Who                         When            What
    -------------------     ----------      ----------------------------------------------
    Ian Tang and Brenton Wu     2011/11/16      Start implementation
    Ian Tang                    2012/3/13       Add MT7650 (RT6590)
*/

//
// CCK/OFDM/HT rate table
//
USHORT CckOfdmHtRateTable[256] = 
{
    //
    // CCK and legacy OFDM
    //
    2, /* 1Mbps*/           4, /* 2Mbps */          11, /* 5.5Mbps */       22, /* 11Mbps */ 
    12, /* 6Mbps */         18, /* 9Mbps */         24, /* 12Mbps */        36, /* 18Mbps */
    48, /* 24Mbps */        72, /* 36Mbps */        96, /* 48Mbps */        108, /* 54Mbps */
    109, /* NA */           110, /* NA */           111, /* NA */           112, /* NA */

    //
    // 1x1 long GI in 20MHz
    //
    13, /* 6.5Mbps */       26, /* 13Mbps */        39, /* 19.5Mbps */      52, /* 26Mbps */
    78, /* 39Mbps*/         104, /* 52Mbps */       117, /* 58.5Mbps */     130, /* 65Mbps */

    //
    // 2x2 long GI in 20MHz
    //
    26, /* 13Mbps */        52, /* 26Mbps */        78, /* 39Mbps */        104, /* 52Mbps */
    156, /* 78Mbps */       208, /* 104Mbps */      234, /* 117Mbps */      260, /* 130Mbp */

    //
    // 1x1 long GI in 40MHz
    //
    27, /* 13.5Mbps */      54, /* 27Mbps */        81, /* 40.5Mbps */      108, /* 54Mbps */
    162, /* 81Mbps */       216, /* 108Mbps */      243, /* 121.5Mbps */    270, /* 135Mbps */

    //
    // 2x2 long GI in 40MHz
    //
    54, /* 27Mbps */        108, /* 54Mbps */       162, /* 81Mbps */       216, /* 108Mbps */
    324, /* 162Mbps */      432, /* 216Mbps */      486, /* 243Mbps */      540, /* 270Mbps */

    //
    // 1x1 short GI in 20MHz
    //
    14, /* 7Mbps */         29, /* 14.5Mbps */      43, /* 21.5Mbps */      57, /* 28.5Mbps */
    87, /* 43.5Mbps */      115, /* 57.5Mbps */     130, /* 65Mbps */       144, /* 72Mbps */

    //
    // 2x2 short GI in 20MHz
    //
    29, /* 14.5Mbps */      59, /* 29.5Mbps */      87, /* 43.5Mbps */      115, /* 57.5Mbps */
    173, /* 86.5Mbps */     230, /* 115Mbps */      260, /* 130Mbps */      288, /* 144Mbps */

    //
    // 1x1 short GI in 40MHz
    //
    30, /* 15Mbps */        60, /* 30Mbps */        90, /* 45Mbps */        120, /* 60Mbps */
    180, /* 90Mbps */       240, /* 120Mbps */      270, /* 135Mbps */      300, /* 150Mbps */

    //
    // 2x2 short GI in 40MHz
    //
    60, /* 30Mbps*/         120, /* 60Mbps */       180, /* 90Mbps */       240, /* 120Mbps */
    360, /* 180Mbps */      480, /* 240Mbps */      540, /* 270Mbps */      600, /* 300Mbps */

    //
    // 3x3 long GI in 20MHz
    //
    39, /* 19.5Mbps */      78, /* 39Mbps */        117, /* 58.5Mbps */     156, /* 78Mbps */
    234, /* 117Mbps */      312, /* 156Mbps */      351, /* 175.5Mbps */    390, /* 195Mbps */

    //
    // 4x4 long GI in 20MHz
    //
    52, /* 26Mbps */        104, /* 52Mbps */       156, /* 78Mbps */       208, /* 104Mbps */
    312, /* 156Mbps */      416, /* 208Mbps */      468, /* 234Mbps */      520, /* 260Mbps */

    //
    // 3x3 long GI in 40MHz
    //
    81, /* 40.5Mbps */      162, /* 81Mbps */       243, /* 121.5Mbps */    324, /* 162Mbps */
    486, /* 243Mbps */      648, /* 324Mbps */      729, /* 364.5Mbps */    810, /* 405Mbps */

    //
    // 4x4 long GI in 40MHz
    //
    108, /* 54Mbps */       216, /* 108Mbps */      324, /* 162Mbps */      432, /* 216Mbps */
    648, /* 324Mbps */      864, /* 432Mbps */      972, /* 486Mbps */      1080, /* 540Mbps*/

    //
    // 3x3 short GI in 20MHz
    //
    43, /* 21.5Mbps */      87, /* 43.5Mbps */      130, /* 65Mbps */       173, /* 86.5Mbps */
    260, /* 130Mbps */      347, /* 173.5Mbps */    390, /* 195Mbps */      433, /* 216.5Mbps */

    //
    // 4x4 short GI in 20MHz
    //
    58, /* 29Mbps */        116, /* 58Mbps */       173, /* 86.5Mbps */     231, /* 115.5Mbps */
    347, /* 173.5Mbps */    462, /* 231Mbps */      520, /* 260Mbps */      578, /* 289Mbps */

    //
    // 3x3 short GI in 40MHz
    //
    90, /* 45Mbps */        180, /* 90Mbps */       270, /* 135Mbps */      360, /* 180Mbps */
    540, /* 270Mbps */      720, /* 360Mbps */      810, /* 405Mbps */      900, /* 450Mbps */

    //
    // 4x4 short GI in 40MHz
    //
    120, /* 60Mbps */       240, /* 120Mbps */      360, /* 180Mbps */      480, /* 240Mbps */
    720, /* 360Mbps */      960, /* 480Mbps */      1080, /* 540Mbps */     1200, /* 600Mbps */

    //
    // NA
    //
    64, /* NA */            65, /* NA */            66, /* NA */            67, /* NA */
    68, /* NA */            69, /* NA */            70, /* NA */            71, /* NA */
    72, /* NA */            73, /* NA */            74, /* NA */            75, /* NA */
    76, /* NA */            77, /* NA */            78, /* NA */            79, /* NA */

    //
    // NA
    //
    80 /* NA */
};

//
// VHT rate table
//
USHORT VhtRateTable[] = 
{
    //
    // 1x1, long GI, 20MHz
    //
    13, /* 6.5Mbps */       26, /* 13Mbps */        39, /* 19.5Mbps */          52, /* 26Mbps */
    78, /* 39Mbps */        104, /* 52Mbps */       117, /* 58.5Mbps */         130, /* 65Mbps */
    156, /* 78Mbps */       0, /* NA */

    //
    // 2x2, long GI, 20MHz
    //
    26, /* 13Mbps */        52, /* 26Mbps */        78, /* 39Mbps */            104, /* 52Mbps */
    156, /* 78Mbps */       208, /* 104Mbps */      234, /* 117Mbps */          260, /* 130Mbps */
    312, /* 156Mbps */      0, /* NA */

    //
    // 3x3, long GI, 20MHz
    //
    39, /* 19.5Mbps */      78, /* 39Mbps */        117, /* 58.5Mbps */         156, /* 78Mbps */
    234, /* 117Mbps */      312, /* 156Mbps */      351, /* 175.5Mbps */        390, /* 195Mbps */
    468, /* 234Mbps */      520, /* 260Mbps */

    //
    // 4x4, long GI, 20MHz
    //
    52, /* 26Mbps */        104, /* 52Mbps */       156, /* 78Mbps */           208, /* 104Mbps */
    312, /* 156Mbps */      416, /* 208Mbps */      468, /* 234Mbps */          520, /* 260Mbps */
    624, /* 312Mbps */      0, /* NA */

    //
    // 1x1, long GI, 40MHz
    //
    27, /* 13.5Mbps */      54, /* 27Mbps */        81, /* 40.5Mbps */          108, /* 54Mbps */
    162, /* 81Mbps */       216, /* 108Mbps */      243, /* 121.5Mbps */        270, /* 135Mbps */
    324, /* 162Mbps */      360, /* 180Mbps */

    //
    // 2x2, long GI, 40MHz
    //
    54, /* 27Mbps */        108, /* 54Mbps */       162, /* 81Mbps */           216, /* 108Mbps */
    324, /* 162Mbps */      432, /* 216Mbps */      486, /* 243Mbps */          540, /* 270Mbps */
    648, /* 324Mbps */      720, /* 360Mbps */
    
    //
    // 3x3, long GI, 40MHz
    //
    81, /* 40.5Mbps */      162, /* 81Mbps */       243, /* 121.5Mbps */        324, /* 162Mbps */
    486, /* 243Mbps */      648, /* 324Mbps */      729, /* 364.5Mbps */        810, /* 405Mbps */
    972, /* 486Mbps */      1080, /* 540Mbps */

    //
    // 4x4, long GI, 40MHz
    //
    108, /* 54Mbps */       216, /* 108Mbps */      324, /* 162Mbps */          432, /* 216Mbps */
    648, /* 324Mbps */      864, /* 432Mbps */      972, /* 486Mbps */          1080, /* 540Mbps */
    1296, /* 648Mbps */     1440, /* 720Mbps */

    //
    // 1x1, long GI, 80MHz
    //
    59, /* 29.3Mbps */      117, /* 58.5Mbps */     176, /* 87.8Mbps */         234, /* 117Mbps */
    351, /* 175.5Mbps */    468, /* 234Mbps */      527, /* 263.3Mbps */        585, /* 292.5Mbps */
    702, /* 351Mbps */      780, /* 390Mbps */

    //
    // 2x2, long GI, 80MHz
    //
    117, /* 58.5Mbps */     234, /* 117Mbps */      351, /* 175.5Mbps */        468, /* 234Mbps */
    702, /* 351Mbps */      936, /* 468Mbps */      1053, /* 526.5Mbps */       1170, /* 585Mbps */
    1404, /* 702Mbps */     1560, /* 780Mbps */

    //
    // 3x3, long GI, 80MHz
    //
    176, /* 87.8Mbps */     351, /* 175.5Mbps */    527, /* 263.3Mbps */        702, /* 351Mbps */
    1053, /* 526.5Mbps */   1404, /* 702Mbps */     0, /* NA */                 1755, /* 877.5Mbps */
    2106, /* 1053Mbps */    2340, /* 1170Mbps */

    //
    // 4x4, long GI, 80MHz
    //
    234, /* 117Mbps */      468, /* 234Mbps */      702, /* 351Mbps */          936, /* 468Mbps */
    1404, /* 702Mbps */     1872, /* 936Mbps */     2106, /* 1053Mbps */        2340, /* 1170Mbps */
    2808, /* 1404Mbps */    3120, /* 1560Mbps */

    //
    // 1x1, long GI, 160MHz
    //
    117, /* 58.5Mbps */     234, /* 117Mbps */      351, /* 175.5Mbps */        468, /* 234Mbps */
    702, /* 351Mbps */      936, /* 468Mbps */      1053, /* 526.5Mbps */       1170, /* 585Mbps */
    1404, /* 702Mbps */     1560, /* 780Mbps */

    //
    // 2x2, long GI, 160MHz
    //
    234, /* 117Mbps */      468, /* 234Mbps */      702, /* 351Mbps */          936, /* 468Mbps */
    1404, /* 702Mbps */     1872, /* 936Mbps */     2106, /* 1053Mbps */        2340, /* 1170Mbps */
    2808, /* 1404Mbps */    3120, /* 1560Mbps */

    //
    // 3x3, long GI, 160MHz
    //
    351, /* 175.5Mbps */    702, /* 351Mbps */      1053, /* 526.5Mbps */       1404, /* 702Mbps */
    2106, /* 1053Mbps */    2808, /* 1404Mbps */    3159, /* 1579.5Mbps */      3510, /* 1755Mbps */
    4212, /* 2106Mbps */    0, /* NA */

    //
    // 4x4, long GI, 160MHz
    //
    468, /* 234Mbps */      936, /* 468Mbps */      1404, /* 702Mbps */         1872, /* 936Mbps */
    2808, /* 1404Mbps */    3744, /* 1872Mbps */    4212, /* 2106Mbps */        0, /* NA */
    5616, /* 2808Mbps */    6240, /* 3120Mbps */

    //
    // 1x1, short GI, 20MHz
    //
    14, /* 7.2Mbps */       29, /* 14.4Mbps */      43, /* 21.7Mbps */          58, /* 28.9Mbps */
    87, /* 43.3Mbps */      116, /* 57.8Mbps */     130, /* 65Mbps */           144, /* 72.2Mbps */
    173, /* 86.7Mbps */     0, /* NA */

    //
    // 2x2, short GI, 20MHz
    //
    29, /* 14.4Mbps */      58, /* 28.9Mbps */      87, /* 43.3Mbps */          116, /* 57.8Mbps */
    173, /* 86.7Mbps */     231, /* 115.6Mbps */    260, /* 130Mbps */          289, /* 144.4Mbps */
    347, /* 173.3Mbps */    0, /* NA */

    //
    // 3x3, short GI, 20MHz
    //
    43, /* 21.7Mbps */      87, /* 43.3Mbps */      130, /* 65Mbps */           173, /* 86.7Mbps */
    260, /* 130Mbps */      347, /* 173.3Mbps */    390, /* 195Mbps */          433, /* 216.7Mbps */
    520, /* 260Mbps */      578, /* 288.9Mbps */

    //
    // 4x4, short GI, 20MHz
    //
    58, /* 28.9Mbps */      116, /* 57.8Mbps */     173, /* 86.7Mbps */         231, /* 115.6Mbps */
    347, /* 173.3Mbps */    462, /* 231.1Mbps */    520, /* 260Mbps */          578, /* 288.9Mbps */
    693, /* 346.7Mbps */    0, /* NA */

    //
    // 1x1, short GI, 40MHz
    //
    30, /* 15Mbps */        60, /* 30Mbps */        90, /* 45Mbps */            120, /* 60Mbps */
    180, /* 90Mbps */       240, /* 120Mbps */      270, /* 135Mbps */          300, /* 150Mbps */
    360, /* 180Mbps */      400, /* 200Mbps */

    //
    // 2x2, short GI, 40MHz
    //
    60, /* 30Mbps */        120, /* 60Mbps */       180, /* 90Mbps */           240, /* 120Mbps */
    360, /* 180Mbps */      480, /* 240Mbps */      540, /* 270Mbps */          600, /* 300Mbps */
    720, /* 360Mbps */      800, /* 400Mbps */

    //
    // 3x3, short GI, 40MHz
    //
    90, /* 45Mbps */        180, /* 90Mbps */       270, /* 135Mbps */          360, /* 180Mbps */
    540, /* 270Mbps */      720, /* 360Mbps */      810, /* 405Mbps */          900, /* 450Mbps */
    1080, /* 540Mbps */     1200, /* 600Mbps */

    //
    // 4x4, short GI, 40MHz
    //
    120, /* 60Mbps */       240, /* 120Mbps */      360, /* 180Mbps */          480, /* 240Mbps */
    720, /* 360Mbps */      960, /* 480Mbps */      1080, /* 540Mbps */         1200, /* 600Mbps */
    1440, /* 720Mbps */     1600, /* 800Mbps */

    //
    // 1x1, short GI, 80MHz
    //
    65, /* 32.5Mbps */      130, /* 65Mbps */       195, /* 97.5Mbps */         260, /* 130Mbps */
    390, /* 195Mbps */      520, /* 260Mbps */      585, /* 292.5Mbps */        650, /* 325Mbps */
    780, /* 390Mbps */      867, /* 433.3Mbps */

    //
    // 2x2, short GI, 80MHz
    //
    130, /* 65Mbps */       260, /* 130Mbps */      390, /* 195Mbps */          520, /* 260Mbps */
    780, /* 390Mbps */      1040, /* 520Mbps */     1170, /* 585Mbps */         1300, /* 650Mbps */
    1560, /* 780Mbps */     1733, /* 866.7Mbps */

    //
    // 3x3, short GI, 80MHz
    //
    195, /* 97.5Mbps */     390, /* 195Mbps */      585, /* 292.5Mbps */        780, /* 390Mbps */
    1170, /* 585Mbps */     1560, /* 780Mbps */     0, /* NA */                 1950, /* 975Mbps */
    2340, /* 1170Mbps */    2600, /* 1300Mbps */

    //
    // 4x4, short GI, 80MHz
    //
    260, /* 130Mbps */      520, /* 260Mbps */      780, /* 390 Mbps */         1040, /* 520Mbps */
    1560, /* 780Mbps */     2080, /* 1040Mbps */    2340, /* 1170Mbps */        2600, /* 1300Mbps */
    3120, /* 1560Mbps */    3467, /* 1733.3Mbps */

    //
    // 1x1, short GI, 160MHz
    //
    130, /* 65Mbps */       260, /* 130Mbps */      390, /* 195Mbps */          520, /* 260Mbps */
    780, /* 390Mbps */      1040, /* 520Mbps */     1170, /* 585Mbps */         1300, /* 650Mbps */
    1560, /* 780Mbps */     1733, /* 866.7Mbps */

    //
    // 2x2, short GI, 160MHz
    //
    260, /* 130Mbps */      520, /* 260Mbps */      780, /* 390Mbps */          1040, /* 520Mbps */
    1560, /* 780Mbps */     2080, /* 1040Mbps */    2340, /* 1170Mbps */        2600, /* 1300Mbps */
    3120, /* 1560Mbps */    3467, /* 1733.3Mbps */

    //
    // 3x3, short GI, 160MHz
    //
    390, /* 195Mbps */      780, /* 390Mbps */      1170, /* 585Mbps */         1560, /* 780Mbps */
    2340, /* 1170Mbps */    3120, /* 1560Mbps */    3510, /* 1755Mbps */        3900, /* 1950Mbps */
    4680, /* 2340Mbps */    0, /* NA */

    //
    // 4x4, short GI, 160MHz
    //
    520, /* 260Mbps */      1040, /* 520Mbps */     1560, /* 780Mbps */         2080, /* 1040Mbps */
    3120, /* 1560Mbps */    4160, /* 2080Mbps */    4680, /* 2340Mbps */        0, /* NA */
    6240, /* 3120Mbps */    6933, /* 3466.7Mbps */
};

//
// AGS: 1x1 VHT-capable rate table
//
//2 TODO:  
//2     1. 2.4GHz only
//2 2. Support 256QAM
//
UCHAR Ags1x1VhtRateTable[] = 
{
    //
    //      [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown] [downMCS] [upMCS3] [upMCS2] [upMCS1]
    //
    // Note: downMCS, upMCS3, upMCS2 and upMCS1 are zero-based array index.
    //
    // [Mode]*: 
    // bit0: STBC -STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~3: BW - BW_XXX  <VHT card has the Rate tuning algorithm that change the Bandwidth.>
    // bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //

// Initial used item after association: the number of rate indexes, the initial MCS (index)
/* 0 */     0x0D,   0x0C,   0x00, 0,                                0,  0,      0,  0,  0,  0,

// The Dara Rate of throughput extension for long distance by using lower data rate (VHT BW80 > BW40 > OFDM)
/* 1 */     0x00,   0x11,   0x00, 0, /* OFDM1x1 MCS 0, BW20 */      40, 101,    0,  0,  0,  1,  // Reserved, not use.
/* 2 */     0x01,   0x11,   0x00, 0, /* OFDM1x1 MCS 0, BW20 */      40, 50,     0,  0,  0,  2,  // Reserved, not use.
/* 3 */     0x02,   0x11,   0x00, 0, /* OFDM1x1 MCS 0, BW20 */      30, 101,    1,  0,  0,  3, 
/* 4 */     0x03,   0x45,   0x00, 0, /* VHT 1x1 MCS 0, BW40 */      30, 50,     2,  0,  0,  4, 

// The Data Rate of VHT 1 Spatial Stream
/* 5 */     0x04,   0x49,   0x00, 0, /* VHT 1x1 MCS 0, BW80 */      30, 50,     3,  0,  0,  5, 
/* 6 */     0x05,   0x49,   0x00, 1, /* VHT 1x1 MCS 1, BW80 */      20, 50,     4,  0,  0,  6, 
/* 7 */     0x06,   0x49,   0x00, 2, /* VHT 1x1 MCS 2, BW80 */      20, 50,     5,  0,  0,  7, 
/* 8 */     0x07,   0x49,   0x00, 3, /* VHT 1x1 MCS 3, BW80 */      15, 50,     6,  0,  0,  8, 
/* 9 */     0x08,   0x49,   0x00, 4, /* VHT 1x1 MCS 4, BW80 */      15, 30,     7,  0,  0,  9, 
/*10 */     0x09,   0x49,   0x00, 5, /* VHT 1x1 MCS 5, BW80 */      10, 25,     8,  0,  0,  10, 
/*11 */     0x0A,   0x49,   0x00, 6, /* VHT 1x1 MCS 6, BW80 */      8,  14,     9,  0,  0,  11, 
/*12 */     0x0B,   0x49,   0x00, 7, /* VHT 1x1 MCS 7, BW80 */      8,  14,     10, 0,  0,  12, 
/*13 */     0x0C,   0x4B,   0x00, 7, /* VHT 1x1 MCS 7, BW80, SGI */ 8,  14,     11, 0,  0,  12, 

//---------------------------------------------------------------------------------------------------------------
//  CurrMCS             downMCS     upMCS3              upMCS2              upMCS1
//---------------------------------------------------------------------------------------------------------------
//  Reserved, not use.
//  Reserved, not use.
//  OFDM1x1 MCS 0 BW20      OFDM1x1 MCS 0       N/A                 N/A                 VHT 1x1 MCS 0 BW40
//  VHT 1x1 MCS 0 BW40      VHT 1x1 MCS 0 BW20  N/A                 N/A                 VHT 1x1 MCS 0 BW80

//  VHT 1x1 MCS 0 BW80      VHT 1x1 MCS 0 BW40  N/A                 N/A                 VHT 1x1 MCS 1 BW80
//  VHT 1x1 MCS 1 BW80      VHT 1x1 MCS 0 BW80  N/A                 N/A                 VHT 1x1 MCS 2 BW80
//  VHT 1x1 MCS 2 BW80      VHT 1x1 MCS 1 BW80  N/A                 N/A                 VHT 1x1 MCS 3 BW80
//  VHT 1x1 MCS 3 BW80      VHT 1x1 MCS 2 BW80  N/A                 N/A                 VHT 1x1 MCS 4 BW80
//  VHT 1x1 MCS 4 BW80      VHT 1x1 MCS 3 BW80  N/A                 N/A                 VHT 1x1 MCS 5 BW80
//  VHT 1x1 MCS 5 BW80      VHT 1x1 MCS 4 BW80  N/A                 N/A                 VHT 1x1 MCS 6 BW80
//  VHT 1x1 MCS 6 BW80      VHT 1x1 MCS 5 BW80  N/A                 N/A                 VHT 1x1 MCS 7 BW80
//  VHT 1x1 MCS 7 BW80      VHT 1x1 MCS 6 BW80  N/A                 N/A                 VHT 1x1 MCS 7, SGI
//  VHT 1x1 MCS 7 BW80,SGI  VHT 1x1 MCS 7 BW80  N/A                 N/A                 VHT 1x1 MCS 7, SGI
//---------------------------------------------------------------------------------------------------------------

};

//
// AGS: 1x1 VHT-capable rate table with 256QAM
//
UCHAR Ags1x1Vht256QAMRateTable[] = 
{
    //
    //      [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown] [downMCS] [upMCS3] [upMCS2] [upMCS1]
    //
    // Note: downMCS, upMCS3, upMCS2 and upMCS1 are zero-based array index.
    //
    // [Mode]*: 
    // bit0: STBC -STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~3: BW - BW_XXX  <VHT card has the Rate tuning algorithm that change the Bandwidth.>
    // bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //

// Initial used item after association: the number of rate indexes, the initial MCS (index)
/* 0 */     0x0F,   0x0E,   0x00, 0,                                0,  0,      0,  0,  0,  0,

// The Dara Rate of throughput extension for long distance by using lower data rate (VHT BW80 > BW40 > OFDM)
/* 1 */     0x00,   0x11,   0x00, 0, /* OFDM1x1 MCS 0, BW20 */      40, 101,        0,  0,  0,  1, // Reserved, not use.
/* 2 */     0x01,   0x11,   0x00, 0, /* OFDM1x1 MCS 0, BW20 */      40, 50,     0,  0,  0,  2, // Reserved, not use.
/* 3 */     0x02,   0x11,   0x00, 0, /* OFDM1x1 MCS 0, BW20 */      30, 101,        1,  0,  0,  3, 
/* 4 */     0x03,   0x45,   0x00, 0, /* VHT 1x1 MCS 0, BW40 */      30, 50,     2,  0,  0,  4, 

// The Data Rate of VHT 1 Spatial Stream
/* 5 */     0x04,   0x49,   0x00, 0, /* VHT 1x1 MCS 0, BW80 */      30, 50,     3,  0,  0,  5, 
/* 6 */     0x05,   0x49,   0x00, 1, /* VHT 1x1 MCS 1, BW80 */      20, 50,     4,  0,  0,  6, 
/* 7 */     0x06,   0x49,   0x00, 2, /* VHT 1x1 MCS 2, BW80 */      20, 50,     5,  0,  0,  7, 
/* 8 */     0x07,   0x49,   0x00, 3, /* VHT 1x1 MCS 3, BW80 */      15, 50,     6,  0,  0,  8, 
/* 9 */     0x08,   0x49,   0x00, 4, /* VHT 1x1 MCS 4, BW80 */      15, 30,     7,  0,  0,  9, 
/*10 */     0x09,   0x49,   0x00, 5, /* VHT 1x1 MCS 5, BW80 */      10, 25,     8,  0,  0,  10, 
/*11 */     0x0A,   0x49,   0x00, 6, /* VHT 1x1 MCS 6, BW80 */      8,  14,     9,  0,  0,  11, 
/*12 */     0x0B,   0x49,   0x00, 7, /* VHT 1x1 MCS 7, BW80 */      8,  14,     10, 0,  0,  12, 
/*13 */     0x0C,   0x49,   0x00, 8, /* VHT 1x1 MCS 8, BW80 */      8,  14,     11, 0,  0,  13, 
/*14 */     0x0D,   0x49,   0x00, 9, /* VHT 1x1 MCS 9, BW80 */      8,  14,     12, 0,  0,  14, 
/*15 */     0x0E,   0x4B,   0x00, 9, /* VHT 1x1 MCS 9, BW80, SGI */ 8,  14,     13, 0,  0,  14, 

//---------------------------------------------------------------------------------------------------------------
//  CurrMCS                 downMCS             upMCS3  upMCS2  upMCS1
//---------------------------------------------------------------------------------------------------------------
//  Reserved, not use.
//  Reserved, not use.
//  OFDM1x1 MCS 0 BW20      OFDM1x1 MCS 0       N/A         N/A     VHT 1x1 MCS 0 BW40
//  VHT 1x1 MCS 0 BW40      VHT 1x1 MCS 0 BW20  N/A         N/A     VHT 1x1 MCS 0 BW80

//  VHT 1x1 MCS 0 BW80      VHT 1x1 MCS 0 BW40  N/A     N/A     VHT 1x1 MCS 1 BW80
//  VHT 1x1 MCS 1 BW80      VHT 1x1 MCS 0 BW80  N/A     N/A     VHT 1x1 MCS 2 BW80
//  VHT 1x1 MCS 2 BW80      VHT 1x1 MCS 1 BW80  N/A     N/A     VHT 1x1 MCS 3 BW80
//  VHT 1x1 MCS 3 BW80      VHT 1x1 MCS 2 BW80  N/A     N/A     VHT 1x1 MCS 4 BW80
//  VHT 1x1 MCS 4 BW80      VHT 1x1 MCS 3 BW80  N/A     N/A     VHT 1x1 MCS 5 BW80
//  VHT 1x1 MCS 5 BW80      VHT 1x1 MCS 4 BW80  N/A     N/A     VHT 1x1 MCS 6 BW80
//  VHT 1x1 MCS 6 BW80      VHT 1x1 MCS 5 BW80  N/A     N/A     VHT 1x1 MCS 7 BW80
//  VHT 1x1 MCS 7 BW80      VHT 1x1 MCS 6 BW80  N/A     N/A     VHT 1x1 MCS 7 BW80
//  VHT 1x1 MCS 8 BW80      VHT 1x1 MCS 7 BW80  N/A     N/A     VHT 1x1 MCS 9 BW80
//  VHT 1x1 MCS 9 BW80      VHT 1x1 MCS 8 BW80  N/A     N/A     VHT 1x1 MCS 9 BW80, SGI
//  VHT 1x1 MCS 9 BW80      VHT 1x1 MCS 9 BW80  N/A     N/A     VHT 1x1 MCS 9 BW80, SGI
//---------------------------------------------------------------------------------------------------------------

};

//
// AGS: 2x2 VHT-capable rate table
//
//2 TODO: 
//2 1. 5GHz only
//2 2. Not support 256QAM
//
UCHAR Ags2x2VhtRateTable[] = 
{
    //
    //      [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown] [downMCS] [upMCS3] [upMCS2] [upMCS1]
    //
    // Note: downMCS, upMCS3, upMCS2 and upMCS1 are zero-based array index.
    //
    // [Mode]*: 
    // bit0: STBC -STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~3: BW - BW_XXX <VHT card has the Rate tuning algorithm that change the Bandwidth.>
    // bit4~6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //

// Initial used item after association: the number of rate indexes, the initial MCS (index)
/* 0 */     0x15,   0x14,   0x00, 0,                                0,  0,      0,  0,  0,  0,  

// The Dara Rate of throughput extension for long distance by using lower data rate (VHT BW80 > BW40 > OFDM)
/* 1 */     0x00,   0x11,   0x00, 0, /* OFDM1x1 MCS 0, BW20 */      40, 101,    0,  0,  1,  1, // Reserved, not use.
/* 2 */     0x01,   0x11,   0x00, 0, /* OFDM1x1 MCS 0, BW20 */      40, 50,     0,  0,  2,  2, // Reserved, not use.
/* 3 */     0x02,   0x11,   0x00, 0, /* OFDM1x1 MCS 0, BW20 */      30, 101,    1,  0,  3,  3, 
/* 4 */     0x03,   0x45,   0x00, 0, /* VHT 1x1 MCS 0, BW40 */      30, 50,     2,  0,  4,  4, 

// The Data Rate of VHT 1 Spatial Stream
/* 5 */     0x04,   0x49,   0x00, 0, /* VHT 1x1 MCS 0, BW80 */      30, 50,     3,  0,  12, 5, 
/* 6 */     0x05,   0x49,   0x00, 1, /* VHT 1x1 MCS 1, BW80 */      20, 50,     4,  0,  13, 6, 
/* 7 */     0x06,   0x49,   0x00, 2, /* VHT 1x1 MCS 2, BW80 */      20, 50,     5,  0,  13, 7, 
/* 8 */     0x07,   0x49,   0x00, 3, /* VHT 1x1 MCS 3, BW80 */      15, 50,     6,  0,  14, 8, 
/* 9 */     0x08,   0x49,   0x00, 4, /* VHT 1x1 MCS 4, BW80 */      15, 30,     7,  0,  15, 9, 
/*10 */     0x09,   0x49,   0x00, 5, /* VHT 1x1 MCS 5, BW80 */      10, 25,     8,  0,  16, 10, 
/*11 */     0x0A,   0x49,   0x00, 6, /* VHT 1x1 MCS 6, BW80 */      8,  14,     9,  0,  16, 11, 
/*12 */     0x0B,   0x49,   0x00, 7, /* VHT 1x1 MCS 7, BW80 */      8,  14,     10, 0,  16, 11, 

// The Data Rate of VHT 2 Spatial Stream
/*13 */     0x0C,   0x48,   0x01, 0, /* VHT 2x2 MCS 0, BW80 */      30, 50,     4,  0,  13, 6, 
/*14 */     0x0D,   0x48,   0x01, 1, /* VHT 2x2 MCS 1, BW80 */      20, 50,     12, 0,  14, 8, 
/*15 */     0x0E,   0x48,   0x01, 2, /* VHT 2x2 MCS 2, BW80 */      20, 50,     13, 0,  15, 9, 
/*16 */     0x0F,   0x48,   0x01, 3, /* VHT 2x2 MCS 3, BW80 */      15, 30,     14, 0,  16, 10, 
/*17 */     0x10,   0x48,   0x01, 4, /* VHT 2x2 MCS 4, BW80 */      15, 30,     15, 0,  17, 16, 
/*18 */     0x11,   0x48,   0x01, 5, /* VHT 2x2 MCS 5, BW80 */      8,  20,     16, 0,  18, 17, 
/*19 */     0x12,   0x48,   0x01, 6, /* VHT 2x2 MCS 6, BW80 */      8,  18,     17, 0,  19, 18, 
/*20 */     0x13,   0x48,   0x01, 7, /* VHT 2x2 MCS 7, BW80 */      8,  25,     18, 0,  20, 19, 
/*21 */     0x14,   0x4A,   0x01, 7, /* VHT 2x2 MCS 7, BW80, SGI */ 8,  25,     19, 0,  20, 20, 


//---------------------------------------------------------------------------------------------------------------
//  CurrMCS             downMCS         upMCS3              upMCS2              upMCS1
//---------------------------------------------------------------------------------------------------------------
//  Reserved, not use.
//  Reserved, not use.
//  OFDM1x1 MCS 0 BW20      OFDM1x1 MCS 0       N/A                 VHT 1x1 MCS 0 BW40  VHT 1x1 MCS 0 BW40
//  VHT 1x1 MCS 0 BW40      VHT 1x1 MCS 0 BW20  N/A                 VHT 1x1 MCS 0 BW80  VHT 1x1 MCS 0 BW80

//  VHT 1x1 MCS 0 BW80      VHT 1x1 MCS 0 BW40  N/A                 VHT 2x2 MCS 0 BW80  VHT 1x1 MCS 1 BW80
//  VHT 1x1 MCS 1 BW80      VHT 1x1 MCS 0 BW80  N/A                 VHT 2x2 MCS 1 BW80  VHT 1x1 MCS 2 BW80
//  VHT 1x1 MCS 2 BW80      VHT 1x1 MCS 1 BW80  N/A                 VHT 2x2 MCS 1 BW80  VHT 1x1 MCS 3 BW80
//  VHT 1x1 MCS 3 BW80      VHT 1x1 MCS 2 BW80  N/A                 VHT 2x2 MCS 2 BW80  VHT 1x1 MCS 4 BW80
//  VHT 1x1 MCS 4 BW80      VHT 1x1 MCS 3 BW80  N/A                 VHT 2x2 MCS 3 BW80  VHT 1x1 MCS 5 BW80
//  VHT 1x1 MCS 5 BW80      VHT 1x1 MCS 4 BW80  N/A                 VHT 2x2 MCS 4 BW80  VHT 1x1 MCS 6 BW80
//  VHT 1x1 MCS 6 BW80      VHT 1x1 MCS 5 BW80  N/A                 VHT 2x2 MCS 4 BW80  VHT 1x1 MCS 7 BW80
//  VHT 1x1 MCS 7 BW80      VHT 1x1 MCS 6 BW80  N/A                 VHT 2x2 MCS 4 BW80  VHT 1x1 MCS 7 BW80

//  VHT 2x2 MCS 0 BW80      VHT 1x1 MCS 0 BW80  N/A                 VHT 2x2 MCS 1 BW80  VHT 1x1 MCS 2 BW80
//  VHT 2x2 MCS 1 BW80      VHT 2x2 MCS 0 BW80  N/A                 VHT 2x2 MCS 2 BW80  VHT 1x1 MCS 4 BW80
//  VHT 2x2 MCS 2 BW80      VHT 2x2 MCS 1 BW80  N/A                 VHT 2x2 MCS 3 BW80  VHT 1x1 MCS 5 BW80
//  VHT 2x2 MCS 3 BW80      VHT 2x2 MCS 2 BW80  N/A                 VHT 2x2 MCS 4 BW80  VHT 1x1 MCS 6 BW80
//  VHT 2x2 MCS 4 BW80      VHT 2x2 MCS 3 BW80  N/A                 VHT 2x2 MCS 5 BW80  VHT 2x2 MCS 4 BW80
//  VHT 2x2 MCS 5 BW80      VHT 2x2 MCS 4 BW80  N/A                 VHT 2x2 MCS 6 BW80  VHT 2x2 MCS 5 BW80
//  VHT 2x2 MCS 6 BW80      VHT 2x2 MCS 5 BW80  N/A                 VHT 2x2 MCS 7 BW80  VHT 2x2 MCS 6 BW80
//  VHT 2x2 MCS 7 BW80      VHT 2x2 MCS 6 BW80  N/A                 VHT 2x2 MCS 7,SGI   VHT 2x2 MCS 7 BW80
//  VHT 2x2 MCS 7 BW80,SGI  VHT 2x2 MCS 7 BW80  N/A                 VHT 2x2 MCS 7,SGI   VHT 2x2 MCS 7,SGI
//---------------------------------------------------------------------------------------------------------------

};

//
// RSSI Offset table for Ags rate tuning
//
UCHAR AgsRssiOffsetTable[3][4] = 
{
    // [i][] MAX System spatial stream capability: 1*1, 2*2, 3*3
    // [i][] MAX System Bandwidth: 20, 40, 80, 160
    // ----------------------
    {0,    2,    4,    6},    // For VHT 1*1: BW20, BW40, BW80, BW160
    {2,    4,    6,    8},    // For VHT 2*2: BW20, BW40, BW80, BW160
    {4,    6,    8,    10}    // For VHT 3*3: BW20, BW40, BW80, BW160               
};

//
// The configuration type of updating MAC register (MAC_UPDATE_CFG_TYPE_XXX)
//
#define MAC_UPDATE_CFG_TYPE_DIRECT 0 // Update directly
#define MAC_UPDATE_CFG_TYPE_CHANNEL_BOUNDING 1 // Base on channel bounding
#define MAC_UPDATE_CFG_TYPE_BW 2 // Base on bandwidth
#define MAC_UPDATE_CFG_TYPE_PHY_MODE 3 // Base on PHY mode
#define MAC_UPDATE_CFG_TYPE_2DOT4_GHZ_ALL_BW 4 // For 2.4GHz channel frequency, all bandwidth
#define MAC_UPDATE_CFG_TYPE_2DOT4_GHZ_BW20 5 // For 2.4GHz channel frequency, BW20
#define MAC_UPDATE_CFG_TYPE_2DOT4_GHZ_BW40 6 // For 2.4GHz channel frequency, BW40
#define MAC_UPDATE_CFG_TYPE_5_GHZ_ALL_BW 7 // For 5GHz channel frequency, all bandwidth
#define MAC_UPDATE_CFG_TYPE_5_GHZ_BW20 8 // For 5GHz channel frequency, BW20
#define MAC_UPDATE_CFG_TYPE_5_GHZ_BW40 9 // For 5GHz channel frequency, BW40
#define MAC_UPDATE_CFG_TYPE_5_GHZ_BW80 10 // For 5GHz channel frequency, BW80

//
// MAC update register value
//
// The relationship between MAC_UPDATE_CFG_TYPE_XXX and structure fields: 
//
// MAC_UPDATE_CFG_TYPE_DIRECT: UpdateCfgType, Register and Value
// MAC_UPDATE_CFG_TYPE_CHANNEL_BOUNDING: UpdateCfgType, Register, Value, StartChannel and EndChannel
// MAC_UPDATE_CFG_TYPE_BW: UpdateCfgType, Register, Value and BW
// MAC_UPDATE_CFG_TYPE_PHY_MODE: UpdateCfgType, Register, Value and PhyMode
// MAC_UPDATE_CFG_TYPE_2DOT4_GHZ_ALL_BW: UpdateCfgType, Register and Value
// MAC_UPDATE_CFG_TYPE_2DOT4_GHZ_BW20: UpdateCfgType, Register and Value
// MAC_UPDATE_CFG_TYPE_2DOT4_GHZ_BW40: UpdateCfgType, Register and Value
// MAC_UPDATE_CFG_TYPE_5_GHZ_ALL_BW: UpdateCfgType, Register and Value
// MAC_UPDATE_CFG_TYPE_5_GHZ_BW20: UpdateCfgType, Register and Value
// MAC_UPDATE_CFG_TYPE_5_GHZ_BW40: UpdateCfgType, Register and Value
// MAC_UPDATE_CFG_TYPE_5_GHZ_BW80: UpdateCfgType, Register and Value
//
typedef struct _MAC_UPDATE_REGISTER_VALUE
{
    UCHAR UpdateCfgType; // The configuration type of updating MAC register (MAC_UPDATE_CFG_TYPE_XXX)
    ULONG Register; // MAC register
    ULONG Value; // MAC register value

    UCHAR StartChannel; // Start channel
    UCHAR EndChannel; // End channel
    UCHAR BW; // Bandwidth (BW_XX)
    UCHAR PhyMode; // PHY mode (MODE_XXX)
} MAC_UPDATE_REGISTER_VALUE, *PMAC_UPDATE_REGISTER_VALUE;

//
// Number of rate entries of VHT rate table
//
#define NUM_RATE_ENTRIES_OF_VHT_RATE_TABLE          (sizeof(VhtRateTable) / sizeof(USHORT))

//
// The BW/Nss/GI segments of VHT rate table
//
#define BW_SEG_OF_VHT_RATE_TABLE    (40)
#define NSS_SEG_OF_VHT_RATE_TABLE   (10)
#define GI_SEG_OF_VHT_RATE_TABLE    (160)

//////////////////////////////////// End of File ////////////////////////////////////

