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
    mlme.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    John Chang  2004-08-25      Modify from RT2500 code base
    John Chang  2004-09-06      modified for RT2600
*/

#include "MtConfig.h"
#include <stdarg.h>

UCHAR RateSwitchTable[] = 
{
    //
    // [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown]
    //
    // [Mode]*: 
    // bit0: STBC - STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~bit3: BW - BW_XXX
    // bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //
    0x11,   0x0f,   0x00,   0,  0,  0, // Initial used item after association: the number of rate indexes, the initial mcs
    0x00,   0x00,   0x00,   0,  40, 101, 
    0x01,   0x00,   0x00,   1,  40, 50, 
    0x02,   0x00,   0x00,   2,  35, 45, 
    0x03,   0x00,   0x00,   3,  20, 45, 
    0x04,   0x21,   0x00,   0,  30, 50, 
    0x05,   0x21,   0x00,   1,  20, 50, 
    0x06,   0x21,   0x00,   2,  20, 50, 
    0x07,   0x21,   0x00,   3,  15, 50, 
    0x08,   0x21,   0x00,   4,  15, 30, 
    0x09,   0x21,   0x00,   5,  10, 25, 
    0x0a,   0x21,   0x00,   6,  8,  25, 
    0x0b,   0x21,   0x00,   7,  8,  25, 
    0x0c,   0x20,   0x00,   12, 15, 20, 
    0x0d,   0x20,   0x00,   13, 8,  20, 
    0x0e,   0x20,   0x00,   14, 8,  20, 
    0x0f,   0x20,   0x00,   15, 8,  25, 
    0x10,   0x22,   0x00,   15, 8,  25, 
    0x11,   0x00,   0x00,   0,  0,  0, 
    0x12,   0x00,   0x00,   0,  0,  0, 
    0x13,   0x00,   0x00,   0,  0,  0, 
    0x14,   0x00,   0x00,   0,  0,  0, 
    0x15,   0x00,   0x00,   0,  0,  0, 
    0x16,   0x00,   0x00,   0,  0,  0, 
    0x17,   0x00,   0x00,   0,  0,  0, 
    0x18,   0x00,   0x00,   0,  0,  0, 
    0x19,   0x00,   0x00,   0,  0,  0, 
    0x1a,   0x00,   0x00,   0,  0,  0, 
    0x1b,   0x00,   0x00,   0,  0,  0, 
    0x1c,   0x00,   0x00,   0,  0,  0, 
    0x1d,   0x00,   0x00,   0,  0,  0, 
    0x1e,   0x00,   0x00,   0,  0,  0, 
    0x1f,   0x00,   0x00,   0,  0,  0, 
};

UCHAR RateSwitchTable11B[] = 
{
    //
    // [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown]
    //
    // [Mode]*: 
    // bit0: STBC - STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~bit3: BW - BW_XXX
    // bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //
    0x04,   0x03,   0x00,   0,  0,  0, // Initial used item after association: the number of rate indexes, the initial mcs
    0x00,   0x00,   0x00,   0,  40, 101, 
    0x01,   0x00,   0x00,   1,  40, 50, 
    0x02,   0x00,   0x00,   2,  35, 45, 
    0x03,   0x00,   0x00,   3,  20, 45, 
};

UCHAR RateSwitchTable11BG[] = 
{
    //
    // [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown]
    //
    // [Mode]*: 
    // bit0: STBC - STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~bit3: BW - BW_XXX
    // bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //
    0x0a,   0x09,   0x00,   0,  0,  0, // Initial used item after association: the number of rate indexes, the initial mcs
    0x00,   0x00,   0x00,   0,  40, 101, 
    0x01,   0x00,   0x00,   1,  40, 50, 
    0x02,   0x00,   0x00,   2,  35, 45, 
    0x03,   0x00,   0x00,   3,  20, 45, 
    0x04,   0x10,   0x00,   2,  20, 35, 
    0x05,   0x10,   0x00,   3,  16, 35, 
    0x06,   0x10,   0x00,   4,  10, 25, 
    0x07,   0x10,   0x00,   5,  16, 25, 
    0x08,   0x10,   0x00,   6,  10, 25, 
    0x09,   0x10,   0x00,   7,  10, 13, 
};

UCHAR RateSwitchTable11G[] = 
{
    //
    // [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown]
    //
    // [Mode]*: 
    // bit0: STBC - STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~bit3: BW - BW_XXX
    // bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //
    0x08,   0x07,   0x00,   0,  0,  0, // Initial used item after association: the number of rate indexes, the initial mcs
    0x00,   0x10,   0x00,   0,  20, 101, 
    0x01,   0x10,   0x00,   1,  20, 35, 
    0x02,   0x10,   0x00,   2,  20, 35, 
    0x03,   0x10,   0x00,   3,  16, 35, 
    0x04,   0x10,   0x00,   4,  10, 25, 
    0x05,   0x10,   0x00,   5,  16, 25, 
    0x06,   0x10,   0x00,   6,  10, 25, 
    0x07,   0x10,   0x00,   7,  10, 13, 
};

UCHAR RateSwitchTable11BGAll[] = 
{
    //
    // [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown]
    //
    // [Mode]*: 
    // bit0: STBC - STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~bit3: BW - BW_XXX
    // bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //
    0x0c,   0x0b,   0x00,   0,  0,  0, // Initial used item after association: the number of rate indexes, the initial mcs
    0x00,   0x00,   0x00,   0,  40, 101, 
    0x01,   0x00,   0x00,   1,  40, 50, 
    0x02,   0x00,   0x00,   2,  35, 45, 
    0x03,   0x00,   0x00,   3,  20, 45, 
    0x04,   0x10,   0x00,   0,  20, 35, 
    0x05,   0x10,   0x00,   1,  20, 35, 
    0x06,   0x10,   0x00,   2,  20, 35, 
    0x07,   0x10,   0x00,   3,  16, 35, 
    0x08,   0x10,   0x00,   4,  10, 25, 
    0x09,   0x10,   0x00,   5,  16, 25, 
    0x0a,   0x10,   0x00,   6,  10, 25, 
    0x0b,   0x10,   0x00,   7,  10, 13, 
};

UCHAR RateSwitchTable11N1S[] = 
{
    //
    // [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown]
    //
    // [Mode]*: 
    // bit0: STBC - STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~bit3: BW - BW_XXX
    // bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //
    0x0c,   0x0a,   0x00,   0,  0,  0, // Initial used item after association: the number of rate indexes, the initial mcs
    0x00,   0x00,   0x00,   0,  40, 101, 
    0x01,   0x00,   0x00,   1,  40, 50, 
    0x02,   0x00,   0x00,   2,  25, 45, 
    0x03,   0x21,   0x00,   0,  20, 35, 
    0x04,   0x21,   0x00,   1,  20, 35, 
    0x05,   0x21,   0x00,   2,  20, 35, 
    0x06,   0x21,   0x00,   3,  15, 35, 
    0x07,   0x21,   0x00,   4,  15, 30, 
    0x08,   0x21,   0x00,   5,  10, 25, 
    0x09,   0x21,   0x00,   6,  8,  14, 
    0x0a,   0x21,   0x00,   7,  8,  14, 
    0x0b,   0x23,   0x00,   7,  8,  14, 
};

UCHAR RateSwitchTable11N1SForABand[] = 
{
    //
    // [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown]
    //
    // [Mode]*: 
    // bit0: STBC - STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~bit3: BW - BW_XXX
    // bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //
    0x09,   0x07,   0x00,   0,  0,  0, // Initial used item after association: the number of rate indexes, the initial mcs
    0x00,   0x21,   0x00,   0,  30, 101, 
    0x01,   0x21,   0x00,   1,  20, 50, 
    0x02,   0x21,   0x00,   2,  20, 35, 
    0x03,   0x21,   0x00,   3,  15, 35, 
    0x04,   0x21,   0x00,   4,  15, 30, 
    0x05,   0x21,   0x00,   5,  10, 25, 
    0x06,   0x21,   0x00,   6,  8,  14, 
    0x07,   0x21,   0x00,   7,  8,  14, 
    0x08,   0x23,   0x00,   7,  8,  14, 
};

UCHAR RateSwitchTable11N2S[] = 
{
    //
    // [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown]
    //
    // [Mode]*: 
    // bit0: STBC - STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~bit3: BW - BW_XXX
    // bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //
    0x0e,   0x0c,   0x00,   0,  0,  0, // Initial used item after association: the number of rate indexes, the initial mcs
    0x00,   0x00,   0x00,   0,  40, 101, 
    0x01,   0x00,   0x00,   1,  40, 50, 
    0x02,   0x00,   0x00,   2,  25, 45, 
    0x03,   0x21,   0x00,   0,  20, 35, 
    0x04,   0x21,   0x00,   1,  20, 35, 
    0x05,   0x21,   0x00,   2,  20, 35, 
    0x06,   0x21,   0x00,   3,  15, 35, 
    0x07,   0x21,   0x00,   4,  15, 30, 
    0x08,   0x20,   0x00,   11, 15, 30, 
    0x09,   0x20,   0x00,   12, 15, 30, 
    0x0a,   0x20,   0x00,   13, 8,  20, 
    0x0b,   0x20,   0x00,   14, 8,  20, 
    0x0c,   0x20,   0x00,   15, 8,  25, 
    0x0d,   0x22,   0x00,   15, 8,  15, 
};

UCHAR RateSwitchTable11N2SForABand[] = 
{
    //
    // [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown]
    //
    // [Mode]*: 
    // bit0: STBC - STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~bit3: BW - BW_XXX
    // bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //
    0x0b,   0x09,   0x00,   0,  0,  0, // Initial used item after association: the number of rate indexes, the initial mcs
    0x00,   0x21,   0x00,   0,  30, 101, 
    0x01,   0x21,   0x00,   1,  20, 50, 
    0x02,   0x21,   0x00,   2,  20, 50, 
    0x03,   0x21,   0x00,   3,  15, 50, 
    0x04,   0x21,   0x00,   4,  15, 30, 
    0x05,   0x21,   0x00,   5,  15, 30, 
    0x06,   0x20,   0x00,   12, 15, 30, 
    0x07,   0x20,   0x00,   13, 8,  20, 
    0x08,   0x20,   0x00,   14, 8,  20, 
    0x09,   0x20,   0x00,   15, 8,  25, 
    0x0a,   0x22,   0x00,   15, 8,  25, 
};

UCHAR RateSwitchTable11BGN1S[] = 
{
    //
    // [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown]
    //
    // [Mode]*: 
    // bit0: STBC - STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~bit3: BW - BW_XXX
    // bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //
    0x0c,   0x0a,   0x00,   0,  0,  0, // Initial used item after association: the number of rate indexes, the initial mcs
    0x00,   0x00,   0x00,   0,  40, 101, 
    0x01,   0x00,   0x00,   1,  40, 50, 
    0x02,   0x00,   0x00,   2,  25, 45, 
    0x03,   0x21,   0x00,   0,  20, 35, 
    0x04,   0x21,   0x00,   1,  20, 35, 
    0x05,   0x21,   0x00,   2,  20, 35, 
    0x06,   0x21,   0x00,   3,  15, 35, 
    0x07,   0x21,   0x00,   4,  15, 30, 
    0x08,   0x21,   0x00,   5,  10, 25, 
    0x09,   0x21,   0x00,   6,  8,  14, 
    0x0a,   0x21,   0x00,   7,  8,  14, 
    0x0b,   0x23,   0x00,   7,  8,  14, 
};

UCHAR RateSwitchTable11BGN1SForABand[] = 
{
    //
    // [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown]
    //
    // [Mode]*: 
    // bit0: STBC - STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~bit3: BW - BW_XXX
    // bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //
    0x09,   0x07,   0x00,   0,  0,  0, // Initial used item after association: the number of rate indexes, the initial mcs
    0x00,   0x21,   0x00,   0,  30, 101,
    0x01,   0x21,   0x00,   1,  20, 50, 
    0x02,   0x21,   0x00,   2,  20, 35, 
    0x03,   0x21,   0x00,   3,  15, 35, 
    0x04,   0x21,   0x00,   4,  15, 30, 
    0x05,   0x21,   0x00,   5,  10, 25, 
    0x06,   0x21,   0x00,   6,  8,  14, 
    0x07,   0x21,   0x00,   7,  8,  14, 
    0x08,   0x23,   0x00,   7,  8,  14, 
};

UCHAR RateSwitchTable11BGN2S[] = 
{
    //
    // [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown]
    //
    // [Mode]*: 
    // bit0: STBC - STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~bit3: BW - BW_XXX
    // bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //
    0x0e,   0x0c,   0x00,   0,  0,  0, // Initial used item after association: the number of rate indexes, the initial mcs
    0x00,   0x00,   0x00,   0,  40, 101, 
    0x01,   0x00,   0x00,   1,  40, 50, 
    0x02,   0x00,   0x00,   2,  25, 45, 
    0x03,   0x21,   0x00,   0,  20, 35, 
    0x04,   0x21,   0x00,   1,  20, 35, 
    0x05,   0x21,   0x00,   2,  20, 35, 
    0x06,   0x21,   0x00,   3,  15, 35, 
    0x07,   0x21,   0x00,   4,  15, 30, 
    0x08,   0x20,   0x00,   11, 15, 30, 
    0x09,   0x20,   0x00,   12, 15, 30, 
    0x0a,   0x20,   0x00,   13, 8,  20, 
    0x0b,   0x20,   0x00,   14, 8,  20, 
    0x0c,   0x20,   0x00,   15, 8,  25, 
    0x0d,   0x22,   0x00,   15, 8,  15, 
};

UCHAR RateSwitchTable11BGN2SForABand[] = 
{
    //
    // [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown]
    //
    // [Mode]*: 
    // bit0: STBC - STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~bit3: BW - BW_XXX
    // bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //
    0x0b,   0x09,   0x00,   0,  0,  0, // Initial used item after association: the number of rate indexes, the initial mcs
    0x00,   0x21,   0x00,   0,  30, 101, 
    0x01,   0x21,   0x00,   1,  20, 50, 
    0x02,   0x21,   0x00,   2,  20, 50, 
    0x03,   0x21,   0x00,   3,  15, 50, 
    0x04,   0x21,   0x00,   4,  15, 30, 
    0x05,   0x21,   0x00,   5,  15, 30, 
    0x06,   0x20,   0x00,   12, 15, 30, 
    0x07,   0x20,   0x00,   13, 8,  20, 
    0x08,   0x20,   0x00,   14, 8,  20, 
    0x09,   0x20,   0x00,   15, 8,  25, 
    0x0a,   0x22,   0x00,   15, 8,  25, 
};

//
// AGS: 1x1 HT-capable rate table
//
UCHAR AGS1x1HTRateTable[] = 
{
    // Add CCK rate
    //
    // [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown] [downMCS] [upMCS3] [upMCS2] [upMCS1]
    //
    // [Mode]*: 
    // bit0: STBC -STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~3: BW - BW_XXX
    // bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //
    0x0c,   0x0b,   0x00, 0,    0,  0,      0,  0,  0,  0,  // Initial used item after association: the number of rate indexes, the initial mcs
    0x00,   0x00,   0x00, 0,    40, 101,        0,  0,  0,  1,  // CCK MCS 0
    0x01,   0x00,   0x00, 1,    40, 50,     0,  0,  0,  2,  // CCK MCS 1
    0x02,   0x00,   0x00, 2,    25, 45,     1,  0,  0,  3,  // CCK MCS 5.5
    0x03,   0x21,   0x00, 0,    25, 40,     2,  0,  0,  4,  // HT MCS 0
    0x04,   0x21,   0x00, 1,    20, 40,     3,  0,  0,  5,  // HT MCS 1
    0x05,   0x21,   0x00, 2,    20, 30,     4,  0,  0,  6,  // HT MCS 2
    0x06,   0x21,   0x00, 3,    15, 30,     5,  0,  0,  7,  // HT MCS 3
    0x07,   0x21,   0x00, 4,    15, 30,     6,  0,  0,  8,  // HT MCS 4
    0x08,   0x21,   0x00, 5,    10, 25,     7,  0,  0,  9,  // HT MCS 5
    0x09,   0x21,   0x00, 6,    8,  14,     8,  0,  0,  10, // HT MCS 6
    0x0a,   0x21,   0x00, 7,    8,  14,     9,  0,  0,  11, // HT MCS 7
    0x0b,   0x23,   0x00, 7,    8,  14,     10, 0,  0,  11, // HT MCS 7 + Short GI
};

//
// AGS 1x1 HT rate table over 5G
//
UCHAR AGS1x1HTRateTable5G[] = 
{
    // Add CCK rate
    //
    // [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown] [downMCS] [upMCS3] [upMCS2] [upMCS1]
    //
    // [Mode]*: 
    // bit0: STBC -STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~3: BW - BW_XXX
    // bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //
    0x09,   0x08,   0x00, 0,    0,  0,      0,  0,  0,  0,  // Initial used item after association: the number of rate indexes, the initial mcs
    0x00,   0x21,   0x00, 0,    30, 101,    0,  0,  0,  1,  // MCS 0
    0x01,   0x21,   0x00, 1,    20, 40,     0,  0,  0,  2,  // MCS 1
    0x02,   0x21,   0x00, 2,    20, 30,     1,  0,  0,  3,  // MCS 2
    0x03,   0x21,   0x00, 3,    15, 30,     2,  0,  0,  4,  // MCS 3
    0x04,   0x21,   0x00, 4,    15, 30,     3,  0,  0,  5,  // MCS 4
    0x05,   0x21,   0x00, 5,    10, 25,     4,  0,  0,  6,  // MCS 5
    0x06,   0x21,   0x00, 6,    8,  14,     5,  0,  0,  7, // MCS 6
    0x07,   0x21,   0x00, 7,    8,  14,     6,  0,  0,  8, // MCS 7
    0x08,   0x23,   0x00, 7,    8,  14,     7,  0,  0,  8, // MCS 7 + Short GI
};

/*
UCHAR AGS1x1HTRateTable[] = 
{
    //
    // [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown] [downMCS] [upMCS3] [upMCS2] [upMCS1]
    //
    // [Mode]*: 
    // bit0: STBC -STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~3: BW - BW_XXX
    // bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //
    0x09,   0x08,   0x00, 0,    0,  0,      0,  0,  0,  0,  // Initial used item after association: the number of rate indexes, the initial mcs
    0x00,   0x21,   0x00, 0,    30, 101,    0,  16, 8,  1,  // MCS 0
    0x01,   0x21,   0x00, 1,    20, 50,     0,  16, 9,  2,  // MCS 1
    0x02,   0x21,   0x00, 2,    20, 30,     1,  17, 9,  3,  // MCS 2
    0x03,   0x21,   0x00, 3,    15, 30,     2,  17, 10, 4,  // MCS 3
    0x04,   0x21,   0x00, 4,    15, 30,     3,  18, 11, 5,  // MCS 4
    0x05,   0x21,   0x00, 5,    10, 25,     4,  18, 12, 6,  // MCS 5
    0x06,   0x21,   0x00, 6,    8,  14,     5,  19, 12, 7,  // MCS 6
    0x07,   0x21,   0x00, 7,    8,  14,     6,  19, 12, 8,  // MCS 7
    0x08,   0x23,   0x00, 7,    8,  14,     7,  19, 12, 8,  // MCS 7 + Short GI
};
*/

//
// AGS: 2x2 HT-capable rate table
//
UCHAR AGS2x2HTRateTable[] = 
{
    //
    // [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown] [downMCS] [upMCS3] [upMCS2] [upMCS1]
    //
    // [Mode]*: 
    // bit0: STBC -STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~3: BW - BW_XXX
    // bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //
    0x11,   0x10,   0x00, 0,    0,  0,      0,  0,  0,  0,  // Initial used item after association: the number of rate indexes, the initial mcs
    0x00,   0x21,   0x00, 0,    30, 101,    0,  16, 8,  1,  // MCS 0
    0x01,   0x21,   0x00, 1,    20, 50,     0,  16, 9,  2,  // MCS 1
    0x02,   0x21,   0x00, 2,    20, 30,     1,  17, 9,  3,  // MCS 2
    0x03,   0x21,   0x00, 3,    15, 30,     2,  17, 10, 4,  // MCS 3
    0x04,   0x21,   0x00, 4,    15, 30,     3,  18, 11, 5,  // MCS 4
    0x05,   0x21,   0x00, 5,    10, 25,     4,  18, 12, 6,  // MCS 5
    0x06,   0x21,   0x00, 6,    8,  14,     5,  19, 12, 7,  // MCS 6
    0x07,   0x21,   0x00, 7,    8,  14,     6,  19, 12, 7,  // MCS 7
    0x08,   0x20,   0x00, 8,    30, 50,     0,  16, 9,  2,  // MCS 8
    0x09,   0x20,   0x00, 9,    20, 50,     8,  17, 10, 4,  // MCS 9
    0x0A,   0x20,   0x00, 10,   20, 50,     9,  18, 11, 5,  // MCS 10
    0x0B,   0x20,   0x00, 11,   15, 30,     10, 18, 12, 6,  // MCS 11
    0x0C,   0x20,   0x00, 12,   15, 30,     11, 20, 13, 12, // MCS 12
    0x0D,   0x20,   0x00, 13,   8,  20,     12, 20, 14, 13, // MCS 13
    0x0E,   0x20,   0x00, 14,   8,  18,     13, 21, 15, 14, // MCS 14
    0x0F,   0x20,   0x00, 15,   8,  25,     14, 21, 16, 15, // MCS 15
    0x10,   0x22,   0x00, 15,   8,  25,     15, 21, 16, 16, // MCS 15 + Short GI
};

//
// AGS: 3x3 HT-capable rate table
//
UCHAR AGS3x3HTRateTable[] = 
{
    //
    // [Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown] [downMCS] [upMCS3] [upMCS2] [upMCS1]
    //
    // [Mode]*: 
    // bit0: STBC -STBC_XXX
    // bit1: Short GI - GI_XXX
    // bit2~3: BW - BW_XXX
    // bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
    // bit7: Reserved
    //
    // [Mode]**
    // bit0~1: Nss - NSS_XXX (VHT only)
    // bit2~7: Reserved
    //
    0x19,   0x18,   0x00, 0,    0,  0,      0,  0,  0,  0,  // Initial used item after association: the number of rate indexes, the initial mcs
    0x00,   0x21,   0x00, 0,    30, 101,    0,  16, 8,  1,  // MCS 0
    0x01,   0x21,   0x00, 1,    20, 50,     0,  16, 9,  2,  // MCS 1
    0x02,   0x21,   0x00, 2,    20, 50,     1,  17, 9,  3,  // MCS 2
    0x03,   0x21,   0x00, 3,    15, 50,     2,  17, 10, 4,  // MCS 3
    0x04,   0x21,   0x00, 4,    15, 30,     3,  18, 11, 5,  // MCS 4
    0x05,   0x21,   0x00, 5,    10, 25,     4,  18, 12, 6,  // MCS 5
    0x06,   0x21,   0x00, 6,    8,  14,     5,  19, 12, 7,  // MCS 6
    0x07,   0x21,   0x00, 7,    8,  14,     6,  19, 12, 7,  // MCS 7
    0x08,   0x20,   0x00, 8,    30, 50,     0,  16, 9,  2,  // MCS 8
    0x09,   0x20,   0x00, 9,    20, 50,     8,  17, 10, 4,  // MCS 9
    0x0A,   0x20,   0x00, 10,   20, 50,     9,  18, 11, 5,  // MCS 10
    0x0B,   0x20,   0x00, 11,   15, 30,     10, 18, 12, 6,  // MCS 11
    0x0C,   0x20,   0x00, 12,   15, 30,     11, 20, 13, 12, // MCS 12
    0x0D,   0x20,   0x00, 13,   8,  20,     12, 20, 14, 13, // MCS 13
    0x0E,   0x20,   0x00, 14,   8,  18,     13, 21, 15, 14, // MCS 14
    0x0F,   0x20,   0x00, 15,   8,  14,     14, 21, 15, 15, // MCS 15
    0x10,   0x20,   0x00, 16,   30, 50,     8,  17, 9,  3,  // MCS 16
    0x11,   0x20,   0x00, 17,   20, 50,     16, 18, 11, 5,  // MCS 17
    0x12,   0x20,   0x00, 18,   20, 50,     17, 19, 12, 7,  // MCS 18
    0x13,   0x20,   0x00, 19,   15, 30,     18, 20, 13, 19, // MCS 19
    0x14,   0x20,   0x00, 20,   15, 30,     19, 21, 15, 20, // MCS 20
    0x15,   0x20,   0x00, 21,   8,  20,     20, 22, 21, 21, // MCS 21
    0x16,   0x20,   0x00, 22,   8,  20,     21, 23, 22, 22, // MCS 22
    0x17,   0x20,   0x00, 23,   6,  18,     22, 24, 23, 23, // MCS 23
    0x18,   0x22,   0x00, 23,   6,  14,     23, 24, 24, 24, // MCS 23 + Short GI
};

PUCHAR ReasonString[] = {
        /* 0  */     "Reserved",
        /* 1  */     "Unspecified Reason",
        /* 2  */     "Previous Auth no longer valid",
        /* 3  */     "STA is leaving / has left",
        /* 4  */     "DIS-ASSOC due to inactivity",
        /* 5  */     "AP unable to hanle all associations",
        /* 6  */     "class 2 error",
        /* 7  */     "class 3 error",
        /* 8  */     "STA is leaving / has left",
        /* 9  */     "require auth before assoc/re-assoc",
        /* 10 */     "Reserved",
        /* 11 */     "Reserved",
        /* 12 */     "Reserved",
        /* 13 */     "invalid IE",
        /* 14 */     "MIC error",
        /* 15 */     "4-way handshake timeout",
        /* 16 */     "2-way (group key) handshake timeout",
        /* 17 */     "4-way handshake IE diff among AssosReq/Rsp/Beacon",
        /* 18 */
};

extern UCHAR     OfdmRateToRxwiMCS[];
// since RT61 has better RX sensibility, we have to limit TX ACK rate not to exceed our normal data TX rate.
// otherwise the WLAN peer may not be able to receive the ACK thus downgrade its data TX rate
ULONG BasicRateMask[12]            = {0xfffff001 /* 1-Mbps */, 0xfffff003 /* 2 Mbps */, 0xfffff007 /* 5.5 */, 0xfffff00f /* 11 */,
                                      0xfffff01f /* 6 */     , 0xfffff03f /* 9 */     , 0xfffff07f /* 12 */ , 0xfffff0ff /* 18 */,
                                      0xfffff1ff /* 24 */    , 0xfffff3ff /* 36 */    , 0xfffff7ff /* 48 */ , 0xffffffff /* 54 */};

UCHAR MULTICAST_ADDR[MAC_ADDR_LEN] = {0x1,  0x00, 0x00, 0x00, 0x00, 0x00};
UCHAR BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// e.g. RssiSafeLevelForTxRate[RATE_36]" means if the current RSSI is greater than 
//      this value, then it's quaranteed capable of operating in 36 mbps TX rate in 
//      clean environment.
//                                TxRate: 1   2   5.5   11   6    9    12   18   24   36   48   54   72  100
CHAR RssiSafeLevelForTxRate[] ={  -92, -91, -90, -87, -88, -86, -85, -83, -81, -78, -72, -71, -40, -40 };

                                  //  1      2       5.5      11  
UCHAR Phy11BNextRateDownward[] = {RATE_1, RATE_1,   RATE_2,  RATE_5_5};
UCHAR Phy11BNextRateUpward[]   = {RATE_2, RATE_5_5, RATE_11, RATE_11};

                                  //  1      2       5.5      11        6        9        12      18       24       36       48       54
UCHAR Phy11BGNextRateDownward[]= {RATE_1, RATE_1,   RATE_2,  RATE_5_5,RATE_11,  RATE_11, RATE_11, RATE_12, RATE_18, RATE_24, RATE_36, RATE_48};
UCHAR Phy11BGNextRateUpward[]  = {RATE_2, RATE_5_5, RATE_11, RATE_12, RATE_11,  RATE_12, RATE_18, RATE_24, RATE_36, RATE_48, RATE_54, RATE_54};

                                  //  1      2       5.5      11        6        9        12      18       24       36       48       54
UCHAR Phy11ANextRateDownward[] = {RATE_6, RATE_6,   RATE_6,  RATE_6,  RATE_6,   RATE_6,  RATE_9,  RATE_12, RATE_18, RATE_24, RATE_36, RATE_48};
UCHAR Phy11ANextRateUpward[]   = {RATE_9, RATE_9,   RATE_9,  RATE_9,  RATE_9,   RATE_12, RATE_18, RATE_24, RATE_36, RATE_48, RATE_54, RATE_54};

//                       RATE_1,  2, 5.5, 11,  6,  9, 12, 18, 24, 36, 48, 54
USHORT RateUpPER[]   = {  40,  40,  35, 20, 20, 20, 20, 16, 10, 16, 10,  6 }; // in percentage
USHORT RateDownPER[] = {  50,  50,  45, 45, 35, 35, 35, 35, 25, 25, 25, 13 }; // in percentage

UCHAR  RateIdToMbps[]    = { 1, 2, 5, 11, 6, 9, 12, 18, 24, 36, 48, 54, 72, 100};
USHORT RateIdTo500Kbps[] = { 2, 4, 11, 22, 12, 18, 24, 36, 48, 72, 96, 108, 144, 200};

UCHAR  SsidIe    = IE_SSID;
UCHAR  SupRateIe = IE_SUPP_RATES;
UCHAR  ExtRateIe = IE_EXT_SUPP_RATES;
UCHAR  HtCapIe = IE_HT_CAP;
UCHAR  AddHtInfoIe = IE_ADD_HT;
UCHAR  NewExtChanIe = IE_NEW_EXT_CHA_OFFSET;
UCHAR  ExtHtCapIe = IE_EXT_CAPABILITY;
UCHAR  ErpIe     = IE_ERP;
UCHAR  DsIe      = IE_DS_PARM;
UCHAR  TimIe     = IE_TIM;
UCHAR  WpaIe     = IE_WPA;
UCHAR  Wpa2Ie    = IE_WPA2;
UCHAR  IbssIe    = IE_IBSS_PARM;
UCHAR  BssIntRepIe = IE_2040_BSS_INTOLERANT_REPORT;
UCHAR  BssCoexistIe = IE_2040_BSS_COEXIST;

UCHAR  VhtCapIe                = IE_VHT_CAP;
UCHAR  VhtOpIe                 = IE_VHT_OP;
UCHAR  VhtExtBssLoadIe         = IE_VHT_EXT_BSS_LOAD;
UCHAR  VhtWideBwChSwitchIe     = IE_VHT_WIDE_BW_CHANNEL_SWITCH;
UCHAR  VhtTxPowerIe            = IE_VHT_TX_POWER;
UCHAR  VhtExtPowerConstraintIe = IE_VHT_EXT_POWER_CONSTRAINT;
UCHAR  VhtAidIe                = IE_VHT_AID;
UCHAR  VhtQuietChannel         = IE_VHT_QUIET_CHANNEL;
UCHAR VhtOperatingModeNotification = IE_VHT_OPERATING_MODE_NOTIFICATION;

extern UCHAR    WPA_OUI[];

UCHAR   SES_OUI[] = {0x00, 0x90, 0x4c};

UCHAR   ZeroSsid[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

RTMP_RF_REGS RF2850RegTable[] = {
//      ch   R1          R2          R3(TX0~4=0) R4
        {1,  0x98402ecc, 0x984c0786, 0x9816b455, 0x9800510b},
        {2,  0x98402ecc, 0x984c0786, 0x98168a55, 0x9800519f},
        {3,  0x98402ecc, 0x984c078a, 0x98168a55, 0x9800518b},
        {4,  0x98402ecc, 0x984c078a, 0x98168a55, 0x9800519f},
        {5,  0x98402ecc, 0x984c078e, 0x98168a55, 0x9800518b},
        {6,  0x98402ecc, 0x984c078e, 0x98168a55, 0x9800519f},
        {7,  0x98402ecc, 0x984c0792, 0x98168a55, 0x9800518b},
        {8,  0x98402ecc, 0x984c0792, 0x98168a55, 0x9800519f},
        {9,  0x98402ecc, 0x984c0796, 0x98168a55, 0x9800518b},
        {10, 0x98402ecc, 0x984c0796, 0x98168a55, 0x9800519f},
        {11, 0x98402ecc, 0x984c079a, 0x98168a55, 0x9800518b},
        {12, 0x98402ecc, 0x984c079a, 0x98168a55, 0x9800519f},
        {13, 0x98402ecc, 0x984c079e, 0x98168a55, 0x9800518b},
        {14, 0x98402ecc, 0x984c07a2, 0x98168a55, 0x98005193},

        // 802.11 UNI / HyperLan 2
        {36, 0x98402ecc, 0x984c099a, 0x98158a55, 0x980ed1a3},
        {38, 0x98402ecc, 0x984c099e, 0x98158a55, 0x980ed193},
        {40, 0x98402ec8, 0x984c0682, 0x98158a55, 0x980ed183},
        {44, 0x98402ec8, 0x984c0682, 0x98158a55, 0x980ed1a3},
        {46, 0x98402ec8, 0x984c0686, 0x98158a55, 0x980ed18b},
        {48, 0x98402ec8, 0x984c0686, 0x98158a55, 0x980ed19b},
        {52, 0x98402ec8, 0x984c068a, 0x98158a55, 0x980ed193},
        {54, 0x98402ec8, 0x984c068a, 0x98158a55, 0x980ed1a3},
        {56, 0x98402ec8, 0x984c068e, 0x98158a55, 0x980ed18b},
        {60, 0x98402ec8, 0x984c0692, 0x98158a55, 0x980ed183},
        {62, 0x98402ec8, 0x984c0692, 0x98158a55, 0x980ed193},
        {64, 0x98402ec8, 0x984c0692, 0x98158a55, 0x980ed1a3}, // Plugfest#4, Day4, change RFR3 left4th 9->5.
        
        // 802.11 HyperLan 2
        {100, 0x98402ec8, 0x984c06b2, 0x98178a55, 0x980ed783},
        {102, 0x98402ec8, 0x985c06b2, 0x98578a55, 0x980ed793},
        {104, 0x98402ec8, 0x985c06b2, 0x98578a55, 0x980ed1a3},
        {108, 0x98402ecc, 0x985c0a32, 0x98578a55, 0x980ed193},
        {110, 0x98402ecc, 0x984c0a36, 0x98178a55, 0x980ed183},
        {112, 0x98402ecc, 0x984c0a36, 0x98178a55, 0x980ed19b},
        {116, 0x98402ecc, 0x984c0a3a, 0x98178a55, 0x980ed1a3},
        {118, 0x98402ecc, 0x984c0a3e, 0x98178a55, 0x980ed193},
        {120, 0x98402ec4, 0x984c0382, 0x98178a55, 0x980ed183},
        {124, 0x98402ec4, 0x984c0382, 0x98178a55, 0x980ed193},
        {126, 0x98402ec4, 0x984c0382, 0x98178a55, 0x980ed15b},
        {128, 0x98402ec4, 0x984c0382, 0x98178a55, 0x980ed1a3},
        {132, 0x98402ec4, 0x984c0386, 0x98178a55, 0x980ed18b},
        {134, 0x98402ec4, 0x984c0386, 0x98178a55, 0x980ed193},
        {136, 0x98402ec4, 0x984c0386, 0x98178a55, 0x980ed19b},
        {140, 0x98402ec4, 0x984c038a, 0x98178a55, 0x980ed183},
        
        // 802.11 UNII
        {149, 0x98402ec4, 0x984c038a, 0x98178a55, 0x980ed1a7},
        {151, 0x98402ec4, 0x984c038e, 0x98178a55, 0x980ed187},
        {153, 0x98402ec4, 0x984c038e, 0x98178a55, 0x980ed18f},
        {157, 0x98402ec4, 0x984c038e, 0x98178a55, 0x980ed19f},
        {159, 0x98402ec4, 0x984c038e, 0x98178a55, 0x980ed1a7},
        {161, 0x98402ec4, 0x984c0392, 0x98178a55, 0x980ed187},
        {165, 0x98402ec4, 0x984c0392, 0x98178a55, 0x980ed197},
        {167, 0x98402ec4, 0x984c0392, 0x98179855, 0x9815531f},
        {169, 0x98402ec4, 0x984c0392, 0x98179855, 0x98155327},
        {171, 0x98402ec4, 0x984c0396, 0x98179855, 0x98155307},
        {173, 0x98402ec4, 0x984c0396, 0x98179855, 0x9815530f},
        
        // Japan
        {184, 0x95002ccc, 0x9500491e, 0x9509be55, 0x950c0a0b},
        {188, 0x95002ccc, 0x95004922, 0x9509be55, 0x950c0a13},
        {192, 0x95002ccc, 0x95004926, 0x9509be55, 0x950c0a1b},
        {196, 0x95002ccc, 0x9500492a, 0x9509be55, 0x950c0a23},
        {208, 0x95002ccc, 0x9500493a, 0x9509be55, 0x950c0a13},
        {212, 0x95002ccc, 0x9500493e, 0x9509be55, 0x950c0a1b},
        {216, 0x95002ccc, 0x95004982, 0x9509be55, 0x950c0a23},
        // still lack of MMAC(Japan) ch 34,38,42,46
};
#define NUM_OF_2860_CHNL    (sizeof(RF2850RegTable) / sizeof(RTMP_RF_REGS))

/*
    ==========================================================================
    Description:
        initialize the MLME task and its data structure (queue, spinlock, 
        timer, state machines).

    IRQL = PASSIVE_LEVEL

    Return:
        always return NDIS_STATUS_SUCCESS

    ==========================================================================
*/
NDIS_STATUS PortMlmeInit(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort
    ) 
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    DBGPRINT(RT_DEBUG_TRACE, ("--> MLME Initialize\n"));

    do 
    {
        Status = MlmeQueueInit(&pPort->Mlme.Queue);
        if(Status != NDIS_STATUS_SUCCESS) 
            break;      

        
        // init STA state machines
        MlmeAssocStateMachineInit(pAd, &pPort->Mlme.AssocMachine, pPort->Mlme.AssocFunc);
        MlmeAuthStateMachineInit(pAd, &pPort->Mlme.AuthMachine, pPort->Mlme.AuthFunc);
        MlmeAuthRspStateMachineInit(pAd, &pPort->Mlme.AuthRspMachine, pPort->Mlme.AuthRspFunc);
        MlmeSyncStateMachineInit(pAd, &pPort->Mlme.SyncMachine, pPort->Mlme.SyncFunc);
        WpaPskStateMachineInit(pAd, &pPort->Mlme.WpaPskMachine, pPort->Mlme.WpaPskFunc);

        DlsStateMachineInit(pAd, &pPort->Mlme.DlsMachine, pPort->Mlme.DlsFunc);
        ActionStateMachineInit(pAd, &pPort->Mlme.ActMachine, pPort->Mlme.ActFunc);

        TdlsStateMachineInit(pAd, &pPort->Mlme.TdlsMachine, pPort->Mlme.TdlsFunc);
        P2pActionStateMachineInit(pAd, &pPort->Mlme.P2PMachine, pAd->Mlme.P2pFunc);

        // init PMF state machines
        PmfStateMachineInit(pAd, &pPort->Mlme.PmfMachine, pAd->Mlme.PmfFunc);

        // Init Wsc state machine       
        ASSERT(WSC_FUNC_SIZE == MAX_WSC_MSG * MAX_WSC_STATE);
        WscStateMachineInit(pAd, &pPort->Mlme.WscMachine, pPort->Mlme.WscFunc);

        // Since we are using switch/case to implement it, the init is different from the above 
        // state machine init
        MlmeCntlInit(pAd, &pPort->Mlme.CntlMachine, NULL);


        // init AP state machines
        ApMlmeAssocStateMachineInit(pAd, &pPort->Mlme.ApAssocMachine, pPort->Mlme.ApAssocFunc);
        ApMlmeAuthStateMachineInit(pAd, &pPort->Mlme.ApAuthMachine, pPort->Mlme.ApAuthFunc);
        ApMlmeAuthRspStateMachineInit(pAd, &pPort->Mlme.ApAuthRspMachine, pPort->Mlme.ApAuthRspFunc);
        ApMlmeSyncStateMachineInit(pAd, &pPort->Mlme.ApSyncMachine, pPort->Mlme.ApSyncFunc);
        ApWpaStateMachineInit(pAd, &pPort->Mlme.ApWpaMachine, pPort->Mlme.ApWpaFunc);   

        //
        // Set mlme periodic timer
        // It might have timing issuel. This periodic timer may not work periodically, if we set this timer 
        // to early before RTMPInitialize return.
        // PlatformSetTimer(pPort, &pPort->Mlme.PeriodicTimer, MLME_TASK_EXEC_INTV);
        //

        MlmePortTimeInit(pAd, pPort);

        MlmePortWorkItemInit(pAd, pPort);
    
    } while (FALSE);

    DBGPRINT(RT_DEBUG_TRACE, ("<-- MLME Initialize\n"));

    return Status;
}

VOID
MlmePortWorkItemInit(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort
    )
{
    FUNC_ENTER;
    DBGPRINT(RT_DEBUG_TRACE, ("%s   port[%d] \n", __FUNCTION__, pPort->PortNumber));
    
    PlatformInitWorktiem(pPort, &pPort->Mlme.ScanTimeoutWorkitem, MlmeSyncMlmeSyncScanTimeoutActionWorkitemCallBack, "MlmeSyncScanTimeoutActionWorkitem");
    PlatformInitWorktiem(pPort, &pPort->Mlme.AsicSwitchChannelWorkitem, AsicSwitchChannelWorkitemCallback, "AsicSwitchChannelWorkitem");

    FUNC_LEAVE;
}

VOID
MlmePortTimeInit(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort
    )
{
    UCHAR i =0;
    static BOOLEAN  bInitBefore =FALSE;

    FUNC_ENTER;
    DBGPRINT(RT_DEBUG_TRACE, ("%s   port[%d] \n", __FUNCTION__, pPort->PortNumber));
    
    
        // initialize the timer
    PlatformInitTimer(pPort, &pPort->Mlme.AssocTimer, MlmeAssocTimeoutTimerCallback, FALSE,"MlmeAssocTimeoutTimerCallbackTimer");
    PlatformInitTimer(pPort, &pPort->Mlme.ReassocTimer, MlmeAssocReAssocTimeoutTimerCallback, FALSE,"MlmeAssocReAssocTimeoutTimerCallbackTimer");
    PlatformInitTimer(pPort, &pPort->Mlme.DisassocTimer, MlmeAssocDisAssocTimeoutTimerCallback, FALSE,"MlmeAssocDisAssocTimeoutTimerCallbackTimer");

    PlatformInitTimer(pPort, &pPort->Mlme.ConnectTimer, MlmeAssocConnectionTimeoutTimerCallback, FALSE,"ConnectionTimeoutTimer");


    PlatformInitTimer(pPort, &pPort->Mlme.AuthTimer, MlmeAuthTimeoutTimerCallback, FALSE,"AuthTimeoutTimer");
        // timer init
    PlatformInitTimer(pPort, &pPort->Mlme.BeaconTimer, MlmeSyncBeaconTimeoutTimerCallback, FALSE,"BeaconTimeoutTimer");
    PlatformInitTimer(pPort, &pPort->Mlme.ScanTimer, MlmeSyncScanTimeoutTimerCallback, FALSE,"ScanTimeoutTimer");
    PlatformInitTimer(pPort, &pPort->Mlme.ChannelTimer, MlmeSyncRestoreChannelTimeoutTimerCallback, FALSE,"RestoreChannelTimeoutTimer");
        for (i = 0; i < MAX_NUM_OF_DLS_ENTRY; i++)
        {
        pPort->StaCfg.DLSEntry[i].pAd = pAd;
        PlatformInitTimer(pPort, &pPort->StaCfg.DLSEntry[i].Timer, DlsTimeoutTimerCallback, FALSE,"DlsTimeoutTimer");
        }       

        ASSERT(WSC_FUNC_SIZE == MAX_WSC_MSG * MAX_WSC_STATE);
    PlatformInitTimer(pPort, &pPort->StaCfg.WscControl.WscConnectTimer, WscConnectTimeoutTimerCallback, FALSE,"WscConnectTimeoutTimer");
    PlatformInitTimer(pPort, &pPort->StaCfg.WscControl.WscScanTimer, WscMlmeSyncScanTimeoutTimerCallback, FALSE,"WscScanTimeoutTimer");
    PlatformInitTimer(pPort, &pPort->StaCfg.WscControl.WscPBCTimer, WscPBCTimeoutTimerCallback, FALSE,"WscPBCTimeoutTimer");
    PlatformInitTimer(pPort, &pPort->StaCfg.WscControl.WscPINTimer, WscPINTimeoutTimerCallback, FALSE,"WscPINTimeoutTimer");
    PlatformInitTimer(pPort, &pPort->StaCfg.WscControl.WscEapRxTimer, WscEAPRxTimeoutTimerCallback, FALSE,"WscEAPRxTimeoutTimer");
    PlatformInitTimer(pPort, &pPort->StaCfg.WscControl.WscM8WaitEapFailTimer, WscM8WaitEapFailTimeoutTimerCallback, FALSE,"WscM8WaitEapFailTimeoutTimer");
    PlatformInitTimer(pPort, &pPort->StaCfg.WscControl.WscEapM2Timer, WscEapM2TimeoutTimerCallback, FALSE,"WscEapM2TimeoutTimer");
    PlatformInitTimer(pPort, &pPort->StaCfg.WscControl.WscLEDTimer, WscLEDTimerTimerCallback, FALSE,"WscLEDTimerTimer");
    PlatformInitTimer(pPort, &pPort->StaCfg.WscControl.WscSkipTurnOffLEDTimer, WscSkipTurnOffLEDTimerCallback, FALSE,"WscSkipTurnOffLEDTimerTimer");
    PlatformInitTimer(pPort, &pPort->StaCfg.WscControl.WscRxTimeOuttoScanTimer, WscRxTimeOuttoScanTimerCallback, FALSE,"WscRxTimeOuttoScanTimer");
    PlatformInitTimer(pPort, &pPort->StaCfg.WscControl.WscExtregPBCTimer, WscExtregCallPBCExecTimerCallback, FALSE,"WscExtregCallPBCExecTimer");
    PlatformInitTimer(pPort, &pPort->StaCfg.WscControl.WscExtregScanTimer, WscExtregCallScanExecTimerCallback, FALSE,"WscExtregCallScanExecTimer");

    PlatformInitTimer(pPort, &pPort->P2PCfg.P2pStopGoTimer, P2PStopGoTimerCallback, FALSE,"P2PStopGoTimer");
    PlatformInitTimer(pPort, &pPort->P2PCfg.P2pCTWindowTimer, P2PCTWindowTimerCallback, FALSE,"P2PCTWindowTimer");
        
    PlatformInitTimer(pPort, &pPort->P2PCfg.P2pStopClientTimer, P2PStopClientTimerCallback, FALSE,"P2PStopClientTimer");
    PlatformInitTimer(pPort, &pPort->P2PCfg.P2pStopWpsTimer, P2PStopWpsTimerCallback, FALSE,"P2PStopWpsTimer");
    PlatformInitTimer(pPort, &pPort->P2PCfg.P2pSwNoATimer, P2pSwNoATimeOutTimerCallback, FALSE,"P2pSwNoATimeOutTimer");
    PlatformInitTimer(pPort, &pPort->P2PCfg.P2pPreAbsenTimer, P2pPreAbsenTimeOutTimerCallback, FALSE,"P2pPreAbsenTimeOutTimer");
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
        // Init mlme periodic timer
    //KeInitializeDpc(&pPort->Mlme.MlmeDpc, MlmePeriodicExecTimerCallback, pAd);

    PlatformInitTimer(pPort, &pPort->P2PCfg.P2pSendProReqTimer, P2pSendProReqTimeOutTimerCallback, FALSE,"P2pSendProReqTimeOutTimer");   
#endif
    PlatformInitTimer(pPort, &pPort->Mlme.PeriodicTimer, MlmePeriodicExecTimerCallback, FALSE,"MlmePeriodicExecTimer");
    PlatformInitTimer(pPort, &pPort->Mlme.WHCKCPUUtilCheckTimer, WhckCPUCheckTimerCallback, FALSE,"WhckCPUCheckTimer");
    PlatformInitTimer(pPort, &pPort->Mlme.WakeTimer, WakeUpExecTimerCallback, FALSE,"WakeUpExecTimer");

        // a timer to resend bulk-in IRPs, triggered after bulk-in IRPs been cancelled to let CPU enter C3.
    PlatformInitTimer(pPort, &pPort->Mlme.ReSendBulkinIRPsTimer, ReSendBulkinIRPsTimerCallback, FALSE,"ReSendBulkinIRPsTimer");

        // a timer to enqueue a force sleep command even lost a beacon.
    PlatformInitTimer(pPort, &pPort->Mlme.ForceSleepTimer, ForceSleepTimeOutTimerCallback, FALSE,"ForceSleepTimeOutTimer");

        // a timer to enqueue a force sleep command even lost a beacon.
    PlatformInitTimer(pPort, &pPort->Mlme.TBTTTimer, ApDataTbttTimeoutTimerCallback, FALSE,"TBTTTimeoutTimer");
    PlatformInitTimer(pPort, &pPort->Mlme.SwTbttTimer, SwTBTTTimeOutTimerCallback, FALSE,"SwTBTTTimeOutTimer");
        
        //
        // Set mlme periodic timer
        // It might have timing issuel. This periodic timer may not work periodically, if we set this timer 
        // to early before RTMPInitialize return.
    // PlatformSetTimer(pPort, &pPort->Mlme.PeriodicTimer, MLME_TASK_EXEC_INTV);
        //

        // software-based RX Antenna diversity
    PlatformInitTimer(pPort, &pPort->Mlme.RxAntEvalTimer, AsicRxAntEvalTimeoutTimerCallback, FALSE,"AsicRxAntEvalTimeoutTimer");

        // Carrier detection
    PlatformInitTimer(pPort, &pPort->Mlme.CarrierDetectTimer, CarrierDetectTimeoutTimerCallback, TRUE,"CarrierDetectTimeoutTimer");
        
    PlatformInitTimer(pPort, &pPort->Mlme.MlmeCntLinkUpTimer, MlmeCntLinkUpExecTimerCallback, FALSE,"MlmeCntLinkUpExecTimerCallbackTimer");

        // Init timer to report link down event
    PlatformInitTimer(pPort, &pPort->Mlme.MlmeCntLinkDownTimer, MlmeCntLinkDownExecTimerCallback, FALSE,"MlmeCntLinkDownExecTimerCallbackTimer");      

        // init the SAQuery Request periodic timer
    PlatformInitTimer(pPort, &pPort->StaCfg.PmfCfg.SAQueryTimer, PmfReqTimeoutActionTimerCallback, TRUE,"PmfReqTimeoutActionTimer");
    PlatformInitTimer(pPort, &pPort->StaCfg.PmfCfg.SAQueryConfirmTimer, PmfConfirmTimeoutActionTimerCallback, TRUE,"PmfConfirmTimeoutActionTimer");

#ifdef MULTI_CHANNEL_SUPPORT
        // Init channel switch timer
    PlatformInitTimer(pPort, &pPort->Mlme.MultiChannelTimer, MultiChannelSwitchTimerCallback, FALSE,"MultiChannelSwitchTimer");
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
        // Channel restore timer
    PlatformInitTimer(pPort, &pPort->Mlme.ChannelRestoreTimer, P2pMsChannelRestoreTimerCallback, FALSE,"P2pMsChannelRestoreTimer");    
#endif
#endif /*MULTI_CHANNEL_SUPPORT*/

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
#ifdef WFD_NEW_PUBLIC_ACTION
    PlatformInitTimer(pPort, &pPort->Mlme.P2pMsPublicActionFrameTimer, P2pMsPublicActionFrameWaitTimerTimeoutCallback, FALSE,"P2pMsPublicActionFrameWaitTimerTimeoutTimer");
    PlatformInitTimer(pPort, &pPort->Mlme.P2pMsTriggerReadTxFifoTimer, P2pMsTriggerReadTXFIFOTimeoutCallback, FALSE,"P2pMsTriggerReadTXFIFOTimeoutTimer"); 
#endif /*WFD_NEW_PUBLIC_ACTION*/
#endif

        // tdls
        for (i=0; i<MAX_NUM_OF_TDLS_ENTRY; i++)
        {
            // !!! ATTENTION !!!
            // Timer is declared in Entry. Please don't zero all of TDLSEntry.
        pPort->StaCfg.TDLSEntry[i].pAd = pAd;
        PlatformInitTimer(pPort, &pPort->StaCfg.TDLSEntry[i].Timer, TdlsTimeoutTimerCallback, FALSE,"TdlsTimeoutTimer");     
        }

    FUNC_LEAVE;
}

/*
    ==========================================================================
    Description:
        initialize the MLME task and its data structure (queue, spinlock, 
        timer, state machines).

    IRQL = PASSIVE_LEVEL

    Return:
        always return NDIS_STATUS_SUCCESS

    ==========================================================================
*/
NDIS_STATUS MlmeInit(
    IN PMP_ADAPTER pAd
    ) 
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    INT i;

    PMP_PORT pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];
    DBGPRINT(RT_DEBUG_TRACE, ("--> MLME Initialize\n"));

    do 
    {
        Status = MlmeQueueInit(&pAd->Mlme.Queue);
        if(Status != NDIS_STATUS_SUCCESS) 
            break;

        // Initialize Mlme Memory Handler
        // Allocate 20 nonpaged memory pool which size are MAX_LEN_OF_MLME_BUFFER for use
        Status = MlmeInitMemoryHandler(pAd, 20, MAX_LEN_OF_MLME_BUFFER);
        if(Status != NDIS_STATUS_SUCCESS)
        {
            MlmeQueueDestroy(&pAd->Mlme.Queue);
            break;
        }

        pAd->Mlme.bRunning = FALSE;
        NdisAllocateSpinLock(&pAd->Mlme.TaskLock);

        MtInitInternalCmdBuffer(pAd);

        NdisAllocateSpinLock(&pAd->Mlme.MlmeInternalCmdBufLock);

        BssTableInit(&pAd->ScanTab);

        //reset Reposit Bss Table
        if(pAd->pNicCfg->RepositBssTable.pBssEntry != NULL)
        {
            for (i = 0; i < MAX_LEN_OF_BSS_TABLE; i++) 
            {
                PlatformZeroMemory(&pAd->pNicCfg->RepositBssTable.pBssEntry[i], sizeof(BSS_ENTRY));
                pAd->pNicCfg->RepositBssTable.ulTime[i].LowPart = 0;
                pAd->pNicCfg->RepositBssTable.ulTime[i].HighPart = 0;
            }
        }

        // init TDLS table
        NdisAllocateSpinLock(&pAd->StaCfg.TDLSEntryLock);
        NdisAllocateSpinLock(&pAd->StaCfg.TdlsSearchTabLock);
        NdisAllocateSpinLock(&pAd->StaCfg.TdlsDiscoveyQueueLock);
        
        TdlsTableInit(pAd);
  
    } while (FALSE);

    PortMlmeInit( pAd, pAd->PortList[PORT_0]);

    for(i=0;i<MAX_LEN_OF_MAC_TABLE;i++)
    {
        MAC_TABLE_ENTRY *pEntry = NULL;

        pEntry = (PMAC_TABLE_ENTRY)&pAd->MacTabPoolArray[i];
        pEntry->RetryTimer.pOption = (PVOID)pEntry;
        pEntry->RetryTimer.bInputOption = TRUE;
        PlatformInitTimer(pPort, &pEntry->RetryTimer, ApWpaRetryExecTimerCallback, FALSE, "WPARetryExecTimer");
    }

    MlmeCntPortCntlInit(pAd->PortList[PORT_0], &pAd->PortList[PORT_0]->Mlme.CntlMachine, NULL);

    PlatformIndicateResetAllStatus(pAd, pAd->PortList[PORT_0]);
    
    DBGPRINT(RT_DEBUG_TRACE, ("<-- MLME Initialize\n"));

    return Status;
}


/*
    ==========================================================================
    Description:
        main loop of the MLME
    Pre:
        Mlme has to be initialized, and there are something inside the queue
    Note:
        This function is invoked from MPSetInformation and MPReceive;
        This task guarantee only one MlmeHandler will run. 

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
 */
VOID MlmeHandler(
    IN PMP_ADAPTER pAd) 
{
    MLME_QUEUE_ELEM *Elem = NULL;
    PMP_PORT      pPort ;
    // Only accept MLME and Frame from peer side, no other (control/data) frame should
    // get into this state machine

    NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
    if(pAd->Mlme.bRunning) 
    {
        NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
        return;
    } 
    else 
    {
        pAd->Mlme.bRunning = TRUE;
    }
    NdisReleaseSpinLock(&pAd->Mlme.TaskLock);

    while (!MlmeQueueEmpty(&pAd->Mlme.Queue)) 
    {
        
        if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_MLME_RESET_IN_PROGRESS) ||
            MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
            MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS) ||
            MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Device Halted or Removed, clear queue and exit MlmeHandler! (queue num = %d)(%d %d %d %d)\n", pAd->Mlme.Queue.Num, MT_TEST_FLAG(pAd, fRTMP_ADAPTER_MLME_RESET_IN_PROGRESS), MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS), MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS), MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));
            break;
        }

        //From message type, determine which state machine I should drive
        if (MlmeDequeue(&pAd->Mlme.Queue, &Elem)) 
        {   
            pPort =pAd->PortList[Elem->PortNum];
            if(pPort == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("pPort == NULL exit MlmeHandler!\n"));
                break;
            }
            // if dequeue success
            switch (Elem->Machine) 
            {
                // STA state machines

                case ASSOC_STATE_MACHINE:
                    StateMachinePerformAction(pAd, &pPort->Mlme.AssocMachine, Elem);
                    break;
                case AUTH_STATE_MACHINE:
                    StateMachinePerformAction(pAd, &pPort->Mlme.AuthMachine, Elem);
                    break;
                case AUTH_RSP_STATE_MACHINE:
                    StateMachinePerformAction(pAd, &pPort->Mlme.AuthRspMachine, Elem);
                    break;
                case SYNC_STATE_MACHINE:
                    StateMachinePerformAction(pAd, &pPort->Mlme.SyncMachine, Elem);
                    break;
                case MLME_CNTL_STATE_MACHINE:
                    MlmeCntlMachinePerformAction(pAd, &pPort->Mlme.CntlMachine, Elem);
                    break;
                case WPA_PSK_STATE_MACHINE:
                    StateMachinePerformAction(pAd, &pPort->Mlme.WpaPskMachine, Elem);
                    break;  

                case DLS_STATE_MACHINE:
                    StateMachinePerformAction(pAd, &pPort->Mlme.DlsMachine, Elem);
                    break;
                    
                case ACTION_STATE_MACHINE:
                    StateMachinePerformAction(pAd, &pPort->Mlme.ActMachine, Elem);
                    break;  
                case WSC_STATE_MACHINE:
                    StateMachinePerformAction(pAd, &pPort->Mlme.WscMachine, Elem);
                    break;  

                case TDLS_STATE_MACHINE:
                    StateMachinePerformAction(pAd, &pPort->Mlme.TdlsMachine, Elem);
                    break;
                    
                case STA_PMF_STATE_MACHINE:
                    StateMachinePerformAction(pAd, &pPort->Mlme.PmfMachine, Elem);
                    break;

                // AP state amchines

                case AP_ASSOC_STATE_MACHINE:
                    StateMachinePerformAction(pAd, &pPort->Mlme.ApAssocMachine, Elem);
                    break;
                case AP_AUTH_STATE_MACHINE:
                    StateMachinePerformAction(pAd, &pPort->Mlme.ApAuthMachine, Elem);
                    break;
                case AP_AUTH_RSP_STATE_MACHINE:
                    StateMachinePerformAction(pAd, &pPort->Mlme.ApAuthRspMachine, Elem);
                    break;
                case AP_SYNC_STATE_MACHINE:
                    StateMachinePerformAction(pAd, &pPort->Mlme.ApSyncMachine, Elem);
                    break;
                case AP_WPA_STATE_MACHINE:
                    StateMachinePerformAction(pAd, &pPort->Mlme.ApWpaMachine, Elem);
                    break;
                case P2P_STATE_MACHINE:
                    StateMachinePerformAction(pAd, &pPort->Mlme.P2PMachine, Elem);
                    break;

                default:
                    DBGPRINT(RT_DEBUG_TRACE, ("ERROR: Illegal machine in MlmeHandler()\n"));
                    break;
            } // end of switch

            // free MLME element
            Elem->Occupied = FALSE;
            Elem->MsgLen = 0;

        }
        else {
            DBGPRINT_ERR(("MlmeHandler: MlmeQueue empty\n"));
        }
    }

    NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
    pAd->Mlme.bRunning = FALSE;
    NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
}

VOID
MlmePortFreeWorkItem(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort
    )
{
    FUNC_ENTER;

    DBGPRINT(RT_DEBUG_TRACE, ("%s   port[%d]", __FUNCTION__, pPort->PortNumber));
    
    PlatformFreeWorkitem(&pPort->Mlme.ScanTimeoutWorkitem);
    PlatformFreeWorkitem(&pPort->Mlme.AsicSwitchChannelWorkitem);

    FUNC_LEAVE;
}


VOID
MlmePortFreeTimer(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort
    )
{
    BOOLEAN Cancelled = FALSE;
    UCHAR   i;

    FUNC_ENTER;

    DBGPRINT(RT_DEBUG_TRACE, ("%s   port[%d]", __FUNCTION__, pPort->PortNumber));
    
    PlatformCancelTimer(&pPort->StaCfg.PmfCfg.SAQueryTimer, &Cancelled);
    PlatformFreeTimer(&pPort->StaCfg.PmfCfg.SAQueryTimer);

    PlatformCancelTimer(&pPort->StaCfg.PmfCfg.SAQueryConfirmTimer, &Cancelled);
    PlatformFreeTimer(&pPort->StaCfg.PmfCfg.SAQueryConfirmTimer);

    // Cancel pending timers
    PlatformCancelTimer(&pPort->Mlme.AssocTimer,       &Cancelled);
    PlatformFreeTimer(&pPort->Mlme.AssocTimer);	

    PlatformCancelTimer(&pPort->Mlme.ReassocTimer,     &Cancelled);
    PlatformFreeTimer(&pPort->Mlme.ReassocTimer); 

    PlatformCancelTimer(&pPort->Mlme.DisassocTimer,    &Cancelled);
    PlatformFreeTimer(&pPort->Mlme.DisassocTimer); 
    
    PlatformCancelTimer(&pPort->Mlme.ConnectTimer, &Cancelled);
    PlatformFreeTimer(&pPort->Mlme.ConnectTimer); 
    
    PlatformCancelTimer(&pPort->Mlme.AuthTimer,        &Cancelled);
    PlatformFreeTimer(&pPort->Mlme.AuthTimer); 
    
    PlatformCancelTimer(&pPort->Mlme.ReSendBulkinIRPsTimer,   &Cancelled);
    PlatformFreeTimer(&pPort->Mlme.ReSendBulkinIRPsTimer); 
    
    PlatformCancelTimer(&pPort->Mlme.ForceSleepTimer,   &Cancelled);
    PlatformFreeTimer(&pPort->Mlme.ForceSleepTimer); 
    
    PlatformCancelTimer(&pPort->Mlme.BeaconTimer,      &Cancelled);
    PlatformFreeTimer(&pPort->Mlme.BeaconTimer); 
    
    PlatformCancelTimer(&pPort->Mlme.ScanTimer,        &Cancelled);
    PlatformFreeTimer(&pPort->Mlme.ScanTimer); 
    
    PlatformCancelTimer(&pPort->Mlme.ChannelTimer,     &Cancelled);
    PlatformFreeTimer(&pPort->Mlme.ChannelTimer); 
    
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    PlatformCancelTimer(&pPort->P2PCfg.P2pSendProReqTimer, &Cancelled);
    PlatformFreeTimer(&pPort->P2PCfg.P2pSendProReqTimer); 
#endif
    PlatformCancelTimer(&pPort->Mlme.PeriodicTimer,       &Cancelled);
    PlatformFreeTimer(&pPort->Mlme.PeriodicTimer); 

    PlatformCancelTimer(&pPort->Mlme.MlmeCntLinkUpTimer,         &Cancelled);
    PlatformFreeTimer(&pPort->Mlme.MlmeCntLinkUpTimer); 
    
    PlatformCancelTimer(&pPort->Mlme.MlmeCntLinkDownTimer,       &Cancelled);
    PlatformFreeTimer(&pPort->Mlme.MlmeCntLinkDownTimer); 
    
    PlatformCancelTimer(&pPort->Mlme.RxAntEvalTimer,      &Cancelled);
    PlatformFreeTimer(&pPort->Mlme.RxAntEvalTimer); 
    
    PlatformCancelTimer(&pPort->Mlme.CarrierDetectTimer,  &Cancelled);
    PlatformFreeTimer(&pPort->Mlme.CarrierDetectTimer); 
    
    PlatformCancelTimer(&pPort->Mlme.WakeTimer,       &Cancelled);
    PlatformFreeTimer(&pPort->Mlme.WakeTimer); 
    
    PlatformCancelTimer(&pPort->Mlme.TBTTTimer, &Cancelled);
    PlatformFreeTimer(&pPort->Mlme.TBTTTimer); 
    
    PlatformCancelTimer(&pPort->Mlme.SwTbttTimer, &Cancelled);
    PlatformFreeTimer(&pPort->Mlme.SwTbttTimer); 
    
    PlatformCancelTimer(&pPort->P2PCfg.P2pStopGoTimer, &Cancelled);
    PlatformFreeTimer(&pPort->P2PCfg.P2pStopGoTimer); 
    
    PlatformCancelTimer(&pPort->P2PCfg.P2pCTWindowTimer, &Cancelled);
    PlatformFreeTimer(&pPort->P2PCfg.P2pCTWindowTimer); 
    
    PlatformCancelTimer(&pPort->P2PCfg.P2pStopClientTimer, &Cancelled);
    PlatformFreeTimer(&pPort->P2PCfg.P2pStopClientTimer); 
    
    PlatformCancelTimer(&pPort->P2PCfg.P2pStopWpsTimer, &Cancelled);
    PlatformFreeTimer(&pPort->P2PCfg.P2pStopWpsTimer); 
    
    PlatformCancelTimer(&pPort->P2PCfg.P2pSwNoATimer, &Cancelled);
    PlatformFreeTimer(&pPort->P2PCfg.P2pSwNoATimer); 
    
    PlatformCancelTimer(&pPort->P2PCfg.P2pPreAbsenTimer, &Cancelled);
    PlatformFreeTimer(&pPort->P2PCfg.P2pPreAbsenTimer); 
    
    //if (pAd->LogoTestCfg.OnTestingWHQL == TRUE)
    {
        PlatformCancelTimer(&pPort->Mlme.WHCKCPUUtilCheckTimer, &Cancelled);
        PlatformFreeTimer(&pPort->Mlme.WHCKCPUUtilCheckTimer); 
    }

    PlatformCancelTimer(&pPort->SoftAP.ApCfg.REKEYTimer, &Cancelled);
    PlatformFreeTimer(&pPort->SoftAP.ApCfg.REKEYTimer);

    PlatformCancelTimer(&pPort->SoftAP.ApCfg.CounterMeasureTimer, &Cancelled);
    PlatformFreeTimer(&pPort->SoftAP.ApCfg.CounterMeasureTimer);

#ifdef MULTI_CHANNEL_SUPPORT

        PlatformCancelTimer(&pPort->Mlme.MultiChannelTimer, &Cancelled);
        PlatformFreeTimer(&pPort->Mlme.MultiChannelTimer);

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        // Channel restore timer
        PlatformCancelTimer(&pPort->Mlme.ChannelRestoreTimer, &Cancelled);
        PlatformFreeTimer(&pPort->Mlme.ChannelRestoreTimer);
#endif
#endif /*MULTI_CHANNEL_SUPPORT*/

    for (i = 0; i < MAX_NUM_OF_DLS_ENTRY; i++)
    {
        PlatformCancelTimer(&pPort->StaCfg.DLSEntry[i].Timer, &Cancelled);
        PlatformFreeTimer(&pPort->StaCfg.DLSEntry[i].Timer); 
    }   
    
    for (i = 0; i < MAX_NUM_OF_TDLS_ENTRY; i++)
    {
        PlatformCancelTimer(&pPort->StaCfg.TDLSEntry[i].Timer, &Cancelled);
        PlatformFreeTimer(&pPort->StaCfg.TDLSEntry[i].Timer);     
    }   

    // Clear timer for Wsc
    WscStop(pAd, pPort);
    WscTimerStop(pAd, pPort);  

    FUNC_LEAVE;
}

/*
    ==========================================================================
    Description:
        Destructor of MLME (Destroy queue, state machine, spin lock and timer)
    Parameters:
        Adapter - NIC Adapter pointer
    Post:
        The MLME task will no longer work properly

    IRQL = PASSIVE_LEVEL

    ==========================================================================
 */
VOID 
MlmeHalt(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort
    ) 
    {
    BOOLEAN Cancelled = FALSE;
    UCHAR   i;
    
    DBGPRINT(RT_DEBUG_TRACE, ("==> MlmeHalt\n"));
    
    
    for(i=0;i<MAX_LEN_OF_MAC_TABLE;i++)
    {
        MAC_TABLE_ENTRY *pEntry = NULL;
        pEntry = (PMAC_TABLE_ENTRY)&pAd->MacTabPoolArray[i];

        PlatformCancelTimer(&pEntry->RetryTimer,   &Cancelled);
        PlatformFreeTimer(&pEntry->RetryTimer);
    }    

    MlmePortFreeWorkItem(pAd, pPort);
    MlmePortFreeTimer(pAd, pPort);
    
    NdisCommonGenericDelay(500000);    // 0.5 sec to gurantee Ant Diversity timer canceled

    MlmeQueueDestroy(&pAd->Mlme.Queue);
    NdisFreeSpinLock(&pAd->Mlme.TaskLock);

    // release memory for TDLS Auto Discovery Table
    TdlsSearchTabDestory(pAd);
    NdisFreeSpinLock(&pAd->StaCfg.TdlsSearchTabLock);
    NdisFreeSpinLock(&pAd->StaCfg.TDLSEntryLock);
    NdisFreeSpinLock(&pAd->StaCfg.TdlsDiscoveyQueueLock);

    MlmeFreeMemoryHandler(pAd);
    DBGPRINT(RT_DEBUG_TRACE, ("<== MlmeHalt\n"));
}

//
// RUN Level : DISPATCH_LEVEL
//
VOID ReSendBulkinIRPsTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3)
{
    UCHAR BulkInIndex = 0; 
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;

    // Enqueue this command because that WdfWaitLockAcquire with NULL parameter in N6UsbRecvStart()
    // is required to run under passive level.
    MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_RESEND_RX_IRP, NULL, 0);
    // trigger another Force Sleep after 20ms
    PlatformSetTimer(pPort, &pPort->Mlme.ForceSleepTimer, 20);
}

VOID ForceSleepTimeOutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3)
{
    UCHAR BulkInIndex = 0; 
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;

    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS))
    {
        DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d]_Already in sleep, skip enqueue MT_CMD_FORCE_SLEEP\n",__FUNCTION__,__LINE__));               
        return;
    }   
    
    DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] ForceSleepTimer timeout, do ForceSleepTimeOutTimerCallback\n",__FUNCTION__,__LINE__));       
    
    if(pAd->ConsecutivelyBeaconLostCount < 10)
    {
        MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_FORCE_SLEEP, NULL, 0);
        pAd->ConsecutivelyBeaconLostCount++;
    }
    else
    {       
        DBGPRINT(RT_DEBUG_TRACE,("pAd->ConsecutivelyBeaconLostCount = %d\n",pAd->ConsecutivelyBeaconLostCount));
        DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Lost too much consecutive beacon, skip a force-sleep\n",__FUNCTION__,__LINE__));
    }
    DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d] Enqueue MT_CMD_FORCE_SLEEP, pAd->ForceSleepCount == %d\n",__FUNCTION__,__LINE__,pAd->ConsecutivelyBeaconLostCount));
}

/*
    ==========================================================================
    Description:
        This routine is executed periodically to -
        1. Decide if it's a right time to turn on PwrMgmt bit of all 
           outgoiing frames
        2. Calculate ChannelQuality based on statistics of the last
           period, so that TX rate won't toggling very frequently between a 
           successful TX and a failed TX.
        3. If the calculated ChannelQuality indicated current connection not 
           healthy, then a ROAMing attempt is tried here.

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */

UCHAR endpoints[5] = {0,1,2,3,5};
UCHAR times[10] = {0,1,2,3,4,9,10,11,12,13};
//MLME_TASK_EXEC_INTV
VOID MlmePeriodicExecTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{       
    //PMP_ADAPTER   pAd = (MP_ADAPTER *)FunctionContext;

    //MTEnqueueInternalCmd(pAd, pPort, MT_CMD_MLME_PERIODIC_EXEC_PASSIVE_LEVEL, NULL, 0);
    //MTEnqueueInternalCmd(pAd, pPort, MT_CMD_AUTO_SCAN, NULL, 0);
    //MTEnqueueInternalCmd(pAd, pPort, MT_CMD_MLME_PERIODIC_EXEC_PASSIVE_LEVEL_TEST, NULL, 0);


//**
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
#ifdef MULTI_CHANNEL_SUPPORT
    MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_MLME_PERIODIC_EXEC_PASSIVE_LEVEL, NULL, 0);
#else
    MlmePeriodicExecPassivLevel(pAd);
#endif
}

VOID MlmePeriodicExecPassivLevel(
    IN PMP_ADAPTER    pAd) 
{
    //MP_ADAPTER  *pAd = (MP_ADAPTER *)FunctionContext; 
    PMP_PORT      pPort = MlmeSyncGetActivePort(pAd);
    UCHAR           i;

    if((MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))       ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SURPRISE_REMOVED)))
    {
        return;
    }

    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s - pPort is  NULL, Error, and return this function", __FUNCTION__));
        return;
    }

#if _WIN8_USB_SS_SUPPORTED
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPENDED))
    {
        return ;
    }
#endif

    // Do nothing if monitor mode is on
    if (pAd->UiCfg.bPromiscuous == TRUE)
        return;

    if ((pAd->Mlme.OneSecPeriodicRound % 3 == 0) &&
        ((pAd->Mlme.PeriodicRound % MLME_TASK_EXEC_MULTIPLE) == 0) &&
        ((pAd->StaCfg.bHardwareRadio == TRUE) || (pAd->HwCfg.NicConfig2.field.EnableWPSPBC)) &&
        (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
        (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
        (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS)))
    {
        MTEnqueueInternalCmd(pAd, pPort, MTK_OID_N5_QUERY_CHECK_GPIO, NULL, 0);
    }

    pAd->Mlme.PeriodicRound ++;

    if(pAd->bInShutdown)
    {
        if(pAd->Shutdowncnt !=0)
            pAd->Shutdowncnt--;
    }

    // Do nothing if the driver is starting halt state.
    // This might happen when timer already been fired before cancel timer with mlmehalt
    if ((MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) || 
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_MEASUREMENT)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)))
    {
        //
        // Also update OneSecPeriodicRound otherwise, MTK_OID_N5_QUERY_CHECK_GPIO will not happen.
        //
        if ((pAd->Mlme.PeriodicRound % MLME_TASK_EXEC_MULTIPLE) == 0)
            pAd->Mlme.OneSecPeriodicRound ++;
    
        // P2P event is for interface with GUI. So in radio off mode, still maintain this.
        if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) && 
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_MEASUREMENT)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)))
            P2pEventMaintain(pAd);

        return;
    }

#if 0
    if (INFRA_ON(pPort) && (pAd->LogoTestCfg.OnTestingWHQL == TRUE) && (pAd->LogoTestCfg.WhckCpuUtilTestRunning == TRUE))
    {
        if (pAd->LogoTestCfg.TurnOffBeaconListen ==  TRUE)
        {
            if (pAd->LogoTestCfg.CounterForTOBL == 0)   
            {
                pAd->LogoTestCfg.TurnOffBeaconListen = FALSE;
                pAd->LogoTestCfg.CounterForTOBL = 2;
                MtAsicSetBssid(pAd, pPort->PortCfg.Bssid, (UINT8)pPort->PortNumber);
                DBGPRINT(RT_DEBUG_TRACE, ("Set Bssid back to AP's Bssid (%02x-%02x-%02x-%02x-%02x-%02x)\n",
                    pPort->PortCfg.Bssid[0],pPort->PortCfg.Bssid[1],pPort->PortCfg.Bssid[2],pPort->PortCfg.Bssid[3],pPort->PortCfg.Bssid[4],pPort->PortCfg.Bssid[5]));
            }
            else
            {
                pAd->LogoTestCfg.CounterForTOBL--;
            }
        }
        else
        {
            if (pAd->LogoTestCfg.CounterForTOBL == 0)   
            {
                pAd->LogoTestCfg.TurnOffBeaconListen = TRUE;
                pAd->LogoTestCfg.CounterForTOBL = 28;
                MtAsicSetBssid(pAd, pAd->HwCfg.PermanentAddress, 0);
                DBGPRINT(RT_DEBUG_TRACE, ("Set Bssid as self mac (%02x-%02x-%02x-%02x-%02x-%02x)\n",
                    pAd->HwCfg.PermanentAddress[0],pAd->HwCfg.PermanentAddress[1],pAd->HwCfg.PermanentAddress[2],
                    pAd->HwCfg.PermanentAddress[3],pAd->HwCfg.PermanentAddress[4],pAd->HwCfg.PermanentAddress[5]));
            }
            else
            {
                pAd->LogoTestCfg.CounterForTOBL--;
            }
        }
    }
#endif    


    if ((pAd->Mlme.PeriodicRound % RECBA_EXEC_MULTIPLE) == 0)
    {
#if 0
        //
        // Check if hardware not exist and we have queued pending packets.
        // Complete it here.
        //
        if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) && ((pAd->Mlme.PeriodicRound % MLME_TASK_EXEC_MULTIPLE) == 0))
        {
            Ndis6RejectPendingPackets(pAd);         
        }
#endif

        RECBATimerTimeout(pAd);
    }

#if 0
    // CCX S72, roaming with RF parameters, check if UI sets the RF Roam flag
    if (pAd->StaCfg.CCXControl.field.RFRoamEnable)
    {
        //DBGPRINT(RT_DEBUG_INFO, ("CCXRFRoamEnable: Rssi=%d\n", RTMPMaxRssi(pAd, pAd->StaCfg.LastRssi, pAd->StaCfg.LastRssi2, pAd->StaCfg.LastRssi3)));
        AironetRFRoaming(pAd, pPort);
    }
#endif

    if(pPort->bRoamingAfterResume && ((pAd->Mlme.PeriodicRound % 1) == 0))
    {
        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_PERIODIC_EXECUT, NULL, 0); //STAMlmePeriodicExec(pAd);
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hMLMEThread)));
    }

    if ((pAd->Mlme.PeriodicRound % MLME_TASK_EXEC_MULTIPLE) == 0)
    {
        pAd->Mlme.OneSecPeriodicRound ++;
        NdisGetCurrentSystemTime((PLARGE_INTEGER)&pAd->Mlme.Now64);
        // if MGMT RING is full more than twice within 1 second, we consider there's
        // a hardware problem stucking the TX path. In this case, try a hardware reset
        // to recover the system
        if (pAd->Counter.MTKCounters.MgmtRingFullCount >= 2)
            MT_SET_FLAG(pAd, fRTMP_ADAPTER_HARDWARE_ERROR);
        else
            pAd->Counter.MTKCounters.MgmtRingFullCount = 0;
    
        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_PERIODIC_EXECUT, NULL, 0); //STAMlmePeriodicExec(pAd);

        DBGPRINT(RT_DEBUG_INFO, ("OneSecAggCnt=%d,OneSecNoAggCnt=%d\n", pAd->Counter.MTKCounters.OneSecAggCnt, pAd->Counter.MTKCounters.OneSecNoAggCnt));
        
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hMLMEThread)));
    }
    
    // Only when count down to zero, can we go to sleep mode.
    // Count down counter is set after link up or after RTMPInitilize. So within 10 seconds after link up, we never go to sleep.
    // 10 seconds period, we can get IP, finish 802.1x authenticaion. and some critical , timing protocol.
    if ((pAd->TrackInfo.CountDowntoPsm > 0) && (pAd->StaCfg.PSControl.field.DisablePS == FALSE))
        pAd->TrackInfo.CountDowntoPsm--;

    if ((pAd->TrackInfo.CountDownFromS3S4 > 0) && ((pAd->Mlme.PeriodicRound % MLME_TASK_EXEC_MULTIPLE) == 0))
    {
        DBGPRINT(RT_DEBUG_TRACE,("pAd->TrackInfo.CountDownFromS3S4 = %d\n",pAd->TrackInfo.CountDownFromS3S4));
        pAd->TrackInfo.CountDownFromS3S4--;  // 
    }

#if 0
#ifdef MULTI_CHANNEL_SUPPORT
    // !!!Skip BBP Reset check!!!
#else

    if (((pPort->PortSubtype == PORTSUBTYPE_STA && !MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) || 
        (pPort->PortSubtype == PORTSUBTYPE_P2PClient && !MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
        (pPort->PortSubtype == PORTSUBTYPE_P2PGO)) &&
        (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
        IS_P2P_SIGMA_OFF(pPort))
    {
        //
        // Bug: BBP dead. Need to Hard-Reset BBP
        //
        if (pAd->Counter.MTKCounters.LastReceivedByteCount == pAd->Counter.MTKCounters.ReceivedByteCount)
        {
            // If ReceiveByteCount doesn't change,  increase SameRxByteCount by 1.
            pAd->TrackInfo.SameRxByteCount++;
        }
        else
        {
            pAd->TrackInfo.SameRxByteCount = 0;
        }

        // Fix SW watchdog issue for PM5
        pAd->Counter.MTKCounters.LastReceivedByteCount = pAd->Counter.MTKCounters.ReceivedByteCount;

        // If SameRxByteCount keeps happens 
        // for 2 second in infra mode
        // for 60 seconds in idle mode
        // for 1 second in P2PGO
        // for OnTestingWHQL, timing *= 5
        if (((INFRA_ON(pPort)) && (pAd->TrackInfo.SameRxByteCount > pAd->TrackInfo.InfraOnSameRxByteCount)) || 
            ((IDLE_ON(pPort)) && (pAd->TrackInfo.SameRxByteCount < pAd->TrackInfo.IdleOnSameRxByteCountUpper) && (pAd->TrackInfo.SameRxByteCount > pAd->TrackInfo.IdleOnSameRxByteCountLower)) ||
            ((pPort->PortSubtype == PORTSUBTYPE_P2PGO) && (pAd->TrackInfo.SameRxByteCount > pAd->TrackInfo.GoOnSameRxByteCount))) 
        {
            DBGPRINT(RT_DEBUG_TRACE, ("SW watchdog[scan no AP] ....\n"));
            if (pAd->StaCfg.bRadio == TRUE)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("--->  PortSubType =%d.   SameRxByteCount = %d !!!!!!!!!!!!!!! \n", pPort->PortSubtype, pAd->TrackInfo.SameRxByteCount));
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_RESET_BBP, NULL, 0);
                pAd->TrackInfo.SameRxByteCount = 0;
            }
        }
    }
#endif  /* MULTI_CHANNEL_SUPPORT */
    
#endif

    //
    // clear all OneSecxxx counters at MTK_OID_N5_QUERY_QKERIODIC_EXECUT
    //
    for ( i = 0; ((pAd->PortList[i] != NULL) && (pAd->PortList[i]->bActive == TRUE)); i++)
    {
        pPort = pAd->PortList[i];

        if (pPort->PortType != WFD_DEVICE_PORT)
        {
            MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_QKERIODIC_EXECUT, NULL, 0);
        }
        else
        {
            MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_P2P_PERIODIC_EXECUT, NULL, 0);
        }
    }

    TxBfApplyMatrixToPacket(pAd); // Determine that the packets should apply the beamforming matrix of the implicit/explicit TxBf, or not

#if DBG
    if (pAd->Mlme.PeriodicRound % (MLME_TASK_EXEC_MULTIPLE*6) == 1)
    {
        if (pAd->pHifCfg->BulkOutReqCount[1] < 10 )
        {
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("====================OUT======================\n"));
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("pAd->Flags=0x%x, pAd->BulkFlags=0x%x\n", pAd->Flags, pAd->pHifCfg->BulkFlags));
            DBGPRINT_RAW(RT_DEBUG_ERROR, (">>BulkOutComplete=0x%x\n", pAd->pHifCfg->BulkOutComplete));
            DBGPRINT_RAW(RT_DEBUG_ERROR, (">>BulkOutCompleteCancel=0x%x, BulkOutCompleteOther=0x%x\n", pAd->pHifCfg->BulkOutCompleteCancel, pAd->pHifCfg->BulkOutCompleteOther));
            DBGPRINT(RT_DEBUG_ERROR,(" --> CurWritePosition = %d.  NextBulkOutPosition = %d. \n", pAd->pHifCfg->TxContext[0].CurWritePosition, pAd->pHifCfg->TxContext[0].NextBulkOutPosition));
            
        }
        DBGPRINT(RT_DEBUG_TRACE, ("BulkOutSize = %d, BulkOutReqCount[0]=%d, %d, %d. SegmentEnd= %d.  \n", pAd->pHifCfg->BulkOutSize, 
            pAd->pHifCfg->BulkOutReqCount[0], pAd->pHifCfg->BulkOutReqCount[1], pAd->pHifCfg->BulkOutReqCount[2], pAd->pHifCfg->SegmentEnd));
        pAd->pHifCfg->BulkOutReqCount[0] = 0;
        pAd->pHifCfg->BulkOutReqCount[1] = 0;        
        pAd->pHifCfg->BulkOutReqCount[2] = 0;        
    }
#endif

    
    if (isEnableETxBf(pAd))
    {
        if ((pAd->Mlme.PeriodicRound % PERIOD_OF_SENDING_NDP_ANNOUNCEMENT) == 0)
        {
            MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_TXBF_NDPANNOUNCE_GEN, NULL, 0); //STAMlmePeriodicExec(pAd);
            //TxBfNDPAnnouncementGeneration(pAd); // The generation of +HTC MPDU(s) with the NDP Announcement set to 1
        }
    }

    PlatformSetTimer(pPort, &pPort->Mlme.PeriodicTimer, MLME_TASK_EXEC_INTV);
}

VOID STAMlmePeriodicExec(
    IN PMP_ADAPTER pAd)
{
    ULONG               TxTotalCnt=0;
    ULONG           RxTotalCnt=0;
    ULONG               MACValue = 0;
    TX_RTY_CFG_STRUC    TxRtyCfg/*,TxRtyCfgtmp*/;
    TX_RTS_CFG_STRUC    RtsCfg/*,RtsCfgtmp*/;
    PMP_PORT          pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER]; //pPort is from port0
    PMP_PORT          pActivePort = MlmeSyncGetActivePort(pAd);
    MP_RW_LOCK_STATE    LockState;
    PRT_802_11_TDLS     pTDLS;
    UCHAR               idx;
    UCHAR               uPortNum;
    ULONGLONG           LastBeaconRxTime = pAd->StaCfg.LastBeaconRxTime;
    ULONG               /*Index, */Count = 0;
    BOOLEAN             TestWHCK = FALSE;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;   
    PQUEUE_HEADER pHeader;
    BOOLEAN bRoaming = FALSE;
    FUNC_ENTER;

    {
        DBGPRINT(RT_DEBUG_TRACE, ("Per sec Receive PktNum = %d\n",(pAd->Counter.MTKCounters.OneSecRxBeaconCnt +pAd->Counter.MTKCounters.OneSecRxOkDataCnt)));     
        if((pAd->Counter.MTKCounters.OneSecRxBeaconCnt +pAd->Counter.MTKCounters.OneSecRxOkDataCnt) ==0)
           bRoaming = TRUE; 
        else
            pPort->CommonCfg.RoamingCnt =0;
    }
    
    NdisAcquireSpinLock(&pAd->AdapterStateLock);
    if(pAd->AdapterState == NicPausing)
    {
        NdisReleaseSpinLock(&pAd->AdapterStateLock);
        return;
    }
    NdisReleaseSpinLock(&pAd->AdapterStateLock);

#if _WIN8_USB_SS_SUPPORTED
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPENDED))
    {
        return ;
    }
#endif

    //prevent wait  too long  in S5  state
    if(pAd->bInShutdown && (pAd->Shutdowncnt == 0))
        return;
    if(pPort ==NULL)
        return;
    if (pPort->PortType == EXTAP_PORT)
        return;

    if ((pAd->LogoTestCfg.WhckCpuUtilTestRunning == TRUE) && (pAd->LogoTestCfg.OnTestingWHQL == TRUE))
    {
        TestWHCK = TRUE;
    }


    if ((pAd->HwCfg.FreqCalibrationCtrl.bEnableFrequencyCalibration == TRUE) && 
         (pPort->SoftAP.bAPStart == FALSE) &&
         (pPort->P2PCfg.bGOStart == FALSE) &&
         (INFRA_ON(pActivePort))
              && (pAd->HwCfg.SkipFreqCaliTimer == 0))
    {
        FrequencyCalibration(pAd);
    }

#if 0//def MULTI_CHANNEL_SUPPORT
    if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) && !MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
    {
        PBF_CFG_STRUC pbfCfg = {0};

        pbfCfg.word = 0;
        MACValue = 0;       
        RTUSBReadMACRegister(pAd, TXRXQ_PCNT, &MACValue);
        RTUSBReadMACRegister(pAd, PBF_CFG, &pbfCfg.word);

        if ((((MACValue & 0xff) != 0) && (pbfCfg.field.Tx0QEnable)) ||
            (((MACValue & 0xff00) != 0) && (pbfCfg.field.Tx1QEnable)) ||
            (((MACValue & 0xff0000) != 0) && (pbfCfg.field.Tx2QEnable)))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("TX QUEUE 0 or 1 or 2 Not EMPTY. !!!!!!!!!!!!!!! MACValue = 0x%08X\n",MACValue));
            
            RTUSBReadMACRegister(pAd, TX_RTY_CFG, &TxRtyCfg.word);
            TxRtyCfgtmp.word = TxRtyCfg.word;
            TxRtyCfg.field.LongRtyLimit = 1;//0x4;
            TxRtyCfg.field.ShortRtyLimit = 1;//0x7;
            RTUSBWriteMACRegister(pAd, TX_RTY_CFG, TxRtyCfg.word);

            //
            // Update RTS retry limit
            //
            RTUSBReadMACRegister(pAd, TX_RTS_CFG, &RtsCfg.word);
            RtsCfgtmp.word = RtsCfg.word;
            RtsCfg.field.AutoRtsRetryLimit = 0;
            RTUSBWriteMACRegister(pAd, TX_RTS_CFG, RtsCfg.word);
#if 1   
            Index = 0;
        
            do
            {
                pbfCfg.word = 0;
                MACValue = 0;
                RTUSBReadMACRegister(pAd, PBF_CFG, &pbfCfg.word);
                RTUSBReadMACRegister(pAd, TXRXQ_PCNT, &MACValue);

                if ((((MACValue & 0xff) != 0) && (pbfCfg.field.Tx0QEnable)) ||
                    (((MACValue & 0xff00) != 0) && (pbfCfg.field.Tx1QEnable)) ||
                    (((MACValue & 0xff0000) != 0) && (pbfCfg.field.Tx2QEnable)))
                {
                }
                else
                    break;
            
                Index++;
                NdisCommonGenericDelay(1000);
            }while((Index < 330)&&(!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)));

            DBGPRINT(RT_DEBUG_TRACE, ("Clear up !!!!!!!!!!!!!!!Index[%d] (MACValue = 0x%08X) \n",Index, MACValue)); 


            // If approach to max waiting time, do channel switch once between EDCA and HCCA
            //
            if ((pAd->MccCfg.MultiChannelEnable == TRUE) && (Index == 330))
            {
                ULONG       NextHwQSEL = 0, NewStatus = 0;                              
                    
                pbfCfg.word = 0;
                MACValue = 0;
                RTUSBReadMACRegister(pAd, PBF_CFG, &pbfCfg.word);
                RTUSBReadMACRegister(pAd, TXRXQ_PCNT, &MACValue);
                    
                // EDCA full, but not enable 
                if (((MACValue & 0xff0000) != 0) && (pbfCfg.field.Tx2QEnable == FALSE))
                    NextHwQSEL = FIFO_EDCA;
                // HCCA full, but not enable 
                else if (((MACValue & 0xff00) != 0) && (pbfCfg.field.Tx1QEnable == FALSE))
                    NextHwQSEL = FIFO_HCCA;

                if (NextHwQSEL != 0)
                {
                    // STA
                    if (INFRA_ON(pAd->PortList[PORT_0]) && (NextHwQSEL == pPort->P2PCfg.STAHwQSEL))
                    {
                        pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_INFRA;
                        MultiChannelSwicthHwQAndCh(pAd, pPort->P2PCfg.STAHwQSEL, pPort->P2PCfg.InfraChannel);
                        pPort->Channel = pPort->P2PCfg.InfraChannel;
                        NewStatus = 1;
                    }
                    // CLI
                    else if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP) && (NextHwQSEL == pPort->P2PCfg.CLIHwQSEL))
                    {
                        pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_MSCLI;
                        MultiChannelSwicthHwQAndCh(pAd, pPort->P2PCfg.CLIHwQSEL, pPort->P2PCfg.P2PChannel);
                        pPort->Channel = pPort->P2PCfg.P2PChannel;
                        NewStatus = 2;
                    }
                    // GO
                    else if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON) && (NextHwQSEL == pPort->P2PCfg.GOHwQSEL))
                    {
                        pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_MSGO;
                        MultiChannelSwicthHwQAndCh(pAd, pPort->P2PCfg.GOHwQSEL, pPort->P2PCfg.GroupOpChannel);
                        pPort->Channel = pPort->P2PCfg.GroupOpChannel;
                        NewStatus = 3;
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts : NewStatus(%d), MultiChannelCurrentMode(%s) \n", __FUNCTION__, NewStatus, decodeMultiChannelMode(pAd->MccCfg.MultiChannelCurrentMode)));
                }
            }

            RTUSBReadMACRegister(pAd, TX_RTY_CFG, &TxRtyCfg.word);
            TxRtyCfg.field.LongRtyLimit = TxRtyCfgtmp.field.LongRtyLimit;
            TxRtyCfg.field.ShortRtyLimit = TxRtyCfgtmp.field.ShortRtyLimit;
            RTUSBWriteMACRegister(pAd, TX_RTY_CFG, TxRtyCfg.word);

            RTUSBReadMACRegister(pAd, TX_RTS_CFG, &RtsCfg.word);
            RtsCfg.field.AutoRtsRetryLimit = RtsCfgtmp.field.AutoRtsRetryLimit;
            RTUSBWriteMACRegister(pAd, TX_RTS_CFG, RtsCfg.word);
#endif          
        }
    }   

//#else
    if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) && !MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
    {   
        RTUSBReadMACRegister(pAd, TXRXQ_PCNT, &MACValue);
        if (((MACValue & 0xff) !=0 )||((MACValue & 0xff00) !=0 ))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("TX QUEUE 0 or 1 Not EMPTY. !!!!!!!!!!!!!!! MACValue = 0x%08X\n",MACValue));
            
            RTUSBReadMACRegister(pAd, TX_RTY_CFG, &TxRtyCfg.word);
            TxRtyCfgtmp.word = TxRtyCfg.word;
            TxRtyCfg.field.LongRtyLimit = 0x4;
            TxRtyCfg.field.ShortRtyLimit = 0x7;
            RTUSBWriteMACRegister(pAd, TX_RTY_CFG, TxRtyCfg.word);

            //
            // Update RTS retry limit
            //
            RTUSBReadMACRegister(pAd, TX_RTS_CFG, &RtsCfg.word);
            RtsCfgtmp.word = RtsCfg.word;
            RtsCfg.field.AutoRtsRetryLimit = 0;
            RTUSBWriteMACRegister(pAd, TX_RTS_CFG, RtsCfg.word);

            Index = 0;
                MACValue = 0;
            do 
            {
                RTUSBReadMACRegister(pAd, TXRXQ_PCNT, &MACValue);
                if ((MACValue & 0x00ffff) == 0)
                    break;
                Index++;
                NdisCommonGenericDelay(1000);
            }while((Index < 330)&&(!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)));

            DBGPRINT(RT_DEBUG_TRACE, ("Clear up !!!!!!!!!!!!!!!Index[%d]\n",Index));    

            RTUSBReadMACRegister(pAd, TX_RTY_CFG, &TxRtyCfg.word);
            TxRtyCfg.field.LongRtyLimit = TxRtyCfgtmp.field.LongRtyLimit;
            TxRtyCfg.field.ShortRtyLimit = TxRtyCfgtmp.field.ShortRtyLimit;
            RTUSBWriteMACRegister(pAd, TX_RTY_CFG, TxRtyCfg.word);

            RTUSBReadMACRegister(pAd, TX_RTS_CFG, &RtsCfg.word);
            RtsCfg.field.AutoRtsRetryLimit = RtsCfgtmp.field.AutoRtsRetryLimit;
            RTUSBWriteMACRegister(pAd, TX_RTS_CFG, RtsCfg.word);
        }
    }   
#endif  /* MULTI_CHANNEL_SUPPORT */

    if ((pAd->LogoTestCfg.WhckCpuUtilTestRunning == TRUE) && (pAd->LogoTestCfg.OnTestingWHQL == TRUE))
    {
        TestWHCK = TRUE;
    }

    // WPA MIC error should block association attempt for 60 seconds
    if (pAd->StaCfg.bBlockAssoc && (pAd->StaCfg.LastMicErrorTime + (60 * ONE_SECOND_TIME) < pAd->Mlme.Now64))
        pAd->StaCfg.bBlockAssoc = FALSE;


    DBGPRINT(RT_DEBUG_INFO,("MMCHK - CommonCfg.Ssid[%d]=%c%c%c%c... MlmeAux.Ssid[%d]=%c%c%c%c...\n",
            pPort->PortCfg.SsidLen, pPort->PortCfg.Ssid[0], pPort->PortCfg.Ssid[1], pPort->PortCfg.Ssid[2], pPort->PortCfg.Ssid[3],
            pAd->MlmeAux.SsidLen, pAd->MlmeAux.Ssid[0], pAd->MlmeAux.Ssid[1], pAd->MlmeAux.Ssid[2], pAd->MlmeAux.Ssid[3]));
    
    if (pAd->StaCfg.PSControl.field.DisablePS == FALSE)
        pAd->CountDowntoRadioOff = (pAd->CountDowntoRadioOff > 0 ? pAd->CountDowntoRadioOff - 1 : 0);

    // If station is idle, go to sleep
    // don't perform idle-power-save mechanism when P2P turns on

    if ((pPort->PortSubtype == PORTSUBTYPE_STA) 
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))            
        &&((pAd->pP2pCtrll->P2pMsConnectedStatus == 0) && (pAd->OpMode != OPMODE_STAP2P))
#else
        && (pAd->OpMode != OPMODE_STAP2P)
#endif
        && (IDLE_ON(pPort))
        && (TestWHCK == FALSE)
        && (P2P_OFF(pPort))
        && (pPort->Mlme.SyncMachine.CurrState == SYNC_IDLE)
        && (pPort->Mlme.CntlMachine.CurrState == CNTL_IDLE)
        && (pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF)
        && (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
        && (pAd->StaCfg.PSControl.field.EnablePSinIdle == TRUE)
        && (pPort->PortCfg.OperationMode != DOT11_OPERATION_MODE_NETWORK_MONITOR)
        && (pPort->SoftAP.bAPStart == FALSE)
        && (pAd->CountDowntoRadioOff == 0)
        && (pAd->LogoTestCfg.OnTestingWHQL == FALSE)
        && (pAd->LogoTestCfg.WhckCpuUtilTestRunning == FALSE)
        )
    {
        if (pAd->Mlme.OneSecPeriodicRound > 30)
    {
        if ((((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_14))&&(pAd->HwCfg.FirmwareVersion == 222))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d]Turn off Led Firmware version =%d)\n",__FUNCTION__,__LINE__, pAd->HwCfg.LedCntl.field.LedMode));
            LedCtrlSetLed(pAd, LED_RADIO_OFF);
        }
        if (pAd->StaCfg.PSControl.field.EnablePSinIdle == TRUE)     // stop Rx BulkIn for USB SS
        {
            DBGPRINT(RT_DEBUG_TRACE, ("AsicRadioOff....2\n"));          
            AsicRadioOff(pAd);
        }
        else
        {
                AsicSendCommanToMcu(pAd, SLEEP_MCU_CMD, 0xff, 0xff, 0x02);   // send POWER-SAVE command to MCU. Timeout unit:40us.
            OPSTATUS_SET_FLAG(pAd, fOP_STATUS_DOZE);
        }
        DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d]PSM - Issue Sleep command, USB SS=%d)\n",__FUNCTION__,__LINE__, pAd->StaCfg.PSControl.field.EnablePSinIdle));
    }
    }
    else
    {

        DBGPRINT(RT_DEBUG_INFO,("MMCHK - CommonCfg.Ssid[%d]=%c%c%c%c... MlmeAux.Ssid[%d]=%c%c%c%c...\n",
            pPort->PortCfg.SsidLen, pPort->PortCfg.Ssid[0], pPort->PortCfg.Ssid[1], pPort->PortCfg.Ssid[2], pPort->PortCfg.Ssid[3],
            pAd->MlmeAux.SsidLen, pAd->MlmeAux.Ssid[0], pAd->MlmeAux.Ssid[1], pAd->MlmeAux.Ssid[2], pAd->MlmeAux.Ssid[3]));
    }

    //start PM4 here
    if (INFRA_ON(pActivePort)
        && ((pActivePort->PortSubtype == PORTSUBTYPE_STA)
        || (pActivePort->PortSubtype == PORTSUBTYPE_P2PClient)))
    {
        // Is PSM bit consistent with user power management policy?
        // This is the only place that will set PSM bit ON.
        // Move the check code to here, to do the power save more aggressively.
        MlmeCheckPsmChange(pAd,pActivePort, pAd->Mlme.Now64);
    }

#if 0   // Rcpi in WTBL has problem. Don't use it.
    // every 500ms
    if ((pAd->Mlme.PeriodicRound % 5) == 0)
    {
        MtAsicRssiUpdate(pAd);
    }
#else
    if (pAd->Mlme.PeriodicRound % 5)
    {
        MAC_TABLE_ENTRY *pEntry;
        pEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, ROLE_WLANIDX_BSSID);
        
        pEntry->RssiSample.AvgRssi[0] = pEntry->RssiSample.LastRssi[0] = pAd->StaCfg.RssiSample.AvgRssi[0];
        pEntry->RssiSample.AvgRssi[1] = pEntry->RssiSample.LastRssi[1] = pAd->StaCfg.RssiSample.AvgRssi[0];
        pEntry->RssiSample.AvgRssi[2] = pEntry->RssiSample.LastRssi[2] = pAd->StaCfg.RssiSample.AvgRssi[0];
    }
#endif

    // add the most up-to-date h/w raw counters into software variable, so that
    // the dynamic tuning mechanism below are based on most up-to-date information
    NICUpdateRawCounters(pAd);

#if 0    
    if (pPort->CommonCfg.PSPXlink && ADHOC_ON(pPort))
    {
    }
    else
    {
        // only apply to STA, SoftAP needs not to tune
        // dynamic tune BBP R66 to find a balance between sensibility and noise isolation
        AsicBbpTuning(pAd,pActivePort);
    }
#endif

    for (uPortNum = 0; uPortNum < RTMP_MAX_NUMBER_OF_PORT/*pAd->NumberOfPorts*/; uPortNum++)
    {
        if (pAd->PortList[uPortNum])
        {
            if ((pAd->PortList[uPortNum]->bActive) &&
                ((pAd->PortList[uPortNum]->PortSubtype == PORTSUBTYPE_STA) ||
                (pAd->PortList[uPortNum]->PortSubtype == PORTSUBTYPE_P2PClient)))
            {
                ORIBATimerTimeout(pAd, pAd->PortList[uPortNum]);
            }
        }
    }


    // The time period for checking antenna is according to traffic
    if (pPort->CommonCfg.bPPAD == FALSE)
    {
        if (pAd->HwCfg.NicConfig2.field.AntDiv == ANT_DIV_CONFIG_ENABLE)
        {
            if (pAd->Mlme.OneSecPeriodicRound % 2 == 1)
            {
                // check every 2 second. If rcv-beacon less than 5 in the past 2 second, then AvgRSSI is no longer a 
                // valid indication of the distance between this AP and its clients.
                if (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED)) 
                {
                    SHORT   realavgrssi1;

                    // if no traffic then reset average rssi to trigger evaluation
                    if (pAd->StaCfg.NumOfAvgRssiSample < 5)
                    {
                        pAd->HwCfg.RxAnt.Pair1LastAvgRssi = (-99);
                        pAd->HwCfg.RxAnt.Pair2LastAvgRssi = (-99);
                        DBGPRINT(RT_DEBUG_TRACE, ("MlmePeriodicExecTimerCallback: no traffic/beacon, reset RSSI\n"));
                    }

                    pAd->StaCfg.NumOfAvgRssiSample = 0;
                    realavgrssi1 = (pAd->HwCfg.RxAnt.Pair1AvgRssi[pAd->HwCfg.RxAnt.Pair1PrimaryRxAnt] >> 3);

                    DBGPRINT(RT_DEBUG_TRACE,("Ant-realrssi0(%d), Lastrssi0(%d)\n", realavgrssi1, pAd->HwCfg.RxAnt.Pair1LastAvgRssi));

                    // if the difference between two rssi is larger or less than 5, then evaluate the other antenna
                    if ((realavgrssi1 > (pAd->HwCfg.RxAnt.Pair1LastAvgRssi + 5)) || (realavgrssi1 < (pAd->HwCfg.RxAnt.Pair1LastAvgRssi - 5)))
                    {
                        pAd->HwCfg.RxAnt.Pair1LastAvgRssi = realavgrssi1;
                        AsicEvaluateRxAnt(pAd,pPort);
                    }
                }
                else
                {
                    // if not connected, always switch antenna to try to connect
                    UCHAR   temp;

                    temp = pAd->HwCfg.RxAnt.Pair1PrimaryRxAnt;
                    pAd->HwCfg.RxAnt.Pair1PrimaryRxAnt = pAd->HwCfg.RxAnt.Pair1SecondaryRxAnt;
                    pAd->HwCfg.RxAnt.Pair1SecondaryRxAnt = temp;

                    DBGPRINT(RT_DEBUG_TRACE, ("MlmePeriodicExecTimerCallback: no connect, switch to another one to try connection"));

                    AsicSetRxAnt(pAd, pAd->HwCfg.RxAnt.Pair1PrimaryRxAnt);
                }
            }
        }
        else
        {
            if (pAd->Mlme.bEnableAutoAntennaCheck)
            {
                TxTotalCnt = pAd->Counter.MTKCounters.OneSecTxNoRetryOkCount + 
                             pAd->Counter.MTKCounters.OneSecTxRetryOkCount + 
                             pAd->Counter.MTKCounters.OneSecTxFailCount;

                // dynamic adjust antenna evaluation period according to the traffic
                if (TxTotalCnt > 50)
                {
                    if (pAd->Mlme.OneSecPeriodicRound % 10 == 0)
                    {
                        AsicEvaluateRxAnt(pAd,pPort);
                    }
                }
                else
                {
                    if (pAd->Mlme.OneSecPeriodicRound % 3 == 0)
                    {
                        AsicEvaluateRxAnt(pAd,pPort);
                    }
                }
            }
        }
    }

    // If another client port connected with WLAN AP, need to calculate its channel quality
    if ((pPort->PortNumber == PORT_0) && 
        (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) && 
        (pAd->StaCfg.bFastRoamingScan == FALSE) &&
        (!MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION)))
    {
        for (idx = 1; idx < RTMP_MAX_NUMBER_OF_PORT; idx ++)
        {
            if ((pAd->PortList[idx] == NULL) || (pAd->PortList[idx]->bActive == FALSE))
                continue;

            if ((IS_P2P_CON_CLI(pAd, pAd->PortList[idx]) || IS_P2P_MS_CLI(pAd, pAd->PortList[idx]) || IS_P2P_MS_CLI2(pAd, pAd->PortList[idx])) &&
                INFRA_ON(pAd->PortList[idx]))
            {
                P2pCalculateChannelQuality(pAd, pAd->PortList[idx], pAd->Mlme.Now64);

                // Support one client with RalinkUI
                if (IS_P2P_CON_CLI(pAd, pAd->PortList[idx]))
                    break;
            }
        }
    }
    
    if (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED) && 
        (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)))
    {
        DBGPRINT(RT_DEBUG_INFO, ("channel stay(%d) \n", pAd->Mlme.ChannelStayCount));
        pAd->Mlme.ChannelStayCount++;

        // update channel quality for Roaming/Fast-Roaming and UI LinkQuality display
        if (pAd->StaCfg.bFastRoamingScan == FALSE)  // bFastRoamingScan True means scan is not completed 
        {
            // The NIC may lost beacons during scaning operation.
            MlmeCalculateChannelQuality(pAd, pPort, pAd->Mlme.Now64);
        }
    }

    pAd->Counter.MTKCounters.LastOneSecRxOkDataCnt = pAd->Counter.MTKCounters.OneSecRxOkDataCnt;
    pAd->Counter.MTKCounters.LastOneSecTotalTxCount = pAd->Counter.MTKCounters.OneSecTxNoRetryOkCount + 
                                                pAd->Counter.MTKCounters.OneSecTxRetryOkCount + 
                                                pAd->Counter.MTKCounters.OneSecTxFailCount;
    RxTotalCnt = pAd->Counter.MTKCounters.OneSecRxOkDataCnt;
    pAd->Counter.MTKCounters.Accu30SecTotalRxCount  = RxTotalCnt + pAd->Counter.MTKCounters.Accu30SecTotalRxCount;
    // Update counter for 30-second period RX OK Data count.
    if ((pAd->Mlme.OneSecPeriodicRound % 30) == 0)
    {
        pAd->Counter.MTKCounters.Last30SecTotalRxCount = pAd->Counter.MTKCounters.Accu30SecTotalRxCount;
        pAd->Counter.MTKCounters.Accu30SecTotalRxCount = 0;
    }
    // Check for recovering TxRty value, both in Adhoc and Infra mode
    if (INFRA_ON(pActivePort) || ADHOC_ON(pActivePort))
    {

    //*****************************//
     
        if ((IS_P2P_CON_CLI(pAd, pActivePort) || IS_P2P_MS_CLI(pAd, pActivePort) || IS_P2P_MS_CLI2(pAd, pActivePort))/* && pEntry->ValidAsCLI*/)
        {
            pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, P2pGetClientWcid(pAd, pPort));
            
            if((pWcidMacTabEntry != NULL) && (pWcidMacTabEntry->ValidAsCLI == TRUE))
            {    
            LastBeaconRxTime = pWcidMacTabEntry->LastBeaconRxTime;
        }
        else
        {
            LastBeaconRxTime = pAd->StaCfg.LastBeaconRxTime;
        }
        }
        else
        {
            LastBeaconRxTime = pAd->StaCfg.LastBeaconRxTime;
        }
    
        if (OPSTATUS_TEST_FLAG(pActivePort, fOP_STATUS_MEDIA_STATE_CONNECTED))
        {
            // patch for IOT with BRCM AP, may get DelBA from AP and cause throughput drop down
            if ((pPort->CommonCfg.DelBACount > 1) &&
                (pPort->CommonCfg.DelBACount < 5))
            {
                ULONG Data;
                pPort->CommonCfg.DelBACount++;

                // 1. BAWinSize
                RTUSBWriteMACRegister(pAd, MAX_LEN_CFG, 0x000A1fff);

                // 2. TxOP
                RTUSBReadMACRegister(pAd, EDCA_AC0_CFG, &Data);
                Data &= 0xFFFFFF00;
                Data |= 0x38;
                RTUSBWriteMACRegister(pAd, EDCA_AC0_CFG, Data);
            }

            //
            // Lost beacon more than one second
            //
            // in noise environment, retry too long may cause normal Auth/Assoc Request sent too late and casue time out
            // do not change the retry limit when trying to connect
            if ((pActivePort->Mlme.CntlMachine.CurrState == CNTL_IDLE) &&
                ((LastBeaconRxTime + ONE_SECOND_TIME < pAd->Mlme.Now64) &&
                (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) &&
                OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_LONG_RETRY)))
            {
                //
                // Change retry setting
                //
                RTUSBReadMACRegister(pAd, TX_RTY_CFG, &TxRtyCfg.word);
                TxRtyCfg.field.LongRtyLimit = 0x7;
                TxRtyCfg.field.ShortRtyLimit = 0x7;
                RTUSBWriteMACRegister(pAd, TX_RTY_CFG, TxRtyCfg.word);

                // Update RTS retry limit
                RTUSBReadMACRegister(pAd, TX_RTS_CFG, &RtsCfg.word);
                RtsCfg.field.AutoRtsRetryLimit = 1;
                RTUSBWriteMACRegister(pAd, TX_RTS_CFG, RtsCfg.word);
                OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_LONG_RETRY);
                DBGPRINT(RT_DEBUG_TRACE, ("%s: recover TX_RTY_CFG",__FUNCTION__));
            }
            else
            {
                //
                // Received beacon within one second
                //
                if (((!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_LONG_RETRY)) && 
                    (LastBeaconRxTime + ONE_SECOND_TIME > pAd->Mlme.Now64)))
                {
                    //
                    // Change retry setting
                    //
                    // do not set long retry if trying to connect an AP
                    if  (pActivePort->Mlme.CntlMachine.CurrState == CNTL_IDLE && !IS_P2P_PSM(pPort))
                    {
                        //
                        // Change retry setting
                        //
                        RTUSBReadMACRegister(pAd, TX_RTY_CFG, &TxRtyCfg.word);
                        TxRtyCfg.field.LongRtyLimit = 0x7;
                        TxRtyCfg.field.ShortRtyLimit = 0x7;
                        RTUSBWriteMACRegister(pAd, TX_RTY_CFG, TxRtyCfg.word);
    
                        // Update RTS retry limit
                        RTUSBReadMACRegister(pAd, TX_RTS_CFG, &RtsCfg.word);
                        RtsCfg.field.AutoRtsRetryLimit = 0x20;
                        RTUSBWriteMACRegister(pAd, TX_RTS_CFG, RtsCfg.word);
                        
                        OPSTATUS_SET_FLAG(pAd, fOP_STATUS_LONG_RETRY);
                        DBGPRINT(RT_DEBUG_TRACE, ("%s: recover TX_RTY_CFG",__FUNCTION__));
                    }
                }
            }
        }

    }
    
    // If another client port connected with WLAN AP, need to check some of link status
    if (pPort->PortNumber == PORT_0)
    {
        for (idx = 1; idx < RTMP_MAX_NUMBER_OF_PORT; idx ++)
        {
            if ((pAd->PortList[idx] == NULL) || (pAd->PortList[idx]->bActive == FALSE))
                continue;

            if ((IS_P2P_CON_CLI(pAd, pAd->PortList[idx]) || IS_P2P_MS_CLI(pAd, pAd->PortList[idx]) || IS_P2P_MS_CLI2(pAd, pAd->PortList[idx])) &&
                INFRA_ON(pAd->PortList[idx]))
            {
                PMP_PORT      pP2PPort = pAd->PortList[idx];

                // Send out a NULL frame every 10 sec to inform AP that STA is still alive (Avoid being age out)
                // When APSD is enabled, the period changes as 20 sec
                if (pPort->CommonCfg.bAPSDCapable && pPort->CommonCfg.APEdcaParm.bAPSDCapable)
                {
                    if (((pAd->Mlme.OneSecPeriodicRound % 20) == 8) && (IS_P2P_SIGMA_OFF(pPort)))
                        XmitSendNullFrame(pAd, pP2PPort, pPort->CommonCfg.TxRate, TRUE);
                }
                else
                {
                    if ((pAd->Mlme.OneSecPeriodicRound % 10) == 8)
                        XmitSendNullFrame(pAd, pP2PPort, pPort->CommonCfg.TxRate, FALSE);
                }

                // CQI is dead
                if (CQI_IS_DEAD(pP2PPort->Mlme.ChannelQuality))
                {
                    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, P2pGetClientWcid(pAd, pP2PPort));
                    
                    if(pWcidMacTabEntry == NULL)
                    {
                        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, P2pGetClientWcid(pAd, pP2PPort)));
                        return;
                    }
                    
                    if (pWcidMacTabEntry->LastBeaconRxTime + P2P_BEACON_LOST_TIME <= pAd->Mlme.Now64)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("(Wcid=%d)MMCHK - No BEACON. Dead CQI. Disassociate to AP \n", P2pGetClientWcid(pAd, pP2PPort)));
                        pAd->MlmeAux.AssocState = dot11_assoc_state_unauth_unassoc;
                        PlatformIndicateDisassociation(pAd, pP2PPort, pP2PPort->PortCfg.Bssid, DOT11_ASSOC_STATUS_UNREACHABLE);
                        // Lost AP, send disconnect & link down event
                        pPort->StaCfg.WscControl.PeerDisconnect = TRUE;

                        if (IS_P2P_CLIENT_OP(pPort) || (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP)))
                            MlmeCntLinkDown(pP2PPort, TRUE);
                        else
                            MlmeCntLinkDown(pP2PPort, FALSE);
                        // TODO: we have deleted auto-reconnection in p2plinkdown 
                        //MlmeAutoReconnectLastSSID(pAd, pP2PPort);
                    }
                }
            }
        }
    }
    
    if (INFRA_ON(pPort))
    {       
        // Check DLS time out, then tear down those session
        RTMPCheckDLSTimeOut(pAd, pPort);
    
        // TDLS direct link maintenance
        if (TDLS_ON(pAd) /*&& (pAd->Mlme.PeriodicRound % MLME_TASK_EXEC_MULTIPLE == 0)*/)
        {
            TdlsLinkMaintenance(pAd);
            TdlsSearchTabMaintain(pAd);
        }

    
        // Is PSM bit consistent with user power management policy?
        // This is the only place that will set PSM bit ON.
        //MlmeCheckPsmChange(pAd, pAd->Mlme.Now64);

// 2011-12-05 For P2pMs multi-channel
#ifdef MULTI_CHANNEL_SUPPORT
#else
        if ((pAd->StaCfg.LastBeaconRxTime + ONE_SECOND_TIME < pAd->Mlme.Now64) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) && 
            (pAd->StaCfg.bFastRoamingScan == FALSE) &&
            ((TxTotalCnt + pAd->Counter.MTKCounters.OneSecRxOkCnt < 600)) &&
            (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_IDLE))
        {
            RTMPSetAGCInitValue(pAd, BW_20);

            BbUpdateRxAgcVgaForLongDistance(pAd);

            DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - No BEACON. restore R66 to the low bound(%d) \n", (0x2E + GET_LNA_GAIN(pAd))));
        }

//
//      Move the flowing code to XmitHardTransmit, after prepare EAPOL frame that idicated MIC error
//      To meet the WiFi Test Plan "The STAUT must deauthenticate itself from the AP".
//      Also need to bloc association frame.
//
//      The flowing code on here can't send disassociation frame to the AP, since AP may send deauthentication
//      frame, on the meanwhile, station will call link down and then this code can't be performed.
//
//      // Check for EAPOL frame sent after MIC countermeasures
//      if (pAd->StaCfg.MicErrCnt >= 3)
//      {
//  
//          MLME_DISASSOC_REQ_STRUCT    DisassocReq;
//
//          // disassoc from current AP first
//          DBGPRINT(RT_DEBUG_TRACE, ("MLME - disassociate with current AP after sending second continuous EAPOL frame\n"));
//          MlmeCntDisassocParmFill(pAd, &DisassocReq, pAd->PortCfg.Bssid, REASON_MIC_FAILURE);
//          MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ, 
//                      sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq);
//
//          pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;
//          pAd->StaCfg.bBlockAssoc = TRUE;
//      }
#endif /* MULTI_CHANNEL_SUPPORT */
        
        // Send out a NULL frame every 10 sec to inform AP that STA is still alive (Avoid being age out)
        // When APSD is enabled, the period changes as 20 sec
        if (pPort->CommonCfg.bAPSDCapable && pPort->CommonCfg.APEdcaParm.bAPSDCapable)
        {
            if (((pAd->Mlme.OneSecPeriodicRound % 20) == 8) && (IS_P2P_SIGMA_OFF(pPort)))
            {
                // send to BSS AP
                XmitSendNullFrame(pAd,pPort, pPort->CommonCfg.TxRate, TRUE);
            }

            if (TDLS_ON(pAd))
            {
                // Send null frame to Peer TDLS in the spread of time.
                // Protect from occupied bulkout null queue.
                for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++)
                {
                    if ((pAd->Mlme.OneSecPeriodicRound % 20) == (UCHAR)(8 - MAX_NUM_OF_TDLS_ENTRY + idx))
                    {
                        pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg.TDLSEntry[idx];

                        if ((pTDLS->Valid) && (pTDLS->Status == TDLS_MODE_CONNECTED))
                        {
                            pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, pTDLS->Wcid);
                            if(pWcidMacTabEntry == NULL)
                            {
                                DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, pTDLS->Wcid));
                                return;
                            }
                            
                            if (pWcidMacTabEntry->ValidAsCLI)
                                XmitSendNullFrameForDirect(pAd, pPort, pWcidMacTabEntry->MinPhyCfg, FALSE, pTDLS->MacAddr);
                        }
                    }
                }
            }

        }
        else
        {
            if ((pAd->Mlme.OneSecPeriodicRound % 10) == 8)
            {
                // send to BSS AP
                //XmitSendNullFrame(pAd, pPort,pPort->CommonCfg.TxRate, FALSE);
            }

            if (TDLS_ON(pAd))
            {
                // Send null frame to Peer TDLS in the spread of time.
                // Protect from occupied bulkout null queue.
                for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++)
                {
                    if ((pAd->Mlme.OneSecPeriodicRound % 10) == (UCHAR)(8 - MAX_NUM_OF_TDLS_ENTRY + idx))
                    {
                        pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg.TDLSEntry[idx];

                        if ((pTDLS->Valid) && (pTDLS->Status == TDLS_MODE_CONNECTED))
                        {
                            pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, pTDLS->Wcid);
                            if(pWcidMacTabEntry == NULL)
                            {
                                DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, pTDLS->Wcid));
                                return;
                            }
                            
                            if (pWcidMacTabEntry->ValidAsCLI)
                            {
                                //XmitSendNullFrameForDirect(pAd, pPort, pPort->MacTab.Content[pTDLS->Wcid].MinHTPhyMode, FALSE, pTDLS->MacAddr);
                                XmitSendNullFrameForDirect(pAd, pPort, pWcidMacTabEntry->MinPhyCfg, FALSE, pTDLS->MacAddr);
                            }
                        }
                    }
                }
            }

        }

#ifdef MULTI_CHANNEL_SUPPORT
        if ((TestWHCK == FALSE) && (pAd->MccCfg.MultiChannelEnable == FALSE))
#else
        if (TestWHCK == FALSE)
#endif /*MULTI_CHANNEL_SUPPORT*/
        {
            //if (CQI_IS_DEAD(pPort->Mlme.ChannelQuality))
            if(bRoaming)
            {
#ifdef MULTI_CHANNEL_SUPPORT    
                //maybe beacon loss is due to scanning
                if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) &&
                    (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION) == FALSE))              
#else
                //maybe beacon loss is due to scanning
                if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))               
#endif
                {
                    // when NoA is ON, beacon lost time not so strict.
                    // when NoA is OFF, keep the old rule.
                    // pPort->P2PCfg.NoAIndex >= MAX_LEN_OF_MAC_TABLE means NoA is Off.
                    if ((pPort->P2PCfg.NoAIndex >= MAX_LEN_OF_MAC_TABLE)
                        || ((pPort->P2PCfg.NoAIndex < MAX_LEN_OF_MAC_TABLE) && (pAd->StaCfg.LastBeaconRxTime + STA_BEACON_LOST_TIME <= pAd->Mlme.Now64)))
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - No BEACON. Dead CQI. Auto Recovery attempt #%d bssid=%2x:%2x:%2x\n", pAd->Counter.MTKCounters.BadCQIAutoRecoveryCount
                            ,pPort->PortCfg.Bssid[3],pPort->PortCfg.Bssid[4],pPort->PortCfg.Bssid[5]));
                        if(pPort->CommonCfg.RoamingCnt <MAX_ROAMING_RETRY_COUNT)
                        {
                            if(!IDLE_ON(pPort))
                            {
                                pPort->CommonCfg.RoamingCnt++;
                                //MlmeCheckForFastRoaming(pAd, pAd->Mlme.Now64);
                                MlmeCheckForRoaming(pAd, pPort, pAd->Mlme.Now64);
                            }        
                        }
                        else
                        {
                        pAd->MlmeAux.AssocState = dot11_assoc_state_unauth_unassoc;
                        PlatformIndicateDisassociation(pAd,pPort, pPort->PortCfg.Bssid, DOT11_ASSOC_STATUS_UNREACHABLE);
                        DBGPRINT(RT_DEBUG_TRACE, ("TONDIS:  STAMlmePeriodicExec::CQI_IS_DEAD, PlatformIndicateDisassociation Reason=0x%08x, [%02x:%02x:%02x:%02x:%02x:%02x]\n",
                                        DOT11_ASSOC_STATUS_UNREACHABLE, pPort->PortCfg.Bssid[0], pPort->PortCfg.Bssid[1], 
                                        pPort->PortCfg.Bssid[2], pPort->PortCfg.Bssid[3], pPort->PortCfg.Bssid[4], pPort->PortCfg.Bssid[5]));                                   
                        // Lost AP, send disconnect & link down event
                        pPort->StaCfg.WscControl.PeerDisconnect = TRUE;
                        if (IS_P2P_CLIENT_OP(pPort) || (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP)))
                            MlmeCntLinkDown(pPort, TRUE);
                        else
                            MlmeCntLinkDown(pPort, FALSE);                
                            pPort->CommonCfg.RoamingCnt =0;
                        MlmeAutoReconnectLastSSID(pAd, pPort);  
                    }

                    }
                }
            }       
            else if (CQI_IS_BAD(pPort->Mlme.ChannelQuality))
            {
                pAd->Counter.MTKCounters.BadCQIAutoRecoveryCount ++;
                DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - Bad CQI. Auto Recovery attempt #%d\n", pAd->Counter.MTKCounters.BadCQIAutoRecoveryCount));
                MlmeAutoReconnectLastSSID(pAd, pPort);
            }
        }

        // Add auto seamless roaming for CCX
        {
            SHORT   dBmToRoam = (-1)*(SHORT)pAd->StaCfg.CCXControl.field.dBmToRoam;
            CHAR    MaxRssi = RTMPMaxRssi(pAd, pAd->StaCfg.LastRssi, pAd->StaCfg.LastRssi2, pAd->StaCfg.LastRssi3);
            DBGPRINT(RT_DEBUG_INFO, ("MaxRssi=%d, dBmToRoam=%d\n", MaxRssi, dBmToRoam));

            if ((pAd->StaCfg.CCXControl.field.FastRoamEnable) && (MaxRssi <= (CHAR)dBmToRoam))
            {
                MlmeCheckForFastRoaming(pAd, pAd->Mlme.Now64);
            }
        }       
    }
    else if (ADHOC_ON(pPort))
    {
        //radar detect
        if ((pPort->CommonCfg.bIEEE80211H == 1) && RadarChannelCheck(pAd, pPort->Channel))
        {
            // need to check channel availability, after switch channel
            if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
            {
                pPort->CommonCfg.RadarDetect.RDCount++;
                
                // channel availability check time is 60sec
                if (pPort->CommonCfg.RadarDetect.RDCount > 65)
                {
                    if (RadarDetectionStop(pAd))
                    {
                        pAd->UiCfg.ExtraInfo = DETECT_RADAR_SIGNAL;
                        pPort->CommonCfg.RadarDetect.RDCount = 0;     // stat at silence mode and detect radar signal
                        DBGPRINT(RT_DEBUG_TRACE, ("Found radar signal!!!\n\n"));
                    }
                    else
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("Not found radar signal, start send beacon\n"));
                        MtAsicEnableIbssSync(pAd);
                        pPort->CommonCfg.RadarDetect.RDMode = RD_NORMAL_MODE;
                    }
                }
            }
        }

#ifndef SINGLE_ADHOC_LINKUP
        // If all peers leave, and this STA becomes the last one in this IBSS, then change MediaState
        // to DISCONNECTED. But still holding this IBSS (i.e. sending BEACON) so that other STAs can
        // join later.
        if ((pAd->StaCfg.LastBeaconRxTime + ADHOC_BEACON_LOST_TIME < pAd->Mlme.Now64) &&
            OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED))
        {
            PQUEUE_ENTRY    pQueueEntry;
            PMAC_TABLE_ENTRY pMacEntry = NULL;
            PMAC_TABLE_ENTRY pNextMacEntry = NULL;   
            PQUEUE_HEADER pHeader;
            
            DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - excessive BEACON lost, last STA in this IBSS, MediaState=Disconnected\n")); 

            OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED);   
            
            // clean up previous SCAN result, add current BSS back to table if any
            BssTableDeleteEntry(&pAd->ScanTab, pPort->PortCfg.Bssid, pPort->Channel);
            // ad-hoc linkdown Reset MAC Table. Start from 1 or 0 ?
            pHeader = &pPort->MacTab.MacTabList;
            pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
            while (pNextMacEntry != NULL)
            {
                pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
                
                if(pMacEntry == NULL)
                {
                    break; 
                }
                
                if(pMacEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST)
                {
                    pNextMacEntry = pNextMacEntry->Next;   
                    pMacEntry = NULL;
                    continue; 
                }
                
                 if (pPort->MacTab.Size == 0)
                    break;
                
                if (pMacEntry->ValidAsCLI == TRUE)
                {
                    MtAsicDelWcidTab(pAd,(UCHAR) pMacEntry->wcid);                  
                    MacTableDeleteAndResetEntry(pAd, pPort, pMacEntry->wcid, pMacEntry->Addr, TRUE);                    
                }
                pNextMacEntry = pNextMacEntry->Next;   
                pMacEntry = NULL;
            }   

            PlatformIndicateDisassociation(pAd, pPort,BROADCAST_ADDR, DOT11_DISASSOC_REASON_PEER_UNREACHABLE);          

            // reset its role
            pAd->StaCfg.AdhocJoiner  = FALSE;
            pAd->StaCfg.AdhocCreator = FALSE;

            // Disable supported Ht rate
            pAd->StaActive.SupportedHtPhy.bHtEnable = FALSE;


            pAd->StaCfg.LastScanTime = pAd->Mlme.Now64;         

            //
            // Clear Adhoc connect list.
            //
            NdisAcquireSpinLock(&pAd->StaCfg.AdhocListLock);
            pQueueEntry = RemoveHeadQueue(&pAd->StaCfg.ActiveAdhocListQueue);
            while (pQueueEntry)
            {
                InsertTailQueue(&pAd->StaCfg.FreeAdhocListQueue, pQueueEntry);
                pQueueEntry = RemoveHeadQueue(&pAd->StaCfg.ActiveAdhocListQueue);
            }           
            NdisReleaseSpinLock(&pAd->StaCfg.AdhocListLock);            
        }
        else
        {
            //check adhoc time stamp
            MlmeInfoMantainAdhcoList(pAd, pPort,0);
        }
#endif
    }
    else if (TestWHCK == FALSE)// no INFRA nor ADHOC connection
    {
        if((pAd->Mlme.OneSecPeriodicRound % 61) == 60)
        {
            if (pPort->bRoamingAfterResume && pAd->LogoTestCfg.OnTestingWHQL) // to avoid can't stop whql testing.
                pPort->bRoamingAfterResume = FALSE;
        }

        DBGPRINT(RT_DEBUG_INFO, ("MLME periodic exec, no association so far\n"));
        DBGPRINT(RT_DEBUG_TRACE, ("pAd->StaCfg.bAutoReconnect(%d)\n", pAd->StaCfg.bAutoReconnect));
        
        if ((pAd->StaCfg.bAutoReconnect == TRUE) && 
            (MlmeValidateSSID(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen) == TRUE))
        {
            if (pPort->bRoamingAfterResume && pPort->CurrentBssEntry.LastBeaconRxTime != 0)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("[080821]call MlmeAutoReconnectLastSSID...\n"));  
                PlatformMoveMemory(&pAd->ScanTab.BssEntry[0], &pPort->CurrentBssEntry, sizeof(BSS_ENTRY));
                // If ScanTab is empty, add one for CurrentBssEntry
                if (pAd->ScanTab.BssNr == 0)
                    pAd->ScanTab.BssNr += 1;

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
                // Win8 Check if this New BSS entry is one of NLO entries
                // 1.) NLO enable and not set to indicate yet
                if (NDIS_WIN8_ABOVE(pAd) && (pAd->StaCfg.NLOEntry.NLOEnable == TRUE) && (pAd->StaCfg.NLOEntry.IndicateNLO == FALSE))
                {
                    PDOT11_OFFLOAD_NETWORK pNLO = NULL;
                    ULONG idxOffloadEntry = 0;
                    PBSS_ENTRY pBssEntry = &pAd->ScanTab.BssEntry[0];

                    for (idxOffloadEntry = 0; idxOffloadEntry < pAd->StaCfg.NLOEntry.uNumOfEntries; idxOffloadEntry++)
                    {
                        pNLO= &pAd->StaCfg.NLOEntry.OffloadNetwork[idxOffloadEntry];

                        // check SSID, Cipher and AuthMode
                        if (!SSID_EQUAL(pBssEntry->Ssid, pBssEntry->SsidLen, pNLO->Ssid.ucSSID, pNLO->Ssid.uSSIDLength))
                            continue;

                        if ((pBssEntry->AuthMode != (ULONG)(pNLO->AuthAlgo)) && (pBssEntry->AuthModeAux != (ULONG)(pNLO->AuthAlgo)))
                            continue;

                        if (pBssEntry->WepStatus != (ULONG)(pNLO->UnicastCipher))
                            continue;

                        // To Consider Connected state
                        if (OPSTATUS_TEST_FLAG(pPort,fOP_STATUS_MEDIA_STATE_CONNECTED))
                        {

                        }

                        // the new BssEntry is in NLO entry, need to indicate NLO event after scan done
                        pAd->StaCfg.NLOEntry.IndicateNLO = TRUE;

                        DBGPRINT(RT_DEBUG_TRACE, ("%s: NLO: Found NLO network\n", __FUNCTION__));
                    }
                }
#endif
                DBGPRINT(RT_DEBUG_TRACE, ("MlmeAutoReconnectLastSSID_1, PortNum(%d)\n",pPort->PortNumber));
                MlmeAutoReconnectLastSSID(pAd, pPort);
            }
            else if ((pAd->ScanTab.BssNr==0) && (pPort->Mlme.CntlMachine.CurrState == CNTL_IDLE))
            {
                MLME_SCAN_REQ_STRUCT       ScanReq;

                if ((pAd->StaCfg.LastScanTime + TEN_SECOND) < pAd->Mlme.Now64)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d]CNTL - No matching BSS, start a new ACTIVE scan SSID[%s]\n", __FUNCTION__, __LINE__, pAd->MlmeAux.AutoReconnectSsid));
                    MlmeCntScanParmFill(pAd, &ScanReq, pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen, BSS_ANY, FAST_SCAN_ACTIVE);
                    MlmeEnqueue(pAd,pPort, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq);
                    pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_LIST_SCAN;
                    // Reset Missed scan number
                    pAd->StaCfg.LastScanTime = pAd->Mlme.Now64;
                }
                else if (pAd->StaCfg.BssType == BSS_ADHOC)  // Quit the forever scan when in a very clean room
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("MlmeAutoReconnectLastSSID_2, PortNum(%d)\n",pPort->PortNumber));
                    MlmeAutoReconnectLastSSID(pAd, pPort);
            }
            }
            else if (pPort->Mlme.CntlMachine.CurrState == CNTL_IDLE )
            {
                if ((pAd->Mlme.OneSecPeriodicRound % 10) == 7)
                {
                    if ((pAd->StaCfg.LastScanTime + TEN_SECOND) < pAd->Mlme.Now64)
                    {
                        MlmeAutoScan(pAd);
                        pAd->StaCfg.LastScanTime = pAd->Mlme.Now64;
                    }
                }
                else
                {   // avoid Auto Reconnect to inturrept other port connection
                    if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
                    {
                        //prevent soft AP disturbed by auto connect process
                        if (pAd->MlmeAux.ScanForConnectCnt >= pAd->StaCfg.KeepReconnectCnt)
                        {
                            DBGPRINT(RT_DEBUG_INFO, ("AutoReconnect hit the max times %d\n",pAd->MlmeAux.ScanForConnectCnt));
                            pAd->MlmeAux.AutoReconnectSsidLen= 32;
                            PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);
                        }
                        else
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("MlmeAutoReconnectLastSSID_3, PortNum(%d)\n",pPort->PortNumber));
                            MlmeAutoReconnectLastSSID(pAd, pPort);
                        }
                    }
                }

                DBGPRINT(RT_DEBUG_INFO, ("pAd->StaCfg.bAutoReconnect is TRUE\n"));
            }
        }   
            else if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) && 
               (pPort->Mlme.CntlMachine.CurrState == CNTL_IDLE ) &&
               (pAd->StaCfg.bAutoReconnect == FALSE) && (pPort->StaCfg.WscControl.WscState == WSC_STATE_START) &&
               ((pPort->P2PCfg.bGOStart == FALSE) ||  (IS_P2P_GO_OP(pPort)))) 
            {
                if ((pAd->Mlme.OneSecPeriodicRound % 10) == 7)
                {
                    if ((pAd->StaCfg.LastScanTime + 10 * 1000) < pAd->Mlme.Now64)
                    {
                        MlmeAutoScan(pAd);
                        pAd->StaCfg.LastScanTime = pAd->Mlme.Now64;
                    }
                }
                else
                {
                    if (pPort->CommonCfg.bCarrierDetect)
                    {
                        if ((pAd->Mlme.OneSecPeriodicRound % 5) == 1)
                                MlmeCntlWscIterate(pAd, pPort);
                    }
                    else
                    {
                            MlmeCntlWscIterate(pAd, pPort);
                    }
                }
            }
    }

    if (((pAd->Mlme.OneSecPeriodicRound % 2) == 0) &&
        (INFRA_ON(pPort) || ADHOC_ON(pPort)))
    {
        //LedCtrlSetSignalLed(pAd, RTMPMaxRssi(pAd, (CHAR)pAd->StaCfg.AvgRssi, (CHAR)pAd->StaCfg.AvgRssi2, (CHAR)pAd->StaCfg.AvgRssi3));

        PlatformIndicateLinkQuality(pAd,pPort->PortNumber);
        PlatformIndicateNewLinkSpeed(pAd,pPort->PortNumber,OPMODE_STA);
    }   
#if DBG 
    if( (pAd->Mlme.OneSecPeriodicRound % 30) == 0 )
    {
        DBGPRINT(RT_DEBUG_TRACE,("BULK: STAMlmePeriodicExec NextRxBulkInReadIndex = %d, NextRxBulkInIndex = %d  PendingRx = %d\n",  pAd->pHifCfg->NextRxBulkInReadIndex,pAd->pHifCfg->NextRxBulkInIndex,pAd->pHifCfg->PendingRx ));        
    }
#endif  
    PlatformAcquireOldRWLockForWrite(&pAd->MacTablePool.MacTabPoolLock, &LockState);

    // Update the specific flags in [ADHOC PER ENTRY].
    if (ADHOC_ON(pPort))
    {
        UCHAR   OperationMode = 0;
        BOOLEAN bHtEnable = FALSE;
        ULONG   HighLastPhyRate = 1, HighPhyRate = 1; // RATE_1 *2
        UCHAR   HighLastWcid = 0, HighWcid = 0;

        do {
            if (pPort->MacTab.Size == 0)
                break;

            pPort->MacTab.fAnyBASession       = FALSE;
            pPort->MacTab.fAnyStationIsLegacy = FALSE;
            pPort->MacTab.fAnyStation20Only    = FALSE;
            pPort->MacTab.fAnyStationIsCCK    = FALSE;
            pPort->MacTab.fAnyStationIsOFDM   = FALSE;
            
            pHeader = &pPort->MacTab.MacTabList;
            pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
            while (pNextMacEntry != NULL)
            {
                pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
                
                if(pMacEntry == NULL)
                {
                    break; 
                }
                
                if(pMacEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST)
                {
                    pNextMacEntry = pNextMacEntry->Next;   
                    pMacEntry = NULL;
                    continue; 
                }

                if (pMacEntry->ValidAsCLI == FALSE)
                {
                    pNextMacEntry = pNextMacEntry->Next;   
                    pMacEntry = NULL;
                    continue; 
                }
                
                if (pMacEntry->TXBAbitmap != 0)
                    pPort->MacTab.fAnyBASession = TRUE;

                if (READ_PHY_CFG_BW(pAd, &pMacEntry->TxPhyCfg) == BW_20)
                {
                    pPort->MacTab.fAnyStation20Only  = TRUE;
                }

                if ((READ_PHY_CFG_MODE(pAd, &pMacEntry->TxPhyCfg) == MODE_OFDM) || 
                     (READ_PHY_CFG_MODE(pAd, &pMacEntry->TxPhyCfg) == MODE_CCK))
                {
                    pPort->MacTab.fAnyStationIsLegacy = TRUE;
                    
                    if (READ_PHY_CFG_MODE(pAd, &pMacEntry->TxPhyCfg) == MODE_CCK)
                    {
                        pPort->MacTab.fAnyStationIsCCK = TRUE;
                    }
                    else if (READ_PHY_CFG_MODE(pAd, &pMacEntry->TxPhyCfg) == MODE_OFDM)
                    {
                        pPort->MacTab.fAnyStationIsOFDM = TRUE;
                }
                }
                else    // HT mode
                {
                    bHtEnable = TRUE;
                    if (pMacEntry->MaxRAmpduFactor == 0)
                        RTUSBWriteMACRegister(pAd, MAX_LEN_CFG, MAC_VALUE_MAX_LEN_CFG_PSDU8K(pAd));
                }


                // Find out the higest tx rate in MacEntry for link speed status used
                if (pAd->Mlme.OneSecPeriodicRound % 4 == 0) /* check per 4 seconds */
                {
                    if ((HighLastPhyRate < XmitMCSToPhyRateIn500kps(READ_PHY_CFG_MCS(pAd, &pMacEntry->TxPhyCfg), 
                                                                                           READ_PHY_CFG_MODE(pAd, &pMacEntry->TxPhyCfg), 
                                                                                           READ_PHY_CFG_BW(pAd, &pMacEntry->TxPhyCfg), 
                                                                                           READ_PHY_CFG_SHORT_GI(pAd, &pMacEntry->TxPhyCfg))) 
                        &&(pMacEntry->LastKickTxCount > 0))
                    {
                        HighLastPhyRate = XmitMCSToPhyRateIn500kps(READ_PHY_CFG_MCS(pAd, &pMacEntry->TxPhyCfg), 
                                                                                        READ_PHY_CFG_MODE(pAd, &pMacEntry->TxPhyCfg), 
                                                                                        READ_PHY_CFG_BW(pAd, &pMacEntry->TxPhyCfg), 
                                                                                        READ_PHY_CFG_SHORT_GI(pAd, &pMacEntry->TxPhyCfg));
                        HighLastWcid = (UCHAR)pMacEntry->wcid;
                    }

                    if ((HighPhyRate < XmitMCSToPhyRateIn500kps(READ_PHY_CFG_MCS(pAd, &pMacEntry->TxPhyCfg), 
                                                                                             READ_PHY_CFG_MODE(pAd, &pMacEntry->TxPhyCfg), 
                                                                                             READ_PHY_CFG_BW(pAd, &pMacEntry->TxPhyCfg), 
                                                                                             READ_PHY_CFG_SHORT_GI(pAd, &pMacEntry->TxPhyCfg))))
                    {
                        HighPhyRate = XmitMCSToPhyRateIn500kps(READ_PHY_CFG_MCS(pAd, &pMacEntry->TxPhyCfg), 
                                                                                          READ_PHY_CFG_MODE(pAd, &pMacEntry->TxPhyCfg), 
                                                                                          READ_PHY_CFG_BW(pAd, &pMacEntry->TxPhyCfg), 
                                                                                          READ_PHY_CFG_SHORT_GI(pAd, &pMacEntry->TxPhyCfg));
                        HighWcid = (UCHAR)pMacEntry->wcid;
                    }

                    // reset the last TxCount
                    pMacEntry->LastKickTxCount = 0;
                }                   

                pNextMacEntry = pNextMacEntry->Next;   
                pMacEntry = NULL;
            }

            // Set Protection           
            if (pPort->MacTab.fAnyStationIsLegacy)
            {
                OperationMode = HT_2040_PROTECT;// the same effect as HT_FORCERTSCTS, turn on BG Protection
            }
            else if (pPort->MacTab.fAnyBASession)
            {
                OperationMode = HT_FORCERTSCTS;
            }
            else if ((pPort->MacTab.fAnyStation20Only) && (pPort->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40))
            {
                // If I am 40MHz IBSS, and there exist HT-20MHz adhoc. 
                // Update to 2 when it's zero.  Because OperationMode = 1 or 3 has more protection.
                OperationMode = HT_40_PROTECT;
            }

            if ((OperationMode != 0) && (OperationMode != pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode))
            {
                pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = OperationMode;
                MtAsicUpdateProtect(pAd, 
                                pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode, 
                                (ALLN_SETPROTECT), 
                                (!pPort->MacTab.fAnyStationIsLegacy) ? TRUE : FALSE,
                                FALSE);
            }


            // Set HtRate flag
            if ((pAd->StaActive.SupportedHtPhy.bHtEnable == FALSE) && (bHtEnable == TRUE))
                pAd->StaActive.SupportedHtPhy.bHtEnable = TRUE;
            else if((pAd->StaActive.SupportedHtPhy.bHtEnable == TRUE) && (bHtEnable == FALSE))
                pAd->StaActive.SupportedHtPhy.bHtEnable = FALSE;


            // Priority 1. Keep the max of the last tx rate onto CommonCfg.HTPhyMode if any data transmission is running now.
            // Priority 2. Keep the max of the supported rate onto CommonCfg.HTPhyMode
            if ((HighLastWcid != HighWcid) && HighLastWcid != 0)
            {
                pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, HighLastWcid); 

                if(pWcidMacTabEntry == NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, HighLastWcid));
                    return;
                }
                
                WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.TxPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pWcidMacTabEntry->TxPhyCfg));
                
                // !!! Update the last tx rate here !!!
                WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->LastTxRatePhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pWcidMacTabEntry->TxPhyCfg));
                
                pAd->StaCfg.LastWcid = HighLastWcid;
            }
            else if (HighWcid != 0)
            {
                pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, HighWcid); 

                if(pWcidMacTabEntry == NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, HighWcid));
                    return;
                }
                WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.TxPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pWcidMacTabEntry->TxPhyCfg));

                
                // !!! Update the last tx rate here !!!
                WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->LastTxRatePhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pWcidMacTabEntry->TxPhyCfg));
                pAd->StaCfg.LastWcid = HighWcid;
            }

        } while(FALSE);
        
    }
    else if (INFRA_ON(pPort))   // infra mode
    {
        PMAC_TABLE_ENTRY pMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
        if(pMacTabEntry == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
            return;
        }
        
        // When Station connects to AP, and STA has BA Originator session, Always turn on RTS/CTS. 2007-8-30
        if ((pMacTabEntry->TXBAbitmap != 0) && 
            (pPort->MacTab.fAnyBASession == FALSE) &&
            !MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
        {
            pPort->MacTab.fAnyBASession = TRUE;
            MtAsicUpdateProtect(pAd, HT_FORCERTSCTS,  ALLN_SETPROTECT, FALSE, FALSE);
        }
        else if ((pMacTabEntry->TXBAbitmap == 0) && 
                (pPort->MacTab.fAnyBASession == TRUE) &&
                !MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
        {
            pPort->MacTab.fAnyBASession = FALSE;
            MtAsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode,  ALLN_SETPROTECT, FALSE, FALSE);
        }
    }
    else if ((pPort != pActivePort) && (IS_P2P_CON_CLI(pAd, pActivePort) || IS_P2P_MS_CLI(pAd, pActivePort) || IS_P2P_MS_CLI2(pAd, pActivePort)) && INFRA_ON(pActivePort))  // P2PClient, Infra on
    {
        pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, P2pGetClientWcid(pAd, pActivePort));
        if(pWcidMacTabEntry == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, pActivePort));
            return;
        }
        
        // When Station connects to AP, and STA has BA Originator session, Always turn on RTS/CTS. 2007-8-30
        if ((pWcidMacTabEntry->TXBAbitmap != 0) && 
            (pPort->MacTab.fAnyBASession == FALSE) &&
            !MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
        {
            pPort->MacTab.fAnyBASession = TRUE;
            MtAsicUpdateProtect(pAd, HT_FORCERTSCTS,  ALLN_SETPROTECT, FALSE, FALSE);
        }
        else if ((pWcidMacTabEntry->TXBAbitmap == 0) && 
                (pPort->MacTab.fAnyBASession == TRUE) &&
                !MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
        {
            pPort->MacTab.fAnyBASession = FALSE;
            MtAsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode,  ALLN_SETPROTECT, FALSE, FALSE);
        }
    }
    
    PlatformReleaseOldRWLock(&pAd->MacTablePool.MacTabPoolLock, &LockState);

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    // NLO Auto Scan
    if (NDIS_WIN8_ABOVE(pAd) && pAd->StaCfg.NLOEntry.NLOEnable)
    {
        // FastScan or SlowScan period
        if (pAd->StaCfg.NLOEntry.FastScanIterations > 0)
        {
            if ((pAd->StaCfg.NLOEntry.OneSecPeriodicRound % pAd->StaCfg.NLOEntry.FastScanPeriod) == 0)
            {
                pAd->StaCfg.NLOEntry.FastScanIterations--;
                MlmeAutoScan(pAd);
            }
        }
        else
        {
            if ((pAd->StaCfg.NLOEntry.OneSecPeriodicRound % pAd->StaCfg.NLOEntry.SlowScanPeriod) == 0)
            {
                MlmeAutoScan(pAd);
            }
        }

        pAd->StaCfg.NLOEntry.OneSecPeriodicRound++;
    }
#endif

    if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) && !MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
    {
        // When Adhoc beacon is enabled and RTS/CTS is enabled, there is a chance that hardware MAC FSM will run into a deadlock
        // and sending CTS-to-self over and over.
        // Software Patch Solution: 
        // 1. Polling debug state register 0x10F4 every one second.
        // 2. If in 0x10F4 the ((bit29==1) && (bit7==1)) OR ((bit29==1) && (bit5==1)), it means the deadlock has occurred.
        // 3. If the deadlock occurred, reset MAC/BBP by setting 0x1004 to 0x0003 for a while then setting it back to 0x000C again.

        ULONG   MacReg;
        
        RTUSBReadMACRegister(pAd, 0x10F4, &MacReg);
        if (((MacReg & 0x20000000) && (MacReg & 0x80)) || ((MacReg & 0x20000000) && (MacReg & 0x20)))
        {
            RTUSBWriteMACRegister(pAd, MAC_SYS_CTRL, 0x1);
            Delay_us(1);
            RTUSBWriteMACRegister(pAd, MAC_SYS_CTRL, 0xC);

            DBGPRINT(RT_DEBUG_WARN,("Warning, MAC specific condition occurs \n"));
        }
    }

    // To patch Atheros send out invalid BAR frame.
    do {

        if (pPort->MacTab.Size == 0)
            break;
        
        pHeader = &pPort->MacTab.MacTabList;
        pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
        while (pNextMacEntry != NULL)
        {
            pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
            
            if(pMacEntry == NULL)
            {
                break; 
            }
            
            if(pMacEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST)
            {
                    AsicSendBA(pAd, pPort, pPort->Channel, pMacEntry->wcid);
                    pMacEntry->BadBARCount = 0; //Reset
            }

            pNextMacEntry = pNextMacEntry->Next;   
            pMacEntry = NULL;
        }

    } while (FALSE);
    
    // Perform 20/40 BSS COEX scan every Dot11BssWidthTriggerScanInt
    if ((OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SCAN_2040)) && 
        (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_SCAN_2040)) &&
        (pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF) &&
        ((pAd->Mlme.OneSecPeriodicRound % pPort->CommonCfg.ScanParameter.Dot11BssWidthTriggerScanInt) == (pPort->CommonCfg.ScanParameter.Dot11BssWidthTriggerScanInt-1)))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - BulkLastOneSecCount  = %d \n", pAd->pHifCfg->BulkLastOneSecCount ));
        // Check last scan time at least 30 seconds from now. 
        // Check traffic is less than about 2~3Mbps.
        // it might cause data lost if we enqueue scanning.
        // This criteria needs to be considered     
        if ((pAd->pHifCfg->BulkLastOneSecCount < 20)
            && ((pAd->StaCfg.LastScanTime + 10 * ONE_SECOND_TIME) < pAd->Mlme.Now64))
        {
            MLME_SCAN_REQ_STRUCT            ScanReq;
            // Fill out stuff for scan request and kick to scan
            MlmeCntScanParmFill(pAd, &ScanReq, ZeroSsid, 0, BSS_ANY, SCAN_2040_BSS_COEXIST);
            MlmeEnqueue(pAd,pPort, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq);
            pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_LIST_SCAN;
            // Set InfoReq = 1, So after scan , alwats sebd 20/40 Coexistence frame to AP
            pPort->CommonCfg.BSSCoexist2040.field.InfoReq = 1;
        }
    }

    pPort->bRoamingAfterResume = FALSE;
     FUNC_LEAVE;
}
// Link down report
VOID MlmeCntLinkDownExecTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    DBGPRINT(RT_DEBUG_TRACE, ("MlmeCntLinkDownExecTimerCallback\n"));
}

VOID MlmeCntLinkUpExecTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
}

VOID WakeUpExecTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;

    N6CompletePendingOID( pAd, PENDING_PNP_SET_POWER, NDIS_STATUS_SUCCESS);
    DBGPRINT(RT_DEBUG_TRACE,("complete wake OID\n"));                       
}

// IRQL = DISPATCH_LEVEL
VOID MlmeAutoScan(
    IN PMP_ADAPTER pAd    
    )
{
#if 0
    PMP_PORT pPort = pAd->PortList[PORT_0];
    DOT11_SCAN_REQUEST_V2      Dot11ScanRequest = {0};  //Init with zero when declare or some garbage data will cause BSOD.

    if(pAd->LogoTestCfg.OnTestingWHQL && 
        (INFRA_ON(pAd->PortList[PORT_0]) || 
        MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP) || 
        MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON) ||
        MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION) ||
        (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_PROVISIONING)))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - OnTestingWHQL, reject this scan\n", __FUNCTION__));
        return;
    }

    Dot11ScanRequest.dot11ScanType = (dot11_scan_type_active | dot11_scan_type_forced);
    Dot11ScanRequest.uNumOfdot11SSIDs = 0;
    // check CntlMachine.CurrState to avoid collision with NDIS SetOID request
    if ((pPort->Mlme.CntlMachine.CurrState == CNTL_IDLE))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - Driver auto scan\n"));
        pAd->pNicCfg->bUpdateBssList = TRUE;
        MlmeEnqueue(pAd, 
                    pPort,
                    MLME_CNTL_STATE_MACHINE, 
                    OID_802_11_BSSID_LIST_SCAN, 
                    sizeof(DOT11_SCAN_REQUEST_V2), 
                    &Dot11ScanRequest);
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hMLMEThread)));  
    }
#endif  
}

// IRQL = DISPATCH_LEVEL
VOID MlmeAutoReconnectLastSSID(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort)
{
    PMP_PORT pWscPort = pPort;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, P2pGetClientWcid(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]));      

    // Do not AutoReconnect if Radio is Off
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d]autoconnection debug\n",__FUNCTION__,__LINE__));
        return;
    }

    if ((IS_P2P_REGISTRA(pPort) ||IS_P2P_GO_WPA2PSKING(pPort) || IS_P2P_GO_OP(pPort)) && (pAd->OpMode == OPMODE_STA))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s(%d): Ignore Auto Reconnect.\n", __FUNCTION__, __LINE__));
        return; 
    }

  
    // If P2P Managed on, we need to check not to connect to Managed AP too often that deauth us with reason code 2 or 4.
    if ((P2P_ON(pPort)) 
        && ((pPort->P2PCfg.P2pManagedParm.APP2pMinorReason == MINOR_REASON_BCZ_MANAGED_BIT_ZERO)
        || (pPort->P2PCfg.P2pManagedParm.APP2pMinorReason == MINOR_REASON_OUTSIDE_IT_DEFINED)))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("  Don't auto-reconnect to the Managed AP too much. Reason=%d. Wait count down = %d. SSID=%c%c%c....\n", 
                pPort->P2PCfg.P2pManagedParm.APP2pMinorReason, pPort->P2PCfg.P2pCounter.ManageAPEnReconCounter, pAd->MlmeAux.Ssid[0], pAd->MlmeAux.Ssid[1], pAd->MlmeAux.Ssid[2]));
        return;
    }
    
    // When WPS is enabled, the station reconnects by AP's BSSID in order to
    // deal with the multiple APs with the same SSID.
    if ((pPort->StaCfg.WscControl.WscState >= WSC_STATE_START) && (pPort->StaCfg.WscControl.WscState < WSC_STATE_WAIT_DISCONN))
    {
        // If concurrent Client(Port2) is walking on, change to use this port
        // 1. WPS re-association(retry).
        // 2. P2P privisioning at Enrollee
        if (P2P_ON(pPort)
            && (pAd->OpMode == OPMODE_STAP2P)
            && (IS_P2P_CONNECT_IDLE(pPort) || IS_P2P_ENROLLEE(pPort))
            && (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PClient))
        {
            if (pAd->PortList[pPort->P2PCfg.PortNumber]->Mlme.CntlMachine.CurrState != CNTL_IDLE)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s(%d): Ignore Auto Reconnect due to WscPort is busy.(state=%d)\n", 
                        __FUNCTION__, __LINE__, pAd->PortList[pPort->P2PCfg.PortNumber]->Mlme.CntlMachine.CurrState));
                return;
            }

            pWscPort = pAd->PortList[pPort->P2PCfg.PortNumber];
        }
        
        DBGPRINT(RT_DEBUG_TRACE, ("%s: MlmeCntlWscIterate - PortNumber= %d\n", __FUNCTION__, pWscPort->PortNumber));
        MlmeCntlWscIterate(pAd, pWscPort);
    }
    else if (INFRA_ON(/*pAd->PortList[pAd->ucActivePortNum]*/pPort))
    {   // TODO:  Ignore reconnection due to connected Active Port 
        DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d]autoconnection debug\n",__FUNCTION__,__LINE__));
    }   
    else
    {
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        //
        // Select the network with highest RSSI
        //
        if (NDIS_WIN8_ABOVE(pAd)  && (pAd->StaCfg.NLOEntry.NLOEnable == TRUE) && (pAd->StaCfg.NLOEntry.IndicateNLO == TRUE))
        {
            CHAR RSSI = -100;
            ULONG idxBssEntry = 0, idxOffloadEntry = 0;
            PBSS_ENTRY pBssEntry = NULL;
            PDOT11_OFFLOAD_NETWORK pNLO = NULL;

            for (idxOffloadEntry = 0; idxOffloadEntry < pAd->StaCfg.NLOEntry.uNumOfEntries; idxOffloadEntry++)
            {
                pNLO= &pAd->StaCfg.NLOEntry.OffloadNetwork[idxOffloadEntry];
                
                for (idxBssEntry = 0; idxBssEntry < pAd->ScanTab.BssNr; idxBssEntry++)
                {
                    pBssEntry = &pAd->ScanTab.BssEntry[idxBssEntry];

                    // check SSID, Cipher and AuthMode
                    if (!SSID_EQUAL(pBssEntry->Ssid, pBssEntry->SsidLen, pNLO->Ssid.ucSSID, pNLO->Ssid.uSSIDLength))
                        continue;

                    if ((pBssEntry->AuthMode != (ULONG)(pNLO->AuthAlgo)) && (pBssEntry->AuthModeAux != (ULONG)(pNLO->AuthAlgo)))
                        continue;

                    if (pBssEntry->WepStatus != (ULONG)(pNLO->UnicastCipher))
                        continue;

                    if (pBssEntry->Rssi > RSSI) // Better RSSI
                    {
                        PlatformMoveMemory(pAd->MlmeAux.AutoReconnectSsid, pNLO->Ssid.ucSSID, pNLO->Ssid.uSSIDLength);
                        pAd->MlmeAux.AutoReconnectSsidLen = (UCHAR)(pNLO->Ssid.uSSIDLength);

                        RSSI = pBssEntry->Rssi;

                        DBGPRINT(RT_DEBUG_TRACE,("%s: (better RSSI = %d) Found NLO SSID = %c%c%c%c%c......, AuthAlgo = %d, UnicastCipher = %d\n", 
                            __FUNCTION__, 
                            RSSI, 
                            pNLO->Ssid.ucSSID[0], pNLO->Ssid.ucSSID[1], pNLO->Ssid.ucSSID[2], pNLO->Ssid.ucSSID[3], pNLO->Ssid.ucSSID[4], 
                            pNLO->AuthAlgo, 
                            pNLO->UnicastCipher));      
                    }
                }
            }
        }
#endif

            // check CntlMachine.CurrState to avoid collision with NDIS SetOID request
            if (((pPort->Mlme.CntlMachine.CurrState == CNTL_IDLE) && (MlmeValidateSSID(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen) == TRUE)) ||
                ((pPort->Mlme.CntlMachine.CurrState == CNTL_IDLE) && (pPort->PortType == WFD_CLIENT_PORT) && (MlmeValidateSSID(pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen) == TRUE)))
            {
                NDIS_802_11_SSID OidSsid;
                OidSsid.SsidLength = pAd->MlmeAux.AutoReconnectSsidLen;
                PlatformMoveMemory(OidSsid.Ssid, pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);
                pAd->MlmeAux.AutoReconnectStatus = TRUE;        

                // The STA is connected to or is attempting to connect a BSS network.
                // Note that when the STA leaves the hibernation state, it connects to a BSS network by using the auto reconnection method.
                // Therefore, the STA will be in the initialization state (INIT_STATE).
                MP_SET_STATE(pPort, OP_STATE);
                DBGPRINT(RT_DEBUG_TRACE, ("Driver auto reconnect to last OID_802_11_SSID setting - %s, len = %d\n", pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen));        

                MlmeEnqueue(pAd,
                        pPort,
                        MLME_CNTL_STATE_MACHINE,
                        OID_DOT11_CONNECT_REQUEST,
                        0,
                        NULL);
        }
    }
}

/*
    ==========================================================================
    Validate SSID for connection try and rescan purpose
    Valid SSID will have visible chars only.
    The valid length is from 0 to 32.
    IRQL = DISPATCH_LEVEL
    ==========================================================================
 */
BOOLEAN MlmeValidateSSID(
    IN PUCHAR   pSsid,
    IN UCHAR    SsidLen)
{
    int index;

    if (SsidLen > MAX_LEN_OF_SSID)
        return (FALSE);

    // Check each character value
    for (index = 0; index < SsidLen; index++)
    {
        if (pSsid[index] < 0x20)
            return (FALSE);
    }

    // All checked
    return (TRUE);
}

/*
    ==========================================================================
    Description:
        This routine checks if there're other APs out there capable for
        roaming. Caller should call this routine only when Link up in INFRA mode
        and channel quality is below CQI_GOOD_THRESHOLD.

    IRQL = DISPATCH_LEVEL

    Output:
    ==========================================================================
 */
BOOLEAN MlmeCheckForRoaming(
    IN PMP_ADAPTER pAd,   
    IN PMP_PORT pPort,    
    IN ULONGLONG    Now64)
{
    USHORT     i;
    BSS_TABLE  *pRoamTab = &pAd->MlmeAux.RoamTab;
    BSS_ENTRY  *pBss;
    BOOLEAN bResult = FALSE;

    DBGPRINT(RT_DEBUG_TRACE, ("==> MlmeCheckForRoaming , scanNUM= %d,BSSID=%x:%x:%x:%x:%x:%x\n",pAd->ScanTab.BssNr,
        pPort->PortCfg.Bssid[0], pPort->PortCfg.Bssid[1], pPort->PortCfg.Bssid[2], pPort->PortCfg.Bssid[3], pPort->PortCfg.Bssid[4],
        pPort->PortCfg.Bssid[5]));
    
    // put all roaming candidates into RoamTab, and sort in RSSI order
    BssTableInit(pRoamTab);
    for (i = 0; i < pAd->ScanTab.BssNr; i++)
    {
        pBss = &pAd->ScanTab.BssEntry[i];       
        DBGPRINT(RT_DEBUG_TRACE, ("ssid =%s,channel=%d\n",pBss->Ssid,pBss->Channel));
        if(pBss->BssType == BSS_ADHOC)
            continue;    // skip different SSID
            
        // Check AuthMode and AuthModeAux for matching, in case AP support dual-mode
        if ((pPort->PortCfg.AuthMode != pBss->AuthMode) && (pPort->PortCfg.AuthMode != pBss->AuthModeAux))
            // None matched
            continue;       
        
        if (MAC_ADDR_EQUAL(pBss->Bssid, pPort->PortCfg.Bssid) && (pBss->Channel == pPort->Channel))
            continue;    // skip current AP
        if (!SSID_EQUAL(pBss->Ssid, pBss->SsidLen, pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen))
            continue;    // skip different SSID     
        
        DBGPRINT(RT_DEBUG_TRACE, ("LastRssi = %d, pBss->Rssi = %d\n", pAd->StaCfg.RssiSample.LastRssi[0], pBss->Rssi));
        // AP passing all above rules is put into roaming candidate table        
        PlatformMoveMemory(&pRoamTab->BssEntry[pRoamTab->BssNr], pBss, sizeof(BSS_ENTRY));
        pRoamTab->BssNr += 1;
    }

    if (pRoamTab->BssNr > 0)
    {
        // check CntlMachine.CurrState to avoid collision with NDIS SetOID request
        if (pPort->Mlme.CntlMachine.CurrState == CNTL_IDLE)
        {           
            bResult = TRUE;
            pAd->Counter.MTKCounters.PoorCQIRoamingCount ++;
            DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - Roaming attempt #%d\n", pAd->Counter.MTKCounters.PoorCQIRoamingCount));
            MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_MLME_ROAMING_REQ, 0, NULL);
        }       
    }
    // Maybe site survey required
    else
    {
        if ((pAd->StaCfg.LastScanTime + TEN_SECOND) < Now64)
        {
            // check CntlMachine.CurrState to avoid collision with NDIS SetOID request
            DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - Roaming, No eligable entry, try new scan!\n"));
            pAd->StaCfg.ScanCnt = 2;
            pAd->StaCfg.LastScanTime = Now64;
            MlmeAutoScan(pAd);
            // Set bResult as FALSE for CCXv5, shorten time to reconnect
            bResult = FALSE;
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Can't Roaming\n"));  
        }
    }

    DBGPRINT(RT_DEBUG_TRACE, ("<== MlmeCheckForRoaming\n"));    
    return bResult;
}

/*
    ==========================================================================
    Description:
        This routine checks if there're other APs out there capable for
        roaming. Caller should call this routine only when link up in INFRA mode
        and channel quality is below CQI_GOOD_THRESHOLD.

    IRQL = DISPATCH_LEVEL

    Output:
    ==========================================================================
 */
VOID MlmeCheckForFastRoaming(
    IN  PMP_ADAPTER   pAd,
    IN  ULONGLONG           Now)
{

    USHORT     i;
    BSS_TABLE  *pRoamTab = &pAd->MlmeAux.RoamTab;
    BSS_ENTRY  *pBss;
    CHAR        MaxRSSI;
    PMP_PORT  pPort = pAd->PortList[PORT_0];
    
    DBGPRINT(RT_DEBUG_TRACE, ("==> MlmeCheckForFastRoaming, ScanTab.BssNr=%d\n", pAd->ScanTab.BssNr));

    // Scanning, ignore Roaming
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("<== Quit MlmeCheckForFastRoaming\n"));
        return;
    }

    // Fast-Roaming Scanning, ignore Roaming except VoiceRate roaming
    if ((pAd->StaCfg.bFastRoamingScan == TRUE) && (pAd->StaCfg.CCXControl.field.VoiceRate == FALSE))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("<== Quit MlmeCheckForFastRoaming, Fast-Roaming Scan in Progress\n"));
        return;
    }

    if (pPort->Mlme.SyncMachine.CurrState != SYNC_IDLE)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("<== Quit MlmeCheckForFastRoaming, Not SYNC_IDLE\n"));
        return;
    }

    //
    // If Scan talbe is fresh within 10 sec, then we can select the AP with the best RSSI
    // Otherwise refresh this scan table.
    // Not for CCXv5 RF roaming, and Voice drastically roaming
    if ((pPort->Mlme.CntlMachine.CurrState == CNTL_IDLE) && ((pAd->StaCfg.LastScanTime + 10 * ONE_SECOND_TIME) >= Now))
    {
        // put all roaming candidates into RoamTab, and sort in RSSI order
        BssTableInit(pRoamTab);
        for (i = 0; i < pAd->ScanTab.BssNr; i++)
        {
            pBss = &pAd->ScanTab.BssEntry[i];       

            // get the current RSSI from AvgRssi
            MaxRSSI = RTMPMaxRssi(pAd, pAd->StaCfg.RssiSample.AvgRssi[0], pAd->StaCfg.RssiSample.AvgRssi[1], pAd->StaCfg.RssiSample.AvgRssi[2]);

            if (MAC_ADDR_EQUAL(pBss->Bssid, pPort->PortCfg.Bssid))
            {
                continue;    // skip current AP
            }

                if (!SSID_EQUAL(pBss->Ssid, pBss->SsidLen, pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen))
                continue;    // skip different SSID

                // check Rssi difference if rx beacon in one sec. (If beacon lost more than one sec, AP may dead, ignore RssiDelta)
            if (pAd->StaCfg.LastBeaconRxTime + ONE_SECOND_TIME > pAd->Mlme.Now64)
            {
                // RssiDelta is read from Registry
                if (pBss->Rssi < (MaxRSSI + pAd->StaCfg.RssiDelta))
                        continue;    // skip AP without better RSSI
            }
                
            if ((pBss->Rssi <= -85) && (pBss->Channel == pPort->Channel))
                continue;    // skip RSSI too weak at the same channel
            
                DBGPRINT(RT_DEBUG_TRACE, ("BSSID = %02X:%02X:%02X:%02X:%02X:%02X, LastRssi = %d, pBss->Rssi = %d\n", 
                    pBss->Bssid[0], pBss->Bssid[1], pBss->Bssid[2], 
                    pBss->Bssid[3], pBss->Bssid[4], pBss->Bssid[5], 
                    pAd->StaCfg.RssiSample.LastRssi[0], pBss->Rssi));
            // AP passing all above rules is put into roaming candidate table        
            PlatformMoveMemory(&pRoamTab->BssEntry[pRoamTab->BssNr], pBss, sizeof(BSS_ENTRY));
            pRoamTab->BssNr += 1;
        }

        if (pRoamTab->BssNr > 0)
        {
            // check CntlMachine.CurrState to avoid collision with NDIS SetOID request
            if (pPort->Mlme.CntlMachine.CurrState == CNTL_IDLE)
            {
                pAd->Counter.MTKCounters.PoorCQIRoamingCount ++;
                    
                DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - Roaming attempt #%d\n", pAd->Counter.MTKCounters.PoorCQIRoamingCount));
                    pAd->MlmeAux.bStaCanRoam = TRUE;
                MlmeEnqueue(pAd, pPort,MLME_CNTL_STATE_MACHINE, MT2_MLME_ROAMING_REQ, 0, NULL);
            }
        }
    }
    // Maybe site survey required
    else
    {
        if ((pAd->StaCfg.LastScanTime + 10 * ONE_SECOND_TIME) < Now)
        {
            // set this flag to TRUE, to fill SSID and SSIDLen in ProbeReq for Fast-Roaming, Hidden-SSID case.
            pAd->StaCfg.bFastRoamRescan = TRUE;         
            pAd->StaCfg.ScanChannelCnt = 0;

            // check CntlMachine.CurrState to avoid collision with NDIS SetOID request
            DBGPRINT(RT_DEBUG_TRACE, ("MlmeCheckForFastRoaming - Roaming, No eligable entry, try new scan!\n"));
            pAd->StaCfg.ScanCnt = 2;
            pAd->StaCfg.LastScanTime = Now;
            MlmeAutoScan(pAd);
        }
    }

    DBGPRINT(RT_DEBUG_TRACE, ("<== MlmeCheckForFastRoaming\n"));

}

/*
    ==========================================================================
    Description:
        This routine calculates TxPER, RxPER of the past N-sec period. And 
        according to the calculation result, ChannelQuality is calculated here 
        to decide if current AP is still doing the job. 

        If ChannelQuality is not good, a ROAMing attempt may be tried later.
    Output:
        StaCfg.ChannelQuality - 0..100

    IRQL = DISPATCH_LEVEL

    NOTE: This routine decide channle quality based on RX CRC error ratio.
        Caller should make sure a function call to NICUpdateRawCounters(pAd)
        is performed right before this routine, so that this routine can decide
        channel quality based on the most up-to-date information
    ==========================================================================
 */
VOID MlmeCalculateChannelQuality(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN ULONGLONG Now64)
{
    ULONG TxOkCnt, TxCnt, TxPER, TxPRR;
    ULONG RxCnt, RxPER;
    UCHAR NorRssi;
    CHAR  MaxRssi;  

    MaxRssi = RTMPMaxRssi(pAd, pAd->StaCfg.RssiSample.LastRssi[0], pAd->StaCfg.RssiSample.LastRssi[1], pAd->StaCfg.RssiSample.LastRssi[2]); 

    //
    // calculate TX packet error ratio and TX retry ratio - if too few TX samples, skip TX related statistics
    //
    TxOkCnt = pAd->Counter.MTKCounters.OneSecTxNoRetryOkCount + pAd->Counter.MTKCounters.OneSecTxRetryOkCount;
    TxCnt = TxOkCnt + (pAd->Counter.MTKCounters.OneSecTxFailCount>>2);
    if (TxCnt < 5) 
    {
        TxPER = 0;
        TxPRR = 0;
    }
    else 
    {
        TxPER = (pAd->Counter.MTKCounters.OneSecTxFailCount * 100) / TxCnt; 
        TxPRR = ((TxCnt - pAd->Counter.MTKCounters.OneSecTxNoRetryOkCount) * 100) / TxCnt;
    }

    //
    // calculate RX PER - don't take RxPER into consideration if too few sample
    //
    RxCnt = pAd->Counter.MTKCounters.OneSecRxOkCnt + pAd->Counter.MTKCounters.OneSecRxFcsErrCnt;
    if (RxCnt < 5)
        RxPER = 0;  
    else
        RxPER = (pAd->Counter.MTKCounters.OneSecRxFcsErrCnt * 100) / RxCnt;

    // use active probe request to check if AP really stoped
    if (INFRA_ON(pPort) &&      
        ((pAd->StaCfg.LastBeaconRxTime + ONE_SECOND_TIME) < Now64) && (pAd->Mlme.ChannelStayCount > 1)
         && (pPort->Mlme.CntlMachine.CurrState == CNTL_IDLE))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("Port%d, BEACON lost > 1 sec \n",pPort->PortNumber)); 
        MlmeSyncProbeInactiveStation(pAd,pPort,pPort->PortCfg.Bssid);
    }

    //
    // decide ChannelQuality based on: 1)last BEACON received time, 2)last RSSI, 3)TxPER, and 4)RxPER
    //
    if (INFRA_ON(pPort) &&      
        (pAd->StaCfg.LastBeaconRxTime +  (pAd->StaCfg.BeaconLostTime * ONE_SECOND_TIME) < Now64) && (pAd->Mlme.ChannelStayCount > 1) && (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
         && (pPort->Mlme.CntlMachine.CurrState == CNTL_IDLE))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("BEACON lost > %d sec with TxOkCnt=%d -> CQI=0 pPortNum =%d\n", pAd->StaCfg.BeaconLostTime, TxOkCnt,pPort->PortNumber)); 
        pPort->Mlme.ChannelQuality = 0;
    }   
    else
    {
        // Normalize Rssi
        if (MaxRssi > -40)
            NorRssi = 100;
        else if (MaxRssi < -90)
            NorRssi = 0;
        else
            NorRssi = (MaxRssi + 90) * 2;
        
        // ChannelQuality = W1*RSSI + W2*TxPRR + W3*RxPER    (RSSI 0..100), (TxPER 100..0), (RxPER 100..0)
        pPort->Mlme.ChannelQuality = (RSSI_WEIGHTING * NorRssi + 
                                   TX_WEIGHTING * (100 - TxPRR) + 
                                   RX_WEIGHTING* (100 - RxPER)) / 100;
        if (pPort->Mlme.ChannelQuality >= 100)
            pPort->Mlme.ChannelQuality = 100;
    }

    DBGPRINT(RT_DEBUG_INFO, ("MMCHK - CQI= %d (Tx Fail=%d/Retry=%d/Total=%d, Rx Fail=%d/Total=%d, RSSI=%d dbm)\n", 
        pAd->Mlme.ChannelQuality, 
        pAd->Counter.MTKCounters.OneSecTxFailCount, 
        pAd->Counter.MTKCounters.OneSecTxRetryOkCount, 
        TxCnt, 
        pAd->Counter.MTKCounters.OneSecRxFcsErrCnt, 
        RxCnt, pAd->StaCfg.RssiSample.LastRssi[0]));

}

VOID MlmeSetTxRate(
    IN PMP_ADAPTER        pAd,
    IN PMAC_TABLE_ENTRY     pEntry,
    IN PRTMP_TX_RATE_SWITCH pTxRate,
    IN PUCHAR               pTable)
{
    PMP_PORT pPort;
    pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

    if (ADHOC_ON(pPort))
    {
        if ((pTxRate->STBC == STBC_USE) && 
             (READ_PHY_CFG_STBC(pAd, &pEntry->MaxPhyCfg) == STBC_USE) && 
             (pAd->HwCfg.Antenna.field.TxPath >= 2))
        {
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_USE);
        }
        else
        {
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_NONE);
        }

        if (pTxRate->CurrMCS < MCS_AUTO)
        {
            WRITE_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg, pTxRate->CurrMCS);
        }

        // Double confirm STBC not use if (MCS >7). Although we already make correct rate table.
        if (READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) > MCS_7)
        {
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_NONE);
        }

        if ((READ_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg) != MODE_HTGREENFIELD) && 
             (pTxRate->Mode <= MODE_VHT))
        {
            WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, pTxRate->Mode);
        }

        if (pTxRate->ShortGI && READ_PHY_CFG_SHORT_GI(pAd, &pEntry->MaxPhyCfg))
        {
            WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg, GI_400);
        }
        else
        {
            WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg, GI_800);
        }

        // Reexam each bandwidth's SGI support.
        if (READ_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg) == GI_400)
        {
            if ((READ_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg) == BW_20) && 
                 (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI20_CAPABLE)))
            {
                WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg, GI_800);
            }
            
            if ((READ_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg) == BW_40) && 
                 (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI40_CAPABLE)))
            {
                WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg, GI_800);
            }
        }

        if (pTxRate->Nss <= NSS_1)
        {
            WRITE_PHY_CFG_NSS(pAd, &pEntry->TxPhyCfg, pTxRate->Nss);
        }

        // VHT Rate table has the Rate tuning algorithm that change the Bandwidth.
        if( VHT_NIC(pAd) && CURRENT_RATETABLE_IS_VHT_TABLE(pTable) )
        {
            WRITE_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg, pTxRate->BW);
        }
        
        // Patch speed error in status page, from XP rate
        if(pPort->CommonCfg.bAutoTxRateSwitch == FALSE)
        {
            WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg, READ_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg));
        }
        
    }
    else
    {
        // for DLS or TDLS
        if (((pPort->PortSubtype == PORTSUBTYPE_STA)||
            ((pAd->OpMode == OPMODE_STAP2P) && (pPort->PortSubtype == PORTSUBTYPE_P2PClient))))
        {   
            if ((pTxRate->STBC == STBC_USE) && 
                 (READ_PHY_CFG_STBC(pAd, &pEntry->MaxPhyCfg) == STBC_USE) && 
                 (pAd->HwCfg.Antenna.field.TxPath >= 2))
            {
                WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_USE);
            }
            else
            {
                WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_NONE);
            }

            if (pTxRate->CurrMCS < MCS_AUTO)
            {
                WRITE_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg, pTxRate->CurrMCS);
            }

            // Double confirm STBC not use if (MCS >7). Although we already make correct rate table.
            if (READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) > MCS_7)
            {
                WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_NONE);
            }

            if (pPort->CommonCfg.TgnControl.field.FixedTXGF == TRUE)
            {
                WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg, MODE_HTGREENFIELD);
            }
            else
            {
                WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg, pTxRate->Mode);
            }
        
            // GI
            if (((pTxRate->ShortGI == GI_400) && (READ_PHY_CFG_SHORT_GI(pAd, &pEntry->MaxPhyCfg) == GI_400)) ||
                (pPort->CommonCfg.TgnControl.field.FixedTxSGI == TRUE))
            {
                WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg, GI_400);
            }
            else
            {
                WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg, GI_800);
            }

            // Reexam SGI support
            if (READ_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg) == GI_400)
            {
                if ((READ_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg) == BW_20) && 
                     (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI20_CAPABLE)))
            {
                    WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg, GI_800);
                }
                
                if ((READ_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg) == BW_40) && 
                     (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI40_CAPABLE)))
                {
                    WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg, GI_800);
                }
            }

            // MODE
            if ((pTxRate->Mode <= MODE_VHT))
            {
                WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, pTxRate->Mode);
            }
                

            if (pTxRate->Nss <= NSS_1)
            {
                WRITE_PHY_CFG_NSS(pAd, &pEntry->TxPhyCfg, pTxRate->Nss);
            }

            // VHT Rate table has the Rate tuning algorithm that change the Bandwidth.
            if( VHT_NIC(pAd) && CURRENT_RATETABLE_IS_VHT_TABLE(pTable) )
            {
                WRITE_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg, pTxRate->BW);
            }
            else
            {
                WRITE_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg, pPort->CommonCfg.TxPhyCfg.Default.BW);
            }

        }
        else
        {
            if ((pTxRate->STBC == STBC_USE) && 
                (READ_PHY_CFG_STBC(pAd, &pPort->CommonCfg.MaxPhyCfg) == STBC_USE) && 
                (pAd->HwCfg.Antenna.field.TxPath >= 2))
            {
                WRITE_PHY_CFG_STBC(pAd, &pPort->CommonCfg.TxPhyCfg, STBC_USE);
            }
            else
            {
                WRITE_PHY_CFG_STBC(pAd, &pPort->CommonCfg.TxPhyCfg, STBC_NONE);
            }           
            
            if (pTxRate->CurrMCS < MCS_AUTO)
            {
                WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg, pTxRate->CurrMCS);
            }

            // Double confirm STBC not use if (MCS >7). Although we already make correct rate table.
            if (READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg) > MCS_7)
            {
                WRITE_PHY_CFG_STBC(pAd, &pPort->CommonCfg.TxPhyCfg, STBC_NONE);
            }

            if (pPort->CommonCfg.TgnControl.field.FixedTXGF == TRUE)
            {
                WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg, MODE_HTGREENFIELD);
            }
            else
            {
                WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg, pTxRate->Mode);
            }

            if ((READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg) != MODE_HTGREENFIELD) && 
                 (pTxRate->Mode <= MODE_VHT))
            {
                WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg, pTxRate->Mode);
            }

            if ((pTxRate->ShortGI == GI_400) && 
                 (READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.MaxPhyCfg) == GI_400))
            {
                WRITE_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg, GI_400);
            }
            else
            {
                WRITE_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg, GI_800);
            }
        
            // Reexam SGI support
            if (READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg) == GI_400)
            {
                if ((READ_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg) == BW_20) && 
                     (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI20_CAPABLE)))
                {
                    WRITE_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg, GI_800);
                }
                
                if ((READ_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg) == BW_40) && 
                     (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI40_CAPABLE)))
            {
                    WRITE_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg, GI_800);
                }
            }

            if (pTxRate->Nss <= NSS_1)
            {
                WRITE_PHY_CFG_NSS(pAd, &pPort->CommonCfg.TxPhyCfg, pTxRate->Nss);
            }

            // VHT Rate table has the Rate tuning algorithm that change the Bandwidth.
            //if( VHT_NIC(pAd) && CURRENT_RATETABLE_IS_VHT_TABLE(pTable))
            {
                PMAC_TABLE_ENTRY pMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
                if(pMacTabEntry == NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
                    return;
                }
                
                DBGPRINT(RT_DEBUG_TRACE,("[%s %d] pTxRate->BW=%x\n",__FUNCTION__, __LINE__,pTxRate->BW));
                DBGPRINT(RT_DEBUG_TRACE,("[%s %d] AP's BW=%x\n",__FUNCTION__, __LINE__,READ_PHY_CFG_BW(pAd, &pMacTabEntry->TxPhyCfg)));
                // Avoid indicating the error Data Rate for OS  
                if (READ_PHY_CFG_BW(pAd, &pMacTabEntry->TxPhyCfg) >= pTxRate->BW)
                {
                    WRITE_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg, pTxRate->BW);
                }   
            }
            
            //
            // Turn RTS/CTS rate to 6Mbps.
            // BW20: 
            // MCS=2,3      Use 18Mbps for RTS
            // MCS=1,9      Use 9Mbps for RTS
            // MCS=0,8,16   Use 6Mbps for RTS
            // BW40:  
            // MCS=1        Use 9Mbps for RTS
            // MCS=0,8      Use 6Mbps for RTS
            //
            
            if (READ_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg) >= MODE_HTMIX)
            {
                if ((READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) == MCS_0) && (READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg) != MCS_0)||
                    ((READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) == MCS_8) && (READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg) != MCS_8))||
                    ((READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) == MCS_16) && (READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg) != MCS_16))||
                    ((READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) == MCS_1) && (READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg) != MCS_1))||
                    ((READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) == MCS_2) && (READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg) != MCS_2))||
                    ((READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) == MCS_3) && (READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg) != MCS_3))||
                    ((READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) == MCS_9) && (READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg) != MCS_9)))
                {
                    WRITE_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg, READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg));
                    
                    if (pPort->MacTab.fAnyBASession)
                    {
                        MtAsicUpdateProtect(pAd, HT_FORCERTSCTS, ALLN_SETPROTECT, TRUE, (BOOLEAN)pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent);
                    }
                    else
                    {
                        MtAsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode, ALLN_SETPROTECT, TRUE, (BOOLEAN)pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent);
                    }
                }
                else if (((READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) != MCS_0) && (READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg) == MCS_0))||
                    ((READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) != MCS_8) && (READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg) == MCS_8))||
                    ((READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) != MCS_16) && (READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg) == MCS_16))||
                    ((READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) != MCS_1) && (READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg) == MCS_1))||
                    ((READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) != MCS_2) && (READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg) == MCS_2))||
                    ((READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) != MCS_3) && (READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg) == MCS_3))||
                    ((READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) != MCS_9) && (READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg) == MCS_9)))
                {
                    MtAsicUpdateProtect(pAd, HT_RTSCTS_6M, ALLN_SETPROTECT, TRUE, (BOOLEAN)pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent);           
                }
            }
            
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, READ_PHY_CFG_STBC(pAd, &pPort->CommonCfg.TxPhyCfg));

            if ((pPort->CommonCfg.IOTestParm.AtherosDIR855 == TRUE) && 
                 (READ_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg) == BW_40))
            {
                WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg, GI_400);
                WRITE_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg, GI_400);
            }
            else
            {
                WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg, READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg));
            }

            WRITE_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg, READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg));
            WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg));
            if( VHT_NIC(pAd) && CURRENT_RATETABLE_IS_VHT_TABLE(pTable) )
            {
                if (READ_PHY_CFG_NSS(pAd, &pPort->CommonCfg.TxPhyCfg) <= pEntry->VhtPeerStaCtrl.MaxNss)
                {
                    WRITE_PHY_CFG_NSS(pAd, &pEntry->TxPhyCfg, READ_PHY_CFG_NSS(pAd, &pPort->CommonCfg.TxPhyCfg));
                }
                else
                {
                    WRITE_PHY_CFG_NSS(pAd, &pEntry->TxPhyCfg, pEntry->VhtPeerStaCtrl.MaxNss);
                }
            }

            // VHT Rate table has the Rate tuning algorithm that change the Bandwidth.
            //if(VHT_NIC(pAd) && CURRENT_RATETABLE_IS_VHT_TABLE(pTable))
            {
                if (READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg) <= pEntry->VhtPeerStaCtrl.MaxBW)
                {
                    WRITE_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg, READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg));
                }
                else
                {
                    WRITE_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg, pEntry->VhtPeerStaCtrl.MaxBW);
                }
            }
        }
    }

    MtAsicMcsLutUpdate(pAd, pEntry);
    
    DBGPRINT_RAW(RT_DEBUG_TRACE,("DRS: MlmeSetTxRate - CurrTxRateIdx=%d, MCS=%d, STBC=%d, ShortGI=%d, Mode=%d, BW=%d, Nss = %d, MaxBW = %d, MaxNss = %d\n", 
        pPort->CommonCfg.TxRateIndex, 
        READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg), 
        READ_PHY_CFG_STBC(pAd, &pPort->CommonCfg.TxPhyCfg), 
        READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg), 
        READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg), 
        READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg), 
        READ_PHY_CFG_NSS(pAd, &pPort->CommonCfg.TxPhyCfg), 
        pEntry->VhtPeerStaCtrl.MaxBW, 
        pEntry->VhtPeerStaCtrl.MaxNss));

}

/*
    ========================================================================
    
    Routine Description:
        AP can advertise STA of their supported MCS. STA will modify their TxRate table.

    Arguments:
        pAd             Pointer to our adapter
        pBasedTable     Original TxRate Table
        pLimitedTable       New TxRate Table
        
    Return Value:
        None
        
    Note:
        
    ========================================================================
*/
VOID MlmeUpdateLimitTxRateTable(
    IN PMP_ADAPTER        pAd,
    IN PUCHAR               pBasedTable,
    IN OUT PUCHAR           pLimitedTable)  
{
    UCHAR RateSwitchTableTmp[100]={0};
    UCHAR TableRowSize = 0;
    UCHAR CurrentRowIndex = 1;
    PRTMP_TX_RATE_SWITCH pCurrTxRate =NULL;
    UCHAR TableMCS,MCSBitMap;
    PUCHAR pRateTable = pBasedTable;
    
    DBGPRINT_RAW(RT_DEBUG_TRACE,("<===== MlmeUpdateLimitTxRateTable"));
    for (CurrentRowIndex = 1; CurrentRowIndex<=*pRateTable; CurrentRowIndex++)
    {
        BOOLEAN bUpdate;
        pCurrTxRate = (PRTMP_TX_RATE_SWITCH)(pRateTable+CurrentRowIndex*SIZE_OF_RATE_TABLE_ENTRY);
        TableMCS = pCurrTxRate->CurrMCS;
        bUpdate = TRUE;

        DBGPRINT_RAW(RT_DEBUG_TRACE,("0x%02x:pCurrTxRate->CurrMCS=%d\n",
            pCurrTxRate->ItemNo, pCurrTxRate->CurrMCS));


        if (pCurrTxRate->Mode >= MODE_HTMIX)
        {
            if (TableMCS<=7)
            {
                MCSBitMap = pAd->StaActive.SupportedHtPhy.MCSSet[0];
                DBGPRINT_RAW(RT_DEBUG_TRACE,("pCurrTxRate->CurrMCS=%d and pAd->StaActive.SupportedHtPhy.MCSSet[0]=0x%02x\n",pCurrTxRate->CurrMCS, pAd->StaActive.SupportedHtPhy.MCSSet[0]));    
            }
            else
            {
                MCSBitMap = pAd->StaActive.SupportedHtPhy.MCSSet[1];
                TableMCS -=8;
                DBGPRINT_RAW(RT_DEBUG_TRACE,("pCurrTxRate->CurrMCS=%d and pAd->StaActive.SupportedHtPhy.MCSSet[0]=0x%02x\n", pCurrTxRate->CurrMCS, pAd->StaActive.SupportedHtPhy.MCSSet[1]));       
            }

            if ((((MCSBitMap>>TableMCS)&0x01) != 0x01))
                bUpdate = FALSE;
        }
        
        if (bUpdate)
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE,("0x%02x->0x%02x, 0x%02x, %d, %d, %d \n",
                        pCurrTxRate->ItemNo, 
                        TableRowSize, 
                        *(pRateTable+CurrentRowIndex*SIZE_OF_RATE_TABLE_ENTRY+1),
                        pCurrTxRate->CurrMCS, 
                        pCurrTxRate->TrainUp, 
                        pCurrTxRate->TrainDown));       
            
            PlatformMoveMemory(&RateSwitchTableTmp[(TableRowSize+1)*SIZE_OF_RATE_TABLE_ENTRY],(pRateTable+CurrentRowIndex*SIZE_OF_RATE_TABLE_ENTRY),SIZE_OF_RATE_TABLE_ENTRY);
            RateSwitchTableTmp[(TableRowSize+1)*SIZE_OF_RATE_TABLE_ENTRY] = TableRowSize;
            TableRowSize++;                     
        }
    }

    RateSwitchTableTmp[0] = TableRowSize;   // Table Size
    RateSwitchTableTmp[1] = TableRowSize-2; // Init Rate Index

    PlatformMoveMemory(pLimitedTable,RateSwitchTableTmp,(TableRowSize+1)*SIZE_OF_RATE_TABLE_ENTRY);
    
    DBGPRINT_RAW(RT_DEBUG_TRACE,("%s: TableRowSize[0x%02x], Init Rate Index[0x%02x] MCSSet[0][0x%02x] MCSSet[1][0x%02x]\n",__FUNCTION__,RateSwitchTableTmp[0],RateSwitchTableTmp[1],pAd->StaActive.SupportedHtPhy.MCSSet[0],pAd->StaActive.SupportedHtPhy.MCSSet[1]));  
    DBGPRINT_RAW(RT_DEBUG_TRACE,("=====> MlmeUpdateLimitTxRateTable"));
}

VOID MlmeSelectTxRateTable(
    IN PMP_ADAPTER        pAd,
    IN PMAC_TABLE_ENTRY     pEntry,
    IN PUCHAR               *ppTable,
    IN PUCHAR               pTableSize,
    IN PUCHAR               pInitTxRateIdx)
{
    PMP_PORT pPort = pAd->PortList[PORT_0];

    // decide the rate table for tuning
    if ((pPort->CommonCfg.TxRateTableSize > 0)||(pPort->CommonCfg.LimitTxRateTableSize > 0))
    {
        *ppTable = RateSwitchTable;
        *pTableSize = RateSwitchTable[0];
        *pInitTxRateIdx = RateSwitchTable[1];

        DBGPRINT_RAW(RT_DEBUG_TRACE,("DRS: rate table is from rate.bin \n"));
    }
    else if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14))
    {
        // CH14 only supports 11b
        *ppTable = RateSwitchTable11B;
        *pTableSize = RateSwitchTable11B[0];
        *pInitTxRateIdx = RateSwitchTable11B[1];

        DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: CH14 only suports 11B\n"));
    }
    else if (ADHOC_ON(pPort))
    {
        // We have already downgrate to legacy mode (used bAdhocNMode=FALSE) in cases of RT3090_RF2020 IC and ProhibitTKIPonHT
        // Here, consider it no more.
        if (VHT_CAPABLE(pAd) && //2 TODO: Check VHT 2x2 STA
             (SUPPORT_AGS(pAd)) && 
             (pAd->HwCfg.Antenna.field.TxPath >= 2) && 
             (FALSE))
        {
            *ppTable = Ags2x2VhtRateTable;
            *pTableSize = Ags2x2VhtRateTable[0];
            *pInitTxRateIdx = Ags2x2VhtRateTable[1];

            DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: VHT 2x2 adhoc, Ags2x2VhtRateTable\n", 
                __FUNCTION__));
        }
        else if (VHT_CAPABLE(pAd) && 
                (SUPPORT_AGS(pAd)) && 
                (pAd->StaActive.SupportedVhtPhy.bVhtEnable ==TRUE) && 
                (pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor1ss == MCS_0_9) && 
                (pAd->HwCfg.Antenna.field.TxPath >= 1) && 
                (pEntry->VhtPeerStaCtrl.bVhtCapable == TRUE) && 
                (pEntry->VhtPeerStaCtrl.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor1ss == MCS_0_9))
        {
            *ppTable = Ags1x1Vht256QAMRateTable;
            *pTableSize = Ags1x1Vht256QAMRateTable[0];
            *pInitTxRateIdx = Ags1x1Vht256QAMRateTable[1];

            DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: VHT 1x1 adhoc, Ags1x1Vht256QAMRateTable\n", 
                __FUNCTION__));
        }
        else if (VHT_CAPABLE(pAd) && //2 TODO: Check VHT 1x1 STA
                    (SUPPORT_AGS(pAd)) && 
                    (pAd->HwCfg.Antenna.field.TxPath >= 1) && 
                    (FALSE))
        {
            *ppTable = Ags1x1VhtRateTable;
            *pTableSize = Ags1x1VhtRateTable[0];
            *pInitTxRateIdx = Ags1x1VhtRateTable[1];

            DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: VHT 1x1 adhoc, Ags1x1VhtRateTable\n", 
                __FUNCTION__));
        }
        else if (SUPPORT_AGS(pAd) && 
             (pEntry->HTCapability.MCSSet[0] == 0xFF) && 
             (pEntry->HTCapability.MCSSet[1] == 0xFF) && 
             (pEntry->HTCapability.MCSSet[2] == 0xFF) && 
             (pAd->HwCfg.Antenna.field.TxPath == 3))
        {// 11N 3S Adhoc
            *ppTable = AGS3x3HTRateTable;
            *pTableSize = AGS3x3HTRateTable[0];
            *pInitTxRateIdx = AGS3x3HTRateTable[1];

            DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 3S adhoc, AGS3x3HTRateTable\n", 
                __FUNCTION__));
        }
        else if ((pAd->StaCfg.bAdhocNMode) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) &&
            (pEntry->HTCapability.MCSSet[0] == 0xff) && ((pEntry->HTCapability.MCSSet[1] == 0x00) ||
            ((pAd->HwCfg.Antenna.field.TxPath == 1) && (pAd->HwCfg.NicConfig3.field.TxStream == 0xF)) || (pAd->HwCfg.NicConfig3.field.TxStream == 1)))
        {
            if (SUPPORT_AGS(pAd))
            {
                if (pPort->Channel <= 14)
                {
                    *ppTable = AGS1x1HTRateTable;
                    *pTableSize = AGS1x1HTRateTable[0];
                    *pInitTxRateIdx = AGS1x1HTRateTable[1];

                    DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 1S adhoc, AGS1x1HTRateTable\n", 
                        __FUNCTION__));
                }
                else
                {
                    *ppTable = AGS1x1HTRateTable5G;
                    *pTableSize = AGS1x1HTRateTable5G[0];
                    *pInitTxRateIdx = AGS1x1HTRateTable5G[1];
                        
                    DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 1S adhoc, AGS1x1HTRateTable5G\n", 
                        __FUNCTION__));
                }
            }
            else
            {
                if (pPort->Channel <= 14)
                {
                    // 11N 1S Adhoc
                    *ppTable = RateSwitchTable11N1S;
                    *pTableSize = RateSwitchTable11N1S[0];
                    *pInitTxRateIdx = RateSwitchTable11N1S[1];
                }
                else
                {
                    // 11N 1S Adhoc
                    *ppTable = RateSwitchTable11N1SForABand;
                    *pTableSize = RateSwitchTable11N1SForABand[0];
                    *pInitTxRateIdx = RateSwitchTable11N1SForABand[1];
                }
                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: 11N 1S Adhoc \n"));
            }
        }
        else if ((pAd->StaCfg.bAdhocNMode) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) &&
                (pEntry->HTCapability.MCSSet[0] == 0xff) && (pEntry->HTCapability.MCSSet[1] == 0xff) &&
            (((pAd->HwCfg.Antenna.field.TxPath >= 2) && (pAd->HwCfg.NicConfig3.field.TxStream == 0xF)) || (pAd->HwCfg.NicConfig3.field.TxStream >= 2)))
        {
            if (SUPPORT_AGS(pAd))
            {
                *ppTable = AGS2x2HTRateTable;
                *pTableSize = AGS2x2HTRateTable[0];
                *pInitTxRateIdx = AGS2x2HTRateTable[1];

                DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 2S adhoc, AGS2x2HTRateTable\n", 
                    __FUNCTION__));
            }
            else
            {
                // 11N 2S Adhoc
                if (pPort->Channel <= 14)
                {
                    *ppTable = RateSwitchTable11N2S;
                    *pTableSize = RateSwitchTable11N2S[0];
                    *pInitTxRateIdx = RateSwitchTable11N2S[1];
                }
                else
                {
                    *ppTable = RateSwitchTable11N2SForABand;
                    *pTableSize = RateSwitchTable11N2SForABand[0];
                    *pInitTxRateIdx = RateSwitchTable11N2SForABand[1];
                }
                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: 11N 2S Adhoc \n"));
            }
        }
        else if ((pAd->StaCfg.bAdhocNMode) && 
                    (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) &&
                    (READ_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg) >= MODE_HTMIX) && 
                    (pEntry->HTCapability.MCSSet[0] != 0xff) && 
                    (pAd->HwCfg.Antenna.field.TxPath >= 1))
        {
            if (SUPPORT_AGS(pAd))
            {
                if (pPort->Channel <= 14)
                {
                    *ppTable = AGS1x1HTRateTable;
                    *pTableSize = AGS1x1HTRateTable[0];
                    *pInitTxRateIdx = AGS1x1HTRateTable[1];

                    DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 1S adhoc, AGS1x1HTRateTable\n", 
                        __FUNCTION__));
                }
                else
                {
                    *ppTable = AGS1x1HTRateTable5G;
                    *pTableSize = AGS1x1HTRateTable5G[0];
                    *pInitTxRateIdx = AGS1x1HTRateTable5G[1];

                    DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 1S adhoc, AGS1x1HTRateTable5G\n", 
                        __FUNCTION__));
                }
            }
            else
            {
                if (pPort->Channel <= 14)
                {
                    // 11N 1S Adhoc for fixed rates
                    *ppTable = RateSwitchTable11N1S;
                    *pTableSize = RateSwitchTable11N1S[0];
                    *pInitTxRateIdx = RateSwitchTable11N1S[1];
                }
                else
                {
                    // 11N 1S Adhoc for fixed rates
                    *ppTable = RateSwitchTable11N1SForABand;
                    *pTableSize = RateSwitchTable11N1SForABand[0];
                    *pInitTxRateIdx = RateSwitchTable11N1SForABand[1];
                }
                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: unkown mode,default use 11N 1S Adhoc \n"));
            }
        }
        else if ((pEntry->RateLen == 4) && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0))
        {
            *ppTable = RateSwitchTable11B;
            *pTableSize = RateSwitchTable11B[0];
            *pInitTxRateIdx = RateSwitchTable11B[1];

            DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: B only Adhoc \n"));
        }
        else if ((pEntry->RateLen == 8) && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0))
        {
            *ppTable = RateSwitchTable11G;
            *pTableSize = RateSwitchTable11G[0];
            *pInitTxRateIdx = RateSwitchTable11G[1];

            DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: A/G Adhoc \n"));
        }
        else
        {
            if (pPort->Channel > 14)
            {
                *ppTable = RateSwitchTable11G;
                *pTableSize = RateSwitchTable11G[0];
                *pInitTxRateIdx = RateSwitchTable11G[1];

                DBGPRINT_RAW(RT_DEBUG_TRACE,("DRS: pEntry->RateLen  = %d, A Mode Adhoc \n", pEntry->RateLen ));
            }
            else
            {
                *ppTable = RateSwitchTable11BG;
                *pTableSize = RateSwitchTable11BG[0];
                *pInitTxRateIdx = RateSwitchTable11BG[1];

                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: pEntry->RateLen  = %d, B/G mixed Adhoc \n", pEntry->RateLen ));
            }
        }
    }
    else
    {
        //DRS for direct link (DLS/TDLS/BT30HS/VSta-P2P)
        if ((MlmeSyncGetRoleTypeByWlanIdx(pPort, pEntry->Aid ) ==ROLE_WLANIDX_CLIENT) ||
            (MlmeSyncGetRoleTypeByWlanIdx(pPort, pEntry->Aid ) ==ROLE_WLANIDX_BSSID))
        {

            if (VHT_CAPABLE(pAd) && //2 TODO: Check VHT 2x2 STA
                 (SUPPORT_AGS(pAd)) && 
                 (pAd->HwCfg.Antenna.field.TxPath >= 2) && 
                 (FALSE))
            {
                *ppTable = Ags2x2VhtRateTable;
                *pTableSize = Ags2x2VhtRateTable[0];
                *pInitTxRateIdx = Ags2x2VhtRateTable[1];

                DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: VHT 2x2 DLS/TDLS/BT30HS/VStat-P2P, Ags2x2VhtRateTable\n", 
                    __FUNCTION__));
            }
            else if (VHT_CAPABLE(pAd) && 
                     (SUPPORT_AGS(pAd) && 
                     (pAd->StaActive.SupportedVhtPhy.bVhtEnable ==TRUE) && 
                     (pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor1ss == MCS_0_9) && 
                     (pAd->HwCfg.Antenna.field.TxPath >= 1) && 
                     (pEntry->VhtPeerStaCtrl.bVhtCapable == TRUE) && 
                     (pEntry->VhtPeerStaCtrl.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor1ss == MCS_0_9)))
            {
                *ppTable = Ags1x1Vht256QAMRateTable;
                *pTableSize = Ags1x1Vht256QAMRateTable[0];
                *pInitTxRateIdx = Ags1x1Vht256QAMRateTable[1];

                DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: VHT 1x1 DLS/TDLS/BT30HS/VStat-P2P, Ags1x1Vht256QAMRateTable\n", 
                    __FUNCTION__));
            }
            else if (VHT_CAPABLE(pAd) && //2 TODO: Check VHT 1x1 STA
                        (SUPPORT_AGS(pAd)) && 
                        (pAd->HwCfg.Antenna.field.TxPath >= 1) && 
                        (FALSE))
            {
                *ppTable = Ags1x1VhtRateTable;
                *pTableSize = Ags1x1VhtRateTable[0];
                *pInitTxRateIdx = Ags1x1VhtRateTable[1];

                DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: VHT 1x1 DLS/TDLS/BT30HS/VStat-P2P, Ags1x1VhtRateTable\n", 
                    __FUNCTION__));
            }
            else if (SUPPORT_AGS(pAd) && 
                 (pEntry->HTCapability.MCSSet[0] == 0xFF) && 
                 (pEntry->HTCapability.MCSSet[1] == 0xFF) && 
                 (pEntry->HTCapability.MCSSet[2] == 0xFF) && 
                 (pAd->HwCfg.Antenna.field.TxPath == 3))
            {// 11N 3S DLS
                *ppTable = AGS3x3HTRateTable;
                *pTableSize = AGS3x3HTRateTable[0];
                *pInitTxRateIdx = AGS3x3HTRateTable[1];

                DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 3S DLS, AGS3x3HTRateTable\n", 
                    __FUNCTION__));
            }
            else if ((pEntry->RateLen == 4) && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0))
            {// B only
                *ppTable = RateSwitchTable11B;
                *pTableSize = RateSwitchTable11B[0];
                *pInitTxRateIdx = RateSwitchTable11B[1];
                
                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: B only DL \n"));
            }
            else if ((pEntry->RateLen > 8) && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0))
            {// B/G  mixed
                *ppTable = RateSwitchTable11BG;
                *pTableSize = RateSwitchTable11BG[0];
                *pInitTxRateIdx = RateSwitchTable11BG[1];

                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: B/G mixed\n"));
            }
            else if ((pEntry->RateLen == 8) && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0))
            {// G only
                *ppTable = RateSwitchTable11G;
                *pTableSize = RateSwitchTable11G[0];
                *pInitTxRateIdx = RateSwitchTable11G[1];

                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: A/G DL \n"));
            }
            else if ((pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0) && (pEntry->HTCapability.MCSSet[2] == 0))
            {
                *ppTable = RateSwitchTable11BGAll;
                *pTableSize = RateSwitchTable11BGAll[0];
                *pInitTxRateIdx = RateSwitchTable11BGAll[1];

                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: Not 11n STA \n"));
            }
            else if ((pEntry->RateLen == 12) && (pEntry->HTCapability.MCSSet[0] == 0xff) &&
                ((pEntry->HTCapability.MCSSet[1] == 0x00) || ((pAd->HwCfg.Antenna.field.TxPath == 1) && (pAd->HwCfg.NicConfig3.field.TxStream == 0xF)) ||
                (pAd->HwCfg.NicConfig3.field.TxStream == 1)))
            {// 11BGN 1S DLS
                if (SUPPORT_AGS(pAd))
                {
                    if (pPort->Channel <= 14)
                    {
                        *ppTable = AGS1x1HTRateTable;
                        *pTableSize = AGS1x1HTRateTable[0];
                        *pInitTxRateIdx = AGS1x1HTRateTable[1];

                        DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 1S DLS, AGS1x1HTRateTable\n", 
                            __FUNCTION__));
                    }
                    else
                    {
                        *ppTable = AGS1x1HTRateTable5G;
                        *pTableSize = AGS1x1HTRateTable5G[0];
                        *pInitTxRateIdx = AGS1x1HTRateTable5G[1];
                                    
                        DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 1S DLS, AGS1x1HTRateTable5G\n", 
                            __FUNCTION__));
                    }
                }
                else
                {
                    if (pPort->Channel <= 14)
                    {
                        *ppTable = RateSwitchTable11BGN1S;
                        *pTableSize = RateSwitchTable11BGN1S[0];
                        *pInitTxRateIdx = RateSwitchTable11BGN1S[1];
                    }
                    else
                    {
                        *ppTable = RateSwitchTable11BGN1SForABand;
                        *pTableSize = RateSwitchTable11BGN1SForABand[0];
                        *pInitTxRateIdx = RateSwitchTable11BGN1SForABand[1];
                    }
                    DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: 11BGN 1S\n"));
                }
            }
            else if ((pEntry->RateLen == 12) && (pEntry->HTCapability.MCSSet[0] == 0xff) &&
                (pEntry->HTCapability.MCSSet[1] == 0xff) && (((pAd->HwCfg.Antenna.field.TxPath >= 2) && (pAd->HwCfg.NicConfig3.field.TxStream == 0xF)) ||
                (pAd->HwCfg.NicConfig3.field.TxStream >= 2)))
            {// 11BGN 2S DLS
                if (SUPPORT_AGS(pAd))
                {
                    *ppTable = AGS2x2HTRateTable;
                    *pTableSize = AGS2x2HTRateTable[0];
                    *pInitTxRateIdx = AGS2x2HTRateTable[1];

                    DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 2S DLS, AGS2x2HTRateTable\n", 
                        __FUNCTION__));
                }
                else
                {
                    if (pAd->HwCfg.LatchRfRegs.Channel <= 14)
                    {
                        *ppTable = RateSwitchTable11BGN2S;
                        *pTableSize = RateSwitchTable11BGN2S[0];
                        *pInitTxRateIdx = RateSwitchTable11BGN2S[1];
                    }
                    else
                    {
                        *ppTable = RateSwitchTable11BGN2SForABand;
                        *pTableSize = RateSwitchTable11BGN2SForABand[0];
                        *pInitTxRateIdx = RateSwitchTable11BGN2SForABand[1];
                    }
                    DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: 11BGN 2S \n"));
                }
            }
            else if ((pEntry->HTCapability.MCSSet[0] == 0xff) && ((pEntry->HTCapability.MCSSet[1] == 0x00) ||
                ((pAd->HwCfg.Antenna.field.TxPath == 1) && (pAd->HwCfg.NicConfig3.field.TxStream == 0xF)) || (pAd->HwCfg.NicConfig3.field.TxStream == 1)))
            {// 11N 1S DLS
                if (SUPPORT_AGS(pAd))
                {
                    if (pPort->Channel <= 14)
                    {
                        *ppTable = AGS1x1HTRateTable;
                        *pTableSize = AGS1x1HTRateTable[0];
                        *pInitTxRateIdx = AGS1x1HTRateTable[1];

                        DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 1S DLS, AGS1x1HTRateTable\n", 
                            __FUNCTION__));
                    }
                    else
                    {
                        *ppTable = AGS1x1HTRateTable5G;
                        *pTableSize = AGS1x1HTRateTable5G[0];
                        *pInitTxRateIdx = AGS1x1HTRateTable5G[1];
                    
                        DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 1S DLS, AGS1x1HTRateTable5G\n", 
                            __FUNCTION__));
                    }
                }
                else
                {
                    if (pPort->Channel <= 14)
                    {
                        *ppTable = RateSwitchTable11N1S;
                        *pTableSize = RateSwitchTable11N1S[0];
                        *pInitTxRateIdx = RateSwitchTable11N1S[1];
                    }
                    else
                    {
                        *ppTable = RateSwitchTable11N1SForABand;
                        *pTableSize = RateSwitchTable11N1SForABand[0];
                        *pInitTxRateIdx = RateSwitchTable11N1SForABand[1];
                    }

                    DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: 11N 1S DL \n"));
                }
            }
            else if ((pEntry->HTCapability.MCSSet[0] == 0xff) && (pEntry->HTCapability.MCSSet[1] == 0xff) &&
                (((pAd->HwCfg.Antenna.field.TxPath >= 2) && (pAd->HwCfg.NicConfig3.field.TxStream == 0xF)) || (pAd->HwCfg.NicConfig3.field.TxStream >= 2)))
            {// 11N 2S DLS
                if (SUPPORT_AGS(pAd))
                {
                    *ppTable = AGS2x2HTRateTable;
                    *pTableSize = AGS2x2HTRateTable[0];
                    *pInitTxRateIdx = AGS2x2HTRateTable[1];

                    DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 2S DLS, AGS2x2HTRateTable\n", 
                        __FUNCTION__));
                }
                else
                {
                    if (pAd->HwCfg.LatchRfRegs.Channel <= 14)
                    {
                        *ppTable = RateSwitchTable11N2S;
                        *pTableSize = RateSwitchTable11N2S[0];
                        *pInitTxRateIdx = RateSwitchTable11N2S[1];

                    }
                    else
                    {
                        *ppTable = RateSwitchTable11N2SForABand;
                        *pTableSize = RateSwitchTable11N2SForABand[0];
                        *pInitTxRateIdx = RateSwitchTable11N2SForABand[1];
                    }
                    DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: 11N 2S DL \n"));
                }
            }
            else
            {
                if (pPort->Channel <= 14)
                {
                    *ppTable = RateSwitchTable11N1S;
                    *pTableSize = RateSwitchTable11N1S[0];
                    *pInitTxRateIdx = RateSwitchTable11N1S[1];
                }
                else
                {
                    *ppTable = RateSwitchTable11N1SForABand;
                    *pTableSize = RateSwitchTable11N1SForABand[0];
                    *pInitTxRateIdx = RateSwitchTable11N1SForABand[1];
                }
                DBGPRINT_RAW(RT_DEBUG_ERROR,("DRS: unkown mode,default use 11N 1S DL \n"));
            }
            return;
        }

        //DRS for AP
        if(((pPort->CommonCfg.bProhibitTKIPonHT) && 
            ((IS_WEP_STATUS_ON(pPort->PortCfg.WepStatus))||
             (pPort->PortCfg.CipherAlg == CIPHER_TKIP)||
             (pPort->PortCfg.CipherAlg == CIPHER_TKIP_NO_MIC))))
        {
            if (pPort->CommonCfg.MaxTxRate <= RATE_11)
            {
                *ppTable = RateSwitchTable11B;
                *pTableSize = RateSwitchTable11B[0];
                *pInitTxRateIdx = RateSwitchTable11B[1];

                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: B only AP (MaxTx=%d,MinTx=%d)\n", pPort->CommonCfg.MaxTxRate, pPort->CommonCfg.MinTxRate));
            }
            else if ((pPort->CommonCfg.MaxTxRate > RATE_11) && (pPort->CommonCfg.MinTxRate > RATE_11))
            {
                *ppTable = RateSwitchTable11G;
                *pTableSize = RateSwitchTable11G[0];
                *pInitTxRateIdx = RateSwitchTable11G[1];

                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: A/G AP (MaxTx=%d,MinTx=%d)\n", pPort->CommonCfg.MaxTxRate, pPort->CommonCfg.MinTxRate));
            }
            else        
            {
                *ppTable = RateSwitchTable11BG;
                *pTableSize = RateSwitchTable11BG[0];
                *pInitTxRateIdx = RateSwitchTable11BG[1];
                
                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: B/G mixed AP (MaxTx=%d,MinTx=%d)\n", pPort->CommonCfg.MaxTxRate, pPort->CommonCfg.MinTxRate));
            }   
        }   
        //2 TODO: Check VHT 2x2 STA
        else if (VHT_CAPABLE(pAd)
                && (SUPPORT_AGS(pAd))
                && (pAd->HwCfg.Antenna.field.TxPath >= 2)   // we support 2x2
                && (pAd->StaActive.SupportedVhtPhy.bVhtEnable==TRUE)
                // the other side support 2x2
                && (pAd->StaActive.SupportedVhtPhy.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor2ss!=NOT_SUPPORTED))
        {
            *ppTable = Ags2x2VhtRateTable;
            *pTableSize = Ags2x2VhtRateTable[0];
            *pInitTxRateIdx = Ags2x2VhtRateTable[1];

            DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: VHT 2x2 AP, Ags2x2VhtRateTable\n", 
                __FUNCTION__));
        }
        else if (VHT_CAPABLE(pAd) && 
                 (SUPPORT_AGS(pAd)) && 
                 (pAd->StaActive.SupportedVhtPhy.bVhtEnable ==TRUE) && 
                 (pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor1ss == MCS_0_9) && 
                 (pAd->HwCfg.Antenna.field.TxPath >= 1) && 
                 (pEntry->VhtPeerStaCtrl.bVhtCapable == TRUE) && 
                 (pEntry->VhtPeerStaCtrl.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor1ss == MCS_0_9))
        {
            *ppTable = Ags1x1Vht256QAMRateTable;
            *pTableSize = Ags1x1Vht256QAMRateTable[0];
            *pInitTxRateIdx = Ags1x1Vht256QAMRateTable[1];

            DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s, LINE%d: VHT 1x1 AP, Ags1x1Vht256QAMRateTable\n", 
                __FUNCTION__,
                __LINE__));
        }
        //2 TODO: Check VHT 1x1 STA
        else if (VHT_CAPABLE(pAd)
                && (SUPPORT_AGS(pAd)) 
                && (pAd->HwCfg.Antenna.field.TxPath >= 1) 
                && (pAd->StaActive.SupportedVhtPhy.bVhtEnable==TRUE)
                // the other side support 1x1
                && (pAd->StaActive.SupportedVhtPhy.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor1ss!=NOT_SUPPORTED))
        {
            *ppTable = Ags1x1VhtRateTable;
            *pTableSize = Ags1x1VhtRateTable[0];
            *pInitTxRateIdx = Ags1x1VhtRateTable[1];

            DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s, LINE%d: VHT 1x1 AP, Ags1x1VhtRateTable\n", 
                __FUNCTION__,
                __LINE__));
        }
        else if (SUPPORT_AGS(pAd) && 
                    (pAd->StaActive.SupportedHtPhy.MCSSet[0] == 0xFF) && 
                    (pAd->StaActive.SupportedHtPhy.MCSSet[1] == 0xFF) && 
                    (pAd->StaActive.SupportedHtPhy.MCSSet[2] == 0xFF) && 
                    (pAd->HwCfg.Antenna.field.TxPath == 3))
        {// 11N 3S 
            *ppTable = AGS3x3HTRateTable;
            *pTableSize = AGS3x3HTRateTable[0];
            *pInitTxRateIdx = AGS3x3HTRateTable[1];

            DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 3S AP, AGS3x3HTRateTable\n", 
                __FUNCTION__));
        }
        else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 12) && (pAd->StaActive.SupportedHtPhy.MCSSet[0] == 0xff) &&
            ((pAd->StaActive.SupportedHtPhy.MCSSet[1] == 0x00) || ((pAd->HwCfg.Antenna.field.TxPath == 1) && (pAd->HwCfg.NicConfig3.field.TxStream == 0xF)) ||
            (pAd->HwCfg.NicConfig3.field.TxStream == 1)))
        {// 11BGN 1S AP
            if (SUPPORT_AGS(pAd))
            {
                if (pPort->Channel <= 14)
                {
                    *ppTable = AGS1x1HTRateTable;
                    *pTableSize = AGS1x1HTRateTable[0];
                    *pInitTxRateIdx = AGS1x1HTRateTable[1];

                    DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 1S AP, AGS1x1HTRateTable\n", 
                        __FUNCTION__));
                }
                else
                {
                    *ppTable = AGS1x1HTRateTable5G;
                    *pTableSize = AGS1x1HTRateTable5G[0];
                    *pInitTxRateIdx = AGS1x1HTRateTable5G[1];           
                
                    DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 1S AP, AGS1x1HTRateTable5G\n", 
                        __FUNCTION__));
                }
            }
            else
            {
                *ppTable = RateSwitchTable11BGN1S;
                *pTableSize = RateSwitchTable11BGN1S[0];
                *pInitTxRateIdx = RateSwitchTable11BGN1S[1];

                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: 11BGN 1S AP \n"));
            }
        }
        else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 12) && (pAd->StaActive.SupportedHtPhy.MCSSet[0] == 0xff) &&
            (pAd->StaActive.SupportedHtPhy.MCSSet[1] == 0xff) && (((pAd->HwCfg.Antenna.field.TxPath >= 2) && (pAd->HwCfg.NicConfig3.field.TxStream == 0xF)) ||
            (pAd->HwCfg.NicConfig3.field.TxStream >= 2)))
        {// 11BGN 2S AP
            if (SUPPORT_AGS(pAd))
            {
                *ppTable = AGS2x2HTRateTable;
                *pTableSize = AGS2x2HTRateTable[0];
                *pInitTxRateIdx = AGS2x2HTRateTable[1];

                DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 2S AP, AGS2x2HTRateTable\n", 
                    __FUNCTION__));
            }
            else
            {
                if (pPort->Channel <= 14)
                {
                    *ppTable = RateSwitchTable11BGN2S;
                    *pTableSize = RateSwitchTable11BGN2S[0];
                    *pInitTxRateIdx = RateSwitchTable11BGN2S[1];
                }
                else
                {
                    *ppTable = RateSwitchTable11BGN2SForABand;
                    *pTableSize = RateSwitchTable11BGN2SForABand[0];
                    *pInitTxRateIdx = RateSwitchTable11BGN2SForABand[1];
                }
                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: 11BGN 2S AP \n"));
            }
        }
        else if ((pAd->StaActive.SupportedHtPhy.MCSSet[0] == 0xff) && ((pAd->StaActive.SupportedHtPhy.MCSSet[1] == 0x00) ||
            ((pAd->HwCfg.Antenna.field.TxPath == 1) && (pAd->HwCfg.NicConfig3.field.TxStream == 0xF)) || (pAd->HwCfg.NicConfig3.field.TxStream == 1)))
        {// 11N 1S AP
            if (SUPPORT_AGS(pAd))
            {
                if (pPort->Channel <= 14)
                {
                    *ppTable = AGS1x1HTRateTable;
                    *pTableSize = AGS1x1HTRateTable[0];
                    *pInitTxRateIdx = AGS1x1HTRateTable[1];

                    DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 1S AP, AGS1x1HTRateTable\n", 
                        __FUNCTION__));
                }
                else
                {
                    *ppTable = AGS1x1HTRateTable5G;
                    *pTableSize = AGS1x1HTRateTable5G[0];
                    *pInitTxRateIdx = AGS1x1HTRateTable5G[1];

                    DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 1S AP, AGS1x1HTRateTable5G\n", 
                        __FUNCTION__));
                }
            }
            else
            {
                if (pPort->Channel <= 14)
                {
                    *ppTable = RateSwitchTable11N1S;
                    *pTableSize = RateSwitchTable11N1S[0];
                    *pInitTxRateIdx = RateSwitchTable11N1S[1];
                }
                else
                {
                    *ppTable = RateSwitchTable11N1SForABand;
                    *pTableSize = RateSwitchTable11N1SForABand[0];
                    *pInitTxRateIdx = RateSwitchTable11N1SForABand[1];
                }
                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: 11N 1S AP \n"));
            }
        }
        else if ((pAd->StaActive.SupportedHtPhy.MCSSet[0] == 0xff) && (pAd->StaActive.SupportedHtPhy.MCSSet[1] == 0xff) &&
            (((pAd->HwCfg.Antenna.field.TxPath >= 2) && (pAd->HwCfg.NicConfig3.field.TxStream == 0xF)) || (pAd->HwCfg.NicConfig3.field.TxStream >= 2)))
        {// 11N 2S AP
            if (SUPPORT_AGS(pAd))
            {
                *ppTable = AGS2x2HTRateTable;
                *pTableSize = AGS2x2HTRateTable[0];
                *pInitTxRateIdx = AGS2x2HTRateTable[1];

                DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: 11N 2S AP, AGS2x2HTRateTable\n", 
                    __FUNCTION__));
            }
            else
            {
                if (pPort->Channel <= 14)
                {
                    *ppTable = RateSwitchTable11N2S;
                    *pTableSize = RateSwitchTable11N2S[0];
                    *pInitTxRateIdx = RateSwitchTable11N2S[1];
                }
                else
                {
                    *ppTable = RateSwitchTable11N2SForABand;
                    *pTableSize = RateSwitchTable11N2SForABand[0];
                    *pInitTxRateIdx = RateSwitchTable11N2SForABand[1];
                }
                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: 11N 2S AP \n"));
            }
        }
        else if ((pAd->StaActive.SupRateLen == 4) && (pAd->StaActive.ExtRateLen == 0) && (pAd->StaActive.SupportedHtPhy.MCSSet[0] == 0) && (pAd->StaActive.SupportedHtPhy.MCSSet[1] == 0))
        {// B only AP
            *ppTable = RateSwitchTable11B;
            *pTableSize = RateSwitchTable11B[0];
            *pInitTxRateIdx = RateSwitchTable11B[1];

            DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: B only AP \n"));
        }
        else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen > 8) && (pAd->StaActive.SupportedHtPhy.MCSSet[0] == 0) && (pAd->StaActive.SupportedHtPhy.MCSSet[1] == 0))
        {// B/G  mixed AP
            *ppTable = RateSwitchTable11BG;
            *pTableSize = RateSwitchTable11BG[0];
            *pInitTxRateIdx = RateSwitchTable11BG[1];
                
            DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: B/G mixed AP \n"));
        }
        else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 8) && (pAd->StaActive.SupportedHtPhy.MCSSet[0] == 0) && (pAd->StaActive.SupportedHtPhy.MCSSet[1] == 0))
        {// G only AP
            *ppTable = RateSwitchTable11G;
            *pTableSize = RateSwitchTable11G[0];
            *pInitTxRateIdx = RateSwitchTable11G[1];
            
            DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: A/G AP \n"));
        }
        else if ((pAd->StaActive.SupportedHtPhy.MCSSet[0] == 0) && (pAd->StaActive.SupportedHtPhy.MCSSet[1] == 0))
        {// Legacy mode
            if (pPort->CommonCfg.MaxTxRate <= RATE_11)
            {
                *ppTable = RateSwitchTable11B;
                *pTableSize = RateSwitchTable11B[0];
                *pInitTxRateIdx = RateSwitchTable11B[1];
                
                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: B only AP (MaxTx=%d,MinTx=%d)\n", pPort->CommonCfg.MaxTxRate, pPort->CommonCfg.MinTxRate));
            }
            else if ((pPort->CommonCfg.MaxTxRate > RATE_11) && (pPort->CommonCfg.MinTxRate > RATE_11))
            {
                *ppTable = RateSwitchTable11G;
                *pTableSize = RateSwitchTable11G[0];
                *pInitTxRateIdx = RateSwitchTable11G[1];

                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: A/G AP (MaxTx=%d,MinTx=%d)\n", pPort->CommonCfg.MaxTxRate, pPort->CommonCfg.MinTxRate));
            }
            else        
            {
                *ppTable = RateSwitchTable11BG;
                *pTableSize = RateSwitchTable11BG[0];
                *pInitTxRateIdx = RateSwitchTable11BG[1];

                DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: B/G mixed AP (MaxTx=%d,MinTx=%d)\n", pPort->CommonCfg.MaxTxRate, pPort->CommonCfg.MinTxRate));
            }
        }
        else
        {
            if (SUPPORT_AGS(pAd) && (pAd->HwCfg.Antenna.field.TxPath == 3))
            {
                *ppTable = AGS3x3HTRateTable;
                *pTableSize = AGS3x3HTRateTable[0];
                *pInitTxRateIdx = AGS3x3HTRateTable[1];

                DBGPRINT_RAW(RT_DEBUG_TRACE,("AGS: %s: Unknown adhoc, DLS or AP, AGS3x3HTRateTable\n", 
                    __FUNCTION__));
            }
            else
            {
                if (pPort->Channel <= 14)
                {
                    if (pAd->HwCfg.Antenna.field.TxPath == 1)
                    {
                        *ppTable = RateSwitchTable11N1S;
                        *pTableSize = RateSwitchTable11N1S[0];
                        *pInitTxRateIdx = RateSwitchTable11N1S[1];
                        
                        DBGPRINT_RAW(RT_DEBUG_ERROR,("DRS: unkown mode,default use 11N 1S AP \n"));
                    }
                    else
                    {
                        *ppTable = RateSwitchTable11N2S;
                        *pTableSize = RateSwitchTable11N2S[0];
                        *pInitTxRateIdx = RateSwitchTable11N2S[1];
                        
                        DBGPRINT_RAW(RT_DEBUG_ERROR,("DRS: unkown mode,default use 11N 2S AP \n"));
                    }
                }
                else
                {
                    if (pAd->HwCfg.Antenna.field.TxPath == 1)
                    {
                        *ppTable = RateSwitchTable11N1S;
                        *pTableSize = RateSwitchTable11N1S[0];
                        *pInitTxRateIdx = RateSwitchTable11N1S[1];

                        DBGPRINT_RAW(RT_DEBUG_ERROR,("DRS: unkown mode,default use 11N 1S AP \n"));
                    }
                    else
                    {
                        *ppTable = RateSwitchTable11N2SForABand;
                        *pTableSize = RateSwitchTable11N2SForABand[0];
                        *pInitTxRateIdx = RateSwitchTable11N2SForABand[1];

                        DBGPRINT_RAW(RT_DEBUG_ERROR,("DRS: unkown mode,default use 11N 2S AP \n"));
                    }
                }
            }
        
            if (!SUPPORT_AGS(pAd) && (pPort->CommonCfg.LimitTxRateTableSize == 0) &&
            (((pAd->StaActive.SupportedHtPhy.MCSSet[0]!=0x00)&&(pAd->StaActive.SupportedHtPhy.MCSSet[0]!=0xFF)) ||((pAd->StaActive.SupportedHtPhy.MCSSet[1]!=0x00)&&(pAd->StaActive.SupportedHtPhy.MCSSet[1]!=0xFF))))
            {       
                MlmeUpdateLimitTxRateTable(pAd,*ppTable,RateSwitchTable);
                *ppTable = RateSwitchTable;
                *pTableSize = pPort->CommonCfg.LimitTxRateTableSize = RateSwitchTable[0];
                *pInitTxRateIdx = RateSwitchTable[1];

                DBGPRINT_RAW(RT_DEBUG_TRACE,("DRS: unknown mode,default use LimitTxRateTable; LatchRfRegs Channel[%d], CommonCfg Channel[%d]\n",pAd->HwCfg.LatchRfRegs.Channel,pPort->Channel));

                if (*pTableSize == 0)
                {
                    DBGPRINT_RAW(RT_DEBUG_TRACE,("DRS: unknown mode,LimitTxRateTable don't have any Tx Rate. Still use RateSwitchTable11N2SForABand Table\n"));
                    *ppTable = RateSwitchTable11N2SForABand;
                    *pTableSize = RateSwitchTable11N2SForABand[0];
                    *pInitTxRateIdx = RateSwitchTable11N2SForABand[1];                  
                }
                else
                    AsicUpdateAutoFallBackTable(pAd, RateSwitchTable, pEntry);
            }
            
            DBGPRINT_RAW(RT_DEBUG_ERROR,("DRS: unkown mode (SupRateLen=%d, ExtRateLen=%d, MCSSet[0]=0x%x, MCSSet[1]=0x%x, TxStream=%d, RxStream=%d)\n",
                pAd->StaActive.SupRateLen, pAd->StaActive.ExtRateLen, pAd->StaActive.SupportedHtPhy.MCSSet[0], pAd->StaActive.SupportedHtPhy.MCSSet[1],
                pAd->HwCfg.NicConfig3.field.TxStream, pAd->HwCfg.NicConfig3.field.RxStream));
        }
    }
}

/*
    ==========================================================================
    Description:
        This routine calculates the acumulated TxPER of eaxh TxRate. And 
        according to the calculation result, change CommonCfg.TxRate which 
        is the stable TX Rate we expect the Radio situation could sustained. 

        CommonCfg.TxRate will change dynamically within {RATE_1/RATE_6, MaxTxRate} 
    Output:
        CommonCfg.TxRate - 

    IRQL = DISPATCH_LEVEL

    NOTE:
        call this routine every second
    ==========================================================================
 */
VOID MlmeDynamicTxRateSwitching(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort)
{
    ULONG                   TxErrorRatio = 0;
    BOOLEAN                 bTxRateChanged = TRUE,bUpgradeQuality = FALSE;
    PUCHAR                  pTable;
    UCHAR                   TableSize = 0;
    UCHAR                   InitTxRateIdx = 0;
    MAC_TABLE_ENTRY     *pEntry;
    UCHAR                   btMCSThreshold = 0x00;
    AGS_STATISTICS_INFO AGSStatisticsInfo = {0};
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;  
    PQUEUE_HEADER pHeader;

    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s - pPort is  NULL, Error, and return this function", __FUNCTION__));
        return;
    }

    FUNC_ENTER;

     DBGPRINT(RT_DEBUG_ERROR, ("%s - pPort %d \n", __FUNCTION__, pPort->PortNumber));
#if 0
    if (TxStaCnt0.field.TxFailCount != 0)
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE,("STA QuickDRS: send BAR \n"));
        pPort->CommonCfg.IOTestParm.bSendBAR = TRUE;

        for (uPortNum = 0; uPortNum < RTMP_MAX_NUMBER_OF_PORT/*pAd->NumberOfPorts*/; uPortNum++)
        {
            if (pAd->PortList[uPortNum])
            {
                if ((pAd->PortList[uPortNum]->bActive) &&
                    ((pAd->PortList[uPortNum]->PortSubtype == PORTSUBTYPE_STA) ||
                    (pAd->PortList[uPortNum]->PortSubtype == PORTSUBTYPE_P2PClient)))
                {
                    ORIBATimerTimeout(pAd, pAd->PortList[uPortNum]);
                }
            }
        }
    }


    if (pAd->HwCfg.Antenna.field.RxPath == 3)
    {
        Rssi = ((pAd->StaCfg.RssiSample.AvgRssi[0] + pAd->StaCfg.RssiSample.AvgRssi[1] + pAd->StaCfg.RssiSample.AvgRssi[2]) / 3);
    }
    else if (pAd->HwCfg.Antenna.field.RxPath == 2)
    {
        Rssi = (pAd->StaCfg.RssiSample.AvgRssi[0] + pAd->StaCfg.RssiSample.AvgRssi[1]) >> 1;
    }
    else
    {
        Rssi = pAd->StaCfg.RssiSample.AvgRssi[0];
    }
#endif

    

    //
    // walk through MAC table, see if need to change AP or AdhocPeer's TX rate toward each entry
    //
    pHeader = &pPort->MacTab.MacTabList;
    pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
    while (pNextMacEntry != NULL)
    {
        pEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;

        if(pEntry == NULL)
        {
            break; 
        }
        
        DBGPRINT(RT_DEBUG_TRACE, ("%s   pEntry->WlanIdxRole = %d \n", __FUNCTION__, pEntry->WlanIdxRole));
        
        if((pEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST) || (pEntry->WlanIdxRole == ROLE_WLANIDX_AP_MBCAST) ||
        (pEntry->WlanIdxRole == ROLE_WLANIDX_ADHOC_MCAST))
        {
            pNextMacEntry = pNextMacEntry->Next;   
            pEntry = NULL;
            continue; 
        }

        //ap mode started , entry 2~255 will be used by soft ap, do not do rate turnning 
        if(pPort->SoftAP.bAPStart == TRUE)
            break;

        //
        // Station mode doesn't need to check (pEntry->Sst), only necessary on AP mode
        //
        if (pEntry->ValidAsCLI == FALSE)
        {
            pNextMacEntry = pNextMacEntry->Next;   
            pEntry = NULL;
            continue; 
        }

        
#if 0
        // If p2p's wcid, use p2p port
        if ((MlmeSyncGetP2pClientPort(pAd, 0) != NULL) && (pEntry->ValidAsP2P == TRUE))
            pPort = MlmeSyncGetP2pClientPort(pAd, 0);
        else if ((MlmeSyncGetP2pClientPort(pAd, 1) != NULL) && (pEntry->ValidAsP2P == TRUE))
            pPort = MlmeSyncGetP2pClientPort(pAd, 1);
        else
            pPort = pAd->PortList[PORT_0];
#endif
        MlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &InitTxRateIdx);

        DBGPRINT(RT_DEBUG_TRACE,("MlmeDynamicTxRateSwitching - InitTxRateIdx = %d   pEntry->wcid = %d\n", InitTxRateIdx, pEntry->wcid));

        pEntry->pTable = RateSwitchTableAdapt11N2S;
        pEntry->LowestTxRateIndex = 19;

        DynamicTxRateSwitchingAdaptMT(pAd, pEntry->wcid);

#if 0
        if ( pPort->MacTab.Size == 1 )
        {
            if ( ((pTable == RateSwitchTableAdapt11N2S) && pEntry->HTPhyMode.field.MCS >= 14 ) ||
                ((pTable == RateSwitchTableAdapt11N1S) && pEntry->HTPhyMode.field.MCS >= 6 ) )
                pAd->bDisableRtsProtect = TRUE;
            else
                pAd->bDisableRtsProtect = FALSE;
        }
        else
        {
            pAd->bDisableRtsProtect = FALSE;
        }
#endif
        
        pNextMacEntry = pNextMacEntry->Next;   
        pEntry = NULL;
    }   
}

/*
    ========================================================================
    Routine Description:
        Station side, Auto TxRate faster train up timer call back function.

    Arguments:
        SystemSpecific1         - Not used.
        FunctionContext         - Pointer to our Adapter context.
        SystemSpecific2         - Not used.
        SystemSpecific3         - Not used.

    Return Value:
        None

    ========================================================================
*/
VOID StaQuickResponeForRateUpExec(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort)
{
    ULONG                   TxErrorRatio = 0;
    BOOLEAN                 bTxRateChanged = TRUE, bUpgradeQuality = FALSE;
    PRTMP_TX_RATE_SWITCH    pNextTxRate = NULL;
    PUCHAR                  pTable;
    UCHAR                   TableSize = 0;
    UCHAR                   InitTxRateIdx = 0;
  
    UCHAR                   btMCSThreshold = 0x00;
    AGS_STATISTICS_INFO AGSStatisticsInfo = {0};
    ULONG                   ratio = 0;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;  
    PMAC_TABLE_ENTRY pEntry = NULL;  
    PQUEUE_HEADER pHeader;    
    
    pAd->StaCfg.StaQuickResponeForRateUpTimerRunning = FALSE;

    FUNC_ENTER;

#if 0
    if (TxStaCnt0.field.TxFailCount != 0)
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE,("STA QuickDRS: send BAR \n"));
        pPort->CommonCfg.IOTestParm.bSendBAR = TRUE;

        for (uPortNum = 0; uPortNum < RTMP_MAX_NUMBER_OF_PORT/*pAd->NumberOfPorts*/; uPortNum++)
        {
            if (pAd->PortList[uPortNum])
            {
                if ((pAd->PortList[uPortNum]->bActive) &&
                    ((pAd->PortList[uPortNum]->PortSubtype == PORTSUBTYPE_STA) ||
                    (pAd->PortList[uPortNum]->PortSubtype == PORTSUBTYPE_P2PClient)))
                {
                    ORIBATimerTimeout(pAd, pAd->PortList[uPortNum]);
                }
            }
        }
    }

    if (pAd->HwCfg.Antenna.field.RxPath == 3)
    {
        Rssi = ((pAd->StaCfg.RssiSample.AvgRssi[0] + pAd->StaCfg.RssiSample.AvgRssi[1] + pAd->StaCfg.RssiSample.AvgRssi[2]) / 3);
    }
    else if (pAd->HwCfg.Antenna.field.RxPath == 2)
    {
        Rssi = (pAd->StaCfg.RssiSample.AvgRssi[0] + pAd->StaCfg.RssiSample.AvgRssi[1]) >> 1;
    }
    else
    {
        Rssi = pAd->StaCfg.RssiSample.AvgRssi[0];
    }
#endif

    //
    // walk through MAC table, see if need to change AP's TX rate toward each entry
    //
    pHeader = &pPort->MacTab.MacTabList;
    pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
    while (pNextMacEntry != NULL)
    {
        pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
        
        if(pMacEntry == NULL)
        {
            break; 
        }
        
        if((pEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST) || (pEntry->WlanIdxRole == ROLE_WLANIDX_AP_MBCAST) ||
        (pEntry->WlanIdxRole == ROLE_WLANIDX_ADHOC_MCAST))        
        {
            pNextMacEntry = pNextMacEntry->Next;   
            pEntry = NULL;
            continue; 
        }

        //ap mode started , entry 2~255 will be used by soft ap, do not do rate turnning 
        if((pPort->SoftAP.bAPStart == TRUE)  && (pMacEntry->WlanIdxRole == ROLE_WLANIDX_VWF_BSSID))
            break;
        //GO mode started, entry 2~32 wikk be used by GO, excluding 22 and 23 for p2p client
        if ((pPort->P2PCfg.bGOStart)  && (!IS_P2P_MS_CLI_WCID(pPort, pMacEntry->wcid)))
        {
            pNextMacEntry = pNextMacEntry->Next;   
            pEntry = NULL;
            continue; 
        }

#ifdef MULTI_CHANNEL_SUPPORT
        if ((pPort->P2PCfg.CurrentWcid != RESERVED_WCID) && (pPort->P2PCfg.CurrentWcid == pMacEntry->wcid))
        {
            pEntry = pMacEntry;
            DBGPRINT(RT_DEBUG_TRACE,("%s - pPort->P2PCfg.CurrentWcid(%d)\n", __FUNCTION__, pMacEntry->wcid));
        }
        else
        {
            pNextMacEntry = pNextMacEntry->Next;   
            pEntry = NULL;
            continue; 
        }
#else
        pEntry = pMacEntry
#endif /*MULTI_CHANNEL_SUPPORT*/
    
        //
        // Station mode doesn't need to check (pEntry->Sst), only necessary on AP mode
        //
        if (pEntry->ValidAsCLI == FALSE)
        {
            pNextMacEntry = pNextMacEntry->Next;   
            pEntry = NULL;
            continue; 
        }          

        // If p2p's wcid, use p2p port
        if ((MlmeSyncGetP2pClientPort(pAd, 0) != NULL) && (pEntry->ValidAsP2P == TRUE))
            pPort = MlmeSyncGetP2pClientPort(pAd, 0);
        else if ((MlmeSyncGetP2pClientPort(pAd, 1) != NULL) && (pEntry->ValidAsP2P == TRUE))
            pPort = MlmeSyncGetP2pClientPort(pAd, 1);       
        else
            pPort = pAd->PortList[PORT_0];
        

        MlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &InitTxRateIdx);

        pEntry->pTable = RateSwitchTableAdapt11N2S;
        pEntry->LowestTxRateIndex = 19;

        QuickResponeForRateUpExecAdaptMT(pAd, pMacEntry->wcid);
        
        pNextMacEntry = pNextMacEntry->Next;   
        pMacEntry = NULL;
    }
    
    FUNC_LEAVE;
}

//
// The dynamic Tx rate switching for AGS (Adaptive Group Switching)
//
// Parameters
//  pAd: The adapter data structure
//  pEntry: Pointer to a caller-supplied variable in which points to a MAC table entry
//  pTable: Pointer to a caller-supplied variable in wich points to a Tx rate switching table
//  TableSize: The size, in bytes, of the specified Tx rate switching table
//  pAGSStatisticsInfo: Pointer to a caller-supplied variable in which points to the statistics information
//  InitTxRateIdx: The initial Tx rate index (3x3: 0x18, 2x2: 0x0F and 1x1: 0x07)
//
// Return Value:
//  None
//
VOID MlmeDynamicTxRateSwitchingAGS(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,    
    IN PMAC_TABLE_ENTRY pEntry, 
    IN PUCHAR pTable, 
    IN UCHAR TableSize, 
    IN PAGS_STATISTICS_INFO pAGSStatisticsInfo, 
    IN UCHAR InitTxRateIdx)
{
    UCHAR UpRateIdx = 0, DownRateIdx = 0, CurrRateIdx = 0;
    BOOLEAN bTxRateChanged = TRUE, bUpgradeQuality = FALSE;
    PRTMP_TX_RATE_SWITCH_AGS pCurrTxRate = NULL;
    PRTMP_TX_RATE_SWITCH    pNextTxRate = NULL;
    UCHAR TrainUp = 0, TrainDown = 0;
    CHAR RssiOffset = 0;

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("AGS: ---> %s\n", __FUNCTION__));

    DBGPRINT(RT_DEBUG_TRACE, ("%s: AGS: AccuTxTotalCnt = %d, TxErrorRatio = %d, TxNoRetryOkCount = %d, TxRetryOkCount = %d, TxFailCount = %d \n", 
        __FUNCTION__, 
        pAGSStatisticsInfo->AccuTxTotalCnt, 
        pAGSStatisticsInfo->TxErrorRatio, 
        pAd->Counter.MTKCounters.OneSecTxNoRetryOkCount, 
        pAd->Counter.MTKCounters.OneSecTxRetryOkCount, 
        pAd->Counter.MTKCounters.OneSecTxFailCount));

    CurrRateIdx = pEntry->CurrTxRateIndex;  

    if (CurrRateIdx >= TableSize)
    {
        CurrRateIdx = TableSize - 1;
    }

    pCurrTxRate = (PRTMP_TX_RATE_SWITCH_AGS)(&pTable[(CurrRateIdx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

    //
    // Select the next upgrade rate and the next downgrade rate, if any
    //
    do 
    {
        if ((pTable == AGS3x3HTRateTable) && 
             (InitTxRateIdx == AGS3x3HTRateTable[1]))
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: pEntry->AGSCtrl.MCSGroup = %d, TxQuality2[%d] = %d,  TxQuality1[%d] = %d, TxQuality0[%d] = %d, pCurrTxRate->upMcs1 = %d, pCurrTxRate->ItemNo = %d\n",
                __FUNCTION__, 
                pEntry->AGSCtrl.MCSGroup, 
                pCurrTxRate->upMcs3, 
                pEntry->TxQuality[pCurrTxRate->upMcs3], 
                pCurrTxRate->upMcs2, 
                pEntry->TxQuality[pCurrTxRate->upMcs2], 
                pCurrTxRate->upMcs1, 
                pEntry->TxQuality[pCurrTxRate->upMcs1], 
                pCurrTxRate->upMcs1, 
                pCurrTxRate->ItemNo));

            //
            // 3x3 peer device (Adhoc, DLS or AP)
            //
            switch (pEntry->AGSCtrl.MCSGroup)
            {
                case 0: // MCS selection in round robin policy (different MCS group)
                {
                    UpRateIdx = pCurrTxRate->upMcs3;

                    // MCS group #2 has better Tx quality
                    if ((pEntry->TxQuality[UpRateIdx] > pEntry->TxQuality[pCurrTxRate->upMcs2]) && 
                         (pCurrTxRate->upMcs2 != pCurrTxRate->ItemNo))
                    {
                        UpRateIdx = pCurrTxRate->upMcs2;
                    }

                    // MCS group #1 has better Tx quality
                    if ((pEntry->TxQuality[UpRateIdx] > pEntry->TxQuality[pCurrTxRate->upMcs1]) && 
                         (pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo))
                    {
                        UpRateIdx = pCurrTxRate->upMcs1;
                    }
                }
                break;
                
                case 3:
                {
                    UpRateIdx = pCurrTxRate->upMcs3;
                }
                break;
                
                case 2:
                {
                    UpRateIdx = pCurrTxRate->upMcs2;
                }
                break;
                
                case 1:
                {
                    UpRateIdx = pCurrTxRate->upMcs1;
                }
                break;
                
                default:
                {
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: AGS: [3x3 peer device (Adhoc, DLS or AP)], Incorrect MCS group, pEntry->AGSCtrl.MCSGroup = %d\n", 
                        __FUNCTION__, 
                        pEntry->AGSCtrl.MCSGroup));
                }
                break;
            }           
            
            if ((pEntry->AGSCtrl.MCSGroup == 0) && 
                 ((pEntry->TxQuality[pCurrTxRate->upMcs3] > pEntry->TxQuality[pCurrTxRate->upMcs2]) && (pCurrTxRate->upMcs2 != pCurrTxRate->ItemNo)) || 
                 ((pEntry->TxQuality[pCurrTxRate->upMcs3] > pEntry->TxQuality[pCurrTxRate->upMcs1]) && (pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo)))
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: ##########################################################################\n", 
                    __FUNCTION__));

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: [3x3 peer device (Adhoc, DLS or AP)], Before - pEntry->AGSCtrl.MCSGroup = %d, TxQuality2[%d] = %d,  TxQuality1[%d] = %d, TxQuality0[%d] = %d\n",
                    __FUNCTION__, 
                    pEntry->AGSCtrl.MCSGroup, 
                    pCurrTxRate->upMcs3, 
                    pEntry->TxQuality[pCurrTxRate->upMcs3], 
                    pCurrTxRate->upMcs2, 
                    pEntry->TxQuality[pCurrTxRate->upMcs2], 
                    pCurrTxRate->upMcs1, 
                    pEntry->TxQuality[pCurrTxRate->upMcs1]));
            }
        } 
        else if (((pTable == AGS2x2HTRateTable) && 
                    (InitTxRateIdx == AGS2x2HTRateTable[1])) || 
                    ((pTable == Ags2x2VhtRateTable) && 
                      (InitTxRateIdx == Ags2x2VhtRateTable[1])))
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: pEntry->AGSCtrl.MCSGroup = %d, TxQuality1[%d] = %d, TxQuality0[%d] = %d, pCurrTxRate->upMcs1 = %d, pCurrTxRate->ItemNo = %d\n",
                __FUNCTION__, 
                pEntry->AGSCtrl.MCSGroup, 
                pCurrTxRate->upMcs2, 
                pEntry->TxQuality[pCurrTxRate->upMcs2], 
                pCurrTxRate->upMcs1, 
                pEntry->TxQuality[pCurrTxRate->upMcs1], 
                pCurrTxRate->upMcs1, 
                pCurrTxRate->ItemNo));
            
            //
            // 2x2 peer device (Adhoc, DLS or AP)
            //
            switch (pEntry->AGSCtrl.MCSGroup)
            {
                case 0: // MCS selection in round robin policy
                {
                    UpRateIdx = pCurrTxRate->upMcs2;

                    // MCS group #1 has better Tx quality
                    if ((pEntry->TxQuality[UpRateIdx] > pEntry->TxQuality[pCurrTxRate->upMcs1]) && 
                         (pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo))
                    {
                        UpRateIdx = pCurrTxRate->upMcs1;
                    }
                }
                break;
                
                case 2:
                {
                    UpRateIdx = pCurrTxRate->upMcs2;
                }
                break;
                
                case 1:
                {
                    UpRateIdx = pCurrTxRate->upMcs1;
                }
                break;
                
                default:
                {
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: AGS: [2x2 peer device (Adhoc, DLS or AP)], Incorrect MCS group, pEntry->AGSCtrl.MCSGroup = %d\n", 
                        __FUNCTION__, 
                        pEntry->AGSCtrl.MCSGroup));
                }
                break;
            }   

            if ((pEntry->AGSCtrl.MCSGroup == 0) && 
                 ((pEntry->TxQuality[pCurrTxRate->upMcs2] > pEntry->TxQuality[pCurrTxRate->upMcs1]) && (pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo)))
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: ##########################################################################\n", 
                    __FUNCTION__));

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: [2x2 peer device (Adhoc, DLS or AP)], Before - pEntry->AGSCtrl.MCSGroup = %d, TxQuality1[%d] = %d, TxQuality0[%d] = %d\n",
                    __FUNCTION__, 
                    pEntry->AGSCtrl.MCSGroup, 
                    pCurrTxRate->upMcs2, 
                    pEntry->TxQuality[pCurrTxRate->upMcs2], 
                    pCurrTxRate->upMcs1, 
                    pEntry->TxQuality[pCurrTxRate->upMcs1]));
            }
        } 
        else 
        {
            //
            // 1x1 peer device (Adhoc, DLS or AP)
            //
            switch (pEntry->AGSCtrl.MCSGroup)
            {
                case 1:
                case 0:
                {
                    UpRateIdx = pCurrTxRate->upMcs1;
                }
                break;
                
                default:
                {
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: AGS: [1x1 peer device (Adhoc, DLS or AP)], Incorrect MCS group, pEntry->AGSCtrl.MCSGroup = %d\n", 
                        __FUNCTION__, 
                        pEntry->AGSCtrl.MCSGroup));
                }
                break;
            }   
        }

        //
        // The STA uses the best Tx rate at this moment.
        //
        if (UpRateIdx == pEntry->CurrTxRateIndex)
        {
            pEntry->AGSCtrl.MCSGroup = 0; // Try to escape the local optima
            break;
        }
        
        if ((pEntry->TxQuality[UpRateIdx] > 0) && (pEntry->AGSCtrl.MCSGroup > 0))
        {
            pEntry->AGSCtrl.MCSGroup--; // Try to use the MCS of the lower MCS group
        }
        else
        {
            break;
        }
    } while (1);
    
    DownRateIdx = pCurrTxRate->downMcs;

    if ((pAGSStatisticsInfo->RSSI > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
    {
        TrainUp = (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
        TrainDown   = (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
    }
    else
    {
        TrainUp = pCurrTxRate->TrainUp;
        TrainDown   = pCurrTxRate->TrainDown;
    }

    //
    // Keep the TxRateChangeAction status
    //
    pEntry->LastTimeTxRateChangeAction = pEntry->LastSecTxRateChangeAction;     

    if (pPort->CommonCfg.bAutoTxRateSwitch == FALSE)
    {
        // force update back to the fixed mode
        if (pPort->CommonCfg.RegTransmitSetting.field.HTMODE >= HTMODE_11B)
        {
            WRITE_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg, pPort->CommonCfg.RegTransmitSetting.field.MCS);
            WRITE_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg, pPort->CommonCfg.RegTransmitSetting.field.BW);
            WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg, pPort->CommonCfg.RegTransmitSetting.field.ShortGI);
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, pPort->CommonCfg.RegTransmitSetting.field.STBC);
            
            switch (pPort->CommonCfg.RegTransmitSetting.field.HTMODE)
            {
                case HTMODE_MM:
                    WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, MODE_HTMIX);
                    break;
                    
                case HTMODE_GF:
                    WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, MODE_HTGREENFIELD);
                    break;

                case HTMODE_11B:
                    WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, MODE_CCK);
                    break;

                case HTMODE_11G:
                    WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, MODE_OFDM);
                    break;
            }
        }

        if ((READ_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg) == STBC_USE) && (pAd->HwCfg.Antenna.field.TxPath >= 2))
        {
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_USE);
        }
        else
        {
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_NONE);
        }

        // Double confirm STBC not use if (MCS >7). Although we already make correct rate table.
        // RT2860 PHY just support STBC=1 when MCS<=7(1S)   
        if (READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) > MCS_7)
        {
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_NONE);
        }

        DBGPRINT_RAW(RT_DEBUG_TRACE,("%s: AGS: Fixed - CurrTxRateIdx=%d, MCS=%d, STBC=%d, ShortGI=%d, Mode=%d, BW=%d, PER=%d%%, Retry=%d, NoRetry=%d\n\n",
            __FUNCTION__, pEntry->CurrTxRateIndex, 
            READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg), 
            READ_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg), 
            READ_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg), 
            READ_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg), 
            READ_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg), 
            pAGSStatisticsInfo->TxErrorRatio, 
            pAGSStatisticsInfo->TxRetransmit, 
            pAGSStatisticsInfo->TxSuccess));

        return;
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: AGS: Rssi = %d, TxSuccess = %lu, TxRetransmit = %lu, TxFailCount = %lu, TxErrorRatio = %lu\n", 
            __FUNCTION__, 
            pAGSStatisticsInfo->RSSI, 
            pAGSStatisticsInfo->TxSuccess, 
            pAGSStatisticsInfo->TxRetransmit, 
            pAGSStatisticsInfo->TxFailCount, 
            pAGSStatisticsInfo->TxErrorRatio));

        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: Before - CurrTxRateIdx = %d, MCS = %d, STBC = %d, ShortGI = %d, Mode = %d, TrainUp = %d, TrainDown = %d, NextUp = %d, NextDown = %d, CurrMCS = %d, pEntry->AGSCtrl.MCSGroup = %d, PER = %lu%%, Retry = %lu, NoRetry = %lu\n", 
            __FUNCTION__, 
            CurrRateIdx, 
            pCurrTxRate->CurrMCS, 
            pCurrTxRate->STBC, 
            pCurrTxRate->ShortGI, 
            pCurrTxRate->Mode, 
            TrainUp, 
            TrainDown, 
            UpRateIdx, 
            DownRateIdx, 
            //pEntry->HTPhyMode.field.MCS, 
            READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg), 
            pEntry->AGSCtrl.MCSGroup, 
            pAGSStatisticsInfo->TxErrorRatio, 
            pAGSStatisticsInfo->TxRetransmit, 
            pAGSStatisticsInfo->TxSuccess));
    }

    if (! OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED))
    {
        return;
    }


    //
    // MCS selection based on the RSSI information when the Tx samples are fewer than 15.
    //
    if (pAGSStatisticsInfo->AccuTxTotalCnt <= 15)
    {
        CHAR idx = 0;
        UCHAR TxRateIdx;
        UCHAR MCS0 = 0, MCS1 = 0, MCS2 = 0, MCS3 = 0, MCS4 = 0,  MCS5 =0, MCS6 = 0, MCS7 = 0;   
        UCHAR MCS8 = 0, MCS9 = 0, MCS10 = 0, MCS11 = 0, MCS12 = 0, MCS13 = 0, MCS14 = 0, MCS15 = 0;
        UCHAR MCS16 = 0, MCS17 = 0, MCS18 = 0, MCS19 = 0, MCS20 = 0, MCS21 = 0, MCS22 = 0, MCS23 = 0;

        //
        // Check the existence and index of each needed MCS
        //
        while (idx < pTable[0])
        {
            pCurrTxRate = (PRTMP_TX_RATE_SWITCH_AGS)(&pTable[(idx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

            if (AGS_HT_MCS0(pTable, pCurrTxRate) || 
                AGS_VHT_NSS0_MCS0(pTable, pCurrTxRate))
            {
                MCS0 = idx;
            }
            else if (AGS_HT_MCS1(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS1(pTable, pCurrTxRate))
            {
                MCS1 = idx;
            }
            else if (AGS_HT_MCS2(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS2(pTable, pCurrTxRate))
            {
                MCS2 = idx;
            }
            else if (AGS_HT_MCS3(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS3(pTable, pCurrTxRate))
            {
                MCS3 = idx;
            }
            else if (AGS_HT_MCS4(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS4(pTable, pCurrTxRate))
            {
                MCS4 = idx;
            }
            else if (AGS_HT_MCS5(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS5(pTable, pCurrTxRate))
            {
                MCS5 = idx;
            }
            else if (AGS_HT_MCS6(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS6(pTable, pCurrTxRate))
            {
                    MCS6 = idx;
            }
            else if (AGS_HT_MCS7(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS7(pTable, pCurrTxRate))
            {
                MCS7 = idx;
            }
            else if (AGS_HT_MCS8(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS1_MCS0(pTable, pCurrTxRate))
            {
                MCS8 = idx;
            }
            else if (AGS_HT_MCS9(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS1_MCS1(pTable, pCurrTxRate))
            {
                MCS9 = idx;
            }
            else if (AGS_HT_MCS10(pTable, pCurrTxRate) ||
                        AGS_VHT_NSS1_MCS2(pTable, pCurrTxRate))
            {
                MCS10 = idx;
            }
            else if (AGS_HT_MCS11(pTable, pCurrTxRate) ||
                        AGS_VHT_NSS1_MCS3(pTable, pCurrTxRate))
            {
                MCS11 = idx;
            }
            else if (AGS_HT_MCS12(pTable, pCurrTxRate) ||
                        AGS_VHT_NSS1_MCS4(pTable, pCurrTxRate))
            {
                MCS12 = idx;
            }
            else if (AGS_HT_MCS13(pTable, pCurrTxRate) ||
                        AGS_VHT_NSS1_MCS5(pTable, pCurrTxRate))
            {
                MCS13 = idx;
            }
            else if (AGS_HT_MCS14(pTable, pCurrTxRate) ||
                        AGS_VHT_NSS1_MCS6(pTable, pCurrTxRate))
            {
                MCS14 = idx;
            }
            else if (AGS_HT_MCS15(pTable, pCurrTxRate) ||
                        AGS_VHT_NSS1_MCS7(pTable, pCurrTxRate))
            {
                MCS15 = idx;
            }
            else if (AGS_HT_MCS16(pTable, pCurrTxRate))
            {
                MCS16 = idx;
            }
            else if (AGS_HT_MCS17(pTable, pCurrTxRate))
            {
                MCS17 = idx;
            }
            else if (AGS_HT_MCS18(pTable, pCurrTxRate))
            {
                MCS18 = idx;
            }
            else if (AGS_HT_MCS19(pTable, pCurrTxRate))
            {
                MCS19 = idx;
            }
            else if (AGS_HT_MCS20(pTable, pCurrTxRate))
            {
                MCS20 = idx;
            }
            else if (AGS_HT_MCS21(pTable, pCurrTxRate))
            {
                MCS21 = idx;
            }
            else if (AGS_HT_MCS22(pTable, pCurrTxRate))
            {
                MCS22 = idx;
            }
            else if (AGS_HT_MCS23(pTable, pCurrTxRate))
            {
                MCS23 = idx;
            }
            
            idx++;
        }

        RssiOffset = 0;
        
        if ((pTable == AGS3x3HTRateTable) && 
            (InitTxRateIdx == AGS3x3HTRateTable[1]))
        {
            //
            // 3x3 peer device (Adhoc, DLS or AP)
            //
            if (MCS23 && (pAGSStatisticsInfo->RSSI > (-67 + RssiOffset)))
            {
                TxRateIdx = MCS23;
            }
            else if (MCS22 && (pAGSStatisticsInfo->RSSI > (-69 + RssiOffset)))
            {
                TxRateIdx = MCS22;
            }
            else if (MCS21 && (pAGSStatisticsInfo->RSSI > (-72 + RssiOffset)))
            {
                TxRateIdx = MCS21;
            }
            else if (MCS20 && (pAGSStatisticsInfo->RSSI > (-74 + RssiOffset)))
            {
                TxRateIdx = MCS20;
            }
            else if (MCS19 && (pAGSStatisticsInfo->RSSI > (-78 + RssiOffset)))
            {
                TxRateIdx = MCS19;
            }
            else if (MCS18 && (pAGSStatisticsInfo->RSSI > (-80 + RssiOffset)))
            {
                TxRateIdx = MCS18;
            }
            else if (MCS17 && (pAGSStatisticsInfo->RSSI > (-85 + RssiOffset)))
            {
                TxRateIdx = MCS17;
            }
            else
            {
                TxRateIdx = MCS16;
            }
            
            pEntry->AGSCtrl.MCSGroup = 3;
        } 
        else if (((pTable == AGS2x2HTRateTable) && 
                    (InitTxRateIdx == AGS2x2HTRateTable[1])) || 
                    ((pTable == Ags2x2VhtRateTable) && 
                    (InitTxRateIdx == Ags2x2VhtRateTable[1])))
        {
            //
            // 2x2 peer device (Adhoc, DLS or AP)
            //
            if (MCS15 && (pAGSStatisticsInfo->RSSI > (-69 + RssiOffset)))
            {
                TxRateIdx = MCS15;
            }
            else if (MCS14 && (pAGSStatisticsInfo->RSSI > (-71 + RssiOffset)))
            {
                TxRateIdx = MCS14;
            }
            else if (MCS13 && (pAGSStatisticsInfo->RSSI > (-74 + RssiOffset)))
            {
                TxRateIdx = MCS13;
            }
            else if (MCS12 && (pAGSStatisticsInfo->RSSI > (-76 + RssiOffset)))
            {
                TxRateIdx = MCS12;
            }
            else if (MCS11 && (pAGSStatisticsInfo->RSSI > (-80 + RssiOffset)))
            {
                TxRateIdx = MCS11;
            }
            else if (MCS10 && (pAGSStatisticsInfo->RSSI > (-82 + RssiOffset)))
            {
                TxRateIdx = MCS10;
            }
            else if (MCS9 && (pAGSStatisticsInfo->RSSI > (-87 + RssiOffset)))
            {
                TxRateIdx = MCS9;
            }
            else
            {
                TxRateIdx = MCS8;
            }
            
            pEntry->AGSCtrl.MCSGroup = 2;
        } 
        else 
        {
            //
            // 1x1 peer device (Adhoc, DLS or AP)
            //
            if (MCS7 && (pAGSStatisticsInfo->RSSI > (-71 + RssiOffset)))
            {
                TxRateIdx = MCS7;
            }
            else if (MCS6 && (pAGSStatisticsInfo->RSSI > (-73 + RssiOffset)))
            {
                TxRateIdx = MCS6;
            }
            else if (MCS5 && (pAGSStatisticsInfo->RSSI > (-76 + RssiOffset)))
            {
                TxRateIdx = MCS5;
            }
            else if (MCS4 && (pAGSStatisticsInfo->RSSI > (-78 + RssiOffset)))
            {
                TxRateIdx = MCS4;
            }
            else if (MCS3 && (pAGSStatisticsInfo->RSSI > (-82 + RssiOffset)))
            {
                TxRateIdx = MCS3;
            }
            else if (MCS2 && (pAGSStatisticsInfo->RSSI > (-84 + RssiOffset)))
            {
                TxRateIdx = MCS2;
            }
            else if (MCS1 && (pAGSStatisticsInfo->RSSI > (-89 + RssiOffset)))
            {
                TxRateIdx = MCS1;
            }
            else
            {
                TxRateIdx = MCS0;
            }
            
            pEntry->AGSCtrl.MCSGroup = 1;
        }

        pEntry->AGSCtrl.lastRateIdx = pEntry->CurrTxRateIndex;
        pEntry->CurrTxRateIndex = TxRateIdx;
        pNextTxRate = (PRTMP_TX_RATE_SWITCH)(&pTable[(pEntry->CurrTxRateIndex + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

        MlmeSetTxRate(pAd, pEntry, pNextTxRate, pTable);

        PlatformZeroMemory(pEntry->TxQuality, (sizeof(USHORT) * MAX_STEP_OF_TX_RATE_SWITCH));
        PlatformZeroMemory(pEntry->PER, (sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH));
        
        AsicCfgPAPEByStreams(pAd, pCurrTxRate->CurrMCS, pNextTxRate->CurrMCS);
        pEntry->fLastSecAccordingRSSI = TRUE;           

        return;
    }

    //
    // The MCS selection is based on the RSSI and skips the rate tuning this time.
    //
    if (pEntry->fLastSecAccordingRSSI  == TRUE)
    {
        pEntry->fLastSecAccordingRSSI = FALSE;
        pEntry->LastSecTxRateChangeAction = 0;
        
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: The MCS selection is based on the RSSI, and skips the rate tuning this time.\n", 
            __FUNCTION__));

        return;
    }

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: TrainUp:%d, TrainDown:%d\n",__FUNCTION__, TrainUp, TrainDown));
    do
    {
        BOOLEAN bTrainUpDown = FALSE;

        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: TxQuality[CurrRateIdx(%d)] = %d, UpPenalty:%d\n",__FUNCTION__, CurrRateIdx, pEntry->TxQuality[CurrRateIdx], pEntry->TxRateUpPenalty));
            
        if (pAGSStatisticsInfo->TxErrorRatio >= TrainDown) // Poor quality
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: TxErrorRatio >= TrainDown\n",__FUNCTION__));
            bTrainUpDown = TRUE;
            pEntry->TxQuality[CurrRateIdx] = AGS_TX_QUALITY_WORST_BOUND;
        }   
        else if (pAGSStatisticsInfo->TxErrorRatio <= TrainUp) // Good quality
        {
            bTrainUpDown = TRUE;
            bUpgradeQuality = TRUE;
            
            if (pEntry->TxQuality[CurrRateIdx])
            {
                pEntry->TxQuality[CurrRateIdx]--; // Good quality in the current Tx rate
            }

            if (pEntry->TxRateUpPenalty)
            {
                pEntry->TxRateUpPenalty--;
            }
            else
            {
                if (pEntry->TxQuality[pCurrTxRate->upMcs3] && (pCurrTxRate->upMcs3 != CurrRateIdx))
                {
                    pEntry->TxQuality[pCurrTxRate->upMcs3]--;
                }
                
                if (pEntry->TxQuality[pCurrTxRate->upMcs2] && (pCurrTxRate->upMcs2 != CurrRateIdx))
                {
                    pEntry->TxQuality[pCurrTxRate->upMcs2]--;
                }
                
                if (pEntry->TxQuality[pCurrTxRate->upMcs1] && (pCurrTxRate->upMcs1 != CurrRateIdx))
                {
                    pEntry->TxQuality[pCurrTxRate->upMcs1]--;
                }               
            }
        }
        else if (pEntry->AGSCtrl.MCSGroup > 0) //even if TxErrorRatio > TrainUp
        {
            if (UpRateIdx != 0)
            {
                bTrainUpDown = TRUE;
                
                if (pEntry->TxQuality[CurrRateIdx])
                {
                    pEntry->TxQuality[CurrRateIdx]--; // Good quality in the current Tx rate
                }

                 if (pEntry->TxQuality[UpRateIdx])
                 {
                    pEntry->TxQuality[UpRateIdx]--; // It may improve next train-up Tx rate's quality
                }
            }
        }

        pEntry->PER[CurrRateIdx] = (UCHAR)(pAGSStatisticsInfo->TxErrorRatio);

        //
        // Update the current Tx rate
        //
        if (bTrainUpDown)
        {
            // Downgrade Tx rate
            if ((CurrRateIdx != DownRateIdx) && 
                 (pEntry->TxQuality[CurrRateIdx] >= AGS_TX_QUALITY_WORST_BOUND))
            {
                pEntry->CurrTxRateIndex = DownRateIdx;
                pEntry->LastSecTxRateChangeAction = 2; // Tx rate down
            }
            else if ((CurrRateIdx != UpRateIdx) && 
                        (pEntry->TxQuality[UpRateIdx] <= 0)) // Upgrade Tx rate
            {
                pEntry->CurrTxRateIndex = UpRateIdx;
                pEntry->LastSecTxRateChangeAction = 1; // Tx rate up
            }
        }
    } while (FALSE);
    
    // Tx rate up
    if ((pEntry->CurrTxRateIndex != CurrRateIdx) && 
         (pEntry->LastSecTxRateChangeAction == 1))
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: ++TX rate from %d to %d\n", 
            __FUNCTION__, 
            CurrRateIdx, 
            pEntry->CurrTxRateIndex));
        
        pEntry->TxRateUpPenalty = 0;
        pEntry->LastSecTxRateChangeAction = 1; // Tx rate up
        PlatformZeroMemory(pEntry->PER, sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH);
        pEntry->AGSCtrl.lastRateIdx = CurrRateIdx;

        //
        // Tx rate fast train up
        //
        if (!pAd->StaCfg.StaQuickResponeForRateUpTimerRunning)
        {
            pAd->StaCfg.StaQuickResponeForRateUpTimerRunning = TRUE;
        }

        bTxRateChanged = TRUE;
    }
    else if ((pEntry->CurrTxRateIndex != CurrRateIdx) && 
                 (pEntry->LastSecTxRateChangeAction == 2)) // Tx rate down
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: --TX rate from %d to %d\n", 
            __FUNCTION__, 
            CurrRateIdx, 
            pEntry->CurrTxRateIndex));
        
        pEntry->TxRateUpPenalty = 0; // No penalty
        pEntry->LastSecTxRateChangeAction = 2; // Tx rate down
        pEntry->TxQuality[pEntry->CurrTxRateIndex] = 0;
        pEntry->PER[pEntry->CurrTxRateIndex] = 0;
        pEntry->AGSCtrl.lastRateIdx = CurrRateIdx;

        //
        // Tx rate fast train down
        // 
        if (!pAd->StaCfg.StaQuickResponeForRateUpTimerRunning)
        {
            pAd->StaCfg.StaQuickResponeForRateUpTimerRunning = TRUE;
        }

        bTxRateChanged = TRUE;
    }
    else // Tx rate remains unchanged.
    {
        pEntry->LastSecTxRateChangeAction = 0; // Tx rate remains unchanged.
        bTxRateChanged = FALSE;
    }

    pEntry->LastTxOkCount = pAGSStatisticsInfo->TxSuccess;

    pNextTxRate = (PRTMP_TX_RATE_SWITCH)(&pTable[(pEntry->CurrTxRateIndex + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

    if ((bTxRateChanged == TRUE) && (pNextTxRate != NULL))
    {
        AsicCfgPAPEByStreams(pAd, pCurrTxRate->CurrMCS, pNextTxRate->CurrMCS);
        MlmeSetTxRate(pAd, pEntry, pNextTxRate, pTable);
    }

    //
    // RDG threshold control for the infrastructure mode only
    //
    if (INFRA_ON(pPort) /*&& (pAd->OpMode == OPMODE_STA)*/ && (!DLS_ON(pAd)) && (!TDLS_ON(pAd)))
    {
        PMAC_TABLE_ENTRY pMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
        if(pMacTabEntry == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
            return;
        }
        
        if ((pPort->CommonCfg.bRdg == TRUE) && CLIENT_STATUS_TEST_FLAG(pMacTabEntry, fCLIENT_STATUS_RDG_CAPABLE)) // RDG capable
        {
            TXOP_THRESHOLD_CFG_STRUC TxopThCfg = {0};
            TX_LINK_CFG_STRUC TxLinkCfg = {0};
            
            if ((pAd->Counter.MTKCounters.OneSecReceivedByteCount > (pAd->Counter.MTKCounters.OneSecTransmittedByteCount * 5)) && 
                 ( ((pTable == AGS3x3HTRateTable) && (pNextTxRate->CurrMCS != MCS_23)) ||
                    ((pTable == AGS2x2HTRateTable) && (pNextTxRate->CurrMCS != MCS_15)) || 
                    ((pTable == AGS1x1HTRateTable) && (pNextTxRate->CurrMCS != MCS_7)) ||
                    ((pTable == AGS1x1HTRateTable5G) && (pNextTxRate->CurrMCS != MCS_7))) && 
                 ((pAd->Counter.MTKCounters.OneSecReceivedByteCount + pAd->Counter.MTKCounters.OneSecTransmittedByteCount) >= (50 * 1024)))
            {
                RTUSBReadMACRegister(pAd, TX_LINK_CFG, &TxLinkCfg.word);
                TxLinkCfg.field.TxRDGEn = 0;
                RTUSBWriteMACRegister(pAd, TX_LINK_CFG, TxLinkCfg.word);

                RTUSBReadMACRegister(pAd, TXOP_THRES_CFG, &TxopThCfg.word);
                TxopThCfg.field.RDG_IN_THRES = 0xFF; // Similar to diable Rx RDG
                TxopThCfg.field.RDG_OUT_THRES = 0x00;
                RTUSBWriteMACRegister(pAd, TXOP_THRES_CFG, TxopThCfg.word);

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("AGS: %s: RDG_IN_THRES = 0xFF\n", __FUNCTION__));
            }
            else
            {
                RTUSBReadMACRegister(pAd, TX_LINK_CFG, &TxLinkCfg.word);
                TxLinkCfg.field.TxRDGEn = 1;
                RTUSBWriteMACRegister(pAd, TX_LINK_CFG, TxLinkCfg.word);

                RTUSBReadMACRegister(pAd, TXOP_THRES_CFG, &TxopThCfg.word);
                TxopThCfg.field.RDG_IN_THRES = 0x00;
                TxopThCfg.field.RDG_OUT_THRES = 0x00;
                RTUSBWriteMACRegister(pAd, TXOP_THRES_CFG, TxopThCfg.word);

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("AGS: %s: RDG_IN_THRES = 0x00\n", __FUNCTION__));
            }
        }
    }

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("AGS: <--- %s\n", __FUNCTION__));
}

//
// The dynamic Tx rate switching for AGS in VHT (Adaptive Group Switching)
//
// Parameters
//  pAd: The adapter data structure
//  pEntry: Pointer to a caller-supplied variable in which points to a MAC table entry
//  pTable: Pointer to a caller-supplied variable in wich points to a Tx rate switching table
//  TableSize: The size, in bytes, of the specified Tx rate switching table
//  pAGSStatisticsInfo: Pointer to a caller-supplied variable in which points to the statistics information
//
// Return Value:
//  None
//
VOID MlmeDynamicTxRateSwitchingAGSv2(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN PMAC_TABLE_ENTRY pEntry, 
    IN PUCHAR pTable, 
    IN UCHAR TableSize, 
    IN PAGS_STATISTICS_INFO pAGSStatisticsInfo,
    IN UCHAR InitTxRateIdx)
{
    UCHAR UpRateIdx = 0, DownRateIdx = 0, CurrRateIdx = 0;
    BOOLEAN bTxRateChanged = TRUE, bUpgradeQuality = FALSE;
    PRTMP_TX_RATE_SWITCH_AGS pCurrTxRate = NULL;
    PRTMP_TX_RATE_SWITCH    pNextTxRate = NULL;
    UCHAR TrainUp = 0, TrainDown = 0;
    CHAR RssiOffsetForAgs = 0;

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("AGS: ---> %s\n", __FUNCTION__));

    DBGPRINT(RT_DEBUG_TRACE, ("%s: QuickAGS: AccuTxTotalCnt = %d, TxSuccess = %d, TxRetransmit = %d, TxFailCount = %d, TxErrorRatio = %d\n",
        __FUNCTION__, 
        pAGSStatisticsInfo->AccuTxTotalCnt, 
        pAGSStatisticsInfo->TxSuccess, 
        pAGSStatisticsInfo->TxRetransmit, 
        pAGSStatisticsInfo->TxFailCount, 
        pAGSStatisticsInfo->TxErrorRatio));

    CurrRateIdx = pEntry->CurrTxRateIndex;  

    if (CurrRateIdx >= TableSize)
    {
        CurrRateIdx = TableSize - 1;
    }

    pCurrTxRate = (PRTMP_TX_RATE_SWITCH_AGS)(&pTable[(CurrRateIdx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

    //
    // Select the next upgrade rate and the next downgrade rate, if any
    //
    do 
    {
        if ((pTable == AGS3x3HTRateTable) && 
             (InitTxRateIdx == AGS3x3HTRateTable[1]))
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: pEntry->AGSCtrl.MCSGroup = %d, TxQuality2[%d] = %d,  TxQuality1[%d] = %d, TxQuality0[%d] = %d, pCurrTxRate->upMcs1 = %d, pCurrTxRate->ItemNo = %d\n",
                __FUNCTION__, 
                pEntry->AGSCtrl.MCSGroup, 
                pCurrTxRate->upMcs3, 
                pEntry->TxQuality[pCurrTxRate->upMcs3], 
                pCurrTxRate->upMcs2, 
                pEntry->TxQuality[pCurrTxRate->upMcs2], 
                pCurrTxRate->upMcs1, 
                pEntry->TxQuality[pCurrTxRate->upMcs1], 
                pCurrTxRate->upMcs1, 
                pCurrTxRate->ItemNo));

            //
            // 3x3 peer device (Adhoc, DLS or AP)
            //
            switch (pEntry->AGSCtrl.MCSGroup)
            {
                case 0: // MCS selection in round robin policy (different MCS group)
                {
                    UpRateIdx = pCurrTxRate->upMcs3;

                    // MCS group #2 has better Tx quality
                    if ((pEntry->TxQuality[UpRateIdx] > pEntry->TxQuality[pCurrTxRate->upMcs2]) && 
                         (pCurrTxRate->upMcs2 != pCurrTxRate->ItemNo))
                    {
                        UpRateIdx = pCurrTxRate->upMcs2;
                    }

                    // MCS group #1 has better Tx quality
                    if ((pEntry->TxQuality[UpRateIdx] > pEntry->TxQuality[pCurrTxRate->upMcs1]) && 
                         (pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo))
                    {
                        UpRateIdx = pCurrTxRate->upMcs1;
                    }
                }
                break;
                
                case 3:
                {
                    UpRateIdx = pCurrTxRate->upMcs3;
                }
                break;
                
                case 2:
                {
                    UpRateIdx = pCurrTxRate->upMcs2;
                }
                break;
                
                case 1:
                {
                    UpRateIdx = pCurrTxRate->upMcs1;
                }
                break;
                
                default:
                {
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: AGS: [3x3 peer device (Adhoc, DLS or AP)], Incorrect MCS group, pEntry->AGSCtrl.MCSGroup = %d\n", 
                        __FUNCTION__, 
                        pEntry->AGSCtrl.MCSGroup));
                }
                break;
            }           
            
            if ((pEntry->AGSCtrl.MCSGroup == 0) && 
                 ((pEntry->TxQuality[pCurrTxRate->upMcs3] > pEntry->TxQuality[pCurrTxRate->upMcs2]) && (pCurrTxRate->upMcs2 != pCurrTxRate->ItemNo)) || 
                 ((pEntry->TxQuality[pCurrTxRate->upMcs3] > pEntry->TxQuality[pCurrTxRate->upMcs1]) && (pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo)))
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: ##########################################################################\n", 
                    __FUNCTION__));

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: [3x3 peer device (Adhoc, DLS or AP)], Before - pEntry->AGSCtrl.MCSGroup = %d, TxQuality2[%d] = %d,  TxQuality1[%d] = %d, TxQuality0[%d] = %d\n",
                    __FUNCTION__, 
                    pEntry->AGSCtrl.MCSGroup, 
                    pCurrTxRate->upMcs3, 
                    pEntry->TxQuality[pCurrTxRate->upMcs3], 
                    pCurrTxRate->upMcs2, 
                    pEntry->TxQuality[pCurrTxRate->upMcs2], 
                    pCurrTxRate->upMcs1, 
                    pEntry->TxQuality[pCurrTxRate->upMcs1]));
            }
        } 
        else if (((pTable == AGS2x2HTRateTable) && 
                    (InitTxRateIdx == AGS2x2HTRateTable[1])) || 
                    ((pTable == Ags2x2VhtRateTable) && 
                      (InitTxRateIdx == Ags2x2VhtRateTable[1])))
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: pEntry->AGSCtrl.MCSGroup = %d, TxQuality1[%d] = %d, TxQuality0[%d] = %d, pCurrTxRate->upMcs1 = %d, pCurrTxRate->ItemNo = %d\n",
                __FUNCTION__, 
                pEntry->AGSCtrl.MCSGroup, 
                pCurrTxRate->upMcs2, 
                pEntry->TxQuality[pCurrTxRate->upMcs2], 
                pCurrTxRate->upMcs1, 
                pEntry->TxQuality[pCurrTxRate->upMcs1], 
                pCurrTxRate->upMcs1, 
                pCurrTxRate->ItemNo));
            
            //
            // 2x2 peer device (Adhoc, DLS or AP)
            //
            switch (pEntry->AGSCtrl.MCSGroup)
            {
                case 0: // MCS selection in round robin policy
                {
                    UpRateIdx = pCurrTxRate->upMcs2;

                    // MCS group #1 has better Tx quality
                    if ((pEntry->TxQuality[UpRateIdx] > pEntry->TxQuality[pCurrTxRate->upMcs1]) && 
                         (pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo))
                    {
                        UpRateIdx = pCurrTxRate->upMcs1;
                    }
                }
                break;
                
                case 2:
                {
                    UpRateIdx = pCurrTxRate->upMcs2;
                }
                break;
                
                case 1:
                {
                    UpRateIdx = pCurrTxRate->upMcs1;
                }
                break;
                
                default:
                {
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: AGS: [2x2 peer device (Adhoc, DLS or AP)], Incorrect MCS group, pEntry->AGSCtrl.MCSGroup = %d\n", 
                        __FUNCTION__, 
                        pEntry->AGSCtrl.MCSGroup));
                }
                break;
            }   

            if ((pEntry->AGSCtrl.MCSGroup == 0) && 
                 ((pEntry->TxQuality[pCurrTxRate->upMcs2] > pEntry->TxQuality[pCurrTxRate->upMcs1]) && (pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo)))
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: ##########################################################################\n", 
                    __FUNCTION__));

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: [2x2 peer device (Adhoc, DLS or AP)], Before - pEntry->AGSCtrl.MCSGroup = %d, TxQuality1[%d] = %d, TxQuality0[%d] = %d\n",
                    __FUNCTION__, 
                    pEntry->AGSCtrl.MCSGroup, 
                    pCurrTxRate->upMcs2, 
                    pEntry->TxQuality[pCurrTxRate->upMcs2], 
                    pCurrTxRate->upMcs1, 
                    pEntry->TxQuality[pCurrTxRate->upMcs1]));
            }
        } 
        else 
        {
            //
            // 1x1 peer device (Adhoc, DLS or AP)
            //
            switch (pEntry->AGSCtrl.MCSGroup)
            {
                case 1:
                case 0:
                {
                    UpRateIdx = pCurrTxRate->upMcs1;
                }
                break;
                
                default:
                {
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: AGS: [1x1 peer device (Adhoc, DLS or AP)], Incorrect MCS group, pEntry->AGSCtrl.MCSGroup = %d\n", 
                        __FUNCTION__, 
                        pEntry->AGSCtrl.MCSGroup));
                }
                break;
            }   
        }

        //
        // The STA uses the best Tx rate at this moment.
        //
        if (UpRateIdx == pEntry->CurrTxRateIndex)
        {
            pEntry->AGSCtrl.MCSGroup = 0; // Try to escape the local optima
            break;
        }
        
        if ((pEntry->TxQuality[UpRateIdx] > 0) && (pEntry->AGSCtrl.MCSGroup > 0))
        {
            pEntry->AGSCtrl.MCSGroup--; // Try to use the MCS of the lower MCS group
        }
        else
        {
            break;
        }
    } while (1);
    
    DownRateIdx = pCurrTxRate->downMcs;

    if ((pAGSStatisticsInfo->RSSI > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
    {
        TrainUp = (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
        TrainDown   = (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
    }
    else
    {
        TrainUp = pCurrTxRate->TrainUp;
        TrainDown   = pCurrTxRate->TrainDown;
    }

    //
    // Keep the TxRateChangeAction status
    //
    pEntry->LastTimeTxRateChangeAction = pEntry->LastSecTxRateChangeAction;

    if (pPort->CommonCfg.bAutoTxRateSwitch == FALSE)
    {
        // force update back to the fixed mode
        if (pPort->CommonCfg.RegTransmitSetting.field.HTMODE >= HTMODE_11B)
        {
            WRITE_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg, pPort->CommonCfg.RegTransmitSetting.field.MCS);
            WRITE_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg, pPort->CommonCfg.RegTransmitSetting.field.BW);
            WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg, pPort->CommonCfg.RegTransmitSetting.field.ShortGI);
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, pPort->CommonCfg.RegTransmitSetting.field.STBC);

            switch (pPort->CommonCfg.RegTransmitSetting.field.HTMODE)
            {
                case HTMODE_MM:
                    WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, MODE_HTMIX);
                    break;
                    
                case HTMODE_GF:
                    WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, MODE_HTGREENFIELD);
                    break;

                case HTMODE_11B:
                    WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, MODE_CCK);
                    break;

                case HTMODE_11G:
                    WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, MODE_OFDM);
                    break;
            }
        }

        if ((READ_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg) == STBC_USE) && (pAd->HwCfg.Antenna.field.TxPath >= 2))
        {
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_USE);
        }
        else
        {
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_NONE);
        }

        // Double confirm STBC not use if (MCS >7). Although we already make correct rate table.
        // RT2860 PHY just support STBC=1 when MCS<=7(1S)   
        if (READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) > MCS_7)
        {
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_NONE);
        }

        DBGPRINT_RAW(RT_DEBUG_TRACE,("%s: AGS: Fixed - CurrTxRateIdx=%d, MCS=%d, STBC=%d, ShortGI=%d, Mode=%d, BW=%d, PER=%d%%, Retry=%d, NoRetry=%d\n\n", 
            __FUNCTION__, pEntry->CurrTxRateIndex, 
            READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg), 
            READ_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg), 
            READ_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg), 
            READ_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg), 
            READ_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg), 
            pAGSStatisticsInfo->TxErrorRatio, 
            pAGSStatisticsInfo->TxRetransmit, 
            pAGSStatisticsInfo->TxSuccess));

        return;
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: AGS: Rssi = %d, TxSuccess = %lu, TxRetransmit = %lu, TxFailCount = %lu, TxErrorRatio = %lu\n", 
            __FUNCTION__, 
            pAGSStatisticsInfo->RSSI, 
            pAGSStatisticsInfo->TxSuccess, 
            pAGSStatisticsInfo->TxRetransmit, 
            pAGSStatisticsInfo->TxFailCount, 
            pAGSStatisticsInfo->TxErrorRatio));

        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: Before - CurrTxRateIdx = %d, MCS = %d, STBC = %d, ShortGI = %d, Mode = %d, TrainUp = %d, TrainDown = %d, NextUp = %d, NextDown = %d, CurrMCS = %d, pEntry->AGSCtrl.MCSGroup = %d, PER = %lu%%, Retry = %lu, NoRetry = %lu\n", 
            __FUNCTION__, 
            CurrRateIdx, 
            pCurrTxRate->CurrMCS, 
            pCurrTxRate->STBC, 
            pCurrTxRate->ShortGI, 
            pCurrTxRate->Mode, 
            TrainUp, 
            TrainDown, 
            UpRateIdx, 
            DownRateIdx, 
            READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg), 
            pEntry->AGSCtrl.MCSGroup, 
            pAGSStatisticsInfo->TxErrorRatio, 
            pAGSStatisticsInfo->TxRetransmit, 
            pAGSStatisticsInfo->TxSuccess));
    }

    if (! OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED))
    {
        return;
    }


    //
    // MCS selection based on the RSSI information when the Tx samples are fewer than 15.
    //
    if (pAGSStatisticsInfo->AccuTxTotalCnt <= 15)
    {
        CHAR idx = 0;
        UCHAR TxRateIdx;
        UCHAR MCS0 = 0, MCS1 = 0, MCS2 = 0, MCS3 = 0, MCS4 = 0,  MCS5 =0, MCS6 = 0, MCS7 = 0;   
        UCHAR MCS8 = 0, MCS9 = 0, MCS10 = 0, MCS11 = 0, MCS12 = 0, MCS13 = 0, MCS14 = 0, MCS15 = 0;
        UCHAR MCS16 = 0, MCS17 = 0, MCS18 = 0, MCS19 = 0, MCS20 = 0, MCS21 = 0, MCS22 = 0, MCS23 = 0;
        UCHAR Mcs8Vht1x1 = 0, Mcs9Vht1x1 = 0; // VHT 1x1 MCS 8/9 (256QAM)
        UCHAR CCKMCS0 = 0, CCKMCS1 = 0, CCKMCS2 = 0;

        //
        // Check the existence and index of each needed MCS
        //
        while (idx < pTable[0])
        {
            pCurrTxRate = (PRTMP_TX_RATE_SWITCH_AGS)(&pTable[(idx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

            if (IS_VHT_1X1_256QAM_RATE_TABLE(pTable) && 
                AGS_VHT_NSS0_MCS9(pTable, pCurrTxRate) && 
                VHT_1X1_256QAM_CAPABLE_ON_PEER_STA(pEntry))
            {
                Mcs9Vht1x1 = idx;
            }
            else if (IS_VHT_1X1_256QAM_RATE_TABLE(pTable) &&
                AGS_VHT_NSS0_MCS8(pTable, pCurrTxRate) && 
                VHT_1X1_256QAM_CAPABLE_ON_PEER_STA(pEntry))
            {
                Mcs8Vht1x1 = idx;
            }
            else if(AGS_CCK_MCS0(pTable, pCurrTxRate))
            {
                CCKMCS0 = idx;
            }
            else if(AGS_CCK_MCS1(pTable, pCurrTxRate))
            {
                CCKMCS1 = idx;
            }
            else if(AGS_CCK_MCS2(pTable, pCurrTxRate))
            {
                CCKMCS2 = idx;
            }
            else if (AGS_HT_MCS0(pTable, pCurrTxRate) ||
                AGS_VHT_NSS0_MCS0(pTable, pCurrTxRate))
            {
                MCS0 = idx;
            }
            else if (AGS_HT_MCS1(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS1(pTable, pCurrTxRate))
            {
                MCS1 = idx;
            }
            else if (AGS_HT_MCS2(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS2(pTable, pCurrTxRate))
            {
                MCS2 = idx;
            }
            else if (AGS_HT_MCS3(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS3(pTable, pCurrTxRate))
            {
                MCS3 = idx;
            }
            else if (AGS_HT_MCS4(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS4(pTable, pCurrTxRate))
            {
                MCS4 = idx;
            }
            else if (AGS_HT_MCS5(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS5(pTable, pCurrTxRate))
            {
                MCS5 = idx;
            }
            else if (AGS_HT_MCS6(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS6(pTable, pCurrTxRate))
            {
                MCS6 = idx;
            }
            else if (AGS_HT_MCS7(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS0_MCS7(pTable, pCurrTxRate))
            {
                MCS7 = idx;
            }
            else if (AGS_HT_MCS8(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS1_MCS0(pTable, pCurrTxRate))
            {
                MCS8 = idx;
            }
            else if (AGS_HT_MCS9(pTable, pCurrTxRate) || 
                        AGS_VHT_NSS1_MCS1(pTable, pCurrTxRate))
            {
                MCS9 = idx;
            }
            else if (AGS_HT_MCS10(pTable, pCurrTxRate) ||
                        AGS_VHT_NSS1_MCS2(pTable, pCurrTxRate))
            {
                MCS10 = idx;
            }
            else if (AGS_HT_MCS11(pTable, pCurrTxRate) ||
                        AGS_VHT_NSS1_MCS3(pTable, pCurrTxRate))
            {
                MCS11 = idx;
            }
            else if (AGS_HT_MCS12(pTable, pCurrTxRate) ||
                        AGS_VHT_NSS1_MCS4(pTable, pCurrTxRate))
            {
                MCS12 = idx;
            }
            else if (AGS_HT_MCS13(pTable, pCurrTxRate) ||
                        AGS_VHT_NSS1_MCS5(pTable, pCurrTxRate))
            {
                MCS13 = idx;
            }
            else if (AGS_HT_MCS14(pTable, pCurrTxRate) ||
                        AGS_VHT_NSS1_MCS6(pTable, pCurrTxRate))
            {
                MCS14 = idx;
            }
            else if (AGS_HT_MCS15(pTable, pCurrTxRate) ||
                        AGS_VHT_NSS1_MCS7(pTable, pCurrTxRate))
            {
                MCS15 = idx;
            }
            else if (AGS_HT_MCS16(pTable, pCurrTxRate))
            {
                MCS16 = idx;
            }
            else if (AGS_HT_MCS17(pTable, pCurrTxRate))
            {
                MCS17 = idx;
            }
            else if (AGS_HT_MCS18(pTable, pCurrTxRate))
            {
                MCS18 = idx;
            }
            else if (AGS_HT_MCS19(pTable, pCurrTxRate))
            {
                MCS19 = idx;
            }
            else if (AGS_HT_MCS20(pTable, pCurrTxRate))
            {
                MCS20 = idx;
            }
            else if (AGS_HT_MCS21(pTable, pCurrTxRate))
            {
                MCS21 = idx;
            }
            else if (AGS_HT_MCS22(pTable, pCurrTxRate))
            {
                MCS22 = idx;
            }
            else if (AGS_HT_MCS23(pTable, pCurrTxRate))
            {
                MCS23 = idx;
            }
            
            idx++;
        }

        if ((pTable == AGS3x3HTRateTable) && 
             (InitTxRateIdx == AGS3x3HTRateTable[1]))
        {
            RssiOffsetForAgs = AgsRssiOffsetTable[2][pPort->BBPCurrentBW];
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: 3*3, RssiOffsetForAgs=%d\n", __FUNCTION__, RssiOffsetForAgs));
            
            //
            // 3x3 peer device (Adhoc, DLS or AP)
            //
            if (MCS23 && (pAGSStatisticsInfo->RSSI > (-67 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS23;
            }
            else if (MCS22 && (pAGSStatisticsInfo->RSSI > (-69 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS22;
            }
            else if (MCS21 && (pAGSStatisticsInfo->RSSI > (-72 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS21;
            }
            else if (MCS20 && (pAGSStatisticsInfo->RSSI > (-74 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS20;
            }
            else if (MCS19 && (pAGSStatisticsInfo->RSSI > (-78 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS19;
            }
            else if (MCS18 && (pAGSStatisticsInfo->RSSI > (-80 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS18;
            }
            else if (MCS17 && (pAGSStatisticsInfo->RSSI > (-85 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS17;
            }
            else
            {
                TxRateIdx = MCS16;
            }
            
            pEntry->AGSCtrl.MCSGroup = 3;
        } 
        else if (((pTable == AGS2x2HTRateTable) && 
                    (InitTxRateIdx == AGS2x2HTRateTable[1])) || 
                    ((pTable == Ags2x2VhtRateTable) && 
                    (InitTxRateIdx == Ags2x2VhtRateTable[1])))
        {
            RssiOffsetForAgs = AgsRssiOffsetTable[1][pPort->BBPCurrentBW];
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: 2*2, RssiOffsetForAgs=%d\n", __FUNCTION__, RssiOffsetForAgs));
            
            //
            // 2x2 peer device (Adhoc, DLS or AP)
            //
            if (MCS15 && (pAGSStatisticsInfo->RSSI > (-69 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS15;
            }
            else if (MCS14 && (pAGSStatisticsInfo->RSSI > (-71 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS14;
            }
            else if (MCS13 && (pAGSStatisticsInfo->RSSI > (-74 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS13;
            }
            else if (MCS12 && (pAGSStatisticsInfo->RSSI > (-76 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS12;
            }
            else if (MCS11 && (pAGSStatisticsInfo->RSSI > (-80 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS11;
            }
            else if (MCS10 && (pAGSStatisticsInfo->RSSI > (-82 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS10;
            }
            else if (MCS9 && (pAGSStatisticsInfo->RSSI > (-87 + RssiOffsetForAgs)))
            {
                TxRateIdx = MCS9;
            }
            else
            {
                TxRateIdx = MCS8;
            }
            
            pEntry->AGSCtrl.MCSGroup = 2;
        } 
        else
        {
            RssiOffsetForAgs = AgsRssiOffsetTable[0][pPort->BBPCurrentBW];
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: 1*1, RssiOffsetForAgs=%d\n", __FUNCTION__, RssiOffsetForAgs));
            
            //
            // 1x1 peer device (Adhoc, DLS or AP)
            //
            {
                if (Mcs9Vht1x1 && (pAGSStatisticsInfo->RSSI > (-67 + RssiOffsetForAgs)))
                {
                    TxRateIdx = Mcs9Vht1x1;
                }
                else if (Mcs8Vht1x1 && (pAGSStatisticsInfo->RSSI > (-69 + RssiOffsetForAgs)))
                {
                    TxRateIdx = Mcs8Vht1x1;
                }
                else if (MCS7 && (pAGSStatisticsInfo->RSSI > (-71 + RssiOffsetForAgs)))
                {
                    TxRateIdx = MCS7;
                }
                else if (MCS6 && (pAGSStatisticsInfo->RSSI > (-73 + RssiOffsetForAgs)))
                {
                    TxRateIdx = MCS6;
                }
                else if (MCS5 && (pAGSStatisticsInfo->RSSI > (-76 + RssiOffsetForAgs)))
                {
                    TxRateIdx = MCS5;
                }
                else if (MCS4 && (pAGSStatisticsInfo->RSSI > (-78 + RssiOffsetForAgs)))
                {
                    TxRateIdx = MCS4;
                }
                else if (MCS3 && (pAGSStatisticsInfo->RSSI > (-82 + RssiOffsetForAgs)))
                {
                    TxRateIdx = MCS3;
                }
                else if (MCS2 && (pAGSStatisticsInfo->RSSI > (-84 + RssiOffsetForAgs)))
                {
                    TxRateIdx = MCS2;
                }
                else if (MCS1 && (pAGSStatisticsInfo->RSSI > (-89 + RssiOffsetForAgs)))
                {
                    TxRateIdx = MCS1;
                }
                else if(MCS0 && (pAGSStatisticsInfo->RSSI > (-91 + RssiOffsetForAgs)))
                {
                    TxRateIdx = MCS0;
                }
                else if (CCKMCS2 && (pAGSStatisticsInfo->RSSI > (-92 + RssiOffsetForAgs)))
                {
                    TxRateIdx = CCKMCS2;
                }               
                else if (CCKMCS1 && (pAGSStatisticsInfo->RSSI > (-94 + RssiOffsetForAgs)))
                {
                    TxRateIdx = CCKMCS1;
                }
                else
                {
                    TxRateIdx = CCKMCS0;
                }
            }
            pEntry->AGSCtrl.MCSGroup = 1;
        }

        pEntry->AGSCtrl.lastRateIdx = pEntry->CurrTxRateIndex;
        pEntry->CurrTxRateIndex = TxRateIdx;
        pNextTxRate = (PRTMP_TX_RATE_SWITCH)(&pTable[(pEntry->CurrTxRateIndex + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

        MlmeSetTxRate(pAd, pEntry, pNextTxRate, pTable);

        PlatformZeroMemory(pEntry->TxQuality, (sizeof(USHORT) * MAX_STEP_OF_TX_RATE_SWITCH));
        PlatformZeroMemory(pEntry->PER, (sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH));
        
        pEntry->fLastSecAccordingRSSI = TRUE;           

        return;
    }

    //
    // The MCS selection is based on the RSSI and skips the rate tuning this time.
    //
    if (pEntry->fLastSecAccordingRSSI  == TRUE)
    {
        pEntry->fLastSecAccordingRSSI = FALSE;
        pEntry->LastSecTxRateChangeAction = 0;
        
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: The MCS selection is based on the RSSI, and skips the rate tuning this time.\n", 
            __FUNCTION__));

        return;
    }

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: TrainUp:%d, TrainDown:%d\n",__FUNCTION__, TrainUp, TrainDown));
    do
    {
        BOOLEAN bTrainUpDown = FALSE;

        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: TxQuality[CurrRateIdx(%d)] = %d, UpPenalty:%d\n",__FUNCTION__, CurrRateIdx, pEntry->TxQuality[CurrRateIdx], pEntry->TxRateUpPenalty));
            
        if (pAGSStatisticsInfo->TxErrorRatio >= TrainDown) // Poor quality
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: TxErrorRatio >= TrainDown\n",__FUNCTION__));
            bTrainUpDown = TRUE;
            pEntry->TxQuality[CurrRateIdx] = AGS_TX_QUALITY_WORST_BOUND;
        }   
        else if (pAGSStatisticsInfo->TxErrorRatio <= TrainUp) // Good quality
        {
            bTrainUpDown = TRUE;
            bUpgradeQuality = TRUE;

            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: pEntry->TxQuality[CurrRateIdx] = %d, pEntry->TxRateUpPenalty = %d\n",
                __FUNCTION__, 
                pEntry->TxQuality[CurrRateIdx], 
                pEntry->TxRateUpPenalty));
            
            if (pEntry->TxQuality[CurrRateIdx])
            {
                pEntry->TxQuality[CurrRateIdx]--; // Good quality in the current Tx rate
            }

            if (pEntry->TxRateUpPenalty)
            {
                pEntry->TxRateUpPenalty--;
            }
            else
            {
                if (pEntry->TxQuality[pCurrTxRate->upMcs3] && (pCurrTxRate->upMcs3 != CurrRateIdx))
                {
                    pEntry->TxQuality[pCurrTxRate->upMcs3]--;
                }
                
                if (pEntry->TxQuality[pCurrTxRate->upMcs2] && (pCurrTxRate->upMcs2 != CurrRateIdx))
                {
                    pEntry->TxQuality[pCurrTxRate->upMcs2]--;
                }
                
                if (pEntry->TxQuality[pCurrTxRate->upMcs1] && (pCurrTxRate->upMcs1 != CurrRateIdx))
                {
                    pEntry->TxQuality[pCurrTxRate->upMcs1]--;
                }
            }
        }
        else if (pEntry->AGSCtrl.MCSGroup > 0) //even if TxErrorRatio > TrainUp
        {
            if (UpRateIdx != 0)
            {
                bTrainUpDown = FALSE;
                
                if (pEntry->TxQuality[CurrRateIdx])
                {
                    pEntry->TxQuality[CurrRateIdx]--; // Good quality in the current Tx rate
                }

                 if (pEntry->TxQuality[UpRateIdx])
                 {
                    pEntry->TxQuality[UpRateIdx]--; // It may improve next train-up Tx rate's quality
                }
            }
        }

        pEntry->PER[CurrRateIdx] = (UCHAR)(pAGSStatisticsInfo->TxErrorRatio);

        //
        // Update the current Tx rate
        //
        if (bTrainUpDown)
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: bTrainUpDown = %d, CurrRateIdx = %d, DownRateIdx = %d, UpRateIdx = %d, pEntry->TxQuality[CurrRateIdx] = %d, pEntry->TxQuality[UpRateIdx] = %d\n",
                __FUNCTION__, 
                bTrainUpDown, 
                CurrRateIdx, 
                DownRateIdx, 
                UpRateIdx, 
                pEntry->TxQuality[CurrRateIdx], 
                pEntry->TxQuality[UpRateIdx]));

            // Downgrade Tx rate
            if ((CurrRateIdx != DownRateIdx) && 
                 (pEntry->TxQuality[CurrRateIdx] >= AGS_TX_QUALITY_WORST_BOUND))
            {
                pEntry->CurrTxRateIndex = DownRateIdx;
                pEntry->LastSecTxRateChangeAction = 2; // Tx rate down
            }
            else if ((CurrRateIdx != UpRateIdx) && 
                        (pEntry->TxQuality[UpRateIdx] <= 0)) // Upgrade Tx rate
            {
                pEntry->CurrTxRateIndex = UpRateIdx;
                pEntry->LastSecTxRateChangeAction = 1; // Tx rate up
            }
        }
    } while (FALSE);

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: pEntry->CurrTxRateIndex = %d, CurrRateIdx = %d, pEntry->LastSecTxRateChangeAction = %d\n", 
        __FUNCTION__, 
        pEntry->CurrTxRateIndex, 
        CurrRateIdx, 
        pEntry->LastSecTxRateChangeAction));
    
    // Tx rate up
    if ((pEntry->CurrTxRateIndex != CurrRateIdx) && 
         (pEntry->LastSecTxRateChangeAction == 1))
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: ++TX rate from %d to %d\n", 
            __FUNCTION__, 
            CurrRateIdx, 
            pEntry->CurrTxRateIndex));
        
        pEntry->TxRateUpPenalty = 0;
        pEntry->LastSecTxRateChangeAction = 1; // Tx rate up
        PlatformZeroMemory(pEntry->PER, sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH);
        pEntry->AGSCtrl.lastRateIdx = CurrRateIdx;

        //
        // Tx rate fast train up
        //
        if (!pAd->StaCfg.StaQuickResponeForRateUpTimerRunning)
        {
            pAd->StaCfg.StaQuickResponeForRateUpTimerRunning = TRUE;
        }

        bTxRateChanged = TRUE;
    }
    else if ((pEntry->CurrTxRateIndex != CurrRateIdx) && 
                 (pEntry->LastSecTxRateChangeAction == 2)) // Tx rate down
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: --TX rate from %d to %d\n", 
            __FUNCTION__, 
            CurrRateIdx, 
            pEntry->CurrTxRateIndex));
        
        pEntry->TxRateUpPenalty = 0; // No penalty
        pEntry->LastSecTxRateChangeAction = 2; // Tx rate down
        pEntry->TxQuality[pEntry->CurrTxRateIndex] = 0;
        pEntry->PER[pEntry->CurrTxRateIndex] = 0;
        pEntry->AGSCtrl.lastRateIdx = CurrRateIdx;

        //
        // Tx rate fast train down
        // 
        if (!pAd->StaCfg.StaQuickResponeForRateUpTimerRunning)
        {
            pAd->StaCfg.StaQuickResponeForRateUpTimerRunning = TRUE;
        }

        bTxRateChanged = TRUE;
    }
    else // Tx rate remains unchanged.
    {
        pEntry->LastSecTxRateChangeAction = 0; // Tx rate remains unchanged.
        bTxRateChanged = FALSE;
    }

    pEntry->LastTxOkCount = pAGSStatisticsInfo->TxSuccess;

    pNextTxRate = (PRTMP_TX_RATE_SWITCH)(&pTable[(pEntry->CurrTxRateIndex + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

    if ((bTxRateChanged == TRUE) && (pNextTxRate != NULL))
    {
        MlmeSetTxRate(pAd, pEntry, pNextTxRate, pTable);
    }

    //
    // RDG threshold control for the infrastructure mode only
    //
    if (INFRA_ON(pPort)/* && (pAd->OpMode == OPMODE_STA) */&& (!DLS_ON(pAd)) && (!TDLS_ON(pAd)))
    {
        PMAC_TABLE_ENTRY pMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
        if(pMacTabEntry == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
            return;
        }
        
        if ((pPort->CommonCfg.bRdg == TRUE) && CLIENT_STATUS_TEST_FLAG(pMacTabEntry, fCLIENT_STATUS_RDG_CAPABLE)) // RDG capable
        {
            TXOP_THRESHOLD_CFG_STRUC TxopThCfg = {0};
            TX_LINK_CFG_STRUC TxLinkCfg = {0};
            
            if ((pAd->Counter.MTKCounters.OneSecReceivedByteCount > (pAd->Counter.MTKCounters.OneSecTransmittedByteCount * 5)) && 
                    (((pTable == AGS3x3HTRateTable) && (pNextTxRate->CurrMCS != MCS_23)) ||
                    ((pTable == AGS2x2HTRateTable) && (pNextTxRate->CurrMCS != MCS_15)) || 
                    ((pTable == AGS1x1HTRateTable) && (pNextTxRate->CurrMCS != MCS_7)) ||
                    ((pTable == AGS1x1HTRateTable5G) && (pNextTxRate->CurrMCS != MCS_7))) && 
                    ((pAd->Counter.MTKCounters.OneSecReceivedByteCount + pAd->Counter.MTKCounters.OneSecTransmittedByteCount) >= (50 * 1024)))
            {
                RTUSBReadMACRegister(pAd, TX_LINK_CFG, &TxLinkCfg.word);
                TxLinkCfg.field.TxRDGEn = 0;
                RTUSBWriteMACRegister(pAd, TX_LINK_CFG, TxLinkCfg.word);

                RTUSBReadMACRegister(pAd, TXOP_THRES_CFG, &TxopThCfg.word);
                TxopThCfg.field.RDG_IN_THRES = 0xFF; // Similar to diable Rx RDG
                TxopThCfg.field.RDG_OUT_THRES = 0x00;
                RTUSBWriteMACRegister(pAd, TXOP_THRES_CFG, TxopThCfg.word);

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("AGS: %s: RDG_IN_THRES = 0xFF\n", __FUNCTION__));
            }
            else
            {
                RTUSBReadMACRegister(pAd, TX_LINK_CFG, &TxLinkCfg.word);
                TxLinkCfg.field.TxRDGEn = 1;
                RTUSBWriteMACRegister(pAd, TX_LINK_CFG, TxLinkCfg.word);

                RTUSBReadMACRegister(pAd, TXOP_THRES_CFG, &TxopThCfg.word);
                TxopThCfg.field.RDG_IN_THRES = 0x00;
                TxopThCfg.field.RDG_OUT_THRES = 0x00;
                RTUSBWriteMACRegister(pAd, TXOP_THRES_CFG, TxopThCfg.word);

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("AGS: %s: RDG_IN_THRES = 0x00\n", __FUNCTION__));
            }
        }
    }

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("AGS: <--- %s\n", __FUNCTION__));
}

//
// Auto Tx rate faster train up/down for AGS in VHT(Adaptive Group Switching)
//
// Parameters
//  pAd: The adapter data structure
//  pEntry: Pointer to a caller-supplied variable in which points to a MAC table entry
//  pTable: Pointer to a caller-supplied variable in wich points to a Tx rate switching table
//  TableSize: The size, in bytes, of the specified Tx rate switching table
//  pAGSStatisticsInfo: Pointer to a caller-supplied variable in which points to the statistics information
//
// Return Value:
//  None
//
VOID StaQuickResponeForRateUpExecAGSv2(
    IN PMP_ADAPTER pAd, 
    IN PMAC_TABLE_ENTRY pEntry, 
    IN PUCHAR pTable, 
    IN UCHAR TableSize, 
    IN PAGS_STATISTICS_INFO pAGSStatisticsInfo,
    IN UCHAR InitTxRateIdx)
{
    UCHAR UpRateIdx = 0, DownRateIdx = 0, CurrRateIdx = 0;
    BOOLEAN bTxRateChanged = TRUE;
    PRTMP_TX_RATE_SWITCH_AGS pCurrTxRate = NULL;
    PRTMP_TX_RATE_SWITCH     pNextTxRate = NULL;
    UCHAR TrainDown = 0, TrainUp = 0;
    CHAR ratio = 0;
    ULONG OneSecTxNoRetryOKRationCount = 0;
    PMP_PORT              pPort;

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("QuickAGS: ---> %s\n", __FUNCTION__));

    if ((pAd->OpMode == OPMODE_STAP2P) )
    {
        //pPort  = pAd->PortList[pPort->P2PCfg.PortNumber];
    }
    else
        pPort  = pAd->PortList[PORT_0];

    pAd->StaCfg.StaQuickResponeForRateUpTimerRunning = FALSE;

    DBGPRINT(RT_DEBUG_TRACE, ("%s: QuickAGS: AccuTxTotalCnt = %d, TxSuccess = %d, TxRetransmit = %d, TxFailCount = %d, TxErrorRatio = %d\n",
        __FUNCTION__, 
        pAGSStatisticsInfo->AccuTxTotalCnt, 
        pAGSStatisticsInfo->TxSuccess, 
        pAGSStatisticsInfo->TxRetransmit, 
        pAGSStatisticsInfo->TxFailCount, 
        pAGSStatisticsInfo->TxErrorRatio));

    CurrRateIdx = pEntry->CurrTxRateIndex;  

    if (CurrRateIdx >= TableSize)
    {
        CurrRateIdx = TableSize - 1;
    }

    UpRateIdx = DownRateIdx = pEntry->AGSCtrl.lastRateIdx;

    pCurrTxRate = (PRTMP_TX_RATE_SWITCH_AGS)(&pTable[(CurrRateIdx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);


    if ((pAGSStatisticsInfo->RSSI > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
    {
        TrainUp = (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
        TrainDown   = (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
    }
    else
    {
        TrainUp = pCurrTxRate->TrainUp;
        TrainDown   = pCurrTxRate->TrainDown;
    }
        
    //
    // MCS selection based on the RSSI information when the Tx samples are fewer than 15.
    //
    if (pAGSStatisticsInfo->AccuTxTotalCnt <= 15)
    {
        PlatformZeroMemory(pEntry->TxQuality, sizeof(USHORT) * MAX_STEP_OF_TX_RATE_SWITCH);
        PlatformZeroMemory(pEntry->PER, sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH);

        if ((pEntry->LastSecTxRateChangeAction == 1) && (CurrRateIdx != DownRateIdx))
        {
            pEntry->CurrTxRateIndex = DownRateIdx;
            pEntry->TxQuality[CurrRateIdx] = AGS_TX_QUALITY_WORST_BOUND;
        }
        else if ((pEntry->LastSecTxRateChangeAction == 2) && (CurrRateIdx != UpRateIdx))
        {
            pEntry->CurrTxRateIndex = UpRateIdx;
        }

        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: AccuTxTotalCnt <= 15, train back to original rate\n", 
            __FUNCTION__));
        
        return;
    }

    do
    {
        if (pEntry->LastTimeTxRateChangeAction == 0)
        {
            ratio = 5;
        }
        else
        {
            ratio = 4;
        }

        if (pAGSStatisticsInfo->TxErrorRatio >= TrainDown) // Poor quality
        {
            pEntry->TxQuality[CurrRateIdx] = AGS_TX_QUALITY_WORST_BOUND;
        }

        pEntry->PER[CurrRateIdx] = (UCHAR)(pAGSStatisticsInfo->TxErrorRatio);

        OneSecTxNoRetryOKRationCount = (pAGSStatisticsInfo->TxSuccess * ratio);
        
        // Tx rate down
        if ((pEntry->LastSecTxRateChangeAction == 1) && (CurrRateIdx != DownRateIdx))
        {
            // Change Auto Rate tuning rule 1: Change from tx_ok to PER
            if (pAGSStatisticsInfo->TxErrorRatio > TrainDown)  // Poor quality
            {
                pEntry->CurrTxRateIndex = DownRateIdx;
                pEntry->TxQuality[CurrRateIdx] = AGS_TX_QUALITY_WORST_BOUND;

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: (UP) bad Tx ok count (Current PER:%d, NewMcs's TrainDown:%d)\n",
                    __FUNCTION__, 
                    pAGSStatisticsInfo->TxErrorRatio,
                    TrainDown));
            }
            else // Good quality
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: (UP) keep rate-up (Current PER:%d, NewMcs's TrainDown:%d)\n", 
                    __FUNCTION__, 
                    pAGSStatisticsInfo->TxErrorRatio,
                    TrainDown));

                PlatformZeroMemory(pEntry->TxQuality, sizeof(USHORT) * MAX_STEP_OF_TX_RATE_SWITCH);

                if (pEntry->AGSCtrl.MCSGroup == 0)
                {
                    if ((pTable == AGS3x3HTRateTable) && 
                         (InitTxRateIdx == AGS3x3HTRateTable[1]))
                    {
                        //
                        // 3x3 peer device (Adhoc, DLS or AP)
                        //
                        pEntry->AGSCtrl.MCSGroup = 3;
                    }
                    else if (((pTable == AGS2x2HTRateTable) && 
                                (InitTxRateIdx == AGS2x2HTRateTable[1])) || 
                                ((pTable == Ags2x2VhtRateTable) && 
                                  (InitTxRateIdx == Ags2x2VhtRateTable[1])))
                    {
                        //
                        // 2x2 peer device (Adhoc, DLS or AP)
                        //
                        pEntry->AGSCtrl.MCSGroup = 2;
                    }
                    else
                    {
                        pEntry->AGSCtrl.MCSGroup = 1;
                    }
                }
            }
        }
        else if ((pEntry->LastSecTxRateChangeAction == 2) && (CurrRateIdx != UpRateIdx)) // Tx rate up
        {

// Don't quick check within train down case
#if 0       
            // Change Auto Rate tuning rule 1: Change from tx_ok to PER
            if (pAGSStatisticsInfo->TxErrorRatio > TrainDown)  // Poor quality
            {
                pEntry->CurrTxRateIndex = UpRateIdx;
                
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: (DOWN) bad tx ok count (Current PER:%d, NewMcs's TrainDown:%d)\n", 
                    __FUNCTION__, 
                    pAGSStatisticsInfo->TxErrorRatio,
                    TrainDown));
            }
            else
            {
                if ((pTable == AGS3x3HTRateTable) && 
                     (InitTxRateIdx == AGS3x3HTRateTable[1]))
                {
                    //
                    // 3x3 peer device (Adhoc, DLS or AP)
                    //
                    pEntry->AGSCtrl.MCSGroup = 3;
                }
                else if (((pTable == AGS2x2HTRateTable) && 
                            (InitTxRateIdx == AGS2x2HTRateTable[1])) || 
                            ((pTable == Ags2x2VhtRateTable) && 
                              (InitTxRateIdx == Ags2x2VhtRateTable[1])))
                {
                    //
                    // 2x2 peer device (Adhoc, DLS or AP)
                    //
                    pEntry->AGSCtrl.MCSGroup = 2;
                }
                else
                {
                    pEntry->AGSCtrl.MCSGroup = 1;
                }

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: (Down) keep rate-down (Current PER:%d, NewMcs's TrainDown:%d)\n", 
                    __FUNCTION__, 
                    pAGSStatisticsInfo->TxErrorRatio,
                    TrainDown));
            }
#endif          
        }
    }while (FALSE);

    //
    // Last action is rate-up
    //
    if (pEntry->LastSecTxRateChangeAction == 1) 
    {
        // looking for the next group with valid MCS
        if ((pEntry->CurrTxRateIndex != CurrRateIdx) && (pEntry->AGSCtrl.MCSGroup > 0))
        {
            pEntry->AGSCtrl.MCSGroup--; // Try to use the MCS of the lower MCS group
            pCurrTxRate = (PRTMP_TX_RATE_SWITCH_AGS)(&pTable[(DownRateIdx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);
        }
        
        // UpRateIdx is for temp use in this section
        switch (pEntry->AGSCtrl.MCSGroup)
        {
            case 3: 
            {
                UpRateIdx = pCurrTxRate->upMcs3;
            }
            break;
            
            case 2: 
            {
                UpRateIdx = pCurrTxRate->upMcs2;
            }
            break;
            
            case 1: 
            {
                UpRateIdx = pCurrTxRate->upMcs1;
            }
            break;
            
            case 0: 
            {
                UpRateIdx = CurrRateIdx;
            }
            break;
            
            default: 
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: Incorrect MCS group, pEntry->AGSCtrl.MCSGroup = %d\n", 
                    __FUNCTION__, 
                    pEntry->AGSCtrl.MCSGroup));
            }
            break;
        }

        if (UpRateIdx == pEntry->CurrTxRateIndex)
        {
            pEntry->AGSCtrl.MCSGroup = 0; // Try to escape the local optima
        }
        
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: next MCS group,  pEntry->AGSCtrl.MCSGroup = %d\n", 
            __FUNCTION__, 
            pEntry->AGSCtrl.MCSGroup));
        
    }

    if ((pEntry->CurrTxRateIndex != CurrRateIdx) && 
         (pEntry->LastSecTxRateChangeAction == 2)) // Tx rate up
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: ++TX rate from %d to %d\n", 
            __FUNCTION__, 
            CurrRateIdx, 
            pEntry->CurrTxRateIndex));
        
        pEntry->TxRateUpPenalty = 0;
        pEntry->TxQuality[pEntry->CurrTxRateIndex] = 0; //restore the TxQuality from max to 0
        PlatformZeroMemory(pEntry->PER, sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH);
    }
    else if ((pEntry->CurrTxRateIndex != CurrRateIdx) && 
                (pEntry->LastSecTxRateChangeAction == 1)) // Tx rate down
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: --TX rate from %d to %d\n", 
            __FUNCTION__, 
            CurrRateIdx, 
            pEntry->CurrTxRateIndex));
        
        pEntry->TxRateUpPenalty = 0; // No penalty
        pEntry->TxQuality[pEntry->CurrTxRateIndex] = 0;
        pEntry->PER[pEntry->CurrTxRateIndex] = 0;
    }
    else
    {
        bTxRateChanged = FALSE;
        
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: rate is not changed\n", 
            __FUNCTION__));
    }

    pNextTxRate = (PRTMP_TX_RATE_SWITCH)(&pTable[(pEntry->CurrTxRateIndex + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);
    if ((bTxRateChanged == TRUE) && (pNextTxRate != NULL))
    {
        AsicCfgPAPEByStreams(pAd, pCurrTxRate->CurrMCS, pNextTxRate->CurrMCS);
        MlmeSetTxRate(pAd, pEntry, pNextTxRate, pTable);
    }

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("QuickAGS: <--- %s\n", __FUNCTION__));
}

//
// Auto Tx rate faster train up/down for AGS (Adaptive Group Switching)
//
// Parameters
//  pAd: The adapter data structure
//  pEntry: Pointer to a caller-supplied variable in which points to a MAC table entry
//  pTable: Pointer to a caller-supplied variable in wich points to a Tx rate switching table
//  TableSize: The size, in bytes, of the specified Tx rate switching table
//  pAGSStatisticsInfo: Pointer to a caller-supplied variable in which points to the statistics information
//  InitTxRateIdx: The initial Tx rate index (3x3: 0x18, 2x2: 0x0F and 1x1: 0x07)
//
// Return Value:
//  None
//
VOID StaQuickResponeForRateUpExecAGS(
    IN PMP_ADAPTER pAd, 
    IN PMAC_TABLE_ENTRY pEntry, 
    IN PUCHAR pTable, 
    IN UCHAR TableSize, 
    IN PAGS_STATISTICS_INFO pAGSStatisticsInfo, 
    IN UCHAR InitTxRateIdx)
{
    UCHAR UpRateIdx = 0, DownRateIdx = 0, CurrRateIdx = 0;
    BOOLEAN bTxRateChanged = TRUE;
    PRTMP_TX_RATE_SWITCH_AGS pCurrTxRate = NULL;
    PRTMP_TX_RATE_SWITCH     pNextTxRate = NULL;
    UCHAR TrainDown = 0, TrainUp = 0;
    CHAR ratio = 0;
    ULONG OneSecTxNoRetryOKRationCount = 0;

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("QuickAGS: ---> %s\n", __FUNCTION__));

    pAd->StaCfg.StaQuickResponeForRateUpTimerRunning = FALSE;

    DBGPRINT(RT_DEBUG_TRACE, ("%s: QuickAGS: AccuTxTotalCnt = %d, TxSuccess = %d, TxRetransmit = %d, TxFailCount = %d, TxErrorRatio = %d\n",
        __FUNCTION__, 
        pAGSStatisticsInfo->AccuTxTotalCnt, 
        pAGSStatisticsInfo->TxSuccess, 
        pAGSStatisticsInfo->TxRetransmit, 
        pAGSStatisticsInfo->TxFailCount, 
        pAGSStatisticsInfo->TxErrorRatio));

    CurrRateIdx = pEntry->CurrTxRateIndex;  

    if (CurrRateIdx >= TableSize)
    {
        CurrRateIdx = TableSize - 1;
    }

    UpRateIdx = DownRateIdx = pEntry->AGSCtrl.lastRateIdx;

    pCurrTxRate = (PRTMP_TX_RATE_SWITCH_AGS)(&pTable[(CurrRateIdx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

    if ((pAGSStatisticsInfo->RSSI > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
    {
        TrainUp = (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
        TrainDown   = (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
    }
    else
    {
        TrainUp = pCurrTxRate->TrainUp;
        TrainDown   = pCurrTxRate->TrainDown;
    }
        
    //
    // MCS selection based on the RSSI information when the Tx samples are fewer than 15.
    //
    if (pAGSStatisticsInfo->AccuTxTotalCnt <= 15)
    {
        PlatformZeroMemory(pEntry->TxQuality, sizeof(USHORT) * MAX_STEP_OF_TX_RATE_SWITCH);
        PlatformZeroMemory(pEntry->PER, sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH);

        if ((pEntry->LastSecTxRateChangeAction == 1) && (CurrRateIdx != DownRateIdx))
        {
            pEntry->CurrTxRateIndex = DownRateIdx;
            pEntry->TxQuality[CurrRateIdx] = AGS_TX_QUALITY_WORST_BOUND;
        }
        else if ((pEntry->LastSecTxRateChangeAction == 2) && (CurrRateIdx != UpRateIdx))
        {
            pEntry->CurrTxRateIndex = UpRateIdx;
        }

        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: AccuTxTotalCnt <= 15, train back to original rate\n", 
            __FUNCTION__));
        
        return;
    }

    do
    {
        if (pEntry->LastTimeTxRateChangeAction == 0)
        {
            ratio = 5;
        }
        else
        {
            ratio = 4;
        }

        if (pAGSStatisticsInfo->TxErrorRatio >= TrainDown) // Poor quality
        {
            pEntry->TxQuality[CurrRateIdx] = AGS_TX_QUALITY_WORST_BOUND;
        }

        pEntry->PER[CurrRateIdx] = (UCHAR)(pAGSStatisticsInfo->TxErrorRatio);

        OneSecTxNoRetryOKRationCount = (pAGSStatisticsInfo->TxSuccess * ratio);
        
        // Tx rate down
        if ((pEntry->LastSecTxRateChangeAction == 1) && (CurrRateIdx != DownRateIdx))
        {
            if ((pEntry->LastTxOkCount + 2) >= OneSecTxNoRetryOKRationCount) // Poor quality
            {
                pEntry->CurrTxRateIndex = DownRateIdx;
                pEntry->TxQuality[CurrRateIdx] = AGS_TX_QUALITY_WORST_BOUND;

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: (UP) bad Tx ok count (L:%d, C:%d)\n", 
                    __FUNCTION__, 
                    pEntry->LastTxOkCount, 
                    OneSecTxNoRetryOKRationCount));
            }
            else // Good quality
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: (UP) keep rate-up (L:%d, C:%d)\n", 
                    __FUNCTION__, 
                    pEntry->LastTxOkCount, 
                    OneSecTxNoRetryOKRationCount));

                PlatformZeroMemory(pEntry->TxQuality, sizeof(USHORT) * MAX_STEP_OF_TX_RATE_SWITCH);

                if (pEntry->AGSCtrl.MCSGroup == 0)
                {
                    if ((pTable == AGS3x3HTRateTable) && 
                        (InitTxRateIdx == AGS3x3HTRateTable[1]))
                    {
                        //
                        // 3x3 peer device (Adhoc, DLS or AP)
                        //
                        pEntry->AGSCtrl.MCSGroup = 3;
                    }
                    else if (((pTable == AGS2x2HTRateTable) && 
                                (InitTxRateIdx == AGS2x2HTRateTable[1])) || 
                                ((pTable == Ags2x2VhtRateTable) && 
                                (InitTxRateIdx == Ags2x2VhtRateTable[1])))
                    {
                        //
                        // 2x2 peer device (Adhoc, DLS or AP)
                        //
                        pEntry->AGSCtrl.MCSGroup = 2;
                    }
                    else
                    {
                        pEntry->AGSCtrl.MCSGroup = 1;
                    }
                }
            }
        }
        else if ((pEntry->LastSecTxRateChangeAction == 2) && (CurrRateIdx != UpRateIdx)) // Tx rate up
        {
            if ((pAGSStatisticsInfo->TxErrorRatio >= 50) || (pAGSStatisticsInfo->TxErrorRatio >= TrainDown)) // Poor quality
            {
                if ((pTable == AGS3x3HTRateTable) && 
                    (InitTxRateIdx == AGS3x3HTRateTable[1]))
                {
                    //
                    // 3x3 peer device (Adhoc, DLS or AP)
                    //
                    pEntry->AGSCtrl.MCSGroup = 3;
                }
                else if (((pTable == AGS2x2HTRateTable) && 
                            (InitTxRateIdx == AGS2x2HTRateTable[1])) || 
                            ((pTable == Ags2x2VhtRateTable) && 
                            (InitTxRateIdx == Ags2x2VhtRateTable[1])))
                {
                    //
                    // 2x2 peer device (Adhoc, DLS or AP)
                    //
                    pEntry->AGSCtrl.MCSGroup = 2;
                }
                else
                {
                    pEntry->AGSCtrl.MCSGroup = 1;
                }

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: (DOWN) direct train down (TxErrorRatio[%d] >= TrainDown[%d])\n", 
                    __FUNCTION__, 
                    pAGSStatisticsInfo->TxErrorRatio, 
                    TrainDown));
            }
            else if ((pEntry->LastTxOkCount + 2) >= OneSecTxNoRetryOKRationCount)
            {
                pEntry->CurrTxRateIndex = UpRateIdx;
                
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: (DOWN) bad tx ok count (L:%d, C:%d)\n", 
                    __FUNCTION__, 
                    pEntry->LastTxOkCount, 
                    OneSecTxNoRetryOKRationCount));
            }
            else
            {
                if ((pTable == AGS3x3HTRateTable) && 
                    (InitTxRateIdx == AGS3x3HTRateTable[1]))
                {
                    //
                    // 3x3 peer device (Adhoc, DLS or AP)
                    //
                    pEntry->AGSCtrl.MCSGroup = 3;
                }
                else if (((pTable == AGS2x2HTRateTable) && 
                            (InitTxRateIdx == AGS2x2HTRateTable[1])) || 
                            ((pTable == Ags2x2VhtRateTable) && 
                            (InitTxRateIdx == Ags2x2VhtRateTable[1])))
                {
                    //
                    // 2x2 peer device (Adhoc, DLS or AP)
                    //
                    pEntry->AGSCtrl.MCSGroup = 2;
                }
                else
                {
                    pEntry->AGSCtrl.MCSGroup = 1;
                }

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: (Down) keep rate-down (L:%d, C:%d)\n", 
                    __FUNCTION__, 
                    pEntry->LastTxOkCount, 
                    OneSecTxNoRetryOKRationCount));
            }
        }
    }while (FALSE);

    //
    // Last action is rate-up
    //
    if (pEntry->LastSecTxRateChangeAction == 1) 
    {
        // looking for the next group with valid MCS
        if ((pEntry->CurrTxRateIndex != CurrRateIdx) && (pEntry->AGSCtrl.MCSGroup > 0))
        {
            pEntry->AGSCtrl.MCSGroup--; // Try to use the MCS of the lower MCS group
            pCurrTxRate = (PRTMP_TX_RATE_SWITCH_AGS)(&pTable[(DownRateIdx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);
        }
        
        // UpRateIdx is for temp use in this section
        switch (pEntry->AGSCtrl.MCSGroup)
        {
            case 3: 
            {
                UpRateIdx = pCurrTxRate->upMcs3;
            }
            break;
            
            case 2: 
            {
                UpRateIdx = pCurrTxRate->upMcs2;
            }
            break;
            
            case 1: 
            {
                UpRateIdx = pCurrTxRate->upMcs1;
            }
            break;
            
            case 0: 
            {
                UpRateIdx = CurrRateIdx;
            }
            break;
            
            default: 
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: Incorrect MCS group, pEntry->AGSCtrl.MCSGroup = %d\n", 
                    __FUNCTION__, 
                    pEntry->AGSCtrl.MCSGroup));
            }
            break;
        }

        if (UpRateIdx == pEntry->CurrTxRateIndex)
        {
            pEntry->AGSCtrl.MCSGroup = 0; // Try to escape the local optima
        }
        
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: next MCS group,  pEntry->AGSCtrl.MCSGroup = %d\n", 
            __FUNCTION__, 
            pEntry->AGSCtrl.MCSGroup));
        
    }

    if ((pEntry->CurrTxRateIndex != CurrRateIdx) && 
         (pEntry->LastSecTxRateChangeAction == 2)) // Tx rate up
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: ++TX rate from %d to %d\n", 
            __FUNCTION__, 
            CurrRateIdx, 
            pEntry->CurrTxRateIndex));  
        
        pEntry->TxRateUpPenalty = 0;
        pEntry->TxQuality[pEntry->CurrTxRateIndex] = 0; //restore the TxQuality from max to 0
        PlatformZeroMemory(pEntry->PER, sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH);
    }
    else if ((pEntry->CurrTxRateIndex != CurrRateIdx) && 
                (pEntry->LastSecTxRateChangeAction == 1)) // Tx rate down
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: --TX rate from %d to %d\n", 
            __FUNCTION__, 
            CurrRateIdx, 
            pEntry->CurrTxRateIndex));
        
        pEntry->TxRateUpPenalty = 0; // No penalty
        pEntry->TxQuality[pEntry->CurrTxRateIndex] = 0;
        pEntry->PER[pEntry->CurrTxRateIndex] = 0;
    }
    else
    {
        bTxRateChanged = FALSE;
        
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: QuickAGS: rate is not changed\n", 
            __FUNCTION__));
    }

    pNextTxRate = (PRTMP_TX_RATE_SWITCH)(&pTable[(pEntry->CurrTxRateIndex + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);
    if ((bTxRateChanged == TRUE) && (pNextTxRate != NULL))
    {
        AsicCfgPAPEByStreams(pAd, pCurrTxRate->CurrMCS, pNextTxRate->CurrMCS);
        MlmeSetTxRate(pAd, pEntry, pNextTxRate, pTable);
    }

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("QuickAGS: <--- %s\n", __FUNCTION__));
}

/*
    ==========================================================================
    Description:
        This routine is executed periodically inside MlmePeriodicExecTimerCallback() after 
        association with an AP.
        It checks if StaCfg.Psm is consistent with user policy (recorded in
        StaCfg.WindowsPowerMode). If not, enforce user policy. However, 
        there're some conditions to consider:
        1. we don't support power-saving in ADHOC mode, so Psm=PWR_ACTIVE all
           the time when Mibss==TRUE
        2. When link up in INFRA mode, Psm should not be switch to PWR_SAVE
           if outgoing traffic available in TxRing or MgmtRing.
    Output:
        1. change pAd->StaCfg.Psm to PWR_SAVE or leave it untouched

    IRQL <= PASSIVE_LEVEL

    ==========================================================================
 */
VOID MlmeCheckPsmChange(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN ULONGLONG    Now64)
{
    ULONG   PowerMode;
    // condition -
    // 1. Psm maybe ON only happen in INFRASTRUCTURE mode
    // 2. user wants either MAX_PSP or FAST_PSP
    // 3. but current psm is not in PWR_SAVE
    // 4. CNTL state machine is not doing SCANning
    // 5. no TX SUCCESS event for the past 1-sec period
#ifdef NDIS51_MINIPORT
    if (pAd->StaCfg.WindowsPowerProfile == NdisPowerProfileBattery)
        PowerMode = pAd->StaCfg.WindowsBatteryPowerMode;
    else
#endif
        PowerMode = pAd->StaCfg.WindowsPowerMode;   

DBGPRINT(RT_DEBUG_TRACE, ("MlmeSetPsm pAd->StaCfg.PSControl.field.DisablePS = %d, pAd->TrackInfo.CountDowntoPsm = %d\n", pAd->StaCfg.PSControl.field.DisablePS, pAd->TrackInfo.CountDowntoPsm));
DBGPRINT(RT_DEBUG_TRACE, ("MlmeSetPsm INFRA_ON(pPort) = %d\n", INFRA_ON(pPort)));
DBGPRINT(RT_DEBUG_TRACE, ("MlmeSetPsm (PowerMode != DOT11_POWER_SAVING_NO_POWER_SAVING) = %d\n", (PowerMode != DOT11_POWER_SAVING_NO_POWER_SAVING)));
DBGPRINT(RT_DEBUG_TRACE, ("MlmeSetPsm (pAd->StaCfg.Psm == PWR_ACTIVE) = %d\n", (pAd->StaCfg.Psm == PWR_ACTIVE)));
DBGPRINT(RT_DEBUG_TRACE, ("MlmeSetPsm (pPort->Mlme.CntlMachine.CurrState == CNTL_IDLE) = %d\n", (pPort->Mlme.CntlMachine.CurrState == CNTL_IDLE)));
DBGPRINT(RT_DEBUG_TRACE, ("MlmeSetPsm (pAd->Counter.MTKCounters.OneSecTxNoRetryOkCount == 0) = %d\n", (pAd->Counter.MTKCounters.OneSecTxNoRetryOkCount == 0)));
DBGPRINT(RT_DEBUG_TRACE, ("MlmeSetPsm (pAd->TrackInfo.CountDowntoPsm <= 1) = %d\n", (pAd->TrackInfo.CountDowntoPsm <= 1)));

    //  rt2860 Firmware auto wake up for every bulk out frame. So we need to send sleep command to firmware if we are using psm policy.
    if (INFRA_ON(pPort) &&
        (PowerMode != DOT11_POWER_SAVING_NO_POWER_SAVING) &&
        (pAd->StaCfg.Psm == PWR_ACTIVE) &&
        (pPort->Mlme.CntlMachine.CurrState == CNTL_IDLE) &&
        (pAd->Counter.MTKCounters.OneSecTxNoRetryOkCount == 0) && 
        (pAd->TrackInfo.CountDowntoPsm <= 1))
    {
        if ((pAd->Counter.MTKCounters.LastOneSecTotalTxCount <= 50) && (pAd->Counter.MTKCounters.LastOneSecRxOkDataCnt <= 50))
        {   
            MlmeSetPsm(pAd, PWR_SAVE);

            if (!(pPort->CommonCfg.bAPSDCapable && pPort->CommonCfg.APEdcaParm.bAPSDCapable))
            {
                XmitSendNullFrame(pAd,pPort, pPort->CommonCfg.TxRate, FALSE);
            }
            else
            {
                XmitSendNullFrame(pAd,pPort, pPort->CommonCfg.TxRate, TRUE);
            }           
        }
    }   
}

// IRQL <= PASSIVE_LEVEL
VOID MlmeSetPsm(
    IN PMP_ADAPTER pAd, 
    IN USHORT psm)
{
    
#if 1
    pAd->StaCfg.Psm = psm;
    DBGPRINT(RT_DEBUG_TRACE, ("MlmeSetPsm : PSM = %d\n", psm));

#else
    AUTO_RSP_CFG_STRUC AutoRspCfg = {0};

    #if DBG 
        if(psm == PWR_ACTIVE)
        {
            DBGPRINT(RT_DEBUG_TRACE,("[%s][%d] Set pAd->StaCfg.Psm = PWR_ACTIVE\n",__FUNCTION__,__LINE__));
        }
        else if(psm == PWR_SAVE)
        {
            DBGPRINT(RT_DEBUG_TRACE,("[%s][%d] Set pAd->StaCfg.Psm = PWR_SAVE\n",__FUNCTION__,__LINE__));
        }   
    #endif
    pAd->StaCfg.Psm = psm;    

    if (KeGetCurrentIrql() != PASSIVE_LEVEL)
    {
        DBGPRINT(RT_DEBUG_INFO, ("%s: (DISPATCH_LEVEL) PSM = %d\n", 
            __FUNCTION__, 
            psm));

        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_CTRL_PWR_BIT, &psm, sizeof(psm));
    }
    else
    {
        RTUSBReadMACRegister(pAd, AUTO_RSP_CFG, &AutoRspCfg.word);
        AutoRspCfg.field.AckCtsPsmBit = ((psm == PWR_SAVE) ? 1 : 0);
        RTUSBWriteMACRegister(pAd, AUTO_RSP_CFG, AutoRspCfg.word);

        DBGPRINT(RT_DEBUG_INFO, ("%s: (PASSIVE_LEVEL) PSM = %d\n", 
            __FUNCTION__, 
            psm));
    }
#endif  
}

// IRQL = DISPATCH_LEVEL
VOID MlmeSetTxPreamble(
    IN PMP_ADAPTER pAd, 
    IN USHORT TxPreamble)
{
#if 0
    AUTO_RSP_CFG_STRUC csr4;

    //
    // Always use Long preamble before verifiation short preamble functionality works well.
    // Todo: remove the following line if short preamble functionality works
    //
    //TxPreamble = Rt802_11PreambleLong;
    
    RTUSBReadMACRegister(pAd, AUTO_RSP_CFG, &csr4.word);
    if (TxPreamble == Rt802_11PreambleLong)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeSetTxPreamble (= LONG PREAMBLE)\n"));
        OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED); 
        csr4.field.AutoResponderPreamble = 0;
    }
    else
    {
        // NOTE: 1Mbps should always use long preamble
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeSetTxPreamble (= SHORT PREAMBLE)\n"));
        OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
        csr4.field.AutoResponderPreamble = 1;
    }

    RTUSBWriteMACRegister(pAd, AUTO_RSP_CFG, csr4.word);
#endif  
}

// IRQL = PASSIVE_LEVEL
// IRQL = DISPATCH_LEVEL
// bMlmeCntLinkUp is to identify the inital link speed.
// TRUE indicates the rate update at linkup, we should not try to set the rate at 54Mbps.
VOID MlmeUpdateTxRates(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN BOOLEAN       bMlmeCntLinkUp)
{
    int i, num;
    UCHAR Rate = RATE_6, MaxDesire = RATE_1, MaxSupport = RATE_1;
    UCHAR MinSupport = RATE_54;
    ULONG BasicRateBitmap = 0x0000013f;
    UCHAR CurrBasicRate = RATE_1;
    UCHAR *pSupRate, SupRateLen, *pExtRate, ExtRateLen;
    PMP_PORT pStaPort  = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];
    PMAC_TABLE_ENTRY pMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_MBCAST);
    PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);

    // find max desired rate
    num = 0;
    for (i = 0; i < MAX_LEN_OF_SUPPORTED_RATES; i++)
    {
        switch (pPort->CommonCfg.DesireRate[i] & 0x7f)
        {
            case 2:  Rate = RATE_1;   num++;   break;
            case 4:  Rate = RATE_2;   num++;   break;
            case 11: Rate = RATE_5_5; num++;   break;
            case 22: Rate = RATE_11;  num++;   break;
            case 12: Rate = RATE_6;   num++;   break;
            case 18: Rate = RATE_9;   num++;   break;
            case 24: Rate = RATE_12;  num++;   break;
            case 36: Rate = RATE_18;  num++;   break;
            case 48: Rate = RATE_24;  num++;   break;
            case 72: Rate = RATE_36;  num++;   break;
            case 96: Rate = RATE_48;  num++;   break;
            case 108: Rate = RATE_54; num++;   break;
            //default: Rate = RATE_1;   break; 
        }
        if (MaxDesire < Rate)  MaxDesire = Rate;
    }

    pPort->CommonCfg.BasicRateBitmap = BasicRateBitmap;

//===========================================================================
//===========================================================================

    // calculate the exptected ACK rate for each TX rate. This info is used to caculate
    // the DURATION field of outgoing uniicast DATA/MGMT frame
    for (i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
    {
        if (BasicRateBitmap & (0x01 << i))
            CurrBasicRate = (UCHAR)i;
        pPort->CommonCfg.ExpectedACKRate[i] = CurrBasicRate;
        DBGPRINT(RT_DEBUG_INFO,("E ACK  [%d] = %d Mbps", RateIdToMbps[i], RateIdToMbps[CurrBasicRate]));
    }
//===========================================================================
//===========================================================================

    // 2003-12-10 802.11g WIFI spec disallow OFDM rates in 802.11g ADHOC mode
    if ((pAd->StaCfg.BssType == BSS_ADHOC)        &&
        ((pPort->CommonCfg.PhyMode == PHY_11BG_MIXED))
        && 
        (pAd->StaCfg.AdhocMode == 0) &&
        (MaxDesire > RATE_11) && (pPort->PortSubtype == PORTSUBTYPE_STA))
    {
        MaxDesire = RATE_11;
        DBGPRINT(RT_DEBUG_INFO,("MlmeUpdateTxRates. 2 MaxDesire = %d. \n", MaxDesire));
    }

    // CH14 only supports 11b
    if ((pPort->CommonCfg.Ch14BOnly) && (pPort->Channel == CHANNEL_14))
    {
        MaxDesire = RATE_11;
    }

    pPort->CommonCfg.MaxDesiredRate = MaxDesire;
    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.MinPhyCfg, 0);
    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.MaxPhyCfg, 0);
    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.TxPhyCfg, 0);

    // Auto rate switching is enabled only if more than one DESIRED RATES are 
    // specified; otherwise disabled
    if (num <= 1)
    {       
        OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED);
        pPort->CommonCfg.bAutoTxRateSwitch    = FALSE;
    }
    else
    {   
        OPSTATUS_SET_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED); 
        pPort->CommonCfg.bAutoTxRateSwitch    = TRUE;
    }

    if (ADHOC_ON(pPort) || INFRA_ON(pPort) || INFRA_ON(pStaPort))
    {
        pSupRate = &pAd->StaActive.SupRate[0];
        pExtRate = &pAd->StaActive.ExtRate[0];
        SupRateLen = pAd->StaActive.SupRateLen;
        ExtRateLen = pAd->StaActive.ExtRateLen;
    }
    else
    {
        pSupRate = &pPort->CommonCfg.SupRate[0];
        pExtRate = &pPort->CommonCfg.ExtRate[0];
        SupRateLen = pPort->CommonCfg.SupRateLen;
        ExtRateLen = pPort->CommonCfg.ExtRateLen;
    }

    // find max supported rate
    for (i = 0; i < SupRateLen; i++)
    {
        switch (pSupRate[i] & 0x7f)
        {
            case 2:   Rate = RATE_1;    if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0001;   break;
            case 4:   Rate = RATE_2;    if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0002;   break;
            case 11:  Rate = RATE_5_5;  if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0004;   break;
            case 22:  Rate = RATE_11;   if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0008;   break;
            case 12:  Rate = RATE_6;    /*if (pSupRate[i] & 0x80)*/  BasicRateBitmap |= 0x0010;  break;
            case 18:  Rate = RATE_9;    if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0020;   break;
            case 24:  Rate = RATE_12;   /*if (pSupRate[i] & 0x80)*/  BasicRateBitmap |= 0x0040;  break;
            case 36:  Rate = RATE_18;   if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0080;   break;
            case 48:  Rate = RATE_24;   /*if (pSupRate[i] & 0x80)*/  BasicRateBitmap |= 0x0100;  break;
            case 72:  Rate = RATE_36;   if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0200;   break;
            case 96:  Rate = RATE_48;   if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0400;   break;
            case 108: Rate = RATE_54;   if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0800;   break;
            default:  Rate = RATE_1;    break;
        }
        if (MaxSupport < Rate)  MaxSupport = Rate;

        if (MinSupport > Rate) MinSupport = Rate;       
    }
    for (i = 0; i < ExtRateLen; i++)
    {
        switch (pExtRate[i] & 0x7f)
        {
            case 2:   Rate = RATE_1;    if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0001;   break;
            case 4:   Rate = RATE_2;    if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0002;   break;
            case 11:  Rate = RATE_5_5;  if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0004;   break;
            case 22:  Rate = RATE_11;   if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0008;   break;
            case 12:  Rate = RATE_6;    /*if (pExtRate[i] & 0x80)*/  BasicRateBitmap |= 0x0010;  break;
            case 18:  Rate = RATE_9;    if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0020;   break;
            case 24:  Rate = RATE_12;   /*if (pExtRate[i] & 0x80)*/  BasicRateBitmap |= 0x0040;  break;
            case 36:  Rate = RATE_18;   if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0080;   break;
            case 48:  Rate = RATE_24;   /*if (pExtRate[i] & 0x80)*/  BasicRateBitmap |= 0x0100;  break;
            case 72:  Rate = RATE_36;   if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0200;   break;
            case 96:  Rate = RATE_48;   if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0400;   break;
            case 108: Rate = RATE_54;   if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0800;   break;
            default:  Rate = RATE_1;    break;
        }
        if (MaxSupport < Rate)  MaxSupport = Rate;

        if (MinSupport > Rate) MinSupport = Rate;       
    }
    RTUSBWriteMACRegister(pAd, LEGACY_BASIC_RATE, BasicRateBitmap);
    pPort->CommonCfg.BasicRateBitmap = BasicRateBitmap;

    // calculate the exptected ACK rate for each TX rate. This info is used to caculate
    // the DURATION field of outgoing uniicast DATA/MGMT frame
    for (i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
    {
        if (BasicRateBitmap & (0x01 << i))
            CurrBasicRate = (UCHAR)i;
        pPort->CommonCfg.ExpectedACKRate[i] = CurrBasicRate;
        DBGPRINT(RT_DEBUG_INFO,("Exptected ACK rate[%d] = %d Mbps\n", RateIdToMbps[i], RateIdToMbps[CurrBasicRate]));
    }

    DBGPRINT(RT_DEBUG_INFO,("MlmeUpdateTxRates[MaxSupport = %d] = MaxDesire %d Mbps\n", MaxSupport, MaxDesire));
    // max tx rate = min {max desire rate, max supported rate}
    if (MaxSupport < MaxDesire)
        pPort->CommonCfg.MaxTxRate = MaxSupport;
    else
        pPort->CommonCfg.MaxTxRate = MaxDesire;

    pPort->CommonCfg.MinTxRate = MinSupport;

    //Set MaxHTPhyMode
    if (pPort->CommonCfg.MaxTxRate <= RATE_11)
    {
        WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.MaxPhyCfg, MODE_CCK);
        WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MaxPhyCfg, pPort->CommonCfg.MaxTxRate);        
    }
    else
    {
        WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.MaxPhyCfg, MODE_OFDM);
        WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MaxPhyCfg, OfdmRateToRxwiMCS[pPort->CommonCfg.MaxTxRate]);
    }

    //Set MinHTPhyMode
    if (pPort->CommonCfg.MinTxRate <= RATE_11)
    {
        WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.MinPhyCfg, MODE_CCK);
        WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MinPhyCfg, pPort->CommonCfg.MinTxRate);
    }
    else
    {
        WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.MinPhyCfg, MODE_OFDM);
        WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MinPhyCfg, OfdmRateToRxwiMCS[pPort->CommonCfg.MinTxRate]);
    }

    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.TxPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.MaxPhyCfg));
    
    // Set fixed legacy rate if previous setting is not 11n. 
   if (!OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_TX_RATE_SWITCH_ENABLED) && (pBssidMacTabEntry != NULL)&&
        (READ_PHY_CFG_MODE(pAd, &pBssidMacTabEntry->TxPhyCfg) <= MODE_OFDM))
    {
        WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pBssidMacTabEntry->TxPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.TxPhyCfg));
    }

    if ((pBssidMacTabEntry != NULL) && bMlmeCntLinkUp && pPort->PortSubtype == PORTSUBTYPE_STA)
    {
        ULONG PerEntryBw = BW_20;

        PerEntryBw = READ_PHY_CFG_BW(pAd, &pBssidMacTabEntry->TxPhyCfg);

        WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pBssidMacTabEntry->TxPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, & pPort->CommonCfg.TxPhyCfg));
        WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pBssidMacTabEntry->MaxPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, & pPort->CommonCfg.MaxPhyCfg));
        WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pBssidMacTabEntry->MinPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, & pPort->CommonCfg.MinPhyCfg));

        //
        // The per-entry BW should be based on HT/VHT IEs (MlmeCntLinkUp).
        //
        WRITE_PHY_CFG_BW(pAd, &pBssidMacTabEntry->TxPhyCfg, PerEntryBw);
        WRITE_PHY_CFG_BW(pAd, &pBssidMacTabEntry->MaxPhyCfg, PerEntryBw);
        WRITE_PHY_CFG_BW(pAd, &pBssidMacTabEntry->MinPhyCfg, PerEntryBw);

        DBGPRINT(RT_DEBUG_TRACE, ("%s: The per-entry BW should be based on HT/VHT IEs (MlmeCntLinkUp). PerEntryBw = %d\n", 
            __FUNCTION__, 
            PerEntryBw));
    }
    else
    {
        switch (pPort->CommonCfg.PhyMode) 
        {
            case PHY_11BG_MIXED:
            case PHY_11B:
            case PHY_11BGN_MIXED:
                pPort->CommonCfg.MlmeRate = RATE_1;
                WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.MgmtPhyCfg, MODE_CCK);
                WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MgmtPhyCfg, RATE_1);
                
#ifdef  WIFI_TEST           
                pPort->CommonCfg.RtsRate = RATE_11;
#else
                pPort->CommonCfg.RtsRate = RATE_11;
#endif
                break;
            case PHY_11AGN_MIXED:
            case PHY_11GN_MIXED:
            case PHY_11N:
            case PHY_11G:
            case PHY_11A:
            case PHY_11AN_MIXED:
                pPort->CommonCfg.MlmeRate = RATE_6;
                pPort->CommonCfg.RtsRate = RATE_6;
                WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.MgmtPhyCfg, MODE_OFDM);
                WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MgmtPhyCfg, OfdmRateToRxwiMCS[pPort->CommonCfg.MlmeRate]);
                
                break;
            case PHY_11ABG_MIXED:
            case PHY_11ABGN_MIXED:
            case PHY_11VHT:
                if (pPort->Channel <= 14)
                {
                    pPort->CommonCfg.MlmeRate = RATE_1;
                    pPort->CommonCfg.RtsRate = RATE_11;
                    WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.MgmtPhyCfg, MODE_CCK);
                    WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MgmtPhyCfg, RATE_1);
                }
                else
                {
                    pPort->CommonCfg.MlmeRate = RATE_6;
                    pPort->CommonCfg.RtsRate = RATE_6;
                    WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.MgmtPhyCfg, MODE_OFDM);
                    WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MgmtPhyCfg, OfdmRateToRxwiMCS[pPort->CommonCfg.MlmeRate]);
                }
                break;
            default: // error
                pPort->CommonCfg.MlmeRate = RATE_6;
                WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.MgmtPhyCfg, MODE_OFDM);
                WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MgmtPhyCfg, OfdmRateToRxwiMCS[pPort->CommonCfg.MlmeRate]);
                pPort->CommonCfg.RtsRate = RATE_11;
                break;
        }
        //
        // Keep Basic Mlme Rate.
        //
        if(pMacTabEntry !=NULL)
        {
            WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pMacTabEntry->TxPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.MgmtPhyCfg));       

            if (READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.MgmtPhyCfg) == MODE_OFDM)
            {
                WRITE_PHY_CFG_MCS(pAd, &pMacTabEntry->TxPhyCfg, OfdmRateToRxwiMCS[RATE_24]);
            }
            else
            {
                WRITE_PHY_CFG_MCS(pAd, &pMacTabEntry->TxPhyCfg, RATE_1);
            }
        }
        
        pPort->CommonCfg.BasicMlmeRate = pPort->CommonCfg.MlmeRate;
    }

    DBGPRINT(RT_DEBUG_TRACE, (" MlmeUpdateTxRates (MaxDesire=%d, MaxSupport=%d, MaxTxRate=%d, MinRate=%d, Rate Switching =%d)\n", 
             RateIdToMbps[MaxDesire], RateIdToMbps[MaxSupport], RateIdToMbps[pPort->CommonCfg.MaxTxRate], RateIdToMbps[pPort->CommonCfg.MinTxRate], 
             OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED)));
    DBGPRINT(RT_DEBUG_TRACE, (" MlmeUpdateTxRates (TxRate=%d, RtsRate=%d, BasicRateBitmap=0x%04x)\n", 
             RateIdToMbps[pPort->CommonCfg.TxRate], RateIdToMbps[pPort->CommonCfg.RtsRate], BasicRateBitmap));
    if(pMacTabEntry !=NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeUpdateTxRates (MgmtPhyCfg=0x%X, MinPhyCfg=0x%X, MaxPhyCfg=0x%X, TxPhyCfg=0x%X)\n", 
                                    READ_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.MgmtPhyCfg), 
                                    READ_PHY_CFG_DOUBLE_WORD(pAd, &pBssidMacTabEntry->MinPhyCfg), 
                                    READ_PHY_CFG_DOUBLE_WORD(pAd, &pBssidMacTabEntry->MaxPhyCfg), 
                                    READ_PHY_CFG_DOUBLE_WORD(pAd, &pBssidMacTabEntry->TxPhyCfg)));
    }
}

/*
    ==========================================================================
    Description:
        This function update HT Rate setting.
        Input Wcid value is valid for 2 case :
        1. it's used for Station in infra mode that copy AP rate to Mactable.
        2. OR Station   in adhoc mode to copy peer's HT rate to Mactable. 

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeUpdateHtVhtTxRates(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN UCHAR         Wcid)
{
#if 1
   // MAC_TABLE_ENTRY *pEntry;

    //pEntry = &pPort->MacTab.Content[Wcid];

    //MtAsicMcsLutUpdate(pAd, pEntry);

#else
    UCHAR   j, StbcMcs, bitmask;
    RT_HT_CAPABILITY *pRtHtCap;
    RT_VHT_CAPABILITY    *pRtVhtCap;
    ULONG       BasicMCS;
    CHAR    i;
    PMP_PORT  pStaPort;

    MAC_TABLE_ENTRY *pEntry;

    pEntry = &pPort->MacTab.Content[Wcid];

    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s - pPort is  NULL, Error, and return this function", __FUNCTION__));
        return;
    }

    DBGPRINT(RT_DEBUG_INFO,("%s===> \n",__FUNCTION__));
    pStaPort =  pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];
    if (ADHOC_ON(pPort) || INFRA_ON(pPort) ||INFRA_ON(pStaPort))
    {
        if (pAd->StaActive.SupportedHtPhy.bHtEnable == FALSE)
            return;
        pRtHtCap = &pAd->StaActive.SupportedHtPhy;
        StbcMcs = (UCHAR)pAd->MlmeAux.AddHtInfo.AddHtInfo3.StbcMcs;
        BasicMCS =pAd->MlmeAux.AddHtInfo.MCSSet[0]+(pAd->MlmeAux.AddHtInfo.MCSSet[1]<<8)+(StbcMcs<<16);
        if ((pPort->CommonCfg.DesiredHtPhy.TxSTBC)&&(pRtHtCap->TxSTBC))
        {
            WRITE_PHY_CFG_STBC(pAd, &pPort->CommonCfg.MaxPhyCfg, STBC_USE);
        }
        else
        {       
            WRITE_PHY_CFG_STBC(pAd, &pPort->CommonCfg.MaxPhyCfg, STBC_NONE);
        }
    }
    else
    {
        if (pPort->CommonCfg.DesiredHtPhy.bHtEnable == FALSE)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s, LINE_%d, both HT and VHT mode is not supported.\n",__FUNCTION__,__LINE__));
            return;
        }

        pRtHtCap = &pPort->CommonCfg.DesiredHtPhy;
        StbcMcs = (UCHAR)pPort->CommonCfg.AddHTInfo.AddHtInfo3.StbcMcs;
        BasicMCS = pPort->CommonCfg.AddHTInfo.MCSSet[0]+(pPort->CommonCfg.AddHTInfo.MCSSet[1]<<8)+(StbcMcs<<16);

        // -------------------------------------------------------
        // Decide STBC in VHT mode/ HT mode
        // -------------------------------------------------------
        if ((pPort->CommonCfg.DesiredVhtPhy.bVhtEnable == TRUE) &&
            (pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.TxStbc) && (pAd->HwCfg.Antenna.field.TxPath >= 2))
        {
            WRITE_PHY_CFG_STBC(pAd, &pPort->CommonCfg.MaxPhyCfg, STBC_USE);
        }
        else if ( (pPort->CommonCfg.DesiredHtPhy.bHtEnable == TRUE) &&
             (pPort->CommonCfg.DesiredHtPhy.TxSTBC) && (pAd->HwCfg.Antenna.field.TxPath >= 2) )
        {
            WRITE_PHY_CFG_STBC(pAd, &pPort->CommonCfg.MaxPhyCfg, STBC_USE);
        }
        else
        {
            WRITE_PHY_CFG_STBC(pAd, &pPort->CommonCfg.MaxPhyCfg, STBC_NONE);
        }
    }
    
    // -------------------------------------------------------
    // Decide MAX Phymode in VHT mode/ HT mode
    // -------------------------------------------------------
    if ( (pPort->CommonCfg.DesiredVhtPhy.bVhtEnable == TRUE) )
    {
        // if we notice the other side is VHT-capable means: both side are VHT capable;
        WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.MaxPhyCfg, MODE_VHT);
    }
    else if ((pRtHtCap->GF) && (pPort->CommonCfg.DesiredHtPhy.GF))
    {
        WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.MaxPhyCfg, MODE_HTGREENFIELD);
    }
    else
    {
        WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.MaxPhyCfg, MODE_HTMIX);
    }
    
    // -------------------------------------------------------
    // Decide Channel width in VHT mode/ HT mode
    // -------------------------------------------------------
    if (pAd->StaActive.SupportedVhtPhy.bVhtEnable== TRUE)  
    {
        LONG MaxBandwidth= NOT_SUPPORT_ABOVE_BW_80;
        ULONG Bandwidth = BW_80;

        // if we notice the other side is VHT-capable means:
        // 1. we are VHT capable;
        // 2. the spec. indicate that 80Mhz is mandatory for VHT        
        if (pAd->MlmeAux.VhtCapability.VhtCapInfo.SupportedChannelWidthSet
            > pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.SupportedChannelWidthSet)
            MaxBandwidth = pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.SupportedChannelWidthSet;
        else
            MaxBandwidth = pAd->MlmeAux.VhtCapability.VhtCapInfo.SupportedChannelWidthSet;

        // decide the max bandwidth that is supported by both side.
        if (MaxBandwidth == SUPPORT_BW_160_80_80)
        {
            DBGPRINT(RT_DEBUG_TRACE,("ERROR!!, %s, LINE%d, BW80+80 is not supported yet!!\n",__FUNCTION__,__LINE__));
        }
        else if (MaxBandwidth == SUPPORT_BW_160)
        {
            DBGPRINT(RT_DEBUG_TRACE,("ERROR!!, %s, LINE%d, BW160 is not supported yet!!\n",__FUNCTION__,__LINE__));
        }
        else if (MaxBandwidth == NOT_SUPPORT_ABOVE_BW_80)// BW 80
        {
            //
            // BW 40 or 20
            //
            if  (((ADHOC_ON(pPort)) || (INFRA_ON(pPort))) && 
                (pAd->MlmeAux.VhtOperation.VhtOpInfo.ChannelWidth == VHT_BW_20_40))
            {
                //
                // Update per-entry bandwidth control based on the HT IE
                //
                Bandwidth = pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth;

                WRITE_PHY_CFG_BW(pAd, &pPort->CommonCfg.MaxPhyCfg, Bandwidth);
            }
            else
        {
            WRITE_PHY_CFG_BW(pAd, &pPort->CommonCfg.MaxPhyCfg, BW_80);            
        }
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE,("ERROR!!, %s, LINE%d, unknow MaxBandwidth(=%d)!!\n",__FUNCTION__,__LINE__,MaxBandwidth));
        }
    }
    else if ( (pPort->CommonCfg.DesiredHtPhy.bHtEnable == TRUE) &&
        (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth) )

    {
        WRITE_PHY_CFG_BW(pAd, &pPort->CommonCfg.MaxPhyCfg, BW_40);
    }
    else
    {
        WRITE_PHY_CFG_BW(pAd, &pPort->CommonCfg.MaxPhyCfg, BW_20);
    }

    // -------------------------------------------------------
    // Decide Short GI support status in VHT mode/ HT mode
    // -------------------------------------------------------
    if (READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.MaxPhyCfg) == BW_20)
    {
        WRITE_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.MaxPhyCfg, (pPort->CommonCfg.DesiredHtPhy.ShortGIfor20 & pRtHtCap->ShortGIfor20));
    }
    else if (READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.MaxPhyCfg) == BW_40)
    {
        WRITE_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.MaxPhyCfg, (pPort->CommonCfg.DesiredHtPhy.ShortGIfor40 & pRtHtCap->ShortGIfor40));
    }
    else if (READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.MaxPhyCfg) == BW_80)
    {
        pRtVhtCap = &pPort->CommonCfg.DesiredVhtPhy;
        //WRITE_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.MaxPhyCfg, (pPort->CommonCfg.DesiredHtPhy.VhtShortGIfor80Mhz & pRtHtCap->VhtShortGIfor80Mhz));
        WRITE_PHY_CFG_SHORT_GI(
                pAd, 
                &pPort->CommonCfg.MaxPhyCfg, 
                (pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtCapInfo.ShortGIfor80Mhz & pRtVhtCap->VhtCapability.VhtCapInfo.ShortGIfor80Mhz));
    }

    if (pAd->StaActive.SupportedVhtPhy.bVhtEnable == TRUE)
    {
        // find the lower capablity of

        //   1. how fast we can send
        //   2. how fast the other side can reveive     
        USHORT MaxNss=NSS_0;  // default, Nss=0:1x1     
        USHORT McsSet=NOT_SUPPORTED;
        USHORT MaxMcs = MCS_0;
        // get the maximal SS supported by both side
        // test 2x2
        if ((pAd->MlmeAux.VhtOperation.VhtBasicMcsSet.MaxMcsFor2ss!=NOT_SUPPORTED)
            && (pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtBasicMcsSet.MaxMcsFor2ss!=NOT_SUPPORTED))
        {
            // both side support 2x2
            MaxNss = NSS_1;
            // decide the McsSet
            if ((pAd->MlmeAux.VhtOperation.VhtBasicMcsSet.MaxMcsFor2ss
                > pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtBasicMcsSet.MaxMcsFor2ss))
            {
                McsSet = pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtBasicMcsSet.MaxMcsFor2ss;
            }
            else
                McsSet = pAd->MlmeAux.VhtOperation.VhtBasicMcsSet.MaxMcsFor2ss;         
        }
        // test 1x1
        else if((pAd->MlmeAux.VhtOperation.VhtBasicMcsSet.MaxMcsFor1ss!=NOT_SUPPORTED)
            && (pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtBasicMcsSet.MaxMcsFor1ss!=NOT_SUPPORTED))
        {
            // both side support 1x1
            MaxNss = NSS_0;
            if ((pAd->MlmeAux.VhtOperation.VhtBasicMcsSet.MaxMcsFor1ss
                > pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtBasicMcsSet.MaxMcsFor1ss))
            {
                McsSet = pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtBasicMcsSet.MaxMcsFor1ss;
            }
            else
                McsSet = pAd->MlmeAux.VhtOperation.VhtBasicMcsSet.MaxMcsFor1ss;
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s, LINE_%d, ERROR!! Nss can not be coordinated on both side",__FUNCTION__,__LINE__));
        }
        
        switch(McsSet)
        {
            case MCS_0_7:
                MaxMcs=MCS_7;
                break;
                
            case MCS_0_8:
                MaxMcs=MCS_8;
                break;

            case MCS_0_9:
                MaxMcs=MCS_9;
                break;

            case NOT_SUPPORTED:
                MaxMcs=MCS_0;
                DBGPRINT(RT_DEBUG_TRACE,("%s, LINE_%d, ERROR!! Nss can not be coordinated on both side",__FUNCTION__,__LINE__));
                break;
                
            default:
                DBGPRINT(RT_DEBUG_ERROR,("ERROR!! %s, LINE_%d invalid McsSet(=%d)\n",__FUNCTION__,__LINE__,McsSet));
        }       
        WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MaxPhyCfg, MaxMcs);
        WRITE_PHY_CFG_NSS(pAd, &pPort->CommonCfg.MaxPhyCfg, MaxNss);
        DBGPRINT(RT_DEBUG_TRACE,("%s, LINE_%d, write CommonCfg.MaxPhyCfg.Mcs=%d, CommonCfg.MaxPhyCfg.Nss=%d",
                    __FUNCTION__,
                    __LINE__,
                    MaxMcs,
                    MaxNss));       
    }
    else
    {

    for (i = 0;i < 8;i++)       
    {
        if ((pRtHtCap->MCSSet[1]&(0x80>>i)) && (pPort->CommonCfg.DesiredHtPhy.MCSSet[1]&(0x80>>i)))
        {
            WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MaxPhyCfg, (15 - i));
            break;
        }
    }

    // For 3x3
    if (pRtHtCap->MCSSet[2] != 0)
    {
        for  (i = 0; i < 8; i++)        
        {   
            if ((pRtHtCap->MCSSet[2] & (0x80 >> i)) && (pPort->CommonCfg.DesiredHtPhy.MCSSet[2] & (0x80 >> i)))
            {
                WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MaxPhyCfg, (23 - i));
                break;
            }       
        }
    }
    
    if (i == 8)
    {
        for (i = 0;i < 8;i++)       
        {
            if ((pRtHtCap->MCSSet[0]&(0x80>>i))&& (pPort->CommonCfg.DesiredHtPhy.MCSSet[0]&(0x80>>i)))
            {
                WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MaxPhyCfg, (7 - i));
                break;
            }
        }
    }
    }

    // Copy MIN ht rate.  rt2860???
    WRITE_PHY_CFG_BW(pAd, &pPort->CommonCfg.MinPhyCfg, BW_20);
    WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MinPhyCfg, MCS_0);
    WRITE_PHY_CFG_STBC(pAd, &pPort->CommonCfg.MinPhyCfg, STBC_NONE);
    WRITE_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.MinPhyCfg, GI_800);
    
    //If STA assigns fixed rate. update to fixed here.
    if ( (pPort->PortType == EXTSTA_PORT ) && (pPort->CommonCfg.DesiredHtPhy.MCSSet[0] != 0xff))
    {
        if ((pPort->CommonCfg.DesiredHtPhy.MCSSet[4] != 0))
        {
            WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MaxPhyCfg, MCS_32);
            WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MinPhyCfg, MCS_32);
            
            DBGPRINT(RT_DEBUG_TRACE,("%s<=== Use Fixed MCS = %d\n",
                        __FUNCTION__,
                        READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MinPhyCfg)));
        }
        for (i = 23 ;i >= 0;i--)
        {   
            j = i/8;    
            bitmask = (1<<(i-(j*8)));
            if ( (pPort->CommonCfg.DesiredHtPhy.MCSSet[j]&bitmask) && (pRtHtCap->MCSSet[j]&bitmask))
            {
                WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MaxPhyCfg, i);
                WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MinPhyCfg, i);
                
                DBGPRINT(RT_DEBUG_TRACE,("%s<=== Use Fixed MCS = %d\n",__FUNCTION__, i));
                break;
            }
            if (i==0)
                break;
        }
    }
    
    // Decide ht rate
    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.TxPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.MaxPhyCfg));


//BJBJ watch it
    // ------------------------------------------------------------------------------------------------
    // Patch SW logic: Driver will not use right Phymode to transmit frame until it call MlmeSetTxRate.
    // ------------------------------------------------------------------------------------------------
    // Update right phymode from link up stage.
    if (pAd->StaActive.SupportedVhtPhy.bVhtEnable == FALSE)
    {
        WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg, MODE_HTMIX);
    }

    if ((Wcid < MAX_LEN_OF_MAC_TABLE) && (INFRA_ON(pPort)))
    {
        // 1. 802.11n rate, 2.Mpdudensity, 3.Amsdu size, 4. MimoPowerSave, 5. Max Rx Factor
        WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->MacTab.Content[Wcid].TxPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.TxPhyCfg));
        WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->MacTab.Content[Wcid].MaxPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.MaxPhyCfg));
        WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->MacTab.Content[Wcid].MinPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.MinPhyCfg));
    
        if(VHT_NIC(pAd))
        {
            //todo
        }
        else if ((READ_PHY_CFG_MCS(pAd, &pPort->MacTab.Content[Wcid].TxPhyCfg) > MCS_7) || 
             (READ_PHY_CFG_MCS(pAd, &pPort->MacTab.Content[Wcid].TxPhyCfg) == MCS_32))
        {
            WRITE_PHY_CFG_STBC(pAd, &pPort->MacTab.Content[Wcid].TxPhyCfg, STBC_NONE);
        }
        
        DBGPRINT(RT_DEBUG_TRACE,("TX: HTPhyMode.word = %x    \n", READ_PHY_CFG_DOUBLE_WORD(pAd, &pPort->MacTab.Content[Wcid].TxPhyCfg)));
    }
    
    // use default now. rt2860
    //RTUSBWriteMACRegister(pAd, HT_BASIC_RATE, BasicMCS);
    if (pPort->CommonCfg.DesiredHtPhy.MCSSet[0] != 0xff)
        pPort->CommonCfg.bAutoTxRateSwitch = FALSE;
    else
        pPort->CommonCfg.bAutoTxRateSwitch = TRUE;

    DBGPRINT(RT_DEBUG_INFO,("%s<=== \n", __FUNCTION__));
#endif  
}

VOID AsicUpdateAutoFallBackTable(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pRateTable,
    IN  MAC_TABLE_ENTRY *pEntry)
{
#if 0
    BOOLEAN bUseAGS = FALSE;
    UCHAR                   i;
    VHT_HT_FBK_CFG0_STRUC VhtHtCfg0;
    VHT_HT_FBK_CFG1_STRUC VhtHtCfg1;
    
    LG_FBK_CFG0_STRUC       LgCfg0;
    VHT_LG_FBK_CFG1_STRUC VhtLgCfg1 = {0};
    PRTMP_TX_RATE_SWITCH    pCurrTxRate, pNextTxRate;
    PRTMP_TX_RATE_SWITCH_AGS pCurrTxRate_AGS = NULL, pNextTxRate_AGS = NULL;
    HT_FBK_3SS_CFG0_STRUC Ht3SSCfg0 = {0};
    HT_FBK_3SS_CFG1_STRUC Ht3SSCfg1 = {0};
    UCHAR CurMcsOfCurrTxRate = MCS_0, CurMcsOfNextTxRate = MCS_0;

    if (SUPPORT_AGS(pAd) && IS_AGS_RATE_TABLE(pRateTable))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: Use AGS\n", __FUNCTION__));
        
        bUseAGS = TRUE;
    }
    
    // set to initial value
    VhtHtCfg0.word = 0x65432100;
    VhtHtCfg1.word = 0xedcba988;
    LgCfg0.word = 0xedcba988;
    VhtLgCfg1.word = 0x00002100;
    Ht3SSCfg0.word = 0x1211100F;
    Ht3SSCfg1.word = 0x16151413;

    if (bUseAGS)
    {
        pNextTxRate_AGS = (PRTMP_TX_RATE_SWITCH_AGS)pRateTable+1;
        pNextTxRate = (PRTMP_TX_RATE_SWITCH)pNextTxRate_AGS;
    }
    else
    {
        pNextTxRate = (PRTMP_TX_RATE_SWITCH)pRateTable+1;
    }
    
    for (i = 1; i < *((PUCHAR) pRateTable); i++)
    {
        if (bUseAGS)
        {
            pCurrTxRate_AGS = (PRTMP_TX_RATE_SWITCH_AGS)pRateTable+1+i;
            pCurrTxRate = (PRTMP_TX_RATE_SWITCH)pCurrTxRate_AGS;
        }
        else
        {
            pCurrTxRate = (PRTMP_TX_RATE_SWITCH)pRateTable+1+i;
        }
        
        switch (pCurrTxRate->Mode)
        {
            case MODE_CCK:      //CCK
                pNextTxRate = pCurrTxRate;
                break;
            case MODE_OFDM:     //OFDM
                {
                    switch(pCurrTxRate->CurrMCS)
                    {
                        case MCS_0:
                            LgCfg0.field.OFDMMCS0FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
                            break;
                        case MCS_1:
                            LgCfg0.field.OFDMMCS1FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
                            break;
                        case MCS_2:
                            LgCfg0.field.OFDMMCS2FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
                            break;
                        case MCS_3:
                            LgCfg0.field.OFDMMCS3FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
                            break;
                        case MCS_4:
                            LgCfg0.field.OFDMMCS4FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
                            break;
                        case MCS_5:
                            LgCfg0.field.OFDMMCS5FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
                            break;
                        case MCS_6:
                            LgCfg0.field.OFDMMCS6FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
                            break;
                        case MCS_7:
                            LgCfg0.field.OFDMMCS7FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
                            break;
                    }
                    // We only do the specify auto rate fall back when only one client
                    if (pAd->OpMode == OPMODE_AP)
                    {
                        if ( (pEntry->SupOFDMMCS[pCurrTxRate->CurrMCS] == TRUE) && (pPort->MacTab.Size == 1))
                            pNextTxRate = pCurrTxRate;
                    }
                    else
                        pNextTxRate = pCurrTxRate;
                }
                break;
            case MODE_HTMIX:        //HT-MIX
            case MODE_HTGREENFIELD:     //HT-GF
                {
                    if ((pNextTxRate->Mode >= MODE_HTMIX) && (pCurrTxRate->CurrMCS != pNextTxRate->CurrMCS))
                    {
                        switch(pCurrTxRate->CurrMCS)
                        {
                            case MCS_0:
                                VhtHtCfg0.field.HTMCS0FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_1:
                                VhtHtCfg0.field.HTMCS1FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_2:
                                VhtHtCfg0.field.HTMCS2FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_3:
                                VhtHtCfg0.field.HTMCS3FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_4:
                                VhtHtCfg0.field.HTMCS4FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_5:
                                VhtHtCfg0.field.HTMCS5FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_6:
                                VhtHtCfg0.field.HTMCS6FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_7:
                                VhtHtCfg0.field.HTMCS7FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_8:
                                VhtHtCfg1.field.HTMCS8FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_9:
                                VhtHtCfg1.field.HTMCS9FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_10:
                                VhtHtCfg1.field.HTMCS10FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_11:
                                VhtHtCfg1.field.HTMCS11FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_12:
                                VhtHtCfg1.field.HTMCS12FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_13:
                                VhtHtCfg1.field.HTMCS13FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_14:
                                VhtHtCfg1.field.HTMCS14FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_15:
                                VhtHtCfg1.field.HTMCS15FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_16:
                                Ht3SSCfg0.field.HTMCS16FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_17:
                                Ht3SSCfg0.field.HTMCS17FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_18:
                                Ht3SSCfg0.field.HTMCS18FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_19:
                                Ht3SSCfg0.field.HTMCS19FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_20:
                                Ht3SSCfg1.field.HTMCS20FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_21:
                                Ht3SSCfg1.field.HTMCS21FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_22:
                                Ht3SSCfg1.field.HTMCS22FBK = pNextTxRate->CurrMCS;
                                break;
                            case MCS_23:
                                Ht3SSCfg1.field.HTMCS23FBK = pNextTxRate->CurrMCS;
                                break;
                            default:
                                DBGPRINT(RT_DEBUG_ERROR, ("AsicUpdateAutoFallBackTable: not support"));
                        }
                    }
                }
                pNextTxRate = pCurrTxRate;
                break;

                case MODE_VHT: 
                {
                    //
                    // Convert VHT to HT
                    //  MCS7~MCS0 (Nss0) ==> MCS7~MCS0
                    //  MCS7~MCS0 (Nss1) ==> MCS15~MCS8
                    //
                    CurMcsOfCurrTxRate = ((pCurrTxRate->Nss * 8) + pCurrTxRate->CurrMCS);
                    CurMcsOfNextTxRate = ((pNextTxRate->Nss * 8) + pNextTxRate->CurrMCS);
                    
                    if (CurMcsOfCurrTxRate != CurMcsOfNextTxRate)
                    {
                        switch(CurMcsOfCurrTxRate)
                        {
                            case MCS_0: 
                                VhtHtCfg0.field.HTMCS0FBK = CurMcsOfNextTxRate;
                                break;
                            case MCS_1: 
                                VhtHtCfg0.field.HTMCS1FBK = CurMcsOfNextTxRate;
                                break;
                            case MCS_2: 
                                VhtHtCfg0.field.HTMCS2FBK = CurMcsOfNextTxRate;
                                break;
                            case MCS_3: 
                                VhtHtCfg0.field.HTMCS3FBK = CurMcsOfNextTxRate;
                                break;
                            case MCS_4: 
                                VhtHtCfg0.field.HTMCS4FBK = CurMcsOfNextTxRate;
                                break;
                            case MCS_5: 
                                VhtHtCfg0.field.HTMCS5FBK = CurMcsOfNextTxRate;
                                break;
                            case MCS_6: 
                                VhtHtCfg0.field.HTMCS6FBK = CurMcsOfNextTxRate;
                                break;
                            case MCS_7: 
                                VhtHtCfg0.field.HTMCS7FBK = CurMcsOfNextTxRate;
                                break;
                            case MCS_8: 
                                VhtHtCfg1.field.HTMCS8FBK = CurMcsOfNextTxRate;
                                break; 
                            case MCS_9: 
                                VhtHtCfg1.field.HTMCS9FBK = CurMcsOfNextTxRate;
                                break;
                            case MCS_10: 
                                VhtHtCfg1.field.HTMCS10FBK = CurMcsOfNextTxRate;
                                break;
                            case MCS_11: 
                                VhtHtCfg1.field.HTMCS11FBK = CurMcsOfNextTxRate;
                                break;
                            case MCS_12: 
                                VhtHtCfg1.field.HTMCS12FBK = CurMcsOfNextTxRate;
                                break;
                            case MCS_13: 
                                VhtHtCfg1.field.HTMCS13FBK = CurMcsOfNextTxRate;
                                break;
                            case MCS_14: 
                                VhtHtCfg1.field.HTMCS14FBK = CurMcsOfNextTxRate;
                                break;
                            case MCS_15: 
                                VhtHtCfg1.field.HTMCS15FBK = CurMcsOfNextTxRate;
                                break;
                            default:
                                DBGPRINT(RT_DEBUG_ERROR, ("AsicUpdateAutoFallBackTable: not support"));
                        }
                    }
                }
                break;
        }

        //pNextTxRate = pCurrTxRate;
    }

    //
    // Support VHT MCS 0~9 (1SS)
    //
    if (VHT_CAPABLE(pAd) && 
        (pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor1ss == MCS_0_9))
    {
        VhtLgCfg1.field.VHT_1SS_MCS9_FBK = MCS_8;
        VhtLgCfg1.field.VHT_1SS_MCS8_FBK = MCS_7;
    }

    RTUSBWriteMACRegister(pAd, VHT_HT_FBK_CFG0, VhtHtCfg0.word);
    RTUSBWriteMACRegister(pAd, VHT_HT_FBK_CFG1, VhtHtCfg1.word);
    RTUSBWriteMACRegister(pAd, LG_FBK_CFG0, LgCfg0.word);
    RTUSBWriteMACRegister(pAd, VHT_LG_FBK_CFG1, VhtLgCfg1.word);

    if((bUseAGS == TRUE))
    {
        RTUSBWriteMACRegister(pAd, HT_FBK_3SS_CFG0, Ht3SSCfg0.word);
        RTUSBWriteMACRegister(pAd, HT_FBK_3SS_CFG1, Ht3SSCfg1.word);
        DBGPRINT(RT_DEBUG_TRACE, ("AsicUpdateAutoFallBackTable: Ht3SSCfg0=0x%x, Ht3SSCfg1=0x%x\n", Ht3SSCfg0.word, Ht3SSCfg1.word));
    }

    DBGPRINT(RT_DEBUG_TRACE, ("AsicUpdateAutoFallBackTable: HtCfg0=0x%x, HtCfg1=0x%x, LgCfg0=0x%x, VhtLgCfg1=0x%x \n", 
                            VhtHtCfg0.word, 
                            VhtHtCfg1.word, 
                            LgCfg0.word, 
                            VhtLgCfg1.word));
#endif  
}

// IRQL = PASSIVE_LEVEL
VOID MlmeRadioOff(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort
    )
{
    ULONG               i;
    MLME_DISASSOC_REQ_STRUCT DisReq;
    PMLME_QUEUE_ELEM          pMsgElem;
//  ULONG               WPSLedMode10;
    BSS_ENTRY           BssEntry0 = {0};
    ULONG               Idx = BSS_NOT_FOUND;
    
    DBGPRINT(RT_DEBUG_TRACE,("MlmeRadioOff()\n"));     

    // Link down first if any association exists
    // Shutdown P2P within P2pDown, we just check if Port0 is connecting. 
    if ((P2P_ON(pPort)) && (pPort->PortNumber > 0) && (pPort->PortNumber == pPort->P2PCfg.PortNumber))
        pPort = pAd->PortList[PORT_0];

    if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
    {
        if (INFRA_ON(pPort) || ADHOC_ON(pPort))
        {
            PlatformAllocateMemory(pAd,  &pMsgElem, sizeof(MLME_QUEUE_ELEM));
            if(pMsgElem != NULL)
            {
                COPY_MAC_ADDR(&DisReq.Addr, pPort->PortCfg.Bssid);
                DisReq.Reason =  REASON_DISASSOC_STA_LEAVING;

                pMsgElem->Machine = ASSOC_STATE_MACHINE;
                pMsgElem->MsgType = MT2_MLME_DISASSOC_REQ;
                pMsgElem->MsgLen = sizeof(MLME_DISASSOC_REQ_STRUCT);
                pMsgElem->PortNum = pPort->PortNumber;
                PlatformMoveMemory(pMsgElem->Msg, &DisReq, sizeof(MLME_DISASSOC_REQ_STRUCT));
                
                MlmeDisassocReqAction(pAd, pMsgElem);               
                NdisCommonGenericDelay(1000);
                PlatformFreeMemory(pMsgElem, sizeof(MLME_QUEUE_ELEM));
            }
            else
            {
                DBGPRINT(RT_DEBUG_ERROR, ("%s:Allocate memory failed for MT2_MLME_DISASSOC_REQ\n", __FUNCTION__));
            }
        }
    }

    // Set Radio off flag
    MT_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);

    if (P2P_ON(pPort))
    {
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    {
        UCHAR   PortNum;
        PMP_PORT  pP2PPort = NULL;
        BOOLEAN     IsNeedDelay = FALSE;
        
        for(PortNum = 0; PortNum < RTMP_MAX_NUMBER_OF_PORT; PortNum++)
        {
            pP2PPort = (PMP_PORT) pAd->PortList[PortNum];
        
            if ((pP2PPort != NULL) && (pP2PPort->bActive == TRUE))
            {
                // Reduce register writing overhead, ignore wfd-device reset settings before RadioOff
                if (((pP2PPort->PortType == WFD_DEVICE_PORT) && (pAd->LogoTestCfg.OnTestingWHQL == FALSE)) || 
                    (pP2PPort->PortType == WFD_GO_PORT) || 
                    (pP2PPort->PortType == WFD_CLIENT_PORT))
                {
                    P2pMsDown(pAd, pAd->PortList[pP2PPort->PortNumber]);
                    IsNeedDelay = TRUE;
                }
            }
        }
    }
#endif  
        if ((pAd->PortList[pPort->P2PCfg.PortNumber]) 
            && (pAd->PortList[pPort->P2PCfg.PortNumber]->bActive) 
            && (pAd->PortList[pPort->P2PCfg.PortNumber]->PortType == EXTSTA_PORT)
            && ((pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PClient) 
            || (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO)))
        {
        P2pDown(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
            pPort->P2PCfg.P2PConnectState = P2P_CONNECT_NOUSE;
        }
        // disable p2p
        pPort->P2PCfg.P2PDiscoProvState = P2P_DISABLE;
    }

    if(pPort)
    {
        // Link down first if any association exists
        if (INFRA_ON(pPort) || ADHOC_ON(pPort))
            MlmeCntLinkDown(pPort, FALSE);
    }

    NdisCommonGenericDelay(10000);

    // Set LED
    //LedCtrlSetLed(pAd, LED_RADIO_OFF);

        // workaround: patch bulk-in failed at sending sleep mcu cmd
    if (pAd->LogoTestCfg.OnTestingWHQL)
            NdisCommonGenericDelay(20000);    // 20 ms

    //==========================================
    // Clean up old bss table
    BssTableInit(&pAd->ScanTab);
    
        //reset Reposit Bss Table
    if(pAd->pNicCfg->RepositBssTable.pBssEntry != NULL)
    {
        for (i = 0; i < MAX_LEN_OF_BSS_TABLE; i++) 
        {
            PlatformZeroMemory(&pAd->pNicCfg->RepositBssTable.pBssEntry[i], sizeof(BSS_ENTRY));
            pAd->pNicCfg->RepositBssTable.ulTime[i].LowPart = 0;
            pAd->pNicCfg->RepositBssTable.ulTime[i].HighPart = 0;
        }
    }

    // sync UI display, if doing RadioOff at APCLIENT mode,
    // it's forced to transfer to STA mode.
    if(pAd->OpMode == OPMODE_APCLIENT)
        pAd->OpMode = OPMODE_STA;

    //if (1)
    if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
    {
        U3DMA_WLCFG         U3DMAWLCFG;
        int i = 0;
        
        do
        {
            HW_IO_READ32(pAd, UDMA_WLCFG_0, &U3DMAWLCFG.word);
            DBGPRINT(RT_DEBUG_TRACE,("%d Wait Tx/Rx to be idle, Rx = %d, Tx = %d\n", i, U3DMAWLCFG.field.WL_RX_BUSY, U3DMAWLCFG.field.WL_TX_BUSY));

            i++;
            if (i == 1000)
                break;
        } while ((U3DMAWLCFG.field.WL_TX_BUSY == 1));
    }

    // Disable Rx
    SendDisableRxCMD(pAd);

    // Polling Rx Empty
    //if (1)
    if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
    {
        U3DMA_WLCFG         U3DMAWLCFG;
        int i = 0;
        
        do
        {
            HW_IO_READ32(pAd, UDMA_WLCFG_0, &U3DMAWLCFG.word);
            DBGPRINT(RT_DEBUG_TRACE,("%d Wait Tx/Rx to be idle, Rx = %d, Tx = %d\n", i, U3DMAWLCFG.field.WL_RX_BUSY, U3DMAWLCFG.field.WL_TX_BUSY));

            i++;
            if (i == 1000)
                break;
        } while ((U3DMAWLCFG.field.WL_RX_BUSY == 1)/* || (U3DMAWLCFG.field.WL_TX_BUSY == 1)*/);
    }

    // Radio off
    SendRadioOnOffCMD(pAd, PWR_RadioOff);

        NdisCommonStopTx(pAd);
        NdisCommonStopRx(pAd);
    
#if 0   
    if(!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("AsicRadioOff....3\n"));
        AsicRadioOff(pAd);
    }
#endif

    DBGPRINT(RT_DEBUG_TRACE,("<== %s\n",__FUNCTION__));
}

// IRQL = PASSIVE_LEVEL
VOID MlmeRadioOn(
    IN PMP_ADAPTER pAd)
{       
    
    ULONG MACValue = 0;
    PMP_PORT pPort =pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

    if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
    {
        DBGPRINT(RT_DEBUG_TRACE,("Radio already On\n"));
        return;
    }

    DBGPRINT(RT_DEBUG_TRACE,("MlmeRadioOn()\n"));

    NdisCommonStartRx(pAd);    
    
    
    NdisCommonStartTx(pAd);

    SendRadioOnOffCMD(pAd, PWR_RadioOn);

    NdisCommonGenericDelay(10000);

    // Clear Radio off flag before NICResetFromError-->InitAsicFromEEPROM, 
    // IndicatePhyChange will be called and immediatelly ScanRequest
    // If the fRTMP_ADAPTER_RADIO_OFF is turned on, the scan will be ignored
    MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);

    NICResetFromError(pAd);

#if 0
    // Enable Tx/Rx
    //RTMPEnableRxTx(pAd);
    
    //AsicRFOn(pAd);
        
    N6UsbRecvStart(pAd);    
    // Generate BulkIn IRP and synchronize NextRxBulkInReadIndex and NextRxBulkInIndex
    for( BulkInIndex = 0; BulkInIndex < pPort->CommonCfg.NumOfBulkInIRP; BulkInIndex++ )
    {
        N6USBBulkReceive(pAd);
//      if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
//          ApDataN6RxPacket(pAd,FALSE);
//      else
//          N6UsbRxStaPacket(pAd, FALSE);
        DBGPRINT_RAW(RT_DEBUG_ERROR, ("NextRxBulkInReadIndex = %d, NextRxBulkInIndex = %d, PendingRx = %d \n",  pAd->pHifCfg->NextRxBulkInReadIndex,pAd->pHifCfg->NextRxBulkInIndex,pAd->pHifCfg->PendingRx ));
    }       

    N6USBBulkReceiveCMD(pAd);
    
    N6UsbXmitStart(pAd);
#endif

    // Set LED
    //LedCtrlSetLed(pAd, LED_RADIO_ON);

    //
    // Clear counter for MpScanRequest
    //
    pAd->pHifCfg->BulkLastOneSecCount = 0;   
}


// ===========================================================================================
// bss_table.c
// ===========================================================================================


/*! \brief initialize BSS table
 *  \param p_tab pointer to the table
 *  \return none
 *  \pre
 *  \post

 IRQL = PASSIVE_LEVEL
 IRQL = DISPATCH_LEVEL
  
 */
VOID BssTableInit(
    IN BSS_TABLE *Tab) 
{
    int i;

    Tab->BssNr = 0;
    Tab->BssOverlapNr = 0;
    for (i = 0; i < MAX_LEN_OF_BSS_TABLE; i++) 
    {
        PlatformZeroMemory(&Tab->BssEntry[i], sizeof(BSS_ENTRY));
    }
}

VOID ResetRepositBssTable(
    IN PMP_ADAPTER pAd)
{
    int i;
    if (pAd->pNicCfg->RepositBssTable.pBssEntry != NULL)
    {
        for (i = 0; i < MAX_LEN_OF_BSS_TABLE; i++) 
        {
            PlatformZeroMemory(&pAd->pNicCfg->RepositBssTable.pBssEntry[i], sizeof(BSS_ENTRY));
            pAd->pNicCfg->RepositBssTable.ulTime[i].LowPart = 0;
            pAd->pNicCfg->RepositBssTable.ulTime[i].HighPart = 0;
        }
    }
}

VOID BATableInit(
    IN BA_TABLE *Tab) 
{
    int i;

    Tab->numAsOriginator = 0;
    Tab->numAsRecipient = 0;
    for (i = 0; i < MAX_LEN_OF_BA_REC_TABLE; i++) 
    {
        Tab->BARecEntry[i].REC_BA_Status = Recipient_NONE;
    }
    for (i = 0; i < MAX_LEN_OF_BA_ORI_TABLE; i++) 
    {
        Tab->BAOriEntry[i].ORI_BA_Status = Originator_NONE;
    }
}
/*! \brief search the BSS table by SSID
 *  \param p_tab pointer to the bss table
 *  \param ssid SSID string 
 *  \return index of the table, BSS_NOT_FOUND if not in the table
 *  \pre
 *  \post
 *  \note search by sequential search

 IRQL = DISPATCH_LEVEL
 
 */
ULONG BssTableSearch(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT       pPort,
    IN BSS_TABLE *Tab, 
    IN PUCHAR    pBssid,
    IN UCHAR     Channel) 
{
    UCHAR i;
    ULONG scanningTableIndex = 0;

    // Patch: The Siemens WPS AP, Gigaset SE366 WLAN, would change its SSID after
    //            its WPS PBC funcationality is enabled.
    if ((pPort->StaCfg.WscControl.WscMode == WSC_PBC_MODE) &&
        (pPort->StaCfg.WscControl.WscState == WSC_STATE_START) && 
        (pPort->StaCfg.WscControl.WscStatus == STATUS_WSC_START_ASSOC))
    {
        WscPBCModeAPSearch(pAd, pBssid, Channel, &pPort->StaCfg.WscControl.WscBssTab, &scanningTableIndex);

        if ((pPort->StaCfg.WscControl.WscBssTab.BssNr == 1) &&
             (scanningTableIndex != BSS_NOT_FOUND))
        {
            if ((((Tab->BssEntry[scanningTableIndex].Channel <= 14) && (Channel <= 14)) ||
                 ((Tab->BssEntry[scanningTableIndex].Channel > 14) && (Channel > 14))) &&
                MAC_ADDR_EQUAL(Tab->BssEntry[scanningTableIndex].Bssid, pBssid)) 
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s: scanningTableIndex = %d\n", __FUNCTION__, scanningTableIndex));
                return scanningTableIndex;
            }
        }
        else
        {
            for (i = 0; i < Tab->BssNr; i++) 
            {
                //
                // Some AP that support A/B/G mode that may used the same BSSID on 11A and 11B/G.
                // We should distinguish this case.
                //      
                if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
                     ((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
                    MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid)) 
                { 
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: i = %d. Channel = %d. \n", __FUNCTION__, i, Tab->BssEntry[i].Channel));
                    return i;
                }
            }

            return (ULONG)BSS_NOT_FOUND;
        }
    }
    // Patch: The Siemens WPS AP, Gigaset SE366 WLAN, would change its SSID after
    //            its WPS PIN funcationality is enabled.
    // STAP2P mode, I must be GO. So doesn't need to find WPS AP. WscState is for Registra function.
    else if (((pPort->StaCfg.WscControl.WscMode == WSC_PIN_MODE_REGISTRA_SPEC)||(pPort->StaCfg.WscControl.WscMode == WSC_PIN_MODE) ||(pPort->StaCfg.WscControl.WscMode == WSC_PIN_MODE_USER_SPEC)) &&
        (pPort->StaCfg.WscControl.WscState == WSC_STATE_START) && 
        (pPort->StaCfg.WscControl.WscStatus == STATUS_WSC_START_ASSOC))
    {
        WscPINModeAPSearch(pAd, pPort, pBssid, Channel, &pPort->StaCfg.WscControl.WscBssTab, &scanningTableIndex);

        // If the PIN method (Device Password ID) is in Beacon/Probe Response,
        // the station connects with this WPS AP.
        if ((pPort->StaCfg.WscControl.WscBssTab.BssNr == 1) &&
             (scanningTableIndex != BSS_NOT_FOUND))
        {
            if ((((Tab->BssEntry[scanningTableIndex].Channel <= 14) && (Channel <= 14)) ||
                 ((Tab->BssEntry[scanningTableIndex].Channel > 14) && (Channel > 14))) &&
                MAC_ADDR_EQUAL(Tab->BssEntry[scanningTableIndex].Bssid, pBssid)) 
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s: scanningTableIndex = %d\n", __FUNCTION__, scanningTableIndex));
                return scanningTableIndex;
            }
        }
        else
        {
            // If the PIN method (Device Password ID) is NOT in Beacon/Probe Response, 
            // the station connects with the WPS AP with the desired BSSID and valid channel only.
            for (i = 0; i < Tab->BssNr; i++) 
            {
                //
                // Some AP that support A/B/G mode that may used the same BSSID on 11A and 11B/G.
                // We should distinguish this case.
                //      
                if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
                     ((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
                    MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid)) 
                { 
                    return i;
                }
            }
            return (ULONG)BSS_NOT_FOUND;
        }
    }
    // Normal BSS table search.
    else
    {
        // If the PIN method (Device Password ID) is NOT in Beacon/Probe Response, 
        // the station connects with the WPS AP with the desired BSSID and valid channel only.
        for (i = 0; i < Tab->BssNr; i++) 
        {
            //
            // Some AP that support A/B/G mode that may used the same BSSID on 11A and 11B/G.
            // We should distinguish this case.
            //      
            if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
                 ((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
                MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid)) 
            { 
                return i;
            }
        }
        return (ULONG)BSS_NOT_FOUND;
    }
    return (ULONG)BSS_NOT_FOUND;
}

ULONG BssSsidTableSearch(
    IN BSS_TABLE *Tab, 
    IN PUCHAR    pBssid,
    IN PUCHAR    pSsid,
    IN UCHAR     SsidLen,
    IN UCHAR     Channel) 
{
    UCHAR i;

    for (i = 0; i < Tab->BssNr; i++) 
    {
        //
        // Some AP that support A/B/G mode that may used the same BSSID on 11A and 11B/G.
        // We should distinguish this case.
        //
        if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
             ((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
            MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid) &&
            SSID_EQUAL(pSsid, SsidLen, Tab->BssEntry[i].Ssid, Tab->BssEntry[i].SsidLen)) 
        { 
            return i;
        }
    }
    return (ULONG)BSS_NOT_FOUND;
}

ULONG BssTableSearchWithSSID(
    IN BSS_TABLE *Tab, 
    IN PUCHAR    Bssid,
    IN PUCHAR    pSsid,
    IN UCHAR     SsidLen,
    IN UCHAR     Channel)
{
    UCHAR i;

    for (i = 0; i < Tab->BssNr; i++) 
    {
        if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
            ((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
            MAC_ADDR_EQUAL(&(Tab->BssEntry[i].Bssid), Bssid) &&
            (SSID_EQUAL(pSsid, SsidLen, Tab->BssEntry[i].Ssid, Tab->BssEntry[i].SsidLen) ||
            (PlatformEqualMemory(pSsid, ZeroSsid, SsidLen)) || 
                        (PlatformEqualMemory(pSsid, WILDP2PSSID, WILDP2PSSIDLEN)) ||
            (PlatformEqualMemory(Tab->BssEntry[i].Ssid, ZeroSsid, Tab->BssEntry[i].SsidLen))))
        { 
            return i;
        }
    }
    return (ULONG)BSS_NOT_FOUND;
}

//
// Remove all BSS entries with the specified channel.
//
VOID BssTableDeleteEntryByChannel(
    IN PMP_ADAPTER pAd, 
    IN OUT PBSS_TABLE pBSSTable, 
    IN UCHAR Channel)
{
    UCHAR i = 0;
    PBSS_ENTRY pBSSEntry = NULL;
    BOOLEAN bSkipAssociatedBSSEntry = FALSE;
    PMP_PORT pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

    // Delete all BSS entries with the desired channel
    while(i < pBSSTable->BssNr)
    {
        pBSSEntry = &pBSSTable->BssEntry[i];

        // When the STA associates with an AP/peer STA, it should not remove the associated BSS entry.
        // This is because the NDIS assumes the associated BSS entry is in the channel scanning table.
        // In addition, some Ralink functions depend on the existence of the associated BSS entry in the channel scanning table.
        if (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED))
        {
            if ((((pBSSEntry->Channel <= 14) && (Channel <= 14)) || 
                   ((pBSSEntry->Channel > 14) && (Channel > 14))) && 
                   MAC_ADDR_EQUAL(pBSSEntry->Bssid, pPort->PortCfg.Bssid) && 
                   SSID_EQUAL(pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen, pBSSEntry->Ssid, pBSSEntry->SsidLen))
            {
                DBGPRINT(RT_DEBUG_INFO, ("%s: Skip the associated BSS entry, BSSID = %02X:%02X:%02X:%02X:%02X:%02X, SSID = %c%c%c..., Channel = %d\n", 
                    __FUNCTION__, 
                    pBSSEntry->Bssid[0], pBSSEntry->Bssid[1], pBSSEntry->Bssid[2], 
                    pBSSEntry->Bssid[3], pBSSEntry->Bssid[4], pBSSEntry->Bssid[5], 
                    pBSSEntry->Ssid[0], pBSSEntry->Ssid[1], pBSSEntry->Ssid[2], 
                    pBSSEntry->Channel));

                bSkipAssociatedBSSEntry = TRUE;
            }
            else
            {
                bSkipAssociatedBSSEntry = FALSE;
            }

        }
        else
        {
            bSkipAssociatedBSSEntry = FALSE;
        }

        if (bSkipAssociatedBSSEntry == TRUE)
        {
            i++; // Skip the associated BSS entry
        }
        else
        {   
            if(pBSSEntry->Channel == Channel)
            {           
                DBGPRINT(RT_DEBUG_INFO, ("%s: Remove BSS entry, BSSID = %02X:%02X:%02X:%02X:%02X:%02X, SSID = %c%c%c..., Channel = %d\n", 
                    __FUNCTION__, 
                    pBSSEntry->Bssid[0], pBSSEntry->Bssid[1], pBSSEntry->Bssid[2], 
                    pBSSEntry->Bssid[3], pBSSEntry->Bssid[4], pBSSEntry->Bssid[5], 
                    pBSSEntry->Ssid[0], pBSSEntry->Ssid[1], pBSSEntry->Ssid[2], 
                    pBSSEntry->Channel));
        
                // do not add index ,because other BSS entries will be moved to front entry.
                BssTableDeleteEntry(pBSSTable, pBSSEntry->Bssid, pBSSEntry->Channel);
            }
            else
            {
                i++;
            }
        }
    }
}

// IRQL = DISPATCH_LEVEL
VOID BssTableDeleteEntry(
    IN OUT  BSS_TABLE *Tab, 
    IN      PUCHAR    pBssid,
    IN      UCHAR     Channel)
{
    UCHAR i, j;

    for (i = 0; i < Tab->BssNr; i++) 
    {
        //printf("comparing %s and %s\n", p_tab->bss[i].ssid, ssid);
        if ((Tab->BssEntry[i].Channel == Channel) && 
            (MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid)))
        {
            for (j = i; j < Tab->BssNr - 1; j++)
            {
                PlatformMoveMemory(&(Tab->BssEntry[j]), &(Tab->BssEntry[j + 1]), sizeof(BSS_ENTRY));
            }

            // To prevent race condiction with scan request
            if((Tab->BssNr > 0) && (Tab->BssNr <= MAX_LEN_OF_BSS_TABLE))
                Tab->BssNr -= 1;

            //After moving entry one by one, make the last entry zero. If not, when an AP with
            //hidden SSID is found and added to the last entry, the SSID field will not be
            //updated and the value will be error.
            PlatformZeroMemory(&(Tab->BssEntry[Tab->BssNr]), sizeof(BSS_ENTRY));
            return;
        }
    }
}

/*
    ========================================================================
    Routine Description:
        Delete the Originator Entry in BAtable. Or decrease numAs Originator by 1 if needed.
        
    Arguments:
    // IRQL = DISPATCH_LEVEL
    ========================================================================
*/
VOID BATableDeleteORIEntry(
    IN OUT  PMP_ADAPTER pAd, 
    IN  PMP_PORT      pPort, 
    IN      BA_ORI_ENTRY    *pBAORIEntry)
{
    PMAC_TABLE_ENTRY pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, pBAORIEntry->Wcid);  
    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, pBAORIEntry->Wcid));
        return;
    }

    
    NdisAcquireSpinLock(&pPort->BATabLock);
    if (pBAORIEntry->ORI_BA_Status != Originator_NONE)
    {
        if (pBAORIEntry->ORI_BA_Status == Originator_Done)
        {
            pPort->BATable.numAsOriginator -= 1;
            DBGPRINT(RT_DEBUG_TRACE, ("BATableDeleteORIEntry numAsOriginator= %d\n", pPort->BATable.numAsRecipient));
            // Erase Bitmap flag.
        }
        // Not clear Sequence here.
    }
    
    pWcidMacTabEntry->TXBAbitmap &= (~(1<<(pBAORIEntry->TID) ));   // If STA mode,  erase flag here
    pWcidMacTabEntry->TXAutoBAbitmap &= (~(1<<(pBAORIEntry->TID) ));   // If STA mode,  erase flag here
    pWcidMacTabEntry->BAOriWcidArray[pBAORIEntry->TID] = 0;    // If STA mode,  erase flag here
    pBAORIEntry->ORI_BA_Status = Originator_NONE;
    pBAORIEntry->Token = 1;
    NdisReleaseSpinLock(&pPort->BATabLock);
}


// IRQL = DISPATCH_LEVEL
VOID BATableDeleteRECEntry(
    IN OUT  PMP_ADAPTER pAd, 
    IN  PMP_PORT      pPort, 
    IN      BA_REC_ENTRY    *pBARECEntry)
{
    UCHAR k;
    PRTMP_REORDERBUF    pBuf;
    UCHAR   Curindidx;
    USHORT  offset; 
    ULONG   MCAValue;
    UCHAR   UserPriority;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, pBARECEntry->Wcid);  
    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, pBARECEntry->Wcid));
        return;
    }
    
    if (pBARECEntry->REC_BA_Status == Recipient_Accept)
    {
        UserPriority = (pBARECEntry->TID & 0x07);
        
        NdisAcquireSpinLock(&pPort->BATabLock);
        Curindidx = pBARECEntry->Curindidx;
        NdisAcquireSpinLock(&pBARECEntry->RxReRingLock);
        pBARECEntry->REC_BA_Status = Recipient_NONE;
        // 1. indicaterx all frames in reordering buffer
        for (k = 0; k < pBARECEntry->BAWinSize; k++)
        {
            pBuf = &pAd->pRxCfg->LocalRxReorderBuf[pBARECEntry->RxBufIdxUsed].MAP_RXBuf[Curindidx];
            if (pBuf->IsFull)
            {
                pBuf->IsFull = FALSE;
                NdisCommonReportEthFrameToLLC(pAd,pPort,pBuf->pBuffer, pBuf->Datasize, UserPriority, FALSE, TRUE);
            }
            INC_RING_INDEX(Curindidx, Max_RX_REORDERBUF);
        }
        NdisReleaseSpinLock(&pBARECEntry->RxReRingLock);
        pAd->pRxCfg->LocalRxReorderBuf[pBARECEntry->RxBufIdxUsed].InUse = FALSE;
        pBARECEntry->RxBufIdxUsed = 0;
        //reset this BARECEn try content
        // Erase Bitmap flag.
        pBARECEntry->LastIndSeq = 0xffff;
        pBARECEntry->LastIndSeqAtTimer= 0xffff;
        pBARECEntry->BAWinSize = 0;
        // Erase Bitmap flag at software mactable
        pWcidMacTabEntry->RXBAbitmap &= (~(1<<(pBARECEntry->TID)));
        pWcidMacTabEntry->BARecWcidArray[pBARECEntry->TID] = 0;    // If STA mode,  erase flag here
        pPort->BATable.numAsRecipient -= 1;


        if (KeGetCurrentIrql() != PASSIVE_LEVEL)
        {
            RT_SET_ASIC_WCID    SetAsicWcid;
            // Another adhoc joining, add to our MAC table. 
            SetAsicWcid.WCID = pBARECEntry->Wcid;
            SetAsicWcid.SetTid = (0xffffffff);
            SetAsicWcid.DeleteTid = (0x10000<<(pBARECEntry->TID) );
            PlatformMoveMemory(SetAsicWcid.Addr, pWcidMacTabEntry->Addr, MAC_ADDR_LEN);            
            
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("Irq != PASSIVE_LEVEL. KeGetCurrentIrql()=%d\n", KeGetCurrentIrql()));
            MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_SET_ASIC_WCID, &SetAsicWcid, sizeof(RT_SET_ASIC_WCID));
            DBGPRINT(RT_DEBUG_TRACE,("Aid= 0x%x.DeleteTid= 0x%x. \n", pBARECEntry->Wcid, SetAsicWcid.DeleteTid));
        }
        else
        {
            offset = MAC_WCID_BASE + (pBARECEntry->Wcid) * HW_WCID_ENTRY_SIZE + 4;
            RTUSBReadMACRegister(pAd, offset, &MCAValue);
            // bitmap field starts at 0x10000 in ASIC WCID table
            MCAValue &= (~(0x10000<<(pBARECEntry->TID) ));
            RTUSBWriteMACRegister(pAd, offset, MCAValue);
        }
        NdisReleaseSpinLock(&pPort->BATabLock);
    }
}
/*
    ========================================================================
    Routine Description:
        Invoke DELBA Action frame to tear down current BA session.
    Arguments:
        PUCHAR    pAddr,  MAC Address
        UCHAR TID,  TID <=16
                pAddr and TID together are used to find exact one BARECEntry.
        BOOLEAN ALL  
            if ALL == TRUE. Ignore TID. Delete All BARECEntries with the same pAddr. 
                Used when delete this MAC Client's association.
                if ALL == FALSE, pAddr and TID together are used to find exact one BARECEntry.
                    when DELBA sent or DELBA received.
    ========================================================================
*/
VOID BATableTearRECEntry(
    IN OUT  PMP_ADAPTER pAd, 
    IN      PMP_PORT  pPort,
    IN      UCHAR TID, 
    IN      UCHAR Wcid, 
    IN      BOOLEAN ALL) 
{
    ULONG   Idx;
    BA_REC_ENTRY    *pBARECEntry;
    UCHAR   TIDStart, TIDEnd, CurTID;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid);  
    PMAC_TABLE_ENTRY pBaWcidMacTabEntry = NULL;

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
        return;
    }
    
    DBGPRINT(RT_DEBUG_TRACE,("BATableTearRECEntry===> Wcid=%d.TID=%d \n", Wcid, TID));

    if (Wcid >= MAX_LEN_OF_MAC_TABLE)
    {
        return;
    }

    //
    // 0 . If to delete all TID with the same pAddr, loop for all TID.
    //
    if (ALL == TRUE)
    {
        TIDStart = 0;
        TIDEnd = 8;
    }
    else
    {
        TIDStart = TID;
        TIDEnd = TID+1;
    }
    

    for (CurTID = TIDStart; CurTID < TIDEnd; CurTID++)
    {
        pBARECEntry = NULL;
        //
        // 2. Find corresponding BA Recipient Entry in BATable using Aid we obtained above.
        //

        // if this receiving packet is from SA that is in our OriEntry. Since WCID <9 has direct mapping. no need search.
        Idx = pWcidMacTabEntry->BARecWcidArray[CurTID];

        pBARECEntry = &pPort->BATable.BARecEntry[Idx];
        //
        // 2.1  Always send DELBA. 
        //
        {
            if ((ALL == FALSE) || ((ALL == TRUE) && (pBARECEntry->REC_BA_Status == Recipient_Accept)))
            {
                MLME_DELBA_REQ_STRUCT   DelbaReq;   
                pBaWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, pBARECEntry->Wcid); 
                if(pBaWcidMacTabEntry == NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, pBARECEntry->Wcid));
                    return;
                }
                
                PlatformZeroMemory(&DelbaReq, sizeof(DelbaReq));
                COPY_MAC_ADDR(DelbaReq.Addr, pBaWcidMacTabEntry->Addr);
                DelbaReq.Wcid = Wcid;
                DelbaReq.TID = CurTID;
                DelbaReq.Initiator = RECIPIENT;
                DelbaReq.BAOriSequence = pBaWcidMacTabEntry->BAOriSequence[CurTID];
                MlmeEnqueue(pAd, pPort,ACTION_STATE_MACHINE, MT2_MLME_ORI_DELBA_CATE, sizeof(MLME_DELBA_REQ_STRUCT), (PVOID)&DelbaReq);
            }
            
            if (pBARECEntry->TID == CurTID)
                BATableDeleteRECEntry(pAd,pPort,pBARECEntry);           
        }

    }
}
/*
    ========================================================================
    Routine Description:
        Invoke DELBA Action frame to tear down current BA session.

    Arguments:
        PUCHAR    pAddr,  MAC Address
        UCHAR TID,  TID <=16
                pAddr and TID together are used to find exact one BARECEntry.
        BOOLEAN ALL  
            if ALL == TRUE. Ignore TID. Delete All BAORIEntries with the same pAddr. 
                Used when delete this MAC Client's association.
                if ALL == FALSE, pAddr and TID together are used to find exact one BAORIEntry.
                    when DELBA sent or DELBA received.
    ========================================================================
*/
VOID BATableTearORIEntry(
    IN OUT  PMP_ADAPTER pAd, 
    IN      PMP_PORT pPort, 
    IN      UCHAR TID, 
    IN      UCHAR Wcid, 
    IN      BOOLEAN bForceDelete, 
    IN      BOOLEAN ALL) 
{
    ULONG   Idx = 0;
    BA_ORI_ENTRY    *pBAORIEntry;
    UCHAR   TIDStart, TIDEnd, CurTID;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid);  
    PMAC_TABLE_ENTRY pBaWcidMacTabEntry = NULL;
    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
        return;
    }
    
    DBGPRINT(RT_DEBUG_TRACE,("BATableTearORIEntry===> Wcid=%d.TID=%d, numAsOriginator= %d \n", Wcid, TID,pPort->BATable.numAsOriginator));
    if (Wcid >= MAX_LEN_OF_MAC_TABLE)
    {
        return;
    }
    //
    // 0 . If wish to delete all TID with the same pAddr, loop for all TID.
    //
    if (ALL == TRUE)
    {
        TIDStart = 0;
        TIDEnd = 8;
    }
    else
    {
        TIDStart = TID;
        TIDEnd = TID+1;
    }
    
    for (CurTID = TIDStart; CurTID < TIDEnd; CurTID++)
    {
        //
        // 2. Locate corresponding BA Originator Entry in BA Table with the (pAddr,TID).
        //
        Idx = pWcidMacTabEntry->BAOriWcidArray[CurTID];
        pBAORIEntry = &pPort->BATable.BAOriEntry[Idx];
        //
        // 2.1  Always send DELBA. 
        //
        {
            MLME_DELBA_REQ_STRUCT   DelbaReq;   

            if ((ALL == FALSE) || ((ALL == TRUE) && (pBAORIEntry->ORI_BA_Status == Originator_Done)))
            {
                pBaWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, pBAORIEntry->Wcid);
                if(pBaWcidMacTabEntry == NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, pBAORIEntry->Wcid));
                    return;
                }
                
                PlatformZeroMemory(&DelbaReq, sizeof(DelbaReq));
                COPY_MAC_ADDR(DelbaReq.Addr, pBaWcidMacTabEntry->Addr);
                DelbaReq.Wcid = Wcid;
                DelbaReq.TID = CurTID;
                DelbaReq.Initiator = ORIGINATOR;
                DelbaReq.BAOriSequence = pWcidMacTabEntry->BAOriSequence[CurTID];               
                MlmeEnqueue(pAd, pPort,ACTION_STATE_MACHINE, MT2_MLME_ORI_DELBA_CATE, sizeof(MLME_DELBA_REQ_STRUCT), (PVOID)&DelbaReq);
            }
            if (pBAORIEntry->TID == CurTID)
                BATableDeleteORIEntry(pAd,pPort, pBAORIEntry);
            
            if ((pPort->CommonCfg.BACapability.field.AutoBA == 1) && (bForceDelete == FALSE))
            {
                BATableInsertEntry(pAd, pPort,Wcid,  0, 0x1, CurTID, pWcidMacTabEntry->BaSizeInUse, Originator_SetAutoAdd,  FALSE);
                pWcidMacTabEntry->TXAutoBAbitmap |= ((1<<(pBAORIEntry->TID) )); // If STA mode,  erase flag here
            }
        }
    }
}
/*! \brief
 *  \param 
 *  \return
 *  \pre
 *  \post
     
 IRQL = DISPATCH_LEVEL
 
 */
VOID BssEntrySet(
    IN  PMP_ADAPTER   pAd, 
    IN PMP_PORT     pPort,
    OUT BSS_ENTRY *pBss, 
    IN PUCHAR pBssid, 
    IN CHAR Ssid[], 
    IN UCHAR SsidLen, 
    IN UCHAR BssType, 
    IN USHORT BeaconPeriod, 
    IN PCF_PARM pCfParm, 
    IN USHORT AtimWin, 
    IN USHORT CapabilityInfo, 
    IN UCHAR SupRate[], 
    IN UCHAR SupRateLen,
    IN UCHAR ExtRate[], 
    IN UCHAR ExtRateLen,
    IN HT_CAPABILITY_IE *pHtCapability,
    IN VHT_CAP_IE *pVhtCapability,
    IN VHT_OP_IE *pVhtOperation,
    IN ADD_HT_INFO_IE *pAddHtInfo,  // AP might use this additional ht info IE 
    IN UCHAR            HtCapabilityLen,
    IN UCHAR            AddHtInfoLen,
    IN UCHAR            NewExtChanOffset,
    IN UCHAR Channel,
    IN CHAR Rssi,
    IN ULONGLONG TimeStamp,
    IN PEDCA_PARM pEdcaParm,
    IN PQOS_CAPABILITY_PARM pQosCapability,
    IN PQBSS_LOAD_PARM pQbssLoad,
    IN USHORT WSCInfoAtBeaconsLen,
    IN PUCHAR WSCInfoAtBeacons,
    IN USHORT WSCInfoAtProbeRspLen,
    IN PUCHAR WSCInfoAtProbeRsp,
    IN USHORT LengthVIE,    
    IN PNDIS_802_11_VARIABLE_IEs pVIE,
    IN BOOLEAN bUpdateRssi)
{

    //DBGPRINT(RT_DEBUG_TRACE, ("BssEntrySet : SSID : %s, Bssid : %02x %02x %02x %02x %02x %02x, Channel = %d\n", Ssid, pBssid[0], pBssid[1], pBssid[2], pBssid[3], pBssid[4], pBssid[5], Channel));
    
    COPY_MAC_ADDR(pBss->Bssid, pBssid);
    // Default Hidden SSID to be TRUE, it will be turned to FALSE after coping SSID
    //pBss->Hidden = 1; 
    if (SsidLen > 0)
    {
        // For hidden SSID AP, it might send beacon with SSID len equal to 0
        // Or send beacon /probe response with SSID len matching real SSID length,
        // but SSID is all zero. such as "00-00-00-00" with length 4.
        // We have to prevent this case overwrite correct table
        if (PlatformEqualMemory(Ssid, ZeroSsid, SsidLen) == 0)
        {
            PlatformMoveMemory(pBss->Ssid, Ssid, SsidLen);
            pBss->SsidLen = SsidLen;
            //pBss->Hidden = 0;
        }
    }
    pBss->BssType = BssType;
    pBss->BeaconPeriod = BeaconPeriod;
    if (BssType == BSS_INFRA) 
    {
        if (pCfParm->bValid) 
        {
            pBss->CfpCount = pCfParm->CfpCount;
            pBss->CfpPeriod = pCfParm->CfpPeriod;
            pBss->CfpMaxDuration = pCfParm->CfpMaxDuration;
            pBss->CfpDurRemaining = pCfParm->CfpDurRemaining;
        }
    } 
    else 
    {
        pBss->AtimWin = AtimWin;
    }

    pBss->CapabilityInfo = CapabilityInfo;
    // The privacy bit indicate security is ON, it maight be WEP, TKIP or AES
    // Combine with AuthMode, they will decide the connection methods.
    pBss->Privacy = CAP_IS_PRIVACY_ON(pBss->CapabilityInfo);
    PlatformMoveMemory(pBss->SupRate, SupRate, (SupRateLen>MAX_LEN_OF_SUPPORTED_RATES)?MAX_LEN_OF_SUPPORTED_RATES:SupRateLen);
    pBss->SupRateLen = SupRateLen;
    PlatformMoveMemory(pBss->ExtRate, ExtRate, (ExtRateLen>MAX_LEN_OF_SUPPORTED_RATES)?MAX_LEN_OF_SUPPORTED_RATES:ExtRateLen);
    PlatformMoveMemory(&pBss->HtCapability, pHtCapability, HtCapabilityLen);
    PlatformMoveMemory(&pBss->AddHtInfo, pAddHtInfo, AddHtInfoLen);
    pBss->NewExtChanOffset = NewExtChanOffset;
    pBss->ExtRateLen = ExtRateLen;
    pBss->Channel = Channel;
    pBss->CentralChannel = Channel;
    pBss->WSCAPSetupLocked = FALSE;

    if (bUpdateRssi == TRUE)
    {
        pBss->Rssi = Rssi;
    }
    else if (pBss->Rssi == 0)
    {
        //
        // This is the first time that insert to the table, we also need to update Rssi.
        // Since Rssi can't be 0.
        //
        pBss->Rssi = Rssi;   
    }
    
    pBss->BeaconTimestamp = TimeStamp;
    NdisGetCurrentSystemTime((PLARGE_INTEGER)&pBss->HostTimestamp);

    pBss->bInRegDomain = TRUE; 

    // New for microsoft Fixed IEs
//  PlatformMoveMemory(pBss->FixIEs.Timestamp, &TimeStamp, 8);
//  pBss->FixIEs.BeaconInterval = BeaconPeriod;
//  pBss->FixIEs.Capabilities = CapabilityInfo;

    //
    // For Win7 WPS Requirement
    //
    if (WSCInfoAtBeaconsLen != 0)
    {
        pBss->WSCInfoAtBeaconsLen = WSCInfoAtBeaconsLen;
        PlatformMoveMemory(pBss->WSCInfoAtBeacons, WSCInfoAtBeacons, pBss->WSCInfoAtBeaconsLen);
    }   

    if (WSCInfoAtProbeRspLen != 0)
    {
        pBss->WSCInfoAtProbeRspLen = WSCInfoAtProbeRspLen;
        PlatformMoveMemory(pBss->WSCInfoAtProbeRsp, WSCInfoAtProbeRsp, pBss->WSCInfoAtProbeRspLen);
    }

    // 1. Check VarIE length
    if ((pBss->WSCInfoAtBeaconsLen > 0)||(pBss->WSCInfoAtProbeRspLen > 0))
    {
        UCHAR   *pData;
        INT     Len = 0;
        PWSC_IE pWscIE;

        if(pBss->WSCInfoAtProbeRspLen > 0)
        {
            pData = (UCHAR *)pBss->WSCInfoAtProbeRsp;
            Len = pBss->WSCInfoAtProbeRspLen;
        }
        else
        {
            pData = (UCHAR *)pBss->WSCInfoAtBeacons;
            Len = pBss->WSCInfoAtBeaconsLen;
        }
        pWscIE = (PWSC_IE) pData;
        
        // 2. WSC IE in beacons, skip 6 bytes = 1 + 1 + 4
        pData += 6;
        Len   -= 6;

        // 3. Start to look the Version within WSC VarIE
        while (Len > 0)
        {
            // Check for WSC IEs
            pWscIE = (PWSC_IE) pData;


            if(be2cpu16(pWscIE->Type) == WSC_IE_VENDOR_EXT)
            {
                PUCHAR  pVendorExtType = pWscIE->Data;
                USHORT  VendorExtLen = be2cpu16(pWscIE->Length);
                UCHAR   VendorExtElemLen = 0;
                if (((*(pVendorExtType) * 256 + *(pVendorExtType+1)) * 256 + (*(pVendorExtType+2))) == WSC_SMI)
                {
                    VendorExtLen -= 3;      //WSC SMI
                    pVendorExtType += 3;    //WSC SMI
                    while(VendorExtLen > 2)
                    {
                        VendorExtElemLen = *(pVendorExtType+1);
                                            
                        switch(*pVendorExtType)
                        {
                            case WSC_IE_VERSION2:
                                pBss->WSCAPVersion = *(pVendorExtType + 2);
                                DBGPRINT(RT_DEBUG_TRACE, ("%s WSC_IE_VENDOR_EXT Parse AP Version2 type 0x%x!!\n", __FUNCTION__, pBss->WSCAPVersion));
                                break;
                            default:
                                DBGPRINT(RT_DEBUG_TRACE, ("%s WSC_IE_VENDOR_EXT Parse UNKNOWN type 0x%x !!\n", __FUNCTION__, *pVendorExtType));
                                break;
                        }
                        pVendorExtType += (VendorExtElemLen + 2);
                        VendorExtLen -= (VendorExtElemLen + 2);
                    }
                }

            }
            else if(be2cpu16(pWscIE->Type) == WSC_IE_AP_SETUP_LOCK)
            {
                pBss->WSCAPSetupLocked = *(pWscIE->Data);
                DBGPRINT(RT_DEBUG_TRACE, ("%s WSC_IE_AP_SETUP_LOCK is %d\n", __FUNCTION__, pBss->WSCAPSetupLocked));
            }
            pData += (be2cpu16(pWscIE->Length) + 4);
            Len   -= (be2cpu16(pWscIE->Length) + 4);
        }
    }
    
    // New for microsoft Variable IEs
    if (LengthVIE != 0)
    {
        //To avoid over write longer to shorter VarIE when doing WPS (IE in beacon always shorter than ProbeRsp when AP enabling WPS).
        if((LengthVIE > pBss->VarIELen) || (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF))
        {
            pBss->VarIELen = LengthVIE;
            PlatformMoveMemory(pBss->VarIEs, pVIE, pBss->VarIELen);
        }
    }
    else
    {
        pBss->VarIELen = 0;
    }

    if ((pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && (HtCapabilityLen> 0) )  
    {
        pBss->HtCapabilityLen = HtCapabilityLen;
        PlatformMoveMemory(&pBss->HtCapability, pHtCapability, HtCapabilityLen);
        if (AddHtInfoLen > 0)
        {
            pBss->AddHtInfoLen = AddHtInfoLen;
            PlatformMoveMemory(&pBss->AddHtInfo, pAddHtInfo, AddHtInfoLen);
            
            if ((pAddHtInfo->ControlChan > 2)&& (pAddHtInfo->AddHtInfo.ExtChanOffset == EXTCHA_BELOW) && (pHtCapability->HtCapInfo.ChannelWidth == BW_40))
            {
                pBss->CentralChannel = pAddHtInfo->ControlChan - 2;
            }
            else if ((pAddHtInfo->AddHtInfo.ExtChanOffset == EXTCHA_ABOVE) && (pHtCapability->HtCapInfo.ChannelWidth == BW_40))
            {
                pBss->CentralChannel = pAddHtInfo->ControlChan + 2;
            }
            if ((pAddHtInfo->ControlChan == 13) && (pHtCapability->HtCapInfo.ChannelWidth == BW_40))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("--> ControlChannel = %d, ExtChanOffset = %d, CentralChannel = %d.\n", pAddHtInfo->ControlChan, pAddHtInfo->AddHtInfo.ExtChanOffset,pBss->CentralChannel ));
                pBss->CentralChannel = 11;
            }
            if ((pAddHtInfo->ControlChan == 12) && (pHtCapability->HtCapInfo.ChannelWidth == BW_40))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("--> ControlChannel = %d, ExtChanOffset = %d, CentralChannel = %d.\n", pAddHtInfo->ControlChan, pAddHtInfo->AddHtInfo.ExtChanOffset,pBss->CentralChannel ));
                pBss->CentralChannel = 10;
            }
        }
        else
            pBss->AddHtInfoLen = 0;

    }
    else
        pBss->HtCapabilityLen = 0;
    

    //
    // save the VHT information of the AP(or peer STA)
    //
    if(pPort->CommonCfg.PhyMode == PHY_11VHT)
    {
        pBss->VhtPeerStaCtrlBssEntry.bVhtCapable = TRUE;
        // save VhtCapability
        PlatformMoveMemory(&pBss->VhtPeerStaCtrlBssEntry.VhtCapabilityIE, pVhtCapability, SIZE_OF_VHT_CAP_IE);
        
        // save VhtOperation
        PlatformMoveMemory(&pBss->VhtPeerStaCtrlBssEntry.VhtOperationIE, pVhtOperation, SIZE_OF_VHT_OP_IE);
    }

    BssCipherParse(pBss);

    // new for QOS
    if (pEdcaParm)
        PlatformMoveMemory(&pBss->EdcaParm, pEdcaParm, sizeof(EDCA_PARM));
    else
        pBss->EdcaParm.bValid = FALSE;
    if (pQosCapability)
        PlatformMoveMemory(&pBss->QosCapability, pQosCapability, sizeof(QOS_CAPABILITY_PARM));
    else
        pBss->QosCapability.bValid = FALSE;
    if (pQbssLoad)
        PlatformMoveMemory(&pBss->QbssLoad, pQbssLoad, sizeof(QBSS_LOAD_PARM));
    else
        pBss->QbssLoad.bValid = FALSE;

    if((WSCInfoAtProbeRspLen != 0) || (WSCInfoAtBeaconsLen != 0))
    {
        pBss->bWPSAP = TRUE;
    }
}


/*! 
 *  \brief insert an entry into the bss table
 *  \param p_tab The BSS table
 *  \param Bssid BSSID
 *  \param ssid SSID
 *  \param ssid_len Length of SSID
 *  \param bss_type
 *  \param beacon_period
 *  \param timestamp
 *  \param p_cf
 *  \param atim_win
 *  \param cap
 *  \param rates
 *  \param rates_len
 *  \param channel_idx
 *  \return none
 *  \pre
 *  \post
 *  \note If SSID is identical, the old entry will be replaced by the new one
     
 IRQL = DISPATCH_LEVEL
 
 */
ULONG BssTableSetEntry(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort,
    OUT BSS_TABLE *Tab, 
    IN PUCHAR pBssid, 
    IN CHAR Ssid[], 
    IN UCHAR SsidLen, 
    IN UCHAR BssType, 
    IN USHORT BeaconPeriod, 
    IN CF_PARM *CfParm, 
    IN USHORT AtimWin, 
    IN USHORT CapabilityInfo, 
    IN UCHAR SupRate[],
    IN UCHAR SupRateLen,
    IN UCHAR ExtRate[],
    IN UCHAR ExtRateLen,
    IN HT_CAPABILITY_IE *pHtCapability,
    IN VHT_CAP_IE *pVhtCapability,
    IN VHT_OP_IE *pVhtOperation,
    IN ADD_HT_INFO_IE *pAddHtInfo,  // AP might use this additional ht info IE 
    IN UCHAR            HtCapabilityLen,
    IN UCHAR            AddHtInfoLen,
    IN UCHAR            NewExtChanOffset,
    IN UCHAR ChannelNo,
    IN CHAR Rssi,
    IN ULONGLONG TimeStamp,
    IN PEDCA_PARM pEdcaParm,
    IN PQOS_CAPABILITY_PARM pQosCapability,
    IN PQBSS_LOAD_PARM pQbssLoad,
    IN USHORT WSCInfoAtBeaconsLen, 
    IN PUCHAR WSCInfoAtBeacons,
    IN USHORT WSCInfoAtProbeRspLen,
    IN PUCHAR WSCInfoAtProbeRsp,
    IN USHORT LengthVIE,    
    IN PNDIS_802_11_VARIABLE_IEs pVIE,
    IN BOOLEAN bUpdateRssi)
{
    ULONG   Idx;
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    ULONG   iCount;
    PBSS_ENTRY  pBss;
    PDOT11_OFFLOAD_NETWORK  pNLO;
#endif
    Idx = BssTableSearchWithSSID(Tab, pBssid,  Ssid, SsidLen, ChannelNo);
    if (Idx == BSS_NOT_FOUND) 
    {
        if (Tab->BssNr >= MAX_LEN_OF_BSS_TABLE)
        {
            //
            // It may happen when BSS Table was full.
            // The desired AP will not be added into BSS Table
            // In this case, if we found then desired AP then overwrite BSS Table.
            //
            if(!OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED))
            {
                if (MAC_ADDR_EQUAL(pAd->MlmeAux.Bssid, pBssid) ||
                    SSID_EQUAL(pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen, Ssid, SsidLen))
                {
                    Tab->BssOverlapNr = Tab->BssOverlapNr + 1; // Update first
                    Tab->BssOverlapNr = Tab->BssOverlapNr % MAX_LEN_OF_BSS_TABLE; // Avoid array bounds violations
                    Idx = Tab->BssOverlapNr;
                    BssEntrySet(pAd, pPort, &Tab->BssEntry[Idx], pBssid, Ssid, SsidLen, BssType, BeaconPeriod, CfParm, AtimWin, 
                        CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen,pHtCapability,  pVhtCapability, pVhtOperation, pAddHtInfo,
                        HtCapabilityLen, AddHtInfoLen, NewExtChanOffset, ChannelNo, Rssi, TimeStamp, pEdcaParm, pQosCapability, pQbssLoad, 
                        WSCInfoAtBeaconsLen, WSCInfoAtBeacons, WSCInfoAtProbeRspLen, WSCInfoAtProbeRsp, LengthVIE, pVIE, bUpdateRssi);
                }
                return Idx;
            }
            else
            {
                return BSS_NOT_FOUND;
            }
        }
        Idx = Tab->BssNr;
        BssEntrySet(pAd, pPort, &Tab->BssEntry[Idx], pBssid, Ssid, SsidLen, BssType, BeaconPeriod, CfParm, AtimWin, 
                    CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen,pHtCapability, pVhtCapability, pVhtOperation,
                    pAddHtInfo, HtCapabilityLen, AddHtInfoLen, NewExtChanOffset, ChannelNo, Rssi, TimeStamp, pEdcaParm, pQosCapability, pQbssLoad, 
                    WSCInfoAtBeaconsLen, WSCInfoAtBeacons, WSCInfoAtProbeRspLen, WSCInfoAtProbeRsp, LengthVIE, pVIE, bUpdateRssi);
        Tab->BssNr++;
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        // Win8 Check if this New BSS entry is one of NLO entries
        // 1.) NLO enable and not set to indicate yet
        if (NDIS_WIN8_ABOVE(pAd) && (pAd->StaCfg.NLOEntry.NLOEnable == TRUE) && (pAd->StaCfg.NLOEntry.IndicateNLO == FALSE))
        {
            pBss = &Tab->BssEntry[Idx];

            for (iCount = 0; iCount < pAd->StaCfg.NLOEntry.uNumOfEntries; iCount++)
            {
                pNLO= &pAd->StaCfg.NLOEntry.OffloadNetwork[iCount];

                // check SSID, Cipher and AuthMode
                if (!SSID_EQUAL(Ssid, SsidLen, pNLO->Ssid.ucSSID, pNLO->Ssid.uSSIDLength))
                    continue;

                if ((pBss->AuthMode != (ULONG)(pNLO->AuthAlgo)) && (pBss->AuthModeAux != (ULONG)(pNLO->AuthAlgo)))
                    continue;

                if (pBss->WepStatus != (ULONG)(pNLO->UnicastCipher))
                    continue;

                // To Consider Connected state
                if (OPSTATUS_TEST_FLAG(pPort,fOP_STATUS_MEDIA_STATE_CONNECTED))
                {

                }

                // the new BssEntry is in NLO entry, need to indicate NLO event after scan done
                pAd->StaCfg.NLOEntry.IndicateNLO = TRUE;

                DBGPRINT(RT_DEBUG_TRACE, ("%s: NLO: <New BSS entry> Found NLO network\n", __FUNCTION__));
            }
        }
#endif
    } 
    else
    {
        BssEntrySet(pAd, pPort, &Tab->BssEntry[Idx], pBssid, Ssid, SsidLen, BssType, BeaconPeriod,CfParm, AtimWin, 
                    CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen,pHtCapability, pVhtCapability, pVhtOperation,
                    pAddHtInfo, HtCapabilityLen, AddHtInfoLen, NewExtChanOffset, ChannelNo, Rssi, TimeStamp, pEdcaParm, pQosCapability, pQbssLoad, 
                    WSCInfoAtBeaconsLen, WSCInfoAtBeacons, WSCInfoAtProbeRspLen, WSCInfoAtProbeRsp, LengthVIE, pVIE, bUpdateRssi);

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        // Win8 Check if this New BSS entry is one of NLO entries
        // 1.) NLO enable and not set to indicate yet
        if (NDIS_WIN8_ABOVE(pAd) && (pAd->StaCfg.NLOEntry.NLOEnable == TRUE) && (pAd->StaCfg.NLOEntry.IndicateNLO == FALSE))
        {
            pBss = &Tab->BssEntry[Idx];

            for (iCount = 0; iCount < pAd->StaCfg.NLOEntry.uNumOfEntries; iCount++)
            {
                pNLO= &pAd->StaCfg.NLOEntry.OffloadNetwork[iCount];

                // check SSID, Cipher and AuthMode
                if (!SSID_EQUAL(Ssid, SsidLen, pNLO->Ssid.ucSSID, pNLO->Ssid.uSSIDLength))
                    continue;

                if ((pBss->AuthMode != (ULONG)(pNLO->AuthAlgo)) && (pBss->AuthModeAux != (ULONG)(pNLO->AuthAlgo)))
                    continue;

                if (pBss->WepStatus != (ULONG)(pNLO->UnicastCipher))
                    continue;

                // To Consider Connected state
                if (OPSTATUS_TEST_FLAG(pPort,fOP_STATUS_MEDIA_STATE_CONNECTED))
                {

                }

                // the new BssEntry is in NLO entry, need to indicate NLO event after scan done
                pAd->StaCfg.NLOEntry.IndicateNLO = TRUE;

                DBGPRINT(RT_DEBUG_TRACE, ("%s: NLO: <Old BSS entry> Found NLO network\n", __FUNCTION__));
            }
        }
#endif
    }

    return Idx;
}
/*
============================================================
Description:  

If insert as recipient, it means accept the ADDBA request, and start receive A-MPDU right now.

If insert as Originator, the initial state is wait_for_response, if ADDBA Rsp not receive or feedback status code is not zero,
the baentry will be deleted, otherwise, change state to originaotr_done and start send A-MPDU.

============================================================
*/
VOID BATableInsertEntry(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort, 
    IN UCHAR Aid, 
    IN USHORT       TimeOutValue,
    IN USHORT       StartingSeq,
    IN UCHAR TID, 
    IN UCHAR BAWinSize, 
    IN UCHAR OriginatorStatus, 
    IN BOOLEAN Recipient) 
{
    UCHAR   Idx = 0xff;
    UCHAR   index;
    UCHAR   i;
    PMAC_TABLE_ENTRY pEntry;
    USHORT  offset; 
    ULONG   Value;
    
    if (Aid >= MAX_LEN_OF_MAC_TABLE)            
        return; 
    
    if (pPort->CommonCfg.BACapability.field.Policy == BA_NOTUSE)
    {       
        return;
    }
    
    pEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Aid);
    if(pEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Aid));
        return;
    }
    
    if (Recipient == TRUE)
    {
        BA_REC_ENTRY    *pBAEntry = NULL;
        UCHAR   ZeroBSSID[6]={0,0,0,0,0,0};

        DBGPRINT(RT_DEBUG_TRACE, ("BATableInsert Recipient Entry ===>Aid = %d. StartingSeq = %x\n", Aid, StartingSeq));
        // concurrent Recipient # is limited by how many reordering buffer we allocate at initialization stage.
        if (pPort->BATable.numAsRecipient >= Max_BARECI_SESSION) 
        {
            return;
        }
        
        Idx = pEntry->BARecWcidArray[TID];
        if ((pEntry->BARecWcidArray[TID]) == 0)
        {
            for (i = 1; i < MAX_LEN_OF_BA_REC_TABLE;i++)
            {
                pBAEntry =& pPort->BATable.BARecEntry[i];
                if (pBAEntry->REC_BA_Status == Recipient_NONE)
                {
                    // find vacant entry with Zero Addr,
                    Idx = i;
                    pEntry->BARecWcidArray[TID] = i;
                    pBAEntry =&pPort->BATable.BARecEntry[Idx];
                    DBGPRINT(RT_DEBUG_TRACE, ("BATableInsert[%d] ===>Wcid = %d\n", Idx, Aid));
                    break;
                }
                pBAEntry = NULL;
            }
        }
        else if ((pPort->BATable.BARecEntry[Idx].REC_BA_Status == Recipient_Accept))
        {
            if ((pPort->BATable.BARecEntry[Idx].BAWinSize != BAWinSize))
            {
                // Update existing entry's parameter. Need to tear down current BA session and set BA session again.
                // shoudle we handle here? why not ask GUI to tear duplicate entry then insert.
            }
            return;
        }
        // Start fill in parameters.
        if ((pBAEntry != NULL))
        {
            NdisAcquireSpinLock(&pPort->BATabLock);
            if (pBAEntry->REC_BA_Status == Recipient_NONE)
            {
                pPort->BATable.numAsRecipient++;
                // Allocate reordering buffer to this Recipient Entry.
                for (index = 0; index < Max_BARECI_SESSION; index++)
                {
                    if (pAd->pRxCfg->LocalRxReorderBuf[index].InUse == FALSE)
                    {
                        pAd->pRxCfg->LocalRxReorderBuf[index].InUse = TRUE;
                        pAd->pRxCfg->LocalRxReorderBuf[index].ByBaRecIndex = Idx;
                        pBAEntry->RxBufIdxUsed = index;
                        break;
                    }
                }
            }
            pBAEntry->Wcid = (UCHAR)Aid;
            pBAEntry->BAWinSize = (UCHAR)BAWinSize;
            pBAEntry->Curindidx = 0;
            pBAEntry->NumOfRxPkt = 0;
            pBAEntry->pAdapter = pAd;
            pBAEntry->TID = TID;
            pBAEntry->TimeOutValue = TimeOutValue;
            pBAEntry->REC_BA_Status = Recipient_Accept;
            //
            pBAEntry->LastIndSeq = StartingSeq;
            pBAEntry->LastIndSeqAtTimer= StartingSeq;
            // Set Bitmap flag.
            pEntry->RXBAbitmap |= (1<<TID );   
            
            offset = MAC_WCID_BASE + (Aid* HW_WCID_ENTRY_SIZE) + 4;
            // Set BA session mask in WCID table.
            if (KeGetCurrentIrql() != PASSIVE_LEVEL)
            {
                RT_SET_ASIC_WCID    SetAsicWcid;
                // Another adhoc joining, add to our MAC table. 
                SetAsicWcid.WCID = Aid;
                SetAsicWcid.SetTid = (0x10000<<TID);
                SetAsicWcid.DeleteTid = 0xffffffff;
                PlatformMoveMemory(SetAsicWcid.Addr, pEntry->Addr, MAC_ADDR_LEN);

                MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_SET_ASIC_WCID, &SetAsicWcid, sizeof(RT_SET_ASIC_WCID));
                DBGPRINT(RT_DEBUG_TRACE,("Aid= 0x%x.SetTid= 0x%x. \n", Aid, SetAsicWcid.SetTid));
            }
            else
            {

                RTUSBReadMACRegister(pAd, offset, &Value);
                Value |= (0x10000<<TID);
                RTUSBWriteMACRegister(pAd, offset, Value);
            }
            DBGPRINT(RT_DEBUG_TRACE,("BATableInsert Recipient : pAddr =%x, %x,  %x, %x, %x, %x. assign %dth reorder buf\n", 
                pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2], pEntry->Addr[3], pEntry->Addr[4], 
                pEntry->Addr[5], pBAEntry->RxBufIdxUsed));
            DBGPRINT(RT_DEBUG_TRACE,("BATableInsert MACEntry[%d] Seq = %x, RXBAbitmap = 0x%x. BARecWcidArray=%d\n", 
                Aid, StartingSeq, pEntry->RXBAbitmap, pEntry->BARecWcidArray[TID]));

            NdisReleaseSpinLock(&pPort->BATabLock);

            MtAsicUpdateBASession(pAd, pBAEntry->Wcid, pBAEntry->TID, StartingSeq, pBAEntry->BAWinSize, 1, RECIPIENT);
            DBGPRINT(RT_DEBUG_TRACE,("BATableInsert Recipient : pBAEntry->BAWinSize = %d\n", pBAEntry->BAWinSize));
        }
    }
    else
    {
        BA_ORI_ENTRY    *pBAEntry = NULL;
        // if FULL, return
        if (pPort->BATable.numAsOriginator >= MAX_LEN_OF_BA_ORI_TABLE) 
            return;

        Idx = pEntry->BAOriWcidArray[TID];
        //Since we want to insert this Entry, 
        if ((pEntry->BAOriWcidArray[TID]) == 0)
        {
            for (i = 1; i < MAX_LEN_OF_BA_ORI_TABLE;i++)
            {
                pBAEntry =&pPort->BATable.BAOriEntry[i];
                if ((pBAEntry->ORI_BA_Status == Originator_NONE))
                {
                    // Find an empty pair. Use this.
                    Idx = i;
                    //Record this index. So at Hardtransmit, only need to search MAC Etnry wcid.
                    //Doesn't need to search the corresponding BA Orient entry .
                    pEntry->BAOriWcidArray[TID] = i;
                    NdisAcquireSpinLock(&pPort->BATabLock);
                    pBAEntry->ORI_BA_Status = OriginatorStatus;
                    pBAEntry->BAWinSize = BAWinSize;
                    pBAEntry->Token = 1;    // Start from non-zero value.
                    NdisReleaseSpinLock(&pPort->BATabLock);
                    DBGPRINT(RT_DEBUG_TRACE, ("BATableInsert ===>Aid=%d, BAIdx = %d, OriginatorStatus = %d \n", Aid, Idx, OriginatorStatus));
                    break;
                }
                else if ((pBAEntry->ORI_BA_Status == Originator_SetAutoAdd) && (OriginatorStatus == Originator_WaitRes))
                {
                    // Set via OID. OID has higher priority of auto add originator. :)
                    // Find an empty pair. Use this.
                    Idx = i;
                    //Record this index. So at Hardtransmit, only need to search MAC Etnry wcid.
                    //Doesn't need to search the corresponding BA Orient entry .
                    pEntry->BAOriWcidArray[TID] = i;
                    NdisAcquireSpinLock(&pPort->BATabLock);
                    pBAEntry->ORI_BA_Status = OriginatorStatus;
                    pBAEntry->BAWinSize = BAWinSize;
                    pBAEntry->Token = 1;    // Start from non-zero value.
                    NdisReleaseSpinLock(&pPort->BATabLock);
                    DBGPRINT(RT_DEBUG_TRACE, ("BATableInsertEntry[%d]. BAWinSize= %d\n",i, BAWinSize));
                    break;
                }
                pBAEntry = NULL;
            }
        }
        else if ((pPort->BATable.BAOriEntry[Idx].ORI_BA_Status == Originator_WaitRes))
        {
            // handshake in progress. return.
            DBGPRINT(RT_DEBUG_TRACE, ("BATableInsert handshake in progress\n"));
            return;
        }
        else if ((pPort->BATable.BAOriEntry[Idx].ORI_BA_Status == Originator_Done))
        {
            if ((pPort->BATable.BAOriEntry[Idx].BAWinSize != BAWinSize))
            {
                // Update existing entry's parameter. Need to tear down current BA session and set BA session again.
                // shoudle we handle here? why not ask GUI to tear duplicate entry then insert.
            }
            DBGPRINT(RT_DEBUG_TRACE, ("BATableInsert - already exists. Aid=%d, BAIdx=%d \n", Aid, Idx));
            return;
        }

        // Fill in parameters.
        if ((pBAEntry != NULL))
        {
            NdisAcquireSpinLock(&pPort->BATabLock);
            pBAEntry->Wcid = (UCHAR)Aid;
            pBAEntry->BAWinSize = (UCHAR)BAWinSize;
            pBAEntry->pAdapter = pAd;
            pBAEntry->TID = TID;
            pBAEntry->Token = 0;
            // set Status to Done immediately. 

            DBGPRINT(RT_DEBUG_TRACE,("MACEntry[%d]TXBAbitmap = 0x%x. BAOriWcidArray=%d\n", 
                Aid, pEntry->TXBAbitmap, pEntry->BAOriWcidArray[TID]));
            NdisReleaseSpinLock(&pPort->BATabLock);
        }
    }
    DBGPRINT(RT_DEBUG_TRACE, ("<===BATableInsertEntry \n"));
}

VOID  TriEventInit(
    IN  PMP_ADAPTER   pAd,
    IN PMP_PORT     pPort) 
{
    UCHAR   i;

    for (i = 0;i < MAX_TRIGGER_EVENT;i++)
        pPort->CommonCfg.TriggerEventTab.EventA[i].bValid = FALSE;
    
    pPort->CommonCfg.TriggerEventTab.EventANo = 0;
    pPort->CommonCfg.TriggerEventTab.EventBCountDown = 0;
}

VOID TriEventTableSetEntry(
    IN  PMP_ADAPTER   pAd, 
    IN PMP_PORT     pPort,
    OUT TRIGGER_EVENT_TAB *Tab, 
    IN PUCHAR pBssid, 
    IN HT_CAPABILITY_IE *pHtCapability,
    IN UCHAR            HtCapabilityLen,
    IN UCHAR            RegClass,
    IN UCHAR ChannelNo)
{
    // Event A. This is Legacy AP
    if (HtCapabilityLen == 0)
    {
        UCHAR index;
        for (index = 0; index<MAX_TRIGGER_EVENT; index++)
        {
            if ((Tab->EventA[index].bValid == TRUE) && (Tab->EventA[index].Channel == ChannelNo) && (Tab->EventA[index].RegClass == RegClass))
                return; 
        }
        
        if (Tab->EventANo < MAX_TRIGGER_EVENT)
        {
            PlatformMoveMemory(Tab->EventA[Tab->EventANo].BSSID, pBssid, 6);
            Tab->EventA[Tab->EventANo].bValid = TRUE;
            Tab->EventA[Tab->EventANo].Channel = ChannelNo;
            if (RegClass != 0)
            {
                // Beacon has Regulatory class IE. So use beacon's
                Tab->EventA[Tab->EventANo].RegClass = RegClass;
            }
            else
            {
                // If no Reg Class in Beacon, set to "unknown"
                Tab->EventA[Tab->EventANo].RegClass = 0; // ????????????????? need to check
            }
                
            Tab->EventANo ++;
        }
    }
    // Event B.   My BSS beacon has Intolerant40 bit set
    else if (pHtCapability->HtCapInfo.Intolerant40)
    {
        Tab->EventBCountDown = pPort->CommonCfg.ScanParameter.Dot11BssWidthChanTranDelay;
    }
    
}

// IRQL = DISPATCH_LEVEL
VOID BssTableSsidSort(
    IN  PMP_ADAPTER   pAd, 
    IN    PMP_PORT pPort,
    OUT BSS_TABLE *OutTab, 
    IN  CHAR Ssid[], 
    IN  UCHAR SsidLen) 
{
    INT i;
    BssTableInit(OutTab);
    pAd->MlmeAux.bWCN = FALSE;

    DBGPRINT(RT_DEBUG_TRACE, ("port [%d] BssTableSsidSort My [AuthMode = %d: WepStatus = %d. PairCipher = %d. BssType = %d] find ssid = %s ssidLen = %d\n", 
                                pPort->PortNumber, pPort->PortCfg.AuthMode, pPort->PortCfg.WepStatus,
                                pPort->PortCfg.PairCipher, pAd->StaCfg.BssType, Ssid, SsidLen));
                                
    for (i = 0; i < pAd->ScanTab.BssNr; i++) 
    {
        BSS_ENTRY *pInBss = &pAd->ScanTab.BssEntry[i];

        //
        // Check if we want connect to Adhoc and this is on Japan Channel
        //
        if ((pAd->StaCfg.BssType == BSS_ADHOC) && (pPort->CommonCfg.bIEEE80211H) && (JapanChannelCheck(pInBss->Channel)))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Skip this BSS since it's on Jappan channel BSSID[%02x:%02x:%02x:%02x:%02x:%02x] Channel=%d\n", 
                                        pInBss->Bssid[0], pInBss->Bssid[1], pInBss->Bssid[2], pInBss->Bssid[3], 
                                        pInBss->Bssid[4], pInBss->Bssid[5], pInBss->Channel));
            continue;
        }

        if ((pInBss->BssType == pAd->StaCfg.BssType) && 
            SSID_EQUAL(Ssid, SsidLen, pInBss->Ssid, pInBss->SsidLen))
        {
            BSS_ENTRY *pOutBss = &OutTab->BssEntry[OutTab->BssNr];

            if (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)
            {
                pPort->PortCfg.AuthMode = pInBss->AuthMode;
                pPort->PortCfg.WepStatus = pInBss->WepStatus; 
            
                if (pPort->PortCfg.AuthMode >= DOT11_AUTH_ALGO_RSNA)
                {
                    pPort->PortCfg.PairCipher = pInBss->WPA2.PairCipher;
                    pPort->PortCfg.GroupCipher = pInBss->WPA2.GroupCipher;
                }
                else if (pPort->PortCfg.AuthMode >= DOT11_AUTH_ALGO_WPA)
                {
                    pPort->PortCfg.PairCipher = pInBss->WPA.PairCipher;
                    pPort->PortCfg.GroupCipher = pInBss->WPA.GroupCipher;
                }

                // copy matching BSS from InTab to OutTab
                PlatformMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));

                OutTab->BssNr++;
                continue;
            }
            else if (pPort->StaCfg.WscControl.WscEnProfile == TRUE)
            {
                // copy matching BSS from InTab to OutTab
                PlatformMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));

                OutTab->BssNr++;
                continue;
            }

            // Vista
            // DTM: DUT can't associate when its desired BSSID list does not contain wildcard entry and does not match APs BSSID.
            //
            if (pPort->PortCfg.AcceptAnyBSSID == FALSE)
            {
                if (!MAC_ADDR_EQUAL(pInBss->Bssid, &pPort->PortCfg.DesiredBSSIDList[0]))
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("BssTableSsidSort::AcceptAnyBSSID == FALSE\n"));
                    continue;
                }
            }

            // New for WPA2
            // Check the Authmode first
            if (pPort->PortCfg.AuthMode >= DOT11_AUTH_ALGO_WPA)
            {
                // CCKM: may consider use AuthModeAux
                // Check AuthMode and AuthModeAux for matching, in case AP support dual-mode
                if ((pPort->PortCfg.AuthMode != pInBss->AuthMode) && (pPort->PortCfg.AuthMode != pInBss->AuthModeAux))
                    continue; // None matched
                
                // Check cipher suite, AP must have more secured cipher than station setting
                if ((pPort->PortCfg.AuthMode == DOT11_AUTH_ALGO_WPA) || (pPort->PortCfg.AuthMode == DOT11_AUTH_ALGO_WPA_PSK))
                {
                    //
                    // To check both the WPA and WPA2 mode is in order to meet the WPA2+CCKM+TKIP case
                    //
                    // If it's not mixed mode, we should only let BSS pass with the same encryption
                    if ((pInBss->WPA.bMixMode == FALSE) && (pInBss->WPA2.bMixMode == FALSE))
                    {
                        if ((pPort->PortCfg.WepStatus != pInBss->WPA.GroupCipher) &&
                            (pPort->PortCfg.WepStatus != pInBss->WPA2.GroupCipher))
                        {
                            continue;
                        }
                    }                       
                    // check group cipher, accept WPA+WEP mixed mode
                    if ((pPort->PortCfg.WepStatus < pInBss->WPA.GroupCipher) && (pInBss->MixedModeGroupCipher == Cipher_Type_NONE))
                    {
                        continue;
                    }

                    //
                    // To check both the WPA and WPA2 mode is in order to meet the WPA2+CCKM+TKIP case
                    //
                    // check pairwise cipher, skip if none matched
                    // If profile set to AES, let it pass without question.
                    // If profile set to TKIP, we must find one mateched
                    if ((pPort->PortCfg.WepStatus == DOT11_CIPHER_ALGO_TKIP) && 
                        (pPort->PortCfg.WepStatus != pInBss->WPA.PairCipher) && 
                        (pPort->PortCfg.WepStatus != pInBss->WPA.PairCipherAux) &&
                        (pPort->PortCfg.WepStatus != pInBss->WPA2.PairCipher) &&
                        (pPort->PortCfg.WepStatus != pInBss->WPA2.PairCipherAux))
                    {
                        continue;                       
                    }
                }
                else if ((pPort->PortCfg.AuthMode == DOT11_AUTH_ALGO_RSNA) || (pPort->PortCfg.AuthMode == DOT11_AUTH_ALGO_RSNA_PSK))
                {
                    //
                    // To check both the WPA and WPA2 mode is in order to meet the WPA+CCKM+AES case
                    //
                    // If it's not mixed mode, we should only let BSS pass with the same encryption
                    if ((pInBss->WPA.bMixMode == FALSE) && (pInBss->WPA2.bMixMode == FALSE))
                    {
                        if ((pPort->PortCfg.WepStatus != pInBss->WPA.GroupCipher) &&
                                        (pPort->PortCfg.WepStatus != pInBss->WPA2.GroupCipher))
                        {
                            continue;
                        }
                    }
                        
                    // check group cipher
                    // For Vista Group can be DOT11_CIPHER_ALGO_WEP (=256).
                    if ((pPort->PortCfg.WepStatus < pInBss->WPA2.GroupCipher) && 
                        (pInBss->WPA2.GroupCipher < DOT11_CIPHER_ALGO_WPA_USE_GROUP) &&
                        (pInBss->WPA2.bMixMode != TRUE))
                    {
                        continue;
                    }

                    //
                    // To check both the WPA and WPA2 mode is in order to meet the WPA+CCKM+AES case
                    //
                    // check pairwise cipher, skip if none matched
                    // If profile set to AES, let it pass without question.
                    // If profile set to TKIP, we must find one mateched
                    if ((pPort->PortCfg.WepStatus == DOT11_CIPHER_ALGO_TKIP) && 
                        (pPort->PortCfg.WepStatus != pInBss->WPA.PairCipher) && 
                        (pPort->PortCfg.WepStatus != pInBss->WPA.PairCipherAux) &&
                        (pPort->PortCfg.WepStatus != pInBss->WPA2.PairCipher) && 
                        (pPort->PortCfg.WepStatus != pInBss->WPA2.PairCipherAux))
                    {
                        continue;
                    }
                }
                else
                    continue;   // CCKM_ON but Cipher did no match
            }
            // Bss Type matched, SSID matched. 
            // We will check wepstatus for qualification Bss
            else if (IS_WEP_STATUS_ON(pPort->PortCfg.WepStatus))
                
            {
                DBGPRINT(RT_DEBUG_TRACE,("PortCfg.WepStatus=%s, while pInBss->WepStatus=%s\n"
                    , decodeCipherAlgorithm(pPort->PortCfg.WepStatus), decodeCipherAlgorithm(pInBss->WepStatus)));          
                //
                // If ExcludeUnencrypted is false, we associate with an AP even if 
                // it is not beaconing privacy bit
                //
                if (pPort->PortCfg.ExcludeUnencrypted)
                {
                     // Patch the issue that p2p client can't find out wps GO if its privacy bit is off.
                    if ((!pInBss->Privacy) && (pAd->LogoTestCfg.OnTestingWHQL == FALSE))
                        continue; // Reject (Privacy bit clear)

                    if(pPort->CommonCfg.WepConnectCheckEncry)
                    {
                        if((pPort->PortCfg.WepStatus != pInBss->WepStatus) &&
                            (!IS_WEP_STATUS_ON(pInBss->WepStatus)))
                        {
                            
                            if(pPort->PortCfg.PrivacyExemptionList->PrivacyExemptionEntries->usEtherType == 0x8e88)
                            {
                                DBGPRINT(RT_DEBUG_TRACE,("Maybe connect with WCN device\n"));           
                                pAd->MlmeAux.bWCN = TRUE;
                            }
                            else
                            {
                                DBGPRINT(RT_DEBUG_TRACE,("AP is not WEP and EtherType is not 8e88! Treat this connection NOT WCN! pPort->PortCfg.WepStatus:%d, PrivacyExemptionEntries->usEtherType:0x%02x\n",
                                    pPort->PortCfg.WepStatus,pPort->PortCfg.PrivacyExemptionList->PrivacyExemptionEntries->usEtherType));
                                continue;
                            }

                        }
                    }
                    else
                    {
                        if(pPort->PortCfg.WepStatus != pInBss->WepStatus)
                        {
                            DBGPRINT(RT_DEBUG_TRACE,("Maybe connect with WCN device\n"));           
                            pAd->MlmeAux.bWCN = TRUE;
                        }
                    }
                }
                else
                {
                    //Mark fowllowing code for ndistest excludeUnencryted_ext test              
            #if 0
                    if (pInBss->bWPSAP == TRUE)
                    {
                        // If this is a WPS AP, the station does not check WepStatus.
                        // Do nothing.
                    }
                    else    if ((!pInBss->bSES) && (pInBss->MixedModeGroupCipher == Cipher_Type_NONE))
                    {
                        //
                        // For the SESv2 case, we will not qualify WepStatus.
                        //
                        continue;
                    }
            #endif 
                }
            }
            else if (pPort->PortCfg.WepStatus == DOT11_CIPHER_ALGO_NONE)                
            {
                if (pInBss->Privacy)
                    continue; // Reject (with Privacy bit on )
            }

            // Since the AP is using hidden SSID, and we are trying to connect to ANY
            // It definitely will fail. So, skip it.
            // CCX also require not even try to connect it!!
            if (SsidLen == 0)
                continue;
            
            // copy matching BSS from InTab to OutTab
            PlatformMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));

            OutTab->BssNr++;
        }
        else if ((pInBss->BssType == pAd->StaCfg.BssType) && (SsidLen == 0))
        {
            BSS_ENTRY *pOutBss = &OutTab->BssEntry[OutTab->BssNr];

            // New for WPA2
            // Check the Authmode first
            if (pPort->PortCfg.AuthMode >= DOT11_AUTH_ALGO_WPA)
            {
                // CCKM: may consider use AuthModeAux
                // Check AuthMode and AuthModeAux for matching, in case AP support dual-mode
                if ((pPort->PortCfg.AuthMode != pInBss->AuthMode) && (pPort->PortCfg.AuthMode != pInBss->AuthModeAux))
                    continue; // None matched
                
                // Check cipher suite, AP must have more secured cipher than station setting
                if ((pPort->PortCfg.AuthMode == DOT11_AUTH_ALGO_WPA) || (pPort->PortCfg.AuthMode == DOT11_AUTH_ALGO_WPA_PSK))
                {
                    //
                    // To check both the WPA and WPA2 mode is in order to meet the WPA2+CCKM+TKIP case
                    //
                    // If it's not mixed mode, we should only let BSS pass with the same encryption
                    if ((pInBss->WPA.bMixMode == FALSE) && (pInBss->WPA2.bMixMode == FALSE) )
                    {
                        if ((pPort->PortCfg.WepStatus != pInBss->WPA.GroupCipher) &&
                            (pPort->PortCfg.WepStatus != pInBss->WPA2.GroupCipher))
                        {
                            continue;
                        }
                    }
                    // check group cipher, accept WPA+WEP mixed mode
                    if ((pPort->PortCfg.WepStatus < pInBss->WPA.GroupCipher) && (pInBss->MixedModeGroupCipher == Cipher_Type_NONE))
                    {
                        continue;
                    }

                    //
                    // To check both the WPA and WPA2 mode is in order to meet the WPA2+CCKM+TKIP case
                    //
                    // check pairwise cipher, skip if none matched
                    // If profile set to AES, let it pass without question.
                    // If profile set to TKIP, we must find one mateched
                    if ((pPort->PortCfg.WepStatus == DOT11_CIPHER_ALGO_TKIP) && 
                        (pPort->PortCfg.WepStatus != pInBss->WPA.PairCipher) && 
                        (pPort->PortCfg.WepStatus != pInBss->WPA.PairCipherAux) &&
                        (pPort->PortCfg.WepStatus != pInBss->WPA2.PairCipher) &&
                        (pPort->PortCfg.WepStatus != pInBss->WPA2.PairCipherAux))
                    {
                        continue;                       
                    }
                }
                else if ((pPort->PortCfg.AuthMode == DOT11_AUTH_ALGO_RSNA) || (pPort->PortCfg.AuthMode == DOT11_AUTH_ALGO_RSNA_PSK))
                {
                    //
                    // To check both the WPA and WPA2 mode is in order to meet the WPA+CCKM+AES case
                    //
                    // If it's not mixed mode, we should only let BSS pass with the same encryption
                    if ((pInBss->WPA.bMixMode == FALSE) && (pInBss->WPA2.bMixMode == FALSE))
                    {
                        if ((pPort->PortCfg.WepStatus != pInBss->WPA.GroupCipher) &&
                            (pPort->PortCfg.WepStatus != pInBss->WPA2.GroupCipher))
                        {
                            continue;
                        }
                    }
                        
                    // check group cipher
                    // For Vista Group can be DOT11_CIPHER_ALGO_WEP (=256).
                    if ((pPort->PortCfg.WepStatus < pInBss->WPA2.GroupCipher) &&
                        (pInBss->WPA2.GroupCipher < DOT11_CIPHER_ALGO_WPA_USE_GROUP) &&
                        (pInBss->WPA2.bMixMode != TRUE))
                    {
                        continue;
                    }

                    //
                    // To check both the WPA and WPA2 mode is in order to meet the WPA+CCKM+AES case
                    //
                    // check pairwise cipher, skip if none matched
                    // If profile set to AES, let it pass without question.
                    // If profile set to TKIP, we must find one mateched
                    if ((pPort->PortCfg.WepStatus == DOT11_CIPHER_ALGO_TKIP) && 
                        (pPort->PortCfg.WepStatus != pInBss->WPA.PairCipher) && 
                        (pPort->PortCfg.WepStatus != pInBss->WPA.PairCipherAux) &&
                        (pPort->PortCfg.WepStatus != pInBss->WPA2.PairCipher) && 
                        (pPort->PortCfg.WepStatus != pInBss->WPA2.PairCipherAux))
                    {
                        continue;
                    }
                }
                else
                    continue;   // CCKM_ON but Cipher did no match
            }
            // Bss Type matched, SSID matched. 
            // We will check wepstatus for qualification Bss
            else if (pPort->PortCfg.WepStatus != pInBss->WepStatus)
                    continue;
            
            // copy matching BSS from InTab to OutTab
            PlatformMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));

            OutTab->BssNr++;
        }

        if (OutTab->BssNr >= MAX_LEN_OF_BSS_TABLE)
            break;
    }

    BssTableSortByRssi(pAd, OutTab);
    //Keep the WPS AP that we do WPS periously to the first entry, for dual band with two interface Atheros AP, we may connect with wrong interface.
    BssTableWPSAPFirst(pAd, pPort, OutTab); 
}

// IRQL = DISPATCH_LEVEL
VOID BssTableSortByRssi(
    IN PMP_ADAPTER pAd,
    IN OUT BSS_TABLE *OutTab) 
{
    INT       i, j;
    PBSS_ENTRY pTmpBss = NULL;

    PlatformAllocateMemory(pAd,  &pTmpBss, sizeof(BSS_ENTRY));
    if (pTmpBss == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s:Allocate memory failed\n", __FUNCTION__));
        return;
    }

    for (i = 0; i < OutTab->BssNr - 1; i++) 
    {
        for (j = i+1; j < OutTab->BssNr; j++) 
        {
            if (OutTab->BssEntry[j].Rssi > OutTab->BssEntry[i].Rssi) 
            {
                PlatformMoveMemory(pTmpBss, &OutTab->BssEntry[j], sizeof(BSS_ENTRY));
                PlatformMoveMemory(&OutTab->BssEntry[j], &OutTab->BssEntry[i], sizeof(BSS_ENTRY));
                PlatformMoveMemory(&OutTab->BssEntry[i], pTmpBss, sizeof(BSS_ENTRY));
            }           
        }
    }

    if (pTmpBss)
        PlatformFreeMemory(pTmpBss, sizeof(BSS_ENTRY));
}

VOID BssTableWPSAPFirst(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN OUT BSS_TABLE *OutTab) 
{
    INT       i;
    PBSS_ENTRY pTmpBss = NULL;
   PlatformAllocateMemory(pAd,  &pTmpBss, sizeof(BSS_ENTRY));

    if ((pTmpBss == NULL) || (pPort->StaCfg.WscControl.WscAPBssid == NULL))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s:Allocate memory failed or WscAPBssid is NULL\n", __FUNCTION__));
        return;
    }

    for (i = 0; i < OutTab->BssNr; i++) 
    {
        DBGPRINT(RT_DEBUG_INFO,("#%d BssTableWPSAPFirst-- AP..:%x:%x:%x, ssidlen:%d\n", i, 
            OutTab->BssEntry[i].Bssid[3], OutTab->BssEntry[i].Bssid[4], OutTab->BssEntry[i].Bssid[5], OutTab->BssEntry[i].SsidLen));
        if((PlatformEqualMemory(OutTab->BssEntry[i].Bssid, pPort->StaCfg.WscControl.WscAPBssid, MAC_ADDR_LEN)) &&
            (i != 0))
        {
            DBGPRINT(RT_DEBUG_TRACE, (" BssTableWPSAPFirst SWAP WPS AP first\n"));
            // switch if QbssLoad of BssEntry[j] is less than BssEntry[i]
            // and the gap should exceed a threshold for QbssLoad but not RSSI
            PlatformMoveMemory(pTmpBss, &OutTab->BssEntry[i], sizeof(BSS_ENTRY));
            PlatformMoveMemory(&OutTab->BssEntry[i], &OutTab->BssEntry[0], sizeof(BSS_ENTRY));
            PlatformMoveMemory(&OutTab->BssEntry[0], pTmpBss, sizeof(BSS_ENTRY));
        }
    }

    if (pTmpBss)
    {
        PlatformFreeMemory(pTmpBss, sizeof(BSS_ENTRY));
    }
}

extern  UCHAR   RSN_OUI[];      // in sanity.c

VOID BssCipherParse(
    IN OUT  PBSS_ENTRY  pBss)
{
    PEID_STRUCT                     pEid;
    PUCHAR                          pTmp;
    PRSN_IE_HEADER_STRUCT           pRsnHeader;
    PCIPHER_SUITE_STRUCT            pCipher;    
    USHORT                          Count;
    INT                             Length;
    INT                             IELength;
    ULONG                           TmpCipher;

    // Bit 0 -> IsWAPI(1) 
    // Bit 1 -> PSK
    // Bit 2 -> Cert
    // Bit 3 -> Unicast SMS4(1) 
    // Bit 4 -> Multicast SMS4(1)       
    ULONG                           WAPICipher = 0; 

    // Determine this is a WPS AP or not.
    WscCheckWPSAP(pBss);

    //
    // WepStatus will be reset later, if AP announce TKIP or AES on the beacon frame.
    //
    if (pBss->Privacy)
    {
        pBss->WepStatus     = DOT11_CIPHER_ALGO_WEP;
    }
    else
    {
        pBss->WepStatus     = DOT11_CIPHER_ALGO_NONE;
    }
    // Set default to disable & open authentication before parsing variable IE
    pBss->AuthMode      = DOT11_AUTH_ALGO_80211_OPEN;
    pBss->AuthModeAux   = DOT11_AUTH_ALGO_80211_OPEN;

    // init the MixedMode group cipher
    pBss->MixedModeGroupCipher = Cipher_Type_NONE;


    // Init WPA setting
    pBss->WPA.PairCipher    = DOT11_CIPHER_ALGO_NONE;
    pBss->WPA.PairCipherAux = DOT11_CIPHER_ALGO_NONE;
    pBss->WPA.GroupCipher   = DOT11_CIPHER_ALGO_NONE;
    pBss->WPA.RsnCapability = 0;
    pBss->WPA.bMixMode      = FALSE;

    // Init WPA2 setting
    pBss->WPA2.PairCipher    = DOT11_CIPHER_ALGO_NONE;
    pBss->WPA2.PairCipherAux = DOT11_CIPHER_ALGO_NONE;
    pBss->WPA2.GroupCipher   = DOT11_CIPHER_ALGO_NONE;
    pBss->WPA2.RsnCapability = 0;
    pBss->WPA2.bAKMwSHA256 = 0;

    pBss->WPA2.bMixMode      = FALSE;
    Length = (INT) pBss->VarIELen;

    while (Length > 0)
    {
        // Parse cipher suite base on WPA1 & WPA2, they should be parsed differently
        pTmp = ((PUCHAR) pBss->VarIEs) + pBss->VarIELen - Length;
        pEid = (PEID_STRUCT) pTmp;
        IELength = pEid->Len + 2; // IE & Len
        switch (pEid->Eid)
        {
            case IE_WPA:
                if (PlatformEqualMemory(pEid->Octet, SES_OUI, 3) && (pEid->Len == 7))
                {
                    pBss->bSES = TRUE;
                    break;
                }               
                else if (PlatformEqualMemory(pEid->Octet, WPA_OUI, 4) != 1)
                {
                    // if unsupported vendor specific IE
                    break;
                }
                // Skip OUI, version, and multicast suite
                // This part should be improved in the future when AP supported multiple cipher suite.
                // For now, it's OK since almost all APs have fixed cipher suite supported.
                // pTmp = (PUCHAR) pEid->Octet;

                //
                // Add sanity check that the IEs may not have larege size than 11.
                // For example: dd 06 00 50 f2 01 01 00
                // 
                if ((pEid->Len + 2) <= 11)  
                    break;              
                
                pTmp   += 11;
                IELength -= 11;

                // Cipher Suite Selectors from Spec P802.11i/D3.2 P26.
                //  Value      Meaning
                //  0           None 
                //  1           WEP-40
                //  2           Tkip
                //  3           WRAP
                //  4           AES
                //  5           WEP-104
                // Parse group cipher
                if (IELength > 0)
                {
                    switch (*pTmp)
                    {
                        case 1: 
                            pBss->WPA.GroupCipher = DOT11_CIPHER_ALGO_WEP40;
                            pBss->MixedModeGroupCipher = Cipher_Type_WEP40;
                            break;
                        case 5:
                            pBss->WPA.GroupCipher = DOT11_CIPHER_ALGO_WEP104;
                            pBss->MixedModeGroupCipher = Cipher_Type_WEP104;
                            break;
                        case 2:
                            pBss->WPA.GroupCipher = DOT11_CIPHER_ALGO_TKIP;
                            break;
                        case 4:
                            pBss->WPA.GroupCipher = DOT11_CIPHER_ALGO_CCMP;
                            break;
                        default:
                            break;
                    }
                }
                // number of unicast suite
                pTmp   += 1;
                IELength -= 1;

                // skip all unicast cipher suites
                Count = *(PUSHORT) pTmp;
                pTmp   += sizeof(USHORT);
                IELength -= sizeof(USHORT);

                // Parsing all unicast cipher suite
                while ((Count > 0) && (IELength > 0))
                {
                    // Skip OUI
                    pTmp += 3;
                    IELength -= 3;
                    TmpCipher = DOT11_CIPHER_ALGO_NONE;
                    switch (*pTmp)
                    {
                        case 1:
                        case 5: // for CCKM WEP only, (may use bSES)
                            TmpCipher = DOT11_CIPHER_ALGO_WEP;
                            break;
                        case 2:
                            TmpCipher = DOT11_CIPHER_ALGO_TKIP;
                            break;
                        case 4:
                            TmpCipher = DOT11_CIPHER_ALGO_CCMP;
                            break;
                        default:
                            break;
                    }
                    if (TmpCipher > pBss->WPA.PairCipher)
                    {
                        // Move the lower cipher suite to PairCipherAux
                        pBss->WPA.PairCipherAux = pBss->WPA.PairCipher;
                        pBss->WPA.PairCipher    = TmpCipher;
                    }
                    else
                    {
                        pBss->WPA.PairCipherAux = TmpCipher;
                    }
                    pTmp++;
                    Count--;
                    IELength--;
                }

                // 4. get AKM suite counts
                Count   = *(PUSHORT) pTmp;
                pTmp   += sizeof(USHORT);
                IELength -= sizeof(USHORT);

                // AKM suite, maybe more than 1, Cisco CCKM cases
                while ((Count > 0) && (IELength > 0))
                {
                    // Skip OUI
                    pTmp   += 3;
                    IELength -= 3;
                    Count --;

                    if (IELength > 0)
                    {
                        switch (*pTmp)
                        {
                            case 1:
                                // Set AP support WPA mode
                                if (pBss->AuthMode == DOT11_AUTH_ALGO_80211_OPEN)
                                    pBss->AuthMode = DOT11_AUTH_ALGO_WPA;
                                else
                                    pBss->AuthModeAux = DOT11_AUTH_ALGO_WPA;
                                break;
                            case 2:
                                // Set AP support WPA mode
                                if (pBss->AuthMode == DOT11_AUTH_ALGO_80211_OPEN)
                                    pBss->AuthMode = DOT11_AUTH_ALGO_WPA_PSK;
                                else
                                    pBss->AuthModeAux = DOT11_AUTH_ALGO_WPA_PSK;
                                break;
                            default:
                                break;
                        }
                    }
                    pTmp   += 1;
                    IELength -= 1;
                    // There maybe more than 1 AKM, Cisco CCKM cases
                    //break;    // only get the first one AKM
                }

                // Fixed for WPA-None
                if (pBss->BssType == BSS_ADHOC)
                {
                    pBss->AuthMode    = DOT11_AUTH_ALGO_WPA_NONE;
                    pBss->AuthModeAux = DOT11_AUTH_ALGO_WPA_NONE;
                    pBss->WepStatus   = pBss->WPA.GroupCipher;
                    // Patched bugs for old driver
                    if (pBss->WPA.PairCipherAux == DOT11_CIPHER_ALGO_NONE)
                        pBss->WPA.PairCipherAux = pBss->WPA.GroupCipher;
                }
                else
                    pBss->WepStatus   = pBss->WPA.PairCipher;

                // Check the Pair & Group, if different, turn on mixed mode flag
                if (pBss->WPA.GroupCipher != pBss->WPA.PairCipher)
                    pBss->WPA.bMixMode = TRUE;

                break;

            case IE_RSN:
                pRsnHeader = (PRSN_IE_HEADER_STRUCT) pTmp;
                
                // 0. Version must be 1
                if (pRsnHeader->Version != 1)
                    break;
                pTmp   += sizeof(RSN_IE_HEADER_STRUCT);
                IELength -= sizeof(RSN_IE_HEADER_STRUCT);

                // 1. Check group cipher
                pCipher = (PCIPHER_SUITE_STRUCT) pTmp;
                if (!PlatformEqualMemory(pTmp, RSN_OUI, 3))
                    break;

                if (IELength > 0)
                {
                    // Parse group cipher
                    switch (pCipher->Type)
                    {
                        case 1:
                            pBss->WPA2.GroupCipher = DOT11_CIPHER_ALGO_WEP40;
                            pBss->MixedModeGroupCipher = Cipher_Type_WEP40;
                            break;
                        case 5: // Although WEP is not allowed in WPA related auth mode, we parse it anyway
                            pBss->WPA2.GroupCipher = DOT11_CIPHER_ALGO_WEP104;
                            pBss->MixedModeGroupCipher = Cipher_Type_WEP104;
                            break;
                        case 2:
                            pBss->WPA2.GroupCipher = DOT11_CIPHER_ALGO_TKIP;
                            break;
                        case 4:
                            pBss->WPA2.GroupCipher = DOT11_CIPHER_ALGO_CCMP;
                            break;
                        default:
                            break;
                    }
                }
                // set to correct offset for next parsing
                pTmp   += sizeof(CIPHER_SUITE_STRUCT);
                IELength -= sizeof(CIPHER_SUITE_STRUCT);

                // 2. Get pairwise cipher counts
                Count = *(PUSHORT) pTmp;
                pTmp   += sizeof(USHORT);
                IELength -= sizeof(USHORT);
                
                // 3. Get pairwise cipher
                // Parsing all unicast cipher suite
                while ((Count > 0) && (IELength > 0))
                {
                    // Skip OUI
                    pCipher = (PCIPHER_SUITE_STRUCT) pTmp;
                    TmpCipher = DOT11_CIPHER_ALGO_NONE;
                    switch (pCipher->Type)
                    {
                        case 1:
                        case 5: // Although WEP is not allowed in WPA related auth mode, we parse it anyway
                            TmpCipher = DOT11_CIPHER_ALGO_WEP;
                            break;
                        case 2:
                            TmpCipher = DOT11_CIPHER_ALGO_TKIP;
                            break;
                        case 4:
                            TmpCipher = DOT11_CIPHER_ALGO_CCMP;
                            break;
                        default:
                            break;
                    }
                    if (TmpCipher > pBss->WPA2.PairCipher)
                    {
                        // Move the lower cipher suite to PairCipherAux
                        pBss->WPA2.PairCipherAux = pBss->WPA2.PairCipher;
                        pBss->WPA2.PairCipher    = TmpCipher;
                    }
                    else
                    {
                        pBss->WPA2.PairCipherAux = TmpCipher;
                    }
                    pTmp += sizeof(CIPHER_SUITE_STRUCT);
                    IELength -= sizeof(CIPHER_SUITE_STRUCT);
                    Count--;
                }
                
                // 4. get AKM suite counts
                Count   = *(PUSHORT) pTmp;
                pTmp   += sizeof(USHORT);
                IELength -= sizeof(USHORT);

                // AKM suite, maybe more than 1, Cisco CCKM cases
                while ((Count > 0) && (IELength > 0))
                {
                    if (!PlatformEqualMemory(pTmp, RSN_OUI, 3))
                    {
                        Count --;
                        pTmp +=4;
                        IELength -= 4;
                        continue;
                    }
                    else
                    {
                        Count --;
                        // Add RSN OUI
                        pTmp   += 3;
                        IELength -= 3;
                    }

                if (IELength > 0)
                {
                        //pAKM = (PAKM_SUITE_STRUCT) pTmp;

                        //switch (pAKM->Type)
                        switch(*pTmp)
                        {
                            case 1:
                                // Set AP support WPA mode
                                if (pBss->AuthMode == DOT11_AUTH_ALGO_80211_OPEN)
                                    pBss->AuthMode = DOT11_AUTH_ALGO_RSNA;
                                else
                                    pBss->AuthModeAux = DOT11_AUTH_ALGO_RSNA;
                                break;
                            case 2:
                                // Set AP support WPA mode
                                if (pBss->AuthMode == DOT11_AUTH_ALGO_80211_OPEN)
                                    pBss->AuthMode = DOT11_AUTH_ALGO_RSNA_PSK;
                                else
                                    pBss->AuthModeAux = DOT11_AUTH_ALGO_RSNA_PSK;
                                break;

                            case 5:
                                // 11w: Force 4-way to use SHA256 if AKM is 5 or 6
                                pBss->WPA2.bAKMwSHA256 = 1;
                                // Set AP support WPA2 mode
                                if (pBss->AuthMode == DOT11_AUTH_ALGO_80211_OPEN)
                                    pBss->AuthMode = DOT11_AUTH_ALGO_RSNA;
                                else
                                    pBss->AuthModeAux = DOT11_AUTH_ALGO_RSNA;
                                break;

                            case 6:
                                // 11w: Force 4-way to use SHA256 if AKM is 5 or 6
                                pBss->WPA2.bAKMwSHA256 = 1;
                                // Set AP support WPA2-PSK mode
                                if (pBss->AuthMode == DOT11_AUTH_ALGO_80211_OPEN)
                                    pBss->AuthMode = DOT11_AUTH_ALGO_RSNA_PSK;
                                else
                                    pBss->AuthModeAux = DOT11_AUTH_ALGO_RSNA_PSK;
                                break;

                            default:
                                break;
                        }
                    }
                    pTmp   += 1;
                    IELength -= 1;
                    // There maybe more than 1 AKM, Cisco CCKM cases
                    //break;    // only get the first one AKM
                }

//
// Vista Support DOT11_AUTH_ALGO_RSNA_PSK for Adhoc
//
#if 0
                // Fixed for WPA-None
                if (pBss->BssType == BSS_ADHOC)
                {
                    pBss->AuthMode = DOT11_AUTH_ALGO_WPA_NONE;
                    pBss->AuthModeAux = DOT11_AUTH_ALGO_WPA_NONE;
                    pBss->WPA.PairCipherAux = pBss->WPA2.PairCipherAux;
                    pBss->WPA.GroupCipher   = pBss->WPA2.GroupCipher;
                    pBss->WepStatus         = pBss->WPA.GroupCipher;
                    // Patched bugs for old driver
                    if (pBss->WPA.PairCipherAux == DOT11_CIPHER_ALGO_NONE)
                        pBss->WPA.PairCipherAux = pBss->WPA.GroupCipher;
                }
#endif              
                pBss->WepStatus   = pBss->WPA2.PairCipher;

                // 6. Get RSN capability
                pBss->WPA2.RsnCapability = *(PUSHORT) pTmp;
                pTmp += sizeof(USHORT);
                IELength -= sizeof(USHORT);

                // Check the Pair & Group, if different, turn on mixed mode flag
                if (pBss->WPA2.GroupCipher != pBss->WPA2.PairCipher)
                    pBss->WPA2.bMixMode = TRUE;

                break;

            default:
                break;
        }
        Length -= (pEid->Len + 2);
    }

}

// ===========================================================================================
// mac_table.c
// ===========================================================================================

/*! \brief generates a random mac address value for IBSS BSSID
 *  \param Addr the bssid location
 *  \return none
 *  \pre
 *  \post
 */
VOID MacAddrRandomBssid(
    IN PMP_ADAPTER pAd, 
    OUT PUCHAR pAddr) 
{
    INT i;

    for (i = 0; i < MAC_ADDR_LEN; i++) 
    {
        pAddr[i] = RandomByte(pAd);
    }

    pAddr[0] = (pAddr[0] & 0xfe) | 0x02;  // the first 2 bits must be 01xxxxxxxx
}

/*! \brief init the management mac frame header
 *  \param p_hdr mac header
 *  \param subtype subtype of the frame
 *  \param p_ds destination address, don't care if it is a broadcast address
 *  \return none
 *  \pre the station has the following information in the pAd->StaCfg
 *   - bssid
 *   - station address
 *  \post
 *  \note this function initializes the following field

 IRQL = PASSIVE_LEVEL
 IRQL = DISPATCH_LEVEL
  
 */
VOID MgtMacHeaderInit(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort, 
    IN OUT PHEADER_802_11 pHdr80211, 
    IN UCHAR SubType, 
    IN UCHAR ToDs, 
    IN PUCHAR pDA, 
    IN PUCHAR pBssid) 
{
    PlatformZeroMemory(pHdr80211, sizeof(HEADER_802_11));
    pHdr80211->FC.Type = BTYPE_MGMT;
    pHdr80211->FC.SubType = SubType;
    pHdr80211->FC.ToDs = ToDs;
    COPY_MAC_ADDR(pHdr80211->Addr1, pDA);
    COPY_MAC_ADDR(pHdr80211->Addr2, pPort->CurrentAddress);
    COPY_MAC_ADDR(pHdr80211->Addr3, pBssid);
}

// ===========================================================================================
// mem_mgmt.c
// ===========================================================================================

/*!***************************************************************************
 * This routine build an outgoing frame, and fill all information specified 
 * in argument list to the frame body. The actual frame size is the summation 
 * of all arguments.
 * input params:
 *      Buffer - pointer to a pre-allocated memory segment
 *      args - a list of <int arg_size, arg> pairs.
 *      NOTE NOTE NOTE!!!! the last argument must be NULL, otherwise this
 *                         function will FAIL!!!
 * return:
 *      Size of the buffer
 * usage:  
 *      MakeOutgoingFrame(Buffer, output_length, 2, &fc, 2, &dur, 6, p_addr1, 6,p_addr2, END_OF_ARGS);

 IRQL = PASSIVE_LEVEL
 IRQL = DISPATCH_LEVEL
  
 ****************************************************************************/
ULONG MakeOutgoingFrame(
    OUT CHAR *Buffer, 
    OUT ULONG *FrameLen, ...) 
{
    CHAR   *p;
    int     leng;
    ULONG   TotLeng;
    va_list Args;

    // calculates the total length
    TotLeng = 0;
    va_start(Args, FrameLen);
    do 
    {
        leng = va_arg(Args, int);
        if (leng == END_OF_ARGS) 
        {
            break;
        }
        p = va_arg(Args, PVOID);
        PlatformMoveMemory(&Buffer[TotLeng], p, leng);
        TotLeng = TotLeng + leng;
    } while(TRUE);

    va_end(Args); /* clean up */
    *FrameLen = TotLeng;
    return TotLeng;
}

// ===========================================================================================
// mlme_queue.c
// ===========================================================================================

/*! \brief  Initialize The MLME Queue, used by MLME Functions
 *  \param  *Queue     The MLME Queue
 *  \return Always     Return NDIS_STATE_SUCCESS in this implementation
 *  \pre
 *  \post
 *  \note   Because this is done only once (at the init stage), no need to be locked

 IRQL = PASSIVE_LEVEL
 
 */
NDIS_STATUS MlmeQueueInit(
    IN MLME_QUEUE *Queue) 
{
    INT i;

    NdisAllocateSpinLock(&Queue->Lock);

    Queue->Num  = 0;
    Queue->Head = 0;
    Queue->Tail = 0;

    for (i = 0; i < MAX_LEN_OF_MLME_QUEUE; i++) 
    {
        Queue->Entry[i].Occupied = FALSE;
        Queue->Entry[i].MsgLen = 0;
        PlatformZeroMemory(Queue->Entry[i].Msg, MGMT_DMA_BUFFER_SIZE);
    }

    return NDIS_STATUS_SUCCESS;
}


/*! \brief   Enqueue a message for other threads, if they want to send messages to MLME thread
 *  \param  *Queue    The MLME Queue
 *  \param   Machine  The State Machine Id
 *  \param   MsgType  The Message Type
 *  \param   MsgLen   The Message length
 *  \param  *Msg      The message pointer
 *  \return  TRUE if enqueue is successful, FALSE if the queue is full
 *  \pre
 *  \post
 *  \note    The message has to be initialized

 IRQL = PASSIVE_LEVEL
 IRQL = DISPATCH_LEVEL
  
 */
BOOLEAN MlmeEnqueue(
    IN  PMP_ADAPTER pAd,
    IN    PMP_PORT pPort,
    IN ULONG Machine, 
    IN ULONG MsgType, 
    IN ULONG MsgLen, 
    IN VOID *Msg)
{
    INT Tail;
    MLME_QUEUE  *Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;    

    // Do nothing if the driver is starting halt state.
    // This might happen when timer already been fired before cancel timer with mlmehalt
    if (    MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS) ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
    {
        DBGPRINT_ERR(("MlmeEnqueue: Flags changed : (%d, %d, %d, %d) \n", MT_TEST_FLAG(pAd, fRTMP_ADAPTER_MLME_RESET_IN_PROGRESS), MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS), MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS), MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));
        return FALSE;
    }

    // First check the size, it MUST not exceed the mlme queue size
    if (MsgLen > MGMT_DMA_BUFFER_SIZE)
    {
        DBGPRINT_ERR(("MlmeEnqueue: msg too large, size = %d \n", MsgLen));
        return FALSE;
    }
    
    if (MlmeQueueFull(Queue)) 
    {
        DBGPRINT_ERR(("MlmeEnqueue: full, msg dropped and may corrupt MLME. Num=%d,Head=%d,Tail=%d,Machine=%d,MsgType=%d,MsgLen=%d\n", 
                Queue->Num, Queue->Head, Queue->Tail, Machine, MsgType, MsgLen));
        return FALSE;
    }

    NdisAcquireSpinLock(&(Queue->Lock));
    Tail = Queue->Tail;
    Queue->Tail++;
    Queue->Num++;
    if (Queue->Tail == MAX_LEN_OF_MLME_QUEUE) 
    {
        Queue->Tail = 0;
    }

    Queue->Entry[Tail].Occupied = TRUE;
    Queue->Entry[Tail].Machine = Machine;
    Queue->Entry[Tail].MsgType = MsgType;
    Queue->Entry[Tail].MsgLen  = MsgLen;
    Queue->Entry[Tail].PortNum =pPort->PortNumber;
    PlatformMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);
    NdisReleaseSpinLock(&(Queue->Lock));

    MlmeHandler(pAd);
    return TRUE;
}

/*! \brief   This function is used when Recv gets a MLME message
 *  \param  *Queue           The MLME Queue
 *  \param   TimeStampHigh   The upper 32 bit of timestamp
 *  \param   TimeStampLow    The lower 32 bit of timestamp
 *  \param   Rssi            The receiving RSSI strength
 *  \param   MsgLen          The length of the message
 *  \param  *Msg             The message pointer
 *  \return  TRUE if everything ok, FALSE otherwise (like Queue Full)
 *  \pre
 *  \post
 
 IRQL = DISPATCH_LEVEL
 
 */
BOOLEAN MlmeEnqueueForRecv(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort, 
    IN ULONG Wcid, 
    IN ULONG TimeStampHigh,
    IN ULONG TimeStampLow, 
    IN UCHAR Rssi0, 
    IN UCHAR Rssi1, 
    IN UCHAR Rssi2, 
    IN UCHAR AntSel,
    IN ULONG MsgLen, 
    IN VOID *Msg) 
{
    INT          Tail, Machine;
    PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;
    ULONG        MsgType;
    MLME_QUEUE  *Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;
    UCHAR       PhysHdlIdx = 0xff;  
    BOOLEAN     ReceiveToBTHS = FALSE;

    // Do nothing if the driver is starting halt state.
    // This might happen when timer already been fired before cancel timer with mlmehalt
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_MLME_RESET_IN_PROGRESS) ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS) ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
    {
        DBGPRINT_ERR(("MlmeEnqueueForRecv: Flags changed : (%d, %d, %d, %d) \n", MT_TEST_FLAG(pAd, fRTMP_ADAPTER_MLME_RESET_IN_PROGRESS), MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS), MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS), MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));
        return FALSE;
    }

    // First check the size, it MUST not exceed the mlme queue size
    if (MsgLen > MGMT_DMA_BUFFER_SIZE)
    {
        DBGPRINT_ERR(("%s: frame too large, size = %d \n", __FUNCTION__, MsgLen));
        return FALSE;
    }

    if (MlmeQueueFull(Queue)) 
    {
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hMLMEThread)));  
#if DBG     
        if((pPort->PortSubtype == PORTSUBTYPE_SoftapAP) || (pPort->PortSubtype == PORTSUBTYPE_VwifiAP) || (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
        {
            if (!ApMlmeMsgTypeSubst(pAd, pPort, pFrame, &Machine, &MsgType)) 
            {
                DBGPRINT_ERR(("%s: MlmeQueueFull mgmt->subtype=%d\n", __FUNCTION__, pFrame->Hdr.FC.SubType));
                return FALSE;
            }
        }
        else
        {
            if (!MsgTypeSubst(pAd, pPort, pFrame, &Machine, &MsgType)) 
            {
                DBGPRINT_ERR(("%s: MlmeQueueFull mgmt->subtype=%d\n", __FUNCTION__, pFrame->Hdr.FC.SubType));
                return FALSE;
            }
        }
#endif
        return FALSE;
    }

    if ((pPort->PortSubtype == PORTSUBTYPE_SoftapAP) || (pPort->PortSubtype == PORTSUBTYPE_VwifiAP) || (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
    {
        // Patched: UI create Port1 and Port2 at the same time ??
        // When GO is active, don't allow SoftAP and VwifiAP to enter AP mlme statemachine
        if ((pPort->P2PCfg.bGOStart == TRUE) && (pPort->PortSubtype != PORTSUBTYPE_P2PGO))
            return FALSE;
        if ((pPort->SoftAP.bAPStart == TRUE) && (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
            return FALSE;

        if (!ApMlmeMsgTypeSubst(pAd, pPort, pFrame, &Machine, &MsgType)) 
        {
            //DBGPRINT_ERR(("%s: un-recongnized mgmt->subtype=%d\n", __FUNCTION__, pFrame->Hdr.FC.SubType));
            return FALSE;
        }

        // if the kind of SoftAp didn't start, don't enqueue beacon and probe request. it will cause queue full. Especially, the probe request needs softap to response.
        if(MsgType == APMT2_PEER_PROBE_REQ)
        {
            if (((pPort->PortSubtype == PORTSUBTYPE_SoftapAP) || (pPort->PortSubtype == PORTSUBTYPE_VwifiAP) && (!pPort->SoftAP.bAPStart)) || ((pPort->PortSubtype == PORTSUBTYPE_P2PGO) && (!pPort->P2PCfg.bGOStart)))
                return FALSE;
        }
        else if((pFrame->Hdr.FC.Type == 0) && ((pFrame->Hdr.FC.SubType == 8)))
        {
            //DBGPRINT(RT_DEBUG_TRACE, ("drop beacon"));
            return FALSE;
        }
    }
    else
    {
        if (!MsgTypeSubst(pAd, pPort, pFrame, &Machine, &MsgType)) 
        {
            DBGPRINT_ERR(("%s: un-recongnized mgmt->subtype=%d\n", __FUNCTION__, pFrame->Hdr.FC.SubType));
            return FALSE;
        }

        if(pPort->PortNumber == PORT_0 && MsgType == MT2_PEER_PROBE_REQ)
            return FALSE;
    }

    // OK, we got all the informations, it is time to put things into queue
    NdisAcquireSpinLock(&(Queue->Lock));
    Tail = Queue->Tail;
    Queue->Tail++;
    Queue->Num++;
    if (Queue->Tail == MAX_LEN_OF_MLME_QUEUE) 
    {
        Queue->Tail = 0;
    }

    Queue->Entry[Tail].Occupied = TRUE;
    Queue->Entry[Tail].Machine = Machine;
    Queue->Entry[Tail].MsgType = MsgType;
    Queue->Entry[Tail].MsgLen  = MsgLen;
    Queue->Entry[Tail].Rssi = Rssi0;
    Queue->Entry[Tail].Rssi2 = Rssi1;
    Queue->Entry[Tail].Rssi3 = Rssi2;   
    Queue->Entry[Tail].AntSel = AntSel;
    Queue->Entry[Tail].Channel = pAd->HwCfg.LatchRfRegs.Channel;
    Queue->Entry[Tail].Wcid = (UCHAR)Wcid;
    if(ReceiveToBTHS)
    {
        Queue->Entry[Tail].PortNum = PORT_0;    //BTHS always use STA port.
    }
    else if ((pPort->PortNumber == pPort->P2PCfg.PortNumber) && (pPort->P2PCfg.PortNumber != PORT_0) &&
            (Machine == SYNC_STATE_MACHINE) && (MsgType == MT2_PEER_PROBE_RSP) &&
            (pAd->PortList[PORT_0]->Mlme.SyncMachine.CurrState != SYNC_IDLE) && (pAd->PortList[pPort->P2PCfg.PortNumber]->Mlme.SyncMachine.CurrState == SYNC_IDLE))
    {
        // Let Port 0 handles P2P Probe Response when Port 0 is doing p2p scan.
        Queue->Entry[Tail].PortNum = PORT_0;
        DBGPRINT(RT_DEBUG_TRACE, ("%s:: Machine(%d),  MsgType(%d), Channel(%d), Wcid(%d), OrgPortNum(%d)-->QPortNum(%d)\n",
                                    __FUNCTION__,
                                    Queue->Entry[Tail].Machine,
                                    Queue->Entry[Tail].MsgType,
                                    Queue->Entry[Tail].Channel,
                                    Queue->Entry[Tail].Wcid,
                                    pPort->PortNumber,
                                    Queue->Entry[Tail].PortNum));
    }
    else
    {
        Queue->Entry[Tail].PortNum =pPort->PortNumber;
    }
    PlatformMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);

    NdisReleaseSpinLock(&(Queue->Lock));

    NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hMLMEThread)));  

    return TRUE;
}

/*! \brief   Dequeue a message from the MLME Queue
 *  \param  *Queue    The MLME Queue
 *  \param  *Elem     The message dequeued from MLME Queue
 *  \return  TRUE if the Elem contains something, FALSE otherwise
 *  \pre
 *  \post

 IRQL = DISPATCH_LEVEL

 */
BOOLEAN MlmeDequeue(
    IN MLME_QUEUE *Queue, 
    OUT MLME_QUEUE_ELEM **Elem) 
{
    NdisAcquireSpinLock(&(Queue->Lock));
    *Elem = &(Queue->Entry[Queue->Head]);
    Queue->Num--;
    Queue->Head++;
    if (Queue->Head == MAX_LEN_OF_MLME_QUEUE) 
    {
        Queue->Head = 0;
    }
    NdisReleaseSpinLock(&(Queue->Lock));

    return TRUE;
}

BOOLEAN MlmeWaitForMlmeHandlerStop(
    IN  PMP_ADAPTER   pAd
    )
{
    while(!MlmeQueueEmpty(&pAd->Mlme.Queue))
    {
        NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
        if( pAd->Mlme.bRunning  == FALSE)   
        {
            NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
            break;
        }
        NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
        Delay_us(100);
    }   
    return TRUE;
}

// IRQL = DISPATCH_LEVEL
VOID    MlmeRestartStateMachine(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort
    )
{
    MLME_QUEUE_ELEM *Elem = NULL;
    BOOLEAN             Cancelled =FALSE;
    UCHAR               i;
    NDIS_STATUS         ndisStatus = NDIS_STATUS_SUCCESS;   
    PUCHAR              TmpMlmeQueue = NULL;
    ULONG               Value = 0;
    UCHAR               ByteValue = 0;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;   


    DBGPRINT(RT_DEBUG_TRACE, ("MlmeRestartStateMachine  PortNum %d\n", pPort->PortNumber));
    
    NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
    if(pAd->Mlme.bRunning) 
    {
        NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeRestartStateMachine Running Wait for mlme Handler Stop\n"));
        
        
        if(MlmeWaitForMlmeHandlerStop(pAd))
        {
            NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
            pAd->Mlme.bRunning = TRUE;  
        }
    } 
    else 
    {
        pAd->Mlme.bRunning = TRUE;
    }
    NdisReleaseSpinLock(&pAd->Mlme.TaskLock);   

    MT_SET_FLAG(pAd, fRTMP_ADAPTER_MLME_RESET_IN_PROGRESS);

    // Remove Mlme queues elements
    // 1. We only consider to keep BTHS element when the port is 1 since the test case is only occurred in port 1.
    // 2. When we are in p2p concurrent mode, we should not clear mlme queues from the other port.
    // 3. When called by non-ndis-default port, we should not clear mlme queues on the this port.
    if(pPort->PortNumber > NDIS_DEFAULT_PORT_NUMBER)        
    {
        //InfoReset with port 1 may clean BTHS element in MlmeQueue to fail the BTHS connection at the moment enabling Wifi card.
        //We use a TmpMlmeQueue to keep the BTHS element in MlmeQueue and insert to MlmeQueue, ie. We don't clean BTHS element in MlmeQueue when inforeset with port 1.
        if(!MlmeQueueEmpty(&pAd->Mlme.Queue))
        {
            ULONG   Num = pAd->Mlme.Queue.Num;
            ULONG   Tail = 0;

            DBGPRINT(RT_DEBUG_TRACE, ("MlmeRestartStateMachine:Allocate %d MLME_QUEUE_ELEM for TmpMlmeQueue\n", pAd->Mlme.Queue.Num));
          PlatformAllocateMemory(pAd,  &TmpMlmeQueue, sizeof(MLME_QUEUE_ELEM)*pAd->Mlme.Queue.Num);
            if(TmpMlmeQueue != NULL)
            {
                while (!MlmeQueueEmpty(&pAd->Mlme.Queue)) 
                {
                    DbgPrint(" %s, %d,   %d",__FUNCTION__, __LINE__, pAd->Mlme.Queue.Num);
                    //From message type, determine which state machine I should drive
                    if (MlmeDequeue(&pAd->Mlme.Queue, &Elem)) 
                    {
                        if (Elem->PortNum != pPort->PortNumber)
                        {
                            if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
                                break;
                            
                            if(Tail < Num)
                            {
                                ((PMLME_QUEUE_ELEM)TmpMlmeQueue)[Tail].Occupied = TRUE;
                                ((PMLME_QUEUE_ELEM)TmpMlmeQueue)[Tail].Machine = Elem->Machine;
                                ((PMLME_QUEUE_ELEM)TmpMlmeQueue)[Tail].MsgType = Elem->MsgType;
                                ((PMLME_QUEUE_ELEM)TmpMlmeQueue)[Tail].MsgLen  = Elem->MsgLen;
                                
                                // Keep mlme queues from the other port and re-enqueue to MlME state machine later
                                ((PMLME_QUEUE_ELEM)TmpMlmeQueue)[Tail].PortNum = Elem->PortNum;
                                PlatformMoveMemory(((PMLME_QUEUE_ELEM)TmpMlmeQueue)[Tail].Msg, Elem->Msg, Elem->MsgLen);
                                Tail++;
                            }
                        }
                        
                        Elem->Occupied = FALSE;
                        Elem->MsgLen = 0;
                    }
                }
                
                if(Tail > 0)
                {
                    ULONG i;
                    for(i = 0 ; i < Tail; i++)
                    {   
                        DbgPrint(" %s, %d,   %d",__FUNCTION__, __LINE__, pAd->Mlme.Queue.Num);
                        MlmeEnqueue(pAd,pPort, 
                                        ((PMLME_QUEUE_ELEM)TmpMlmeQueue)[i].Machine, 
                                        ((PMLME_QUEUE_ELEM)TmpMlmeQueue)[i].MsgType,
                                        ((PMLME_QUEUE_ELEM)TmpMlmeQueue)[i].MsgLen,
                                        ((PMLME_QUEUE_ELEM)TmpMlmeQueue)[i].Msg);
                    }
                }
                PlatformFreeMemory(TmpMlmeQueue, sizeof(MLME_QUEUE_ELEM)*pAd->Mlme.Queue.Num);
            }
            else
            {
                while (!MlmeQueueEmpty(&pAd->Mlme.Queue)) 
                {
                    DbgPrint(" %s, %d,   %d",__FUNCTION__, __LINE__, pAd->Mlme.Queue.Num);
                    //From message type, determine which state machine I should drive
                    if (MlmeDequeue(&pAd->Mlme.Queue, &Elem)) 
                    {
                        // free MLME element
                        Elem->Occupied = FALSE;
                        Elem->MsgLen = 0;

                    }
                    else 
                    {
                        DBGPRINT_ERR(("MlmeRestartStateMachine: MlmeQueue empty\n"));
                    }
                }
            }
        }
    }
    else
    {
        while (!MlmeQueueEmpty(&pAd->Mlme.Queue)) 
        {
            DbgPrint(" %s, %d,   %d",__FUNCTION__, __LINE__, pAd->Mlme.Queue.Num);
            //From message type, determine which state machine I should drive
            if (MlmeDequeue(&pAd->Mlme.Queue, &Elem)) 
            {
                // free MLME element
                Elem->Occupied = FALSE;
                Elem->MsgLen = 0;
            }
            else 
            {
                DBGPRINT_ERR(("MlmeRestartStateMachine: MlmeQueue empty\n"));
            }
        }
    }
    
    ////////////////////////////////////////////////////////////////
    // Cancel all timer events
    //
    // 1. Can reset connection related timers on default port and client port
    if ((pPort->PortNumber == NDIS_DEFAULT_PORT_NUMBER) || (pPort->PortType == WFD_CLIENT_PORT))
    {
        PlatformCancelTimer(&pPort->Mlme.AssocTimer,     &Cancelled);
        PlatformCancelTimer(&pPort->Mlme.ReassocTimer,   &Cancelled);
        PlatformCancelTimer(&pPort->Mlme.DisassocTimer,  &Cancelled);
        PlatformCancelTimer(&pPort->Mlme.AuthTimer,       &Cancelled);
        PlatformCancelTimer(&pPort->Mlme.BeaconTimer,     &Cancelled);

        if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_JOIN_IN_PROGRESS))
            MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_JOIN_IN_PROGRESS);//steven:for test
        if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_REASSOC_IN_PROGRESS))
            MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_REASSOC_IN_PROGRESS);//steven:for test

        if (pPort->PortNumber == NDIS_DEFAULT_PORT_NUMBER) 
        {
            // 2. ALWAYS USE PORT_0 DO SCAN/DLS/TDLS/WAPI

            // Don't reset scan timer if it's not from Port 0
            PlatformCancelTimer(&pPort->Mlme.ScanTimer, &Cancelled); // 2012.03.16 Cancel Scan Timer

            PlatformIndicateScanStatus(pAd, pPort, NDIS_STATUS_REQUEST_ABORTED, TRUE, TRUE);
            
            if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
                MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);//steven:for test

            for (i = 0; i < MAX_NUM_OF_DLS_ENTRY; i++)
            {
                PlatformCancelTimer(&pPort->StaCfg.DLSEntry[i].Timer, &Cancelled);
            }
            for (i=0; i<MAX_NUM_OF_TDLS_ENTRY; i++)
            {
                PlatformCancelTimer(&pPort->StaCfg.TDLSEntry[i].Timer, &Cancelled);
            }   
        }
    }

    if (pPort->PortNumber == NDIS_DEFAULT_PORT_NUMBER) 
    {
        if (pPort->SoftAP.bAPStart && (pPort->PortSubtype != PORTSUBTYPE_P2PGO))
        {
            if ((pPort->CommonCfg.PhyMode == PHY_11VHT)
                &&(pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelWidth==VHT_BW_80))
            {
                UCHAR PrimaryChannelLocation;
                //AsicSwitchChannel(pAd, pPort->CentralChannel, FALSE);
                //AsicSwitchChannel(pAd, pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1, FALSE);

                AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
                //
                // retrive primary channel offset
                //
                GetPrimaryChannelLocation(
                        pAd,
                        pPort->Channel,
                        BW_80,
                        pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1,
                        &PrimaryChannelLocation
                        );
                
                SwitchBandwidth(pAd, FALSE, pPort->CentralChannel, BW_80, PrimaryChannelLocation);

                DBGPRINT(RT_DEBUG_TRACE,("%s, channel switching to Primary channel = %d, bandwidth = %d, Central channel = %d, PrimaryLocation = %d\n",
                        __FUNCTION__,
                        pPort->Channel,
                        BW_80,
                        pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1,
                        PrimaryChannelLocation
                        ));
            }
            else if (pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
            {
                //AsicSwitchChannel(pAd, pPort->CentralChannel, FALSE);
                AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);

                if (pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset == EXTCHA_ABOVE)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("40MHz  Extended channel above. Channel = %d\n", pPort->Channel));

                    SwitchBandwidth(pAd, FALSE, pPort->CentralChannel, BW_40, EXTCHA_ABOVE);
                }
                else if (pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset == EXTCHA_BELOW)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("40MHz  Extended channel below. Channel = %d\n", pPort->Channel));

                    SwitchBandwidth(pAd, FALSE, pPort->CentralChannel, BW_40, EXTCHA_BELOW);
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("40MHz  Extended channel ERROR!!! Channel = %d\n", pPort->Channel));
                }
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("20MHz  . Channel = %d\n", pPort->Channel));
                //AsicSwitchChannel(pAd, pPort->Channel, FALSE);
                AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
            }

            //During scan, beacon will be disabled, need to enabled here.
            AsicResumeBssSync(pAd);
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("pPort->BBPCurrentBW = %d (init from SetHtVHT)\n", pPort->BBPCurrentBW));  
#if 0
            if (pPort->BBPCurrentBW == BW_80)
            {
                AsicSwitchChannel(pAd, pPort->CentralChannel, FALSE);

                BbSetTxRxBwCtrl(pAd, BW_80);
            }
            else if (pPort->BBPCurrentBW == BW_40)
            {
                AsicSwitchChannel(pAd, pPort->CentralChannel, FALSE);

                BbSetTxRxBwCtrl(pAd, BW_40);
            }
            else
            {
                AsicSwitchChannel(pAd, pPort->Channel, FALSE);
            }
#endif
            AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
        }

        // Port0 scan confirm
        if ((pAd->MlmeAux.bNeedPlatformIndicateScanStatus) && (pAd->ucScanPortNum == pPort->PortNumber))
        {
            PMP_PORT pIndicatePort = pAd->PortList[pAd->ucScanPortNum];

            //
            // Report Scan fail to let UI not to update BSS list.
            // Based on whql test's scan_cwn.c, we should use NDIS_STATUS_REQUEST_ABORTED;
            //
            // Avoid PnP too fast will cause crash~
            PlatformIndicateScanStatus(pAd, pIndicatePort, NDIS_STATUS_REQUEST_ABORTED, TRUE, TRUE);
        }

#if 0       
        // Port0 scan confirm
        if ((pAd->MlmeAux.bQueueScan) && (pAd->ucQueueScanPortNum == pPort->PortNumber))
        {
            pAd->MlmeAux.bQueueScan = FALSE;
            ndisStatus = NDIS_STATUS_REQUEST_ABORTED;
            PlatformIndicateDot11Status(pAd,
                pAd->PortList[pAd->ucQueueScanPortNum],
                NDIS_STATUS_DOT11_SCAN_CONFIRM,
                pAd->pNicCfg->ScanRequestID,
                &ndisStatus,
                sizeof(NDIS_STATUS));
            DBGPRINT(RT_DEBUG_TRACE, ("MlmeRestartStateMachine, Indicate Queued scan confirm. scanInProgress:%d\n", MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)));
        }
#endif

        // init to FastRoaming scan
        if (pAd->StaCfg.bImprovedScan == FALSE)
        pAd->StaCfg.bFastRoamingScan = FALSE;

        pAd->MlmeAux.bActiveScanForHiddenAP = FALSE; // Reset to default
        pAd->MlmeAux.bRetainBSSEntriesInScanTable = FALSE;

        // Resume MSDU which is turned off durning scan
        XmitResumeMsduTransmission(pAd);
    }

    //DbgPrint(" %s, %d",__FUNCTION__, __LINE__);
    // Set all state machines back IDLE
    pPort->Mlme.CntlMachine.CurrState    = CNTL_IDLE;
    pPort->Mlme.AssocMachine.CurrState   = ASSOC_IDLE;
    pPort->Mlme.AuthMachine.CurrState    = AUTH_REQ_IDLE;
    pPort->Mlme.AuthRspMachine.CurrState = AUTH_RSP_IDLE;
    pPort->Mlme.SyncMachine.CurrState    = SYNC_IDLE;
    pPort->Mlme.DlsMachine.CurrState    = DLS_IDLE;
    pPort->Mlme.ActMachine.CurrState    = ACT_IDLE;
    pPort->Mlme.TdlsMachine.CurrState   = TDLS_IDLE;
    pPort->Mlme.PmfMachine.CurrState    = PMF_IDLE;
    pPort->Mlme.P2PMachine.CurrState    = P2P_IDLE_STATE;   

    MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_MLME_RESET_IN_PROGRESS);


    if (pPort->P2PCfg.P2PChannelState != P2P_DIFF_CHANNEL_OFF)
    {
        P2pStopNoA(pAd, NULL);
        pPort->P2PCfg.QueuedScanReq.ScanType = 0;
        pPort->P2PCfg.P2PChannelState = P2P_DIFF_CHANNEL_OFF;
    }
    
    // Remove running state
    NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
    pAd->Mlme.bRunning = FALSE;
    NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
}

/*! \brief  test if the MLME Queue is empty
 *  \param  *Queue    The MLME Queue
 *  \return TRUE if the Queue is empty, FALSE otherwise
 *  \pre
 *  \post
 
 IRQL = DISPATCH_LEVEL
 
 */
BOOLEAN MlmeQueueEmpty(
    IN MLME_QUEUE *Queue) 
{
    BOOLEAN Ans;

    NdisAcquireSpinLock(&(Queue->Lock));
    Ans = (Queue->Num == 0);
    NdisReleaseSpinLock(&(Queue->Lock));

    return Ans;
}

/*! \brief   test if the MLME Queue is full
 *  \param   *Queue      The MLME Queue
 *  \return  TRUE if the Queue is empty, FALSE otherwise
 *  \pre
 *  \post

 IRQL = PASSIVE_LEVEL
 IRQL = DISPATCH_LEVEL
  
 */
BOOLEAN MlmeQueueFull(
    IN MLME_QUEUE *Queue) 
{
    BOOLEAN Ans;

    NdisAcquireSpinLock(&(Queue->Lock));
    Ans = ((Queue->Num == MAX_LEN_OF_MLME_QUEUE) || (Queue->Entry[Queue->Tail].Occupied == TRUE));
    NdisReleaseSpinLock(&(Queue->Lock));

    return Ans;
}

/*! \brief   The destructor of MLME Queue
 *  \param 
 *  \return
 *  \pre
 *  \post
 *  \note   Clear Mlme Queue, Set Queue->Num to Zero.

 IRQL = PASSIVE_LEVEL
 
 */
VOID MlmeQueueDestroy(
    IN MLME_QUEUE *pQueue) 
{
    NdisAcquireSpinLock(&(pQueue->Lock));
    pQueue->Num  = 0;
    pQueue->Head = 0;
    pQueue->Tail = 0;   
    NdisReleaseSpinLock(&(pQueue->Lock));
    NdisFreeSpinLock(&(pQueue->Lock));
}

/*! \brief   To substitute the message type if the message is coming from external
 *  \param  pFrame         The frame received
 *  \param  *Machine       The state machine
 *  \param  *MsgType       the message type for the state machine
 *  \return TRUE if the substitution is successful, FALSE otherwise
 *  \pre
 *  \post
 
 IRQL = DISPATCH_LEVEL
 
 */
BOOLEAN MsgTypeSubst(
    IN PMP_ADAPTER  pAd,
    IN  PMP_PORT      pPort,
    IN PFRAME_802_11 pFrame, 
    OUT INT *Machine, 
    OUT INT *MsgType) 
{
    USHORT  Seq;
    USHORT  ReasonCode;
    UCHAR   EAPType, EAPCode;
    PUCHAR  pData;

    // Pointer to start of data frames including SNAP header
    pData = (PUCHAR) pFrame + LENGTH_802_11;

    // The only data type will pass to this function is EAPOL frame
    if (pFrame->Hdr.FC.Type == BTYPE_DATA) 
    {    
        // TDLS only for None and RSN (802.11z/D5.0 @8.5.9)
        if (((pPort->PortCfg.AuthMode == Ralink802_11AuthModeOpen) && (pPort->PortCfg.WepStatus == Ralink802_11EncryptionDisabled)) || 
            (((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) || (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK)) && 
             ((pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) || (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled))))
        {
            if (TDLS_ON(pAd) &&(PlatformEqualMemory(TDLS_LLC_SNAP_WITH_CATEGORY, pData, LENGTH_802_1_H + 2)))
            {
                // ieee802.11z TDLS SNAP header
                *Machine = TDLS_STATE_MACHINE;
                *MsgType = MT2_PEER_TDLS_MSG;
                return (TRUE);          
            }
        }
            
        // check for WSC state machine first
        if (pPort->StaCfg.WscControl.WscState >= WSC_STATE_LINK_UP)
        {
            *Machine = WSC_STATE_MACHINE;
            EAPType = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 1);
            EAPCode = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 4);
            return(WscMsgTypeSubst(EAPType, EAPCode, MsgType));
        }   
        else
        {
            *Machine = WPA_PSK_STATE_MACHINE;
            EAPType = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 1);
            return(WpaMsgTypeSubst(EAPType, MsgType));
        }
    }

    if (pFrame->Hdr.FC.Type != BTYPE_MGMT)
        return FALSE;

    switch (pFrame->Hdr.FC.SubType) 
    {
        case SUBTYPE_ASSOC_REQ:
            *Machine = ASSOC_STATE_MACHINE;
            *MsgType = MT2_PEER_ASSOC_REQ;
            break;
        case SUBTYPE_ASSOC_RSP:
            *Machine = ASSOC_STATE_MACHINE;
            *MsgType = MT2_PEER_ASSOC_RSP;
            break;
        case SUBTYPE_REASSOC_REQ:
            *Machine = ASSOC_STATE_MACHINE;
            *MsgType = MT2_PEER_REASSOC_REQ;
            break;
        case SUBTYPE_REASSOC_RSP:
            *Machine = ASSOC_STATE_MACHINE;
            *MsgType = MT2_PEER_REASSOC_RSP;
            break;
        case SUBTYPE_PROBE_REQ:
            *Machine = SYNC_STATE_MACHINE;
            *MsgType = MT2_PEER_PROBE_REQ;
            break;
        case SUBTYPE_PROBE_RSP:
            *Machine = SYNC_STATE_MACHINE;
            *MsgType = MT2_PEER_PROBE_RSP;
            break;
        case SUBTYPE_BEACON:
            *Machine = SYNC_STATE_MACHINE;
            *MsgType = MT2_PEER_BEACON;
            break;
        case SUBTYPE_ATIM:
            *Machine = SYNC_STATE_MACHINE;
            *MsgType = MT2_PEER_ATIM;
            break;
        case SUBTYPE_DISASSOC:
            PlatformMoveMemory(&ReasonCode, &pFrame->Octet[0], sizeof(USHORT));
            if(MFP_ACT(pAd, pPort) && !(pFrame->Hdr.FC.Wep) && !(pFrame->Hdr.Addr1[0]& 0x01) &&(ReasonCode == 6 || ReasonCode == 7))
            {
                *Machine = STA_PMF_STATE_MACHINE;
                *MsgType = PMF_MLME_SAQUERY_REQ;
            }
            else
            {
            *Machine = ASSOC_STATE_MACHINE;
            *MsgType = MT2_PEER_DISASSOC_REQ;
            }
            break;
        case SUBTYPE_AUTH:
            // get the sequence number from payload 24 Mac Header + 2 bytes algorithm
            PlatformMoveMemory(&Seq, &pFrame->Octet[2], sizeof(USHORT));
            if (Seq == 1 || Seq == 3) 
            {
                *Machine = AUTH_RSP_STATE_MACHINE;
                *MsgType = MT2_PEER_AUTH_ODD;
            } 
            else if (Seq == 2 || Seq == 4) 
            {
                *Machine = AUTH_STATE_MACHINE;
                *MsgType = MT2_PEER_AUTH_EVEN;
            } 
            else 
            {
                return FALSE;
            }
            break;
        case SUBTYPE_DEAUTH:
            PlatformMoveMemory(&ReasonCode, &pFrame->Octet[0], sizeof(USHORT));
            if(MFP_ACT(pAd, pPort) && !(pFrame->Hdr.FC.Wep) && !(pFrame->Hdr.Addr1[0]& 0x01) && (ReasonCode == 6 || ReasonCode == 7))
            {
                *Machine = STA_PMF_STATE_MACHINE;
                *MsgType = PMF_MLME_SAQUERY_REQ;
            }
            else
            {
            *Machine = AUTH_RSP_STATE_MACHINE;
            *MsgType = MT2_PEER_DEAUTH;
            }
            break;
        case SUBTYPE_ACTION:
        case SUBTYPE_ACTION_NO_ACK:
            if (pFrame->Octet[0] == CATEGORY_DLS)
            {
                if (pFrame->Octet[1] == ACTION_DLS_REQUEST)
                {
                    *Machine = DLS_STATE_MACHINE;
                    *MsgType = MT2_PEER_DLS_REQ;
                }
                else if (pFrame->Octet[1] == ACTION_DLS_RESPONSE)
                {
                    *Machine = DLS_STATE_MACHINE;
                    *MsgType = MT2_PEER_DLS_RSP;
                }
                else if (pFrame->Octet[1] == ACTION_DLS_TEARDOWN)
                {
                    *Machine = DLS_STATE_MACHINE;
                    *MsgType = MT2_PEER_DLS_TEAR_DOWN;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE,("CATEGORY_DLS not support(action=%d)\n",pFrame->Octet[1]));
                    return FALSE;
                }
                return TRUE;
            }           
            else if (pFrame->Octet[0] == CATEGORY_SAQUERY)
            {
                if (pFrame->Octet[1] == PMF_ACTION_SAQUERY_REQ) // Get SAQuery Request, should enter RSP state to Ans the req
                {
                    *Machine = STA_PMF_STATE_MACHINE;
                    *MsgType = PMF_MLME_SAQUERY_RSP;
                }
                        else if (pFrame->Octet[1] == PMF_ACTION_SAQUERY_RSP)
                        {
                    *Machine = STA_PMF_STATE_MACHINE;
                    *MsgType = PMF_PEER_SAQUERY_RSP;
                }
                else
                {
                    return FALSE;
                }
                return TRUE;
            }
            //  Vendor specific usage.
            else if ((pFrame->Octet[0] & 0x7F) == MT2_ACT_VENDOR) //  Sometimes Sta will return with category bytes with MSB = 1, if they receive catogory out of their support
            {
                // Now support WFA P2P
                if ((*(PULONG)&pFrame->Octet[1] == P2P_OUI) && (IS_P2P_OP(pPort)))
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("Vendor Action frame P2P OUI= 0x%x. subtye = %d.\n", *(PULONG)&pFrame->Octet[1], pFrame->Octet[5]));
                    *Machine = P2P_STATE_MACHINE;
                    if ((pFrame->Octet[5] >= 0) && (pFrame->Octet[5] <= MT2_MAX_PEER_SUPPORT))
                    {
                        *MsgType = pFrame->Octet[5]; //OUI subtype. 
                    }
                    else
                        return FALSE;
                }
                else
                {
                    return FALSE;
                }
            } 
            //  Sometimes Sta will return with category bytes with MSB = 1, if they receive catogory out of their support
            else if ((pFrame->Octet[0]&0x7F) > MAX_PEER_CATE_MSG) 
            {
                *Machine = ACTION_STATE_MACHINE;
                *MsgType = MT2_ACT_INVALID;
            } 
            else
            {
                *Machine = ACTION_STATE_MACHINE;
                *MsgType = (pFrame->Octet[0]&0x7F);
            } 
            break;
        default:
            return FALSE;
            break;
    }

    return TRUE;
}

// ===========================================================================================
// state_machine.c
// ===========================================================================================

/*! \brief Initialize the state machine.
 *  \param *S           pointer to the state machine 
 *  \param  Trans       State machine transition function
 *  \param  StNr        number of states 
 *  \param  MsgNr       number of messages 
 *  \param  DefFunc     default function, when there is invalid state/message combination 
 *  \param  InitState   initial state of the state machine 
 *  \param  Base        StateMachine base, internal use only
 *  \pre p_sm should be a legal pointer
 *  \post

 IRQL = PASSIVE_LEVEL
 
 */

VOID StateMachineInit(
    IN STATE_MACHINE *S, 
    IN STATE_MACHINE_FUNC Trans[], 
    IN ULONG StNr, 
    IN ULONG MsgNr, 
    IN STATE_MACHINE_FUNC DefFunc, 
    IN ULONG InitState, 
    IN ULONG Base) 
{
    ULONG i, j;

    // set number of states and messages
    S->NrState = StNr;
    S->NrMsg   = MsgNr;
    S->Base    = Base;

    S->TransFunc  = Trans;

    // init all state transition to default function
    for (i = 0; i < StNr; i++) 
    {
        for (j = 0; j < MsgNr; j++) 
        {
            S->TransFunc[i * MsgNr + j] = DefFunc;
        }
    }

    // set the starting state
    S->CurrState = InitState;

}

/*! \brief This function fills in the function pointer into the cell in the state machine 
 *  \param *S   pointer to the state machine
 *  \param St   state
 *  \param Msg  incoming message
 *  \param f    the function to be executed when (state, message) combination occurs at the state machine
 *  \pre *S should be a legal pointer to the state machine, st, msg, should be all within the range, Base should be set in the initial state
 *  \post

 IRQL = PASSIVE_LEVEL
 
 */
VOID StateMachineSetAction(
    IN STATE_MACHINE *S, 
    IN ULONG St, 
    IN ULONG Msg, 
    IN STATE_MACHINE_FUNC Func) 
{
    ULONG MsgIdx;

    MsgIdx = Msg - S->Base;

    if (St < S->NrState && MsgIdx < S->NrMsg) 
    {
        // boundary checking before setting the action
        S->TransFunc[St * S->NrMsg + MsgIdx] = Func;
    } 
}

/*! \brief   This function does the state transition
 *  \param   *Adapter the NIC adapter pointer
 *  \param   *S       the state machine
 *  \param   *Elem    the message to be executed
 *  \return   None
 
 IRQL = DISPATCH_LEVEL
 
 */
VOID StateMachinePerformAction(
    IN  PMP_ADAPTER   pAd, 
    IN STATE_MACHINE *S, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    (*(S->TransFunc[S->CurrState * S->NrMsg + Elem->MsgType - S->Base]))(pAd, Elem);
}

/*
    ==========================================================================
    Description:
        The drop function, when machine executes this, the message is simply 
        ignored. This function does nothing, the message is freed in 
        StateMachinePerformAction()
    ==========================================================================
 */
VOID Drop(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
}

// ===========================================================================================
// lfsr.c
// ===========================================================================================

/*
    ==========================================================================
    Description:

    IRQL = PASSIVE_LEVEL

    ==========================================================================
 */
VOID LfsrInit(
    IN PMP_ADAPTER pAd, 
    IN ULONG Seed) 
{
    if (Seed == 0) 
        pAd->Mlme.ShiftReg = 1;
    else 
        pAd->Mlme.ShiftReg = Seed;
}

/*
    ==========================================================================
    Description:
    ==========================================================================
 */
UCHAR RandomByte(
    IN PMP_ADAPTER pAd) 
{
    ULONG i;
    UCHAR R, Result;

    R = 0;

    for (i = 0; i < 8; i++) 
    {
        if (pAd->Mlme.ShiftReg & 0x00000001) 
        {
            pAd->Mlme.ShiftReg = ((pAd->Mlme.ShiftReg ^ LFSR_MASK) >> 1) | 0x80000000;
            Result = 1;
        } 
        else 
        {
            pAd->Mlme.ShiftReg = pAd->Mlme.ShiftReg >> 1;
            Result = 0;
        }
        R = (R << 1) | Result;
    }

    return R;
}

/*
    ==========================================================================
    Description:

    IRQL = PASSIVE_LEVEL
    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
 */
VOID AsicSwitchChannel(
    IN  PMP_PORT    pPort, 
    IN  UCHAR           Channel,
    IN  UCHAR           CentralChannel,
    IN  UCHAR           BW,
    IN  BOOLEAN         bScan) 
{
    PMP_ADAPTER pAd = pPort->pAd;
    EXT_CMD_CHAN_SWITCH_T *CmdChSwitch = &pPort->SwitchChannelSturct;
    UCHAR i = 0;
    
    FUNC_ENTER;

    DBGPRINT(RT_DEBUG_TRACE,("%s Port[%d] switching to Channel = %d CentralChannel = %d BW = %d bScan = %d\n", __FUNCTION__, pPort->PortNumber, Channel, CentralChannel, BW, bScan));
    DBGPRINT(RT_DEBUG_TRACE,("%s port->Channel = %d, port->CentralChannel = %d, port->BW = %d, pPort->ScaningChannel = %d\n", __FUNCTION__, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, pPort->ScaningChannel));
	
    //mt7603_tx_pwr_gain(pAd, pPort->Channel);

    if (Channel == 0)
    {
        Channel = 1;
        CentralChannel = 1;
    }

    if (bScan)
    {
        BW = BW_20;
    }

    if (BW == BW_20)
    {
        CentralChannel = Channel;
    }

    CmdChSwitch->ucCtrlCh = Channel;
    CmdChSwitch->ucCentralCh = CentralChannel;
    CmdChSwitch->ucBW = BW;            
    CmdChSwitch->ucRxStreamNum = pPort->CommonCfg.RxStream;
    CmdChSwitch->ucTxStreamNum = pPort->CommonCfg.TxStream;

    DBGPRINT(RT_DEBUG_TRACE, ("%s   BBPCurrentBW = %d   CentralChannel %d", __FUNCTION__, pPort->BBPCurrentBW, pPort->CentralChannel));
    
    for(i=0;i<SKU_SIZE;i++)
    {
        CmdChSwitch->aucTxPowerSKU[i]=0xff;
    }

    pPort->SwitchChannelSturct.ucCtrlCh = Channel;
    

    if(KeGetCurrentIrql() == DISPATCH_LEVEL)
    {
        PlatformScheduleWorktiem(&pPort->Mlme.AsicSwitchChannelWorkitem);
    }
    else
    {
        SendSwitchChannelCMD(pAd, pPort->SwitchChannelSturct);
    }

    // latch channel for future usage.
    //pAd->HwCfg.LatchRfRegs.Channel = Channel;

    //if (pAd->chipCap.hif_type == HIF_MT) 
    {
        UINT32 val;

        HW_IO_READ32(pAd, RMAC_CHFREQ, &val);
        val = 1;
        HW_IO_WRITE32(pAd, RMAC_CHFREQ, val);

    }   

   MtAsicSetBW(pAd, BW);

    pAd->HwCfg.HWChannelBWInfo.Channel = Channel;
    pAd->HwCfg.HWChannelBWInfo.CentralChannel = CentralChannel;
    pAd->HwCfg.HWChannelBWInfo.BW = BW;
    pAd->HwCfg.HWChannelBWInfo.OperatingPortNumber = pPort->PortNumber;

    FUNC_LEAVE;
}

VOID
AsicSwitchChannelWorkitemCallback(
    IN PVOID Context
    )
{
   
    PMP_PORT  pPort = (PMP_PORT)Context;
    PMP_ADAPTER pAd = pPort->pAd;
    
    FUNC_ENTER;
    SendSwitchChannelCMD(pAd, pPort->SwitchChannelSturct);
    FUNC_LEAVE;
}

/*
    ==========================================================================
    Description:
        put PHY to sleep here, and set next wakeup timer. PHY doesn't not wakeup 
        automatically. Instead, MCU will issue a TwakeUpInterrupt to host after
        the wakeup timer timeout. Driver has to issue a separate command to wake
        PHY up.

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID AsicSleepThenAutoWakeup(
    IN PMP_ADAPTER pAd,   
    IN USHORT TbttNumToNextWakeUp) 
{
//  ULONG MacValue;

#if 1
    // Not going to sleep if in the Count Down Time
    if (pAd->TrackInfo.CountDowntoPsm > 0)
        return;
    
    // we have decided to SLEEP, so at least do it for a BEACON period.
    if (TbttNumToNextWakeUp == 0)
        TbttNumToNextWakeUp = 1;

    DBGPRINT(RT_DEBUG_TRACE,("AsicSleepThenAutoWakeup ==> TBTT = %d\n", TbttNumToNextWakeUp));
    
    // Polling Rx Empty
    if (0)
    {
        U3DMA_WLCFG         U3DMAWLCFG;
        int i = 0;
        
        do
        {
			HW_IO_READ32(pAd, UDMA_WLCFG_0, &U3DMAWLCFG.word);
			DBGPRINT(RT_DEBUG_TRACE,("%d Wait Tx/Rx to be idle, Rx = %d, Tx = %d\n", i, U3DMAWLCFG.field.WL_RX_BUSY, U3DMAWLCFG.field.WL_TX_BUSY));

            i++;
            if (i == 1000)
                break;
		} while ((U3DMAWLCFG.field.WL_RX_BUSY == 1));
    }
    
    //OPSTATUS_SET_FLAG(pAd, fOP_STATUS_DOZE);

    SendPowerManagementCMD(pAd, PM4, PWR_PmOn, pAd->HwCfg.CurrentAddress, 1, 1);

    OPSTATUS_SET_FLAG(pAd, fOP_STATUS_DOZE);

    
#else
    AUTO_WAKEUP_STRUC   AutoWakeupCfg;

    // Not going to sleep if in the Count Down Time
    if (pAd->TrackInfo.CountDowntoPsm > 0)
        return;
    
    // we have decided to SLEEP, so at least do it for a BEACON period.
    if (TbttNumToNextWakeUp == 0)
        TbttNumToNextWakeUp = 1;

    DBGPRINT(RT_DEBUG_TRACE,("%s ==> TBTT = %d\n",__FUNCTION__,TbttNumToNextWakeUp));
    
    AutoWakeupCfg.word = 0;
    RTUSBWriteMACRegister(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);

    AutoWakeupCfg.field.NumofSleepingTbtt = TbttNumToNextWakeUp - 1;
    AutoWakeupCfg.field.EnableAutoWakeup = 1;
    AutoWakeupCfg.field.AutoLeadTime = 5;
    RTUSBWriteMACRegister(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);

    AsicSendCommanToMcu(pAd, SLEEP_MCU_CMD, 0xff, 0xff, 0x02);   // send POWER-SAVE command to MCU. Timeout unit:40us.

    OPSTATUS_SET_FLAG(pAd, fOP_STATUS_DOZE);

    //
    // Stop bulkin traffic to enable selective suspend
    //      
    if(pAd->StaCfg.PSControl.field.EnablePSinIdle && (!pPort->SoftAP.bAPStart) && (!pPort->P2PCfg.bGOStart))
    {
        if(pPort->CommonCfg.BeaconPeriod >= 100)
        {
            // cancel bulk-in IRPs prevent blocking CPU enter C3.   
            MT_SET_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS);

            N6UsbRecvStop(pAd);

            // resend bulk-in IRPs to receive beacons after a period of (pPort->CommonCfg.BeaconPeriod - 15) ms
            DBGPRINT(RT_DEBUG_TRACE,("$$ [%s][LINE_%d]_USB pipes stopped\n",__FUNCTION__,__LINE__));
            DBGPRINT(RT_DEBUG_TRACE,("$$ [%s][LINE_%d]_start USB pipes after %d ms\n",__FUNCTION__,__LINE__,pPort->CommonCfg.BeaconPeriod * TbttNumToNextWakeUp - 15));
            PlatformSetTimer(pPort, &pPort->Mlme.ReSendBulkinIRPsTimer, pPort->CommonCfg.BeaconPeriod * TbttNumToNextWakeUp - 15);
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] BeaconPeriod < 100 milliseconds, DO NOT stop USB bulkin pipes.\n",__FUNCTION__,__LINE__));
        }
        
    }

    DBGPRINT(RT_DEBUG_TRACE, ("<-- AsicSleepThenAutoWakeup, TbttNumToNextWakeUp=%d \n", TbttNumToNextWakeUp));
#endif
}

/*
    ==========================================================================
    Description:
        AsicForceWakeup() is used whenever Twakeup timer (set via AsicSleepThenAutoWakeup)
        expired.

    IRQL = PASSIVE_LEVEL
    IRQL = DISPATCH_LEVEL
    ==========================================================================
 */
VOID AsicForceWakeup(
    IN PMP_ADAPTER pAd)
{
//  AUTO_WAKEUP_STRUC   AutoWakeupCfg;
    //ULONG MacValue;

    if (KeGetCurrentIrql() != PASSIVE_LEVEL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: KeGetCurrentIrql() != PASSIVE_LEVEL, KeGetCurrentIrql() = %d\n", __FUNCTION__, KeGetCurrentIrql()));
        return;
    }

    SendPowerManagementCMD(pAd, PM4, PWR_PmOff, pAd->HwCfg.CurrentAddress, 1, 1);

    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);

    DBGPRINT(RT_DEBUG_TRACE, ("<-- AsicForceWakeup \n"));
}

/*
    ==========================================================================
    Description:  Bssid also is a Wireless Client . So add Bssid MAC.

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID AsicSetBssidWC(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT pPort, 
    IN PUCHAR pBssid) 
{
#if 0
    ULONG   Addr4;
    USHORT  offset;

    if (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))
    {
        offset = MAC_WCID_BASE + P2pGetClientWcid(pAd, pPort) * HW_WCID_ENTRY_SIZE;
    }
    else
        offset = MAC_WCID_BASE + BSSID_WCID* HW_WCID_ENTRY_SIZE;

        //Emulation Only
    if(pAd->bFPGAEmulation && pAd->FPGACfg.TestWCID > 1)
        offset  = MAC_WCID_BASE + pAd->FPGACfg.TestWCID * HW_WCID_ENTRY_SIZE;

    Addr4 = (ULONG)(pBssid[0])       | 
            (ULONG)(pBssid[1] << 8)  | 
            (ULONG)(pBssid[2] << 16) |
            (ULONG)(pBssid[3] << 24);
    RTUSBWriteMACRegister(pAd, offset, Addr4);  
    offset += 4;
    Addr4 = (ULONG)(pBssid[4]) | (ULONG)(pBssid[5] << 8);
    RTUSBWriteMACRegister(pAd, offset, Addr4);
#endif  
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
 */
VOID AsicSetCarrierDetectAction(
    IN PMP_ADAPTER    pAd,
    IN UCHAR            Action,
    IN ULONG            Period) 
{
    UCHAR   LighByte = 0;
     PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    

    DBGPRINT(RT_DEBUG_TRACE, ("--->AsicSetCarrierDetectAction (Action=%d, Period=%dms)\n", Action, Period));

    LighByte += (((pPort->Channel > 14) ? (0) : (1)) << 7);     // [7]
    LighByte += (Action << 5);                                          // [6:5]
    LighByte += (27);                                                   // [4:0]
    
    AsicSendCommanToMcu(pAd, 0x63, 0xff, (UCHAR)(Period/100), LighByte);
}

VOID MlmeUpdateBeacon(
    IN PMP_ADAPTER pAd)
{
    BCN_OFFSET_0 BeaconOffset0;

    ULONG Index = 0;
    PMP_PORT pPort = NULL;

    UCHAR BeaconNum = 0;
    UCHAR BeaconBufferUsed = 0;
    //Beacon content location: BCN_BASE + BCN_OFFSET * 0x40
    ULONG HwBeaconBase = HW_BEACON_BASE0;
    UCHAR HwBeaconOffsetBase = HW_BEACON_OFFSET;
    MAC_DW1_EX_STRUC StaMacReg1;

    //
    // Init local variables.
    //
    StaMacReg1.word = 0;

    // =====================================================
    // Beacon Configuration overview
    // =====================================================
    // First version of Beacon configuration
    //    Beacon offset 0~3:         0xECE8E4E0
    //    Beacon Storage Base:       0x7800(MAC Ver=1) / 0xB800(MAC Ver=2)
    //    One beacon buffer length:  0x100
    // Extended version of Beacon configuration
    //    Beacon offset 0~3:         0x18100800
    //    Beacon Storage Base:       0xC000
    //    One beacon buffer length:  0x200

    BeaconOffset0.field.Bcn0Offset = HW_BEACON_OFFSET;
    BeaconOffset0.field.Bcn1Offset = HW_BEACON_OFFSET + HW_BEACON_UNITS;
    BeaconOffset0.field.Bcn2Offset = HW_BEACON_OFFSET + HW_BEACON_UNITS*2;
    BeaconOffset0.field.Bcn3Offset = HW_BEACON_OFFSET + HW_BEACON_UNITS*3;

    DBGPRINT(RT_DEBUG_TRACE, ("---> MlmeUpdateBeacon\n"));

    //
    // Beacon base,
    // for old chipset, beacon will be placed starting from 1st buffer.
    // for new chipset supporting hardware WAPI, beacon will be placed starting from 3rd buffer.
    //
    HwBeaconBase = HW_BEACON_BASE0;
    HwBeaconOffsetBase = HW_BEACON_OFFSET;

    //
    // Disable beacon before updateing the content
    //
    AsicPauseBssSync(pAd);

    //
    // Update beacon content/offset.
    //
    for (Index = 0; Index < RTMP_MAX_NUMBER_OF_PORT; Index++)
    {
        pPort = pAd->PortList[Index];

        if (pPort == NULL)
        {
            continue;
        }

        if (!pPort->bActive)
        {
            continue;
        }

        DBGPRINT(RT_DEBUG_TRACE, ("port(%u), bBeaconing = %d\n", Index, pPort->bBeaconing));
        if (pPort->bBeaconing)
        {
            // Increase beacon number.
            BeaconNum++;

            // Beacon offset
            if (BeaconNum == 1)
            {
                BeaconOffset0.field.Bcn0Offset = HwBeaconOffsetBase + HW_BEACON_UNITS * BeaconBufferUsed;
            }
            else if (BeaconNum == 2)
            {
                BeaconOffset0.field.Bcn1Offset = HwBeaconOffsetBase + HW_BEACON_UNITS * BeaconBufferUsed;
            }
            else if (BeaconNum == 3)
            {
                BeaconOffset0.field.Bcn2Offset = HwBeaconOffsetBase + HW_BEACON_UNITS * BeaconBufferUsed;
            }

            // Update beacon content.
            pPort->HwBeaconBase = HwBeaconBase;
            if ((pPort->PortType == EXTSTA_PORT) && (pPort->PortSubtype == PORTSUBTYPE_STA))
            {
                HwBeaconBase += HW_BEACON_BUFFER_SIZE;
                BeaconBufferUsed += 1;
            }
            else if ((pPort->PortSubtype == PORTSUBTYPE_VwifiAP) || (pPort->PortSubtype == PORTSUBTYPE_SoftapAP))
            {
                //vwifi softap beacon, 2 buffer used.
                MgntPktConstructAPBeaconFrame(pAd, pPort);
                ApConnectUpdateBeaconFrame(pAd, pPort);

                HwBeaconBase += (HW_BEACON_BUFFER_SIZE * 2);
                BeaconBufferUsed += 2;
            }
            else// if (pPort->PortSubtype == PORTSUBTYPE_P2PGO)
            {
                //WFD beacon, 2 buffer used.
                P2pMakeBssBeacon(pAd, pPort);

                HwBeaconBase += (HW_BEACON_BUFFER_SIZE * 2);
                BeaconBufferUsed += 2;
            }
        }
    }

    DBGPRINT(RT_DEBUG_TRACE, ("BeaconNum: %u, BCN_OFFSET0: %08x\n", BeaconNum, BeaconOffset0.word));

    //
    // Update beacon number
    //
    RTUSBReadMACRegister(pAd, MAC_BSSID_DW1, &StaMacReg1.word);
    StaMacReg1.field.MultiBcnNum = BeaconNum;
    if (StaMacReg1.field.MultiBcnNum > 0)
    {
        StaMacReg1.field.MultiBcnNum--;
    }
    RTUSBWriteMACRegister(pAd, MAC_BSSID_DW1, StaMacReg1.word);

    //
    // Update beacon offset
    //
    RTUSBWriteMACRegister(pAd, BCN_OFFSET0, BeaconOffset0.word);

    //
    // Keep the beacon number for reference.
    //
    pAd->HwCfg.BeaconNum = BeaconNum;

    //
    // if beacon number is 0, disable it.
    //
    if (BeaconNum == 0)
    {
        MtAsicDisableSync(pAd);
    }

    DBGPRINT(RT_DEBUG_TRACE, ("<--- MlmeUpdateBeacon\n"));
}


/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
 */
VOID AsicEnableRDG(
    IN PMP_ADAPTER pAd) 
{
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
 */
VOID AsicDisableRDG(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort) 
{
    DBGPRINT(RT_DEBUG_INFO, ("--->AsicDisableRDG \n"));

    // RDG TxBurst will use the same place(AC0_TXOP), so we should also check TxBurst when we want to disable RDG   
    if (pPort->CommonCfg.bEnableTxBurst)
    {
        if (((pAd->StaActive.SupportedHtPhy.bHtEnable == FALSE) && (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_WMM_INUSED))) 
        || ((pAd->StaActive.SupportedHtPhy.bHtEnable == TRUE) && (pPort->CommonCfg.BACapability.field.Policy == BA_NOTUSE)))
        {
            // do not turn on TxBurst
            // if 1. Legacy AP WMM on,  
            //    2. 11n AP, AMPDU disable.
        }
        else
        {
    	// TODO: Shiang-usw-win, should not use this function directly unless you already stop the Tx/Rx
            MtAsicSetWmmParam(pAd, WMM_PARAM_AC_0, WMM_PARAM_TXOP, 0x60);
        }
    }   
}


/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
 */
VOID AsicEnablePiggyB(
    IN PMP_ADAPTER pAd) 
{
    TX_LINK_CFG_STRUC   TxLinkCfg;

    DBGPRINT(RT_DEBUG_INFO, ("--->AsicEnableRDG \n"));

    RTUSBReadMACRegister(pAd, TX_LINK_CFG, &TxLinkCfg.word);
    TxLinkCfg.field.TxCFAckEn = 1;
    RTUSBWriteMACRegister(pAd, TX_LINK_CFG, TxLinkCfg.word);
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
 */
VOID AsicDisablePiggyB(
    IN PMP_ADAPTER pAd) 
{
    TX_LINK_CFG_STRUC   TxLinkCfg;

    DBGPRINT(RT_DEBUG_INFO, ("--->AsicEnableRDG \n"));

    RTUSBReadMACRegister(pAd, TX_LINK_CFG, &TxLinkCfg.word);
    TxLinkCfg.field.TxCFAckEn = 0;
    RTUSBWriteMACRegister(pAd, TX_LINK_CFG, TxLinkCfg.word);
}

/*
    ==========================================================================
    Description:
        Pause sending beacon.
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID AsicPauseBssSync(
    IN PMP_ADAPTER pAd)
{
    BCN_TIME_CFG_STRUC csr;
    
    DBGPRINT(RT_DEBUG_TRACE, ("--->AsicPauseBssSync\n"));
    
    RTUSBReadMACRegister(pAd, BCN_TIME_CFG, &csr.word);
    csr.field.bBeaconGen  = 0; 
    RTUSBWriteMACRegister(pAd, BCN_TIME_CFG, csr.word);
    pAd->StaCfg.IBSSync.csr9.word = csr.word;
}

/*  
    ==========================================================================
    Description: 
        Resume sending beacon.
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID AsicResumeBssSync(
    IN PMP_ADAPTER pAd)
{
    BCN_TIME_CFG_STRUC csr;
    
    if (pAd->HwCfg.BeaconNum > 0)
    {
        RTUSBReadMACRegister(pAd, BCN_TIME_CFG, &csr.word);
        csr.field.bBeaconGen  = 1; 
        RTUSBWriteMACRegister(pAd, BCN_TIME_CFG, csr.word);
        pAd->StaCfg.IBSSync.csr9.word = csr.word;
        DBGPRINT(RT_DEBUG_TRACE, ("--->AsicResumeBssSync (pAd->HwCfg.BeaconNum=%d)(%x)\n", pAd->HwCfg.BeaconNum, csr.word));
    }
}

/*
    ==========================================================================
    Description:
        danamic tune BBP R17 to find a balance between sensibility and 
        noise isolation

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID AsicBbpTuning(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort)
{
    UCHAR   OrigR66Value, R66;
    CHAR    Rssi;

    // 2860C did not support Fase CCA, therefore can't tune
    if (pAd->HwCfg.BbpTuning.bEnable == FALSE) 
        return; 

    //
    // work as a STA
    //
    if (pPort->Mlme.CntlMachine.CurrState != CNTL_IDLE)  // no R66 tuning when SCANNING
        return;  

    if ((pPort->PortSubtype == PORTSUBTYPE_STA || pPort->PortSubtype == PORTSUBTYPE_P2PClient) 
            && (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED)))
    {
        RTUSBReadBBPRegister(pAd, BBP_R66, &OrigR66Value);
        R66 = OrigR66Value;

        if (pAd->HwCfg.Antenna.field.RxPath > 1)
            Rssi = (pAd->StaCfg.AvgRssi + pAd->StaCfg.AvgRssi2) >> 1;
        else
            Rssi = pAd->StaCfg.AvgRssi;
        
        //
        // calculating gains for R66
        //

        if (EXT_BB_CAPABLE(pAd))
        {
        }
        //
        // for other device, channel should be concerned.
        //
        else if (pAd->HwCfg.LatchRfRegs.Channel <= 14)
        {   //BG band
            if (Rssi > RSSI_FOR_MID_LOW_SENSIBILITY)
            {
                R66 = 0x2E + GET_LNA_GAIN(pAd) + 0x10;
                if (OrigR66Value != R66)
                {       
                    RTUSBWriteBBPRegister(pAd, BBP_R66, R66);
                }       
            }
            else
            {
                R66 = 0x2E + GET_LNA_GAIN(pAd);
                if (OrigR66Value != R66)
                {               
                    RTUSBWriteBBPRegister(pAd, BBP_R66, R66);
                }           
            }
        }
        else
        {   //A band
            if (pPort->BBPCurrentBW == BW_20)
            {
                if (Rssi > RSSI_FOR_MID_LOW_SENSIBILITY)
                {
                    R66 = 0x32 + (GET_LNA_GAIN(pAd)*5)/3 + 0x10;
                    
                    if (OrigR66Value != R66)
                    {                   
                        RTUSBWriteBBPRegister(pAd, BBP_R66, R66);
                    }               
                }
                else
                {
                    R66 = 0x32 + (GET_LNA_GAIN(pAd)*5)/3;
                    
                    if (OrigR66Value != R66)
                    {           
                        RTUSBWriteBBPRegister(pAd, BBP_R66, R66);
                    }
                }
            }
            else
            {
                if (Rssi > RSSI_FOR_MID_LOW_SENSIBILITY)
                {
                    R66 = 0x3A + (GET_LNA_GAIN(pAd)*5)/3 + 0x10;
                    
                    if (OrigR66Value != R66)
                    {    
                        RTUSBWriteBBPRegister(pAd, BBP_R66, R66);   
                    }       
                }
                else
                {
                    R66 = 0x3A + (GET_LNA_GAIN(pAd)*5)/3;
                    
                    if (OrigR66Value != R66)
                    {              
                        RTUSBWriteBBPRegister(pAd, BBP_R66, R66);
                    }
                }
            }
        }

        if (pAd->Mlme.OneSecPeriodicRound % 4 == 0)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("RSSI=%d, CCA=%d, BW=%d, LNA_GAIN=0x%x, fixed R66 at 0x%x\n", 
                Rssi, pAd->Counter.MTKCounters.OneSecFalseCCACnt, pPort->BBPCurrentBW, GET_LNA_GAIN(pAd), R66));
        }
    }

    DBGPRINT(RT_DEBUG_TRACE,("%s <---\n", __FUNCTION__));

}

//
// Update the Tx chain address (stream mode)
//
// Parameters
//  pAd: The adapter data structure
//  pMacAddress: The MAC address of the peer STA
//
// Return Value:
//  None
//
VOID AiscUpdateTxChainAddress(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR pMacAddress)
{
    static UCHAR whichTxChain = 0; // Tx chain 1~3
     PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    PMAC_TABLE_ENTRY pMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);

    if(pMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
        return;
    }
    DBGPRINT(RT_DEBUG_INFO, ("---> %s\n", __FUNCTION__));

    if (MAC_ADDR_EQUAL(pMacAddress, pMacTabEntry->Addr))
    {
        TX_CHAIN_ADDR1_L_STRUC TxChainAddr1L = {0};
        TX_CHAIN_ADDR1_H_STRUC TxChainAddr1H = {0};

        TxChainAddr1L.field.TxChainAddr1L_Byte3 = pMacAddress[3];
        TxChainAddr1L.field.TxChainAddr1L_Byte2 = pMacAddress[2];
        TxChainAddr1L.field.TxChainAddr1L_Byte1 = pMacAddress[1];
        TxChainAddr1L.field.TxChainAddr1L_Byte0 = pMacAddress[0];
        RTUSBWriteMACRegister(pAd, TX_CHAIN_ADDR1_L, TxChainAddr1L.word);

        TxChainAddr1H.field.TxChainAddr1H_Byte5 = pMacAddress[5];
        TxChainAddr1H.field.TxChainAddr1H_Byte4 = pMacAddress[4];
        if (pPort->CommonCfg.StreamMode == 1)
        {
            TxChainAddr1H.field.TxChainSel0 = 0xF;
            pPort->CommonCfg.bStaInStreamMode = TRUE;
        }
        RTUSBWriteMACRegister(pAd, TX_CHAIN_ADDR1_H, TxChainAddr1H.word);

        DBGPRINT(RT_DEBUG_TRACE, ("%s: Tx chain 1 is reserved for Port[0] STA, MAC address = %02X:%02X:%02X:%02X:%02X:%02X\n", 
            __FUNCTION__, 
            pMacAddress[0], pMacAddress[1], pMacAddress[2], 
            pMacAddress[3], pMacAddress[4], pMacAddress[5]));
        return;
    }
    
    switch (whichTxChain + 2) // Skip the Tx chain #0 (Broadcast) Chain #1 (STA)
    {

        case 2: // Tx chain address 2
        {
            TX_CHAIN_ADDR2_L_STRUC TxChainAddr2L = {0};
            TX_CHAIN_ADDR2_H_STRUC TxChainAddr2H = {0};

            TxChainAddr2L.field.TxChainAddr2L_Byte3 = pMacAddress[3];
            TxChainAddr2L.field.TxChainAddr2L_Byte2 = pMacAddress[2];
            TxChainAddr2L.field.TxChainAddr2L_Byte1 = pMacAddress[1];
            TxChainAddr2L.field.TxChainAddr2L_Byte0 = pMacAddress[0];
            RTUSBWriteMACRegister(pAd, TX_CHAIN_ADDR2_L, TxChainAddr2L.word);

            TxChainAddr2H.field.TxChainAddr2H_Byte5 = pMacAddress[5];
            TxChainAddr2H.field.TxChainAddr2H_Byte4 = pMacAddress[4];
            TxChainAddr2H.field.TxChainSel0 = 0xF;
            RTUSBWriteMACRegister(pAd, TX_CHAIN_ADDR2_H, TxChainAddr2H.word);

            DBGPRINT(RT_DEBUG_TRACE, ("%s: Tx chain address 2, MAC address = %02X:%02X:%02X:%02X:%02X:%02X\n", 
                __FUNCTION__, 
                pMacAddress[0], pMacAddress[1], pMacAddress[2], 
                pMacAddress[3], pMacAddress[4], pMacAddress[5]));
        }
        break;

        case 3: // Tx chain address 3
        {
            TX_CHAIN_ADDR3_L_STRUC TxChainAddr3L = {0};
            TX_CHAIN_ADDR3_H_STRUC TxChainAddr3H = {0};

            TxChainAddr3L.field.TxChainAddr3L_Byte3 = pMacAddress[3];
            TxChainAddr3L.field.TxChainAddr3L_Byte2 = pMacAddress[2];
            TxChainAddr3L.field.TxChainAddr3L_Byte1 = pMacAddress[1];
            TxChainAddr3L.field.TxChainAddr3L_Byte0 = pMacAddress[0];
            RTUSBWriteMACRegister(pAd, TX_CHAIN_ADDR3_L, TxChainAddr3L.word);

            TxChainAddr3H.field.TxChainAddr3H_Byte5 = pMacAddress[5];
            TxChainAddr3H.field.TxChainAddr3H_Byte4 = pMacAddress[4];
            TxChainAddr3H.field.TxChainSel0 = 0xF;
            RTUSBWriteMACRegister(pAd, TX_CHAIN_ADDR3_H, TxChainAddr3H.word);

            DBGPRINT(RT_DEBUG_TRACE, ("%s: Tx chain address 3, MAC address = %02X:%02X:%02X:%02X:%02X:%02X\n", 
                __FUNCTION__, 
                pMacAddress[0], pMacAddress[1], pMacAddress[2], 
                pMacAddress[3], pMacAddress[4], pMacAddress[5]));
        }
        break;

        default: 
        {
            DBGPRINT(RT_DEBUG_ERROR, ("%s: Incorrect whichTxChain (=%d)\n", __FUNCTION__, whichTxChain));
        }
        break;
    }

    whichTxChain = whichTxChain + 1;
    whichTxChain = whichTxChain % (NUM_OF_TX_CHAIN - 2); // Skip the Tx chain #0 (Broadcast) Chain #1 (STA)

    DBGPRINT(RT_DEBUG_INFO, ("<--- %s\n", __FUNCTION__));
}

//
// Configure PA_PE by streams
// 
// 1. if MCS0, MCS1, MCS3, MCS8, MCS9, MCS11, Enable stream mode.
// 2. if not (MCS0, MCS1, MCS3, MCS8, MCS9, MCS11), disable stream mode.
//  2.1     If 1ss/2ss change to 3ss, enable PA_PE_2
//  2.2 If 3ss change to 1ss/2ss, Disable PA_PE_2
//
// Parameters
// IN PMP_ADAPTER pAd,
// IN PRTMP_TX_RATE_SWITCH_AGS pCurrTxRate
// IN PRTMP_TX_RATE_SWITCH  pNextTxRate)
//
// Return Value:
//  None
//
VOID AsicCfgPAPEByStreams(
    IN PMP_ADAPTER pAd,
    IN UCHAR CurrMCS,
    IN UCHAR    NextMCS
    )
{
}

VOID AsicUpdateWCIDAttribute(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN USHORT       WCID,
    IN UCHAR        BssIndex,
    IN UCHAR        CipherAlg,
    IN BOOLEAN      bUsePairewiseKeyTable)
{
    ULONG   WCIDAttri = 0;
    USHORT  offset;

    //
    // Update WCID attribute.
    // Only TxKey could update WCID attribute.
    //
    offset = MAC_WCID_ATTRIBUTE_BASE + (WCID * HW_WCID_ATTRI_SIZE);
    WCIDAttri = (BssIndex << 4) | (CipherAlg << 1) | (bUsePairewiseKeyTable);
    RTUSBWriteMACRegister(pAd, offset, WCIDAttri);
}

VOID AsicUpdateWCIDIVEIV(
    IN PMP_ADAPTER pAd,
    IN USHORT       WCID,
    IN ULONG        uIV,
    IN ULONG        uEIV)
{
    USHORT  offset;

    offset = PAIRWISE_IVEIV_TABLE_BASE + (WCID * HW_IVEIV_ENTRY_SIZE);

    RTUSBWriteMACRegister(pAd, offset, uIV);
    RTUSBWriteMACRegister(pAd, offset + 4, uEIV);
}


VOID hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen)
{
    unsigned char *pt;
    unsigned int x;

//  if (RTDebugLevel < RT_DEBUG_TRACE)
//      return;
    
    pt = pSrcBufVA;
    DBGPRINT(RT_DEBUG_ERROR, ("%s: %p, len = %d\n",str,  pSrcBufVA, SrcBufLen));

    for (x=0; x < SrcBufLen; x++)
    {
        if (x % 16 == 0) 
            DBGPRINT(RT_DEBUG_ERROR, ("0x%04x : ", x));
        DBGPRINT(RT_DEBUG_ERROR, ("%02x ", ((unsigned char)pt[x])));
        if (x%16 == 15) DBGPRINT(RT_DEBUG_ERROR,("\n"));
    }
    DBGPRINT(RT_DEBUG_ERROR,("\n"));
}


VOID DumpWTBLInfo(PMP_ADAPTER pAd, UINT32 wtbl_idx, PVOID wtblentry)
{
    UINT32 idx, start_idx, end_idx, tok;
    UINT32 addr, val[16];

    struct wtbl_entry   WTBLEntry;
        
    struct wtbl_1_struc *wtbl_1;
    struct wtbl_2_struc *wtbl_2;
    union wtbl_3_struc *wtbl_3;
    struct wtbl_4_struc *wtbl_4;

    wtbl_1 = &WTBLEntry.wtbl_1;
    wtbl_2 = &WTBLEntry.wtbl_2;
    wtbl_3 = &WTBLEntry.wtbl_3;
    wtbl_4 = &WTBLEntry.wtbl_4;

    DBGPRINT(RT_DEBUG_OFF, ("Dump WTBL info of WLAN_IDX:%d\n", wtbl_idx));

    if (wtbl_idx == RESERVED_WCID)
    {
        start_idx = 0;
        end_idx = (MT_WTBL_SIZE - 1);
    }
    else if (wtbl_idx < MT_WTBL_SIZE)
    {
        start_idx = end_idx = wtbl_idx;
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s():Invalid WTBL index(%d)!\n",
                    __FUNCTION__, wtbl_idx));
        return;
    }

    for (idx = start_idx; idx <= end_idx; idx++)
    {
        /* Read WTBL 1 */
        PlatformZeroMemory((UCHAR *)wtbl_1, sizeof(struct wtbl_1_struc));
        addr = pAd->mac_ctrl.wtbl_base_addr[0] + idx * pAd->mac_ctrl.wtbl_entry_size[0];
        DBGPRINT(RT_DEBUG_OFF, ("WTBL Segment 1 HW Addr:0x%x\n", addr));
        HW_IO_READ32(pAd, addr, &wtbl_1->wtbl_1_d0.word);
        HW_IO_READ32(pAd, addr + 4, &wtbl_1->wtbl_1_d1.word);
        HW_IO_READ32(pAd, addr + 8, &wtbl_1->wtbl_1_d2.word);
        HW_IO_READ32(pAd, addr + 12, &wtbl_1->wtbl_1_d3.word);
        HW_IO_READ32(pAd, addr + 16, &wtbl_1->wtbl_1_d4.word);
        //dump_wtbl_1_info(pAd, wtbl_1);

        /* Read WTBL 2 */
        PlatformZeroMemory((UCHAR *)wtbl_2, sizeof(struct wtbl_2_struc));
        addr = pAd->mac_ctrl.wtbl_base_addr[1] + idx * pAd->mac_ctrl.wtbl_entry_size[1];
        DBGPRINT(RT_DEBUG_OFF, ("WTBL Segment 2 HW Addr:0x%x\n", addr));
        for (tok = 0; tok < sizeof(struct wtbl_2_struc) / 4; tok++)
        {
            HW_IO_READ32(pAd, addr + tok * 4, &val[tok]);
        }
        //dump_wtbl_2_info(pAd, (struct wtbl_2_struc *)&val[0]);
        PlatformMoveMemory(wtbl_2, &val[0], sizeof(struct wtbl_2_struc));

        /* Read WTBL 3 */
        PlatformZeroMemory((UCHAR *)wtbl_3, sizeof(union wtbl_3_struc));
        addr = pAd->mac_ctrl.wtbl_base_addr[2] + idx * pAd->mac_ctrl.wtbl_entry_size[2];
        DBGPRINT(RT_DEBUG_OFF, ("WTBL Segment 3 HW Addr:0x%x\n", addr));
        for (tok = 0; tok < sizeof(union wtbl_3_struc) / 4; tok++)
        {
            HW_IO_READ32(pAd, addr + tok * 4, &val[tok]);
        }
        //dump_wtbl_3_info(pAd, (union wtbl_3_struc *)&val[0]);
        //RTMP_IO_READ32(pAd, addr, wtbl_3.);
        //dump_wtbl_3_info(pAd, &wtbl_3);
        PlatformMoveMemory(wtbl_3, &val[0], sizeof(union wtbl_3_struc));

        /* Read WTBL 4 */
        PlatformZeroMemory((UCHAR *)wtbl_4, sizeof(struct wtbl_4_struc));
        addr = pAd->mac_ctrl.wtbl_base_addr[3] + idx * pAd->mac_ctrl.wtbl_entry_size[3];
        DBGPRINT(RT_DEBUG_OFF, ("WTBL Segment 4 HW Addr:0x%x\n", addr));
        HW_IO_READ32(pAd, addr, &wtbl_4->ac0.word[0]);
        HW_IO_READ32(pAd, addr+4, &wtbl_4->ac0.word[1]);
        HW_IO_READ32(pAd, addr + 8, &wtbl_4->ac1.word[0]);
        HW_IO_READ32(pAd, addr + 12, &wtbl_4->ac1.word[1]);
        HW_IO_READ32(pAd, addr + 16, &wtbl_4->ac2.word[0]);
        HW_IO_READ32(pAd, addr + 20, &wtbl_4->ac2.word[1]);
        HW_IO_READ32(pAd, addr + 24, &wtbl_4->ac3.word[0]);
        HW_IO_READ32(pAd, addr + 28, &wtbl_4->ac3.word[1]);
        //dump_wtbl_4_info(pAd, wtbl_4);
    }

    if (wtblentry != NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("copy whole WTBL\n"));
        PlatformMoveMemory(wtblentry, &WTBLEntry, sizeof(struct wtbl_entry));
    }
}

VOID show_mib_proc(MP_ADAPTER *pAd, PMIB_DATA pMibData)
{
    MIB_DATA    MibData;
    UINT32 idx;
    
    HW_IO_READ32(pAd, MIB_MSCR, &MibData.mibmscr);
    DBGPRINT(RT_DEBUG_TRACE, ("MIB Status Control=0x%x\n", MibData.mibmscr));
    HW_IO_READ32(pAd, MIB_MPBSCR, &MibData.mibmpbscr);
    DBGPRINT(RT_DEBUG_TRACE, ("MIB Per-BSS Status Control=0x%x\n", MibData.mibmpbscr));

    HW_IO_READ32(pAd, MIB_MSDR6, &MibData.msdr6);
    HW_IO_READ32(pAd, MIB_MSDR7, &MibData.msdr7);
    HW_IO_READ32(pAd, MIB_MSDR8, &MibData.msdr8);
    HW_IO_READ32(pAd, MIB_MSDR9, &MibData.msdr9);
    HW_IO_READ32(pAd, MIB_MSDR10, &MibData.msdr10);
    HW_IO_READ32(pAd, MIB_MSDR16, &MibData.msdr16);
    HW_IO_READ32(pAd, MIB_MSDR17, &MibData.msdr17);
    HW_IO_READ32(pAd, MIB_MSDR18, &MibData.msdr18);
    DBGPRINT(RT_DEBUG_TRACE, ("===Phy/Timing Related Counters===\n"));
    DBGPRINT(RT_DEBUG_TRACE, ("\tChannelIdleCnt=0x%x\n", MibData.msdr6 & 0xffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tCCA_NAV_Tx_Time=0x%x\n", MibData.msdr9 & 0xffffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tRx_MDRDY_CNT=0x%x\n", MibData.msdr10 & 0x3ffffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tPrim CCA Time=0x%x\n", MibData.msdr16 & 0xffffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tSec CCA Time=0x%x\n", MibData.msdr17 & 0xffffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tPrim ED Time=0x%x\n", MibData.msdr18 & 0xffffff));
    
    DBGPRINT(RT_DEBUG_TRACE, ("===Tx Related Counters(Generic)===\n"));
    HW_IO_READ32(pAd, MIB_MSDR0, &MibData.mibmsdr0);
    DBGPRINT(RT_DEBUG_TRACE, ("\tBeaconTxCnt=0x%x\n", MibData.mibmsdr0));
    HW_IO_READ32(pAd, MIB_MDR0, &MibData.mibmdr0);
    DBGPRINT(RT_DEBUG_TRACE, ("\tTx 40MHz Cnt=0x%x\n", (MibData.mibmdr0 >> 16) & 0xffff));
    HW_IO_READ32(pAd, MIB_MDR1, &MibData.mibmdr1);
    DBGPRINT(RT_DEBUG_TRACE, ("\tTx 80MHz Cnt=0x%x\n", MibData.mibmdr1& 0xffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tTx 160MHz Cnt=0x%x\n", (MibData.mibmdr1 >> 16) & 0xffff));
    
    DBGPRINT(RT_DEBUG_TRACE, ("===AMPDU Related Counters===\n"));
    HW_IO_READ32(pAd, MIB_MSDR12, &MibData.ampdu_cnt[0]);
    HW_IO_READ32(pAd, MIB_MSDR14, &MibData.ampdu_cnt[1]);
    HW_IO_READ32(pAd, MIB_MSDR15, &MibData.ampdu_cnt[2]);
    HW_IO_READ32(pAd, MIB_MDR2, &MibData.ampdu_cnt[3]);
    HW_IO_READ32(pAd, MIB_MDR3, &MibData.ampdu_cnt[4]);
    
    DBGPRINT(RT_DEBUG_TRACE, ("\tRx BA_Cnt=0x%x\n", MibData.ampdu_cnt[0] & 0xffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tTx AMPDU_Burst_Cnt=0x%x\n", (MibData.ampdu_cnt[0] >> 16 ) & 0xffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tTx AMPDU_Pkt_Cnt=0x%x\n", MibData.ampdu_cnt[1] & 0xffffff));   
    DBGPRINT(RT_DEBUG_TRACE, ("\tAMPDU SuccessCnt=0x%x\n", MibData.ampdu_cnt[2] & 0xffffff));   
    DBGPRINT(RT_DEBUG_TRACE, ("\tTx Agg Range: \t1 \t2~5 \t6~15 \t16~\n"));
    DBGPRINT(RT_DEBUG_TRACE, ("\t\t\t0x%x \t0x%x \t0x%x \t0x%x \n", 
                    (MibData.ampdu_cnt[3]) & 0xffff, (MibData.ampdu_cnt[3] >> 16) & 0xffff,
                    (MibData.ampdu_cnt[4]) & 0xffff, (MibData.ampdu_cnt[4] >> 16) & 0xfff));

    DBGPRINT(RT_DEBUG_TRACE, ("===Rx Related Counters(Generic)===\n"));
    DBGPRINT(RT_DEBUG_TRACE, ("\tVector Overflow Drop Cnt=0x%x\n", (MibData.msdr6 >> 16 ) & 0xffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tVector Missmacth Cnt=0x%x\n", (MibData.msdr7 >> 16 ) & 0xffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tDelimiter Fail Cnt=0x%x\n", MibData.msdr8& 0xffff));
    HW_IO_READ32(pAd, MIB_MSDR4, &MibData.mibmsdr4);
    DBGPRINT(RT_DEBUG_TRACE, ("\tRxFifoFullCnt=0x%x\n", MibData.mibmsdr4 & 0xffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tRxFCSErrCnt=0x%x\n", (MibData.mibmsdr4 >> 16 ) & 0xffff));
    HW_IO_READ32(pAd, MIB_MSDR5, &MibData.mibmsdr5);
    DBGPRINT(RT_DEBUG_TRACE, ("\tRxMPDUCnt=0x%x\n", MibData.mibmsdr5 & 0xffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tPFDropCnt=0x%x\n", (MibData.mibmsdr5 >> 16 ) & 0x00ff));
    HW_IO_READ32(pAd, MIB_MSDR22, &MibData.mibmsdr22);
    DBGPRINT(RT_DEBUG_TRACE, ("\tRx AMPDU Cnt=0x%x\n", MibData.mibmsdr22 & 0xffff));
    HW_IO_READ32(pAd, MIB_MSDR23, &MibData.mibmsdr23);
    DBGPRINT(RT_DEBUG_TRACE, ("\tRx Total ByteCnt=0x%x\n", MibData.mibmsdr23));

    for (idx = 0; idx < 4; idx++) {
        HW_IO_READ32(pAd, WTBL_BTCRn + idx * 4, &MibData.btcr[idx]);
        HW_IO_READ32(pAd, WTBL_BTBCRn + idx * 4, &MibData.btbcr[idx]);
        HW_IO_READ32(pAd, WTBL_BRCRn + idx * 4, &MibData.brcr[idx]);
        HW_IO_READ32(pAd, WTBL_BRBCRn + idx * 4, &MibData.brbcr[idx]);
    }
    DBGPRINT(RT_DEBUG_TRACE, ("===Per-BSS Related Tx/Rx Counters===\n"));
    DBGPRINT(RT_DEBUG_TRACE, ("BSS Idx   TxCnt/DataCnt  TxByteCnt  RxCnt/DataCnt  RxByteCnt\n"));
    for (idx = 0; idx < 4; idx++) {
        DBGPRINT(RT_DEBUG_TRACE, ("%d\t 0x%x/0x%x\t 0x%x \t 0x%x/0x%x \t 0x%x\n",
                    idx, (MibData.btcr[idx] >> 16) & 0xffff, MibData.btcr[idx] & 0xffff, MibData.btbcr[idx],
                    (MibData.brcr[idx] >> 16) & 0xffff, MibData.brcr[idx] & 0xffff, MibData.brbcr[idx]));
    }


    for (idx = 0; idx < 4; idx++)
    {
        HW_IO_READ32(pAd, MIB_MB0SDR0 + idx * 0x10, &MibData.mbxsdr[idx][0]);
        HW_IO_READ32(pAd, MIB_MB0SDR1 + idx * 0x10, &MibData.mbxsdr[idx][1]);
        HW_IO_READ32(pAd, MIB_MB0SDR2 + idx * 0x10, &MibData.mbxsdr[idx][2]);
    }
    DBGPRINT(RT_DEBUG_TRACE, ("===Per-MBSS Related MIB Counters===\n"));
    DBGPRINT(RT_DEBUG_TRACE, ("BSS Idx   RTSTx/RetryCnt  BAMissCnt  AckFailCnt  FrmRetry1/2Cnt\n"));
    for (idx = 0; idx < 4; idx++) {
        DBGPRINT(RT_DEBUG_TRACE, ("%d:\t0x%x/0x%x  0x%x \t 0x%x \t  0x%x/0x%x\n",
                    idx, MibData.mbxsdr[idx][0], (MibData.mbxsdr[idx][0] >> 16) & 0xffff,
                    MibData.mbxsdr[idx][1], (MibData.mbxsdr[idx][1] >> 16) & 0xffff,
                    MibData.mbxsdr[idx][2], (MibData.mbxsdr[idx][2] >> 16) & 0xffff));
    }


    for (idx = 0; idx < 8; idx++) {
        HW_IO_READ32(pAd, WTBL_MBTCRn + idx * 4, &MibData.mbtcr[idx]);
        HW_IO_READ32(pAd, WTBL_MBRCRn + idx * 4, &MibData.mbrcr[idx]);
    }
    
    for (idx = 0; idx < 16; idx++) {
        HW_IO_READ32(pAd, WTBL_MBTBCRn + idx * 4, &MibData.mbtbcr[idx]);
        HW_IO_READ32(pAd, WTBL_MBRBCRn + idx * 4, &MibData.mbrbcr[idx]);
    }   
    DBGPRINT(RT_DEBUG_TRACE, ("===Per-MBSS Related Tx/Rx Counters===\n"));
    DBGPRINT(RT_DEBUG_TRACE, ("MBSSIdx   TxDataCnt  TxByteCnt  RxDataCnt  RxByteCnt\n"));
    for (idx = 0; idx < 16; idx++) {
        DBGPRINT(RT_DEBUG_TRACE, ("%d\t 0x%x\t 0x%x \t 0x%x \t 0x%x\n",
                    idx, 
                    ((idx % 2 == 1) ? (MibData.mbtcr[idx/2] >> 16) & 0xffff : MibData.mbtcr[idx/2] & 0xffff), 
                    MibData.mbtbcr[idx],
                    ((idx % 2 == 1)  ? (MibData.mbrcr[idx/2] >> 16) & 0xffff : MibData.mbrcr[idx/2] & 0xffff), 
                    MibData.mbrbcr[idx]));
    }

    if (pMibData != NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("copy MIB\n"));
        PlatformMoveMemory(pMibData, &MibData, sizeof(MIB_DATA));
    }   
}

INT show_mib_proc2(MP_ADAPTER *pAd)
{
    UINT32 mac_val, idx, ampdu_cnt[5];
    UINT32 msdr6, msdr7, msdr8, msdr9, msdr10, msdr16, msdr17, msdr18;
    UINT32 mbxsdr[4][3];
    UINT32 mbtcr[16], mbtbcr[16], mbrcr[16], mbrbcr[16];
    UINT32 btcr[4], btbcr[4], brcr[4], brbcr[4];
    
    HW_IO_READ32(pAd, MIB_MSCR, &mac_val);
    DBGPRINT(RT_DEBUG_TRACE, ("MIB Status Control=0x%x\n", mac_val));
    HW_IO_READ32(pAd, MIB_MPBSCR, &mac_val);
    DBGPRINT(RT_DEBUG_TRACE, ("MIB Per-BSS Status Control=0x%x\n", mac_val));

    HW_IO_READ32(pAd, MIB_MSDR6, &msdr6);
    HW_IO_READ32(pAd, MIB_MSDR7, &msdr7);
    HW_IO_READ32(pAd, MIB_MSDR8, &msdr8);
    HW_IO_READ32(pAd, MIB_MSDR9, &msdr9);
    HW_IO_READ32(pAd, MIB_MSDR10, &msdr10);
    HW_IO_READ32(pAd, MIB_MSDR16, &msdr16);
    HW_IO_READ32(pAd, MIB_MSDR17, &msdr17);
    HW_IO_READ32(pAd, MIB_MSDR18, &msdr18);
    DBGPRINT(RT_DEBUG_TRACE, ("===Phy/Timing Related Counters===\n"));
    DBGPRINT(RT_DEBUG_TRACE, ("\tChannelIdleCnt=0x%x\n", msdr6 & 0xffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tCCA_NAV_Tx_Time=0x%x\n", msdr9 & 0xffffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tRx_MDRDY_CNT=0x%x\n", msdr10 & 0x3ffffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tPrim CCA Time=0x%x\n", msdr16 & 0xffffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tSec CCA Time=0x%x\n", msdr17 & 0xffffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tPrim ED Time=0x%x\n", msdr18 & 0xffffff));
    
    DBGPRINT(RT_DEBUG_TRACE, ("===Tx Related Counters(Generic)===\n"));
    HW_IO_READ32(pAd, MIB_MSDR0, &mac_val);
    DBGPRINT(RT_DEBUG_TRACE, ("\tBeaconTxCnt=0x%x\n", mac_val));
    HW_IO_READ32(pAd, MIB_MDR0, &mac_val);
    DBGPRINT(RT_DEBUG_TRACE, ("\tTx 40MHz Cnt=0x%x\n", (mac_val >> 16) & 0xffff));
    HW_IO_READ32(pAd, MIB_MDR1, &mac_val);
    DBGPRINT(RT_DEBUG_TRACE, ("\tTx 80MHz Cnt=0x%x\n", mac_val& 0xffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tTx 160MHz Cnt=0x%x\n", (mac_val >> 16) & 0xffff));
    
    DBGPRINT(RT_DEBUG_TRACE, ("===AMPDU Related Counters===\n"));
    HW_IO_READ32(pAd, MIB_MSDR12, &ampdu_cnt[0]);
    HW_IO_READ32(pAd, MIB_MSDR14, &ampdu_cnt[1]);
    HW_IO_READ32(pAd, MIB_MSDR15, &ampdu_cnt[2]);
    HW_IO_READ32(pAd, MIB_MDR2, &ampdu_cnt[3]);
    HW_IO_READ32(pAd, MIB_MDR3, &ampdu_cnt[4]);
    DBGPRINT(RT_DEBUG_TRACE, ("\tRx BA_Cnt=0x%x\n", ampdu_cnt[0] & 0xffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tTx AMPDU_Burst_Cnt=0x%x\n", (ampdu_cnt[0] >> 16 ) & 0xffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tTx AMPDU_Pkt_Cnt=0x%x\n", ampdu_cnt[1] & 0xffffff));   
    DBGPRINT(RT_DEBUG_TRACE, ("\tAMPDU SuccessCnt=0x%x\n", ampdu_cnt[2] & 0xffffff));   
    DBGPRINT(RT_DEBUG_TRACE, ("\tTx Agg Range: \t1 \t2~5 \t6~15 \t16~\n"));
    DBGPRINT(RT_DEBUG_TRACE, ("\t\t\t0x%x \t0x%x \t0x%x \t0x%x \n", 
                    (ampdu_cnt[3]) & 0xffff, (ampdu_cnt[3] >> 16) & 0xffff,
                    (ampdu_cnt[4]) & 0xffff, (ampdu_cnt[4] >> 16) & 0xfff));

    DBGPRINT(RT_DEBUG_TRACE, ("===Rx Related Counters(Generic)===\n"));
    DBGPRINT(RT_DEBUG_TRACE, ("\tVector Overflow Drop Cnt=0x%x\n", (msdr6 >> 16 ) & 0xffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tVector Missmacth Cnt=0x%x\n", (msdr7 >> 16 ) & 0xffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tDelimiter Fail Cnt=0x%x\n", msdr8& 0xffff));
    HW_IO_READ32(pAd, MIB_MSDR4, &mac_val);
    DBGPRINT(RT_DEBUG_TRACE, ("\tRxFifoFullCnt=0x%x\n", mac_val & 0xffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tRxFCSErrCnt=0x%x\n", (mac_val >> 16 ) & 0xffff));
    HW_IO_READ32(pAd, MIB_MSDR5, &mac_val);
    DBGPRINT(RT_DEBUG_TRACE, ("\tRxMPDUCnt=0x%x\n", mac_val & 0xffff));
    DBGPRINT(RT_DEBUG_TRACE, ("\tPFDropCnt=0x%x\n", (mac_val >> 16 ) & 0x00ff));
    HW_IO_READ32(pAd, MIB_MSDR22, &mac_val);
    DBGPRINT(RT_DEBUG_TRACE, ("\tRx AMPDU Cnt=0x%x\n", mac_val & 0xffff));
    HW_IO_READ32(pAd, MIB_MSDR23, &mac_val);
    DBGPRINT(RT_DEBUG_TRACE, ("\tRx Total ByteCnt=0x%x\n", mac_val));

    for (idx = 0; idx < 4; idx++) {
        HW_IO_READ32(pAd, WTBL_BTCRn + idx * 4, &btcr[idx]);
        HW_IO_READ32(pAd, WTBL_BTBCRn + idx * 4, &btbcr[idx]);
        HW_IO_READ32(pAd, WTBL_BRCRn + idx * 4, &brcr[idx]);
        HW_IO_READ32(pAd, WTBL_BRBCRn + idx * 4, &brbcr[idx]);
    }
    DBGPRINT(RT_DEBUG_TRACE, ("===Per-BSS Related Tx/Rx Counters===\n"));
    DBGPRINT(RT_DEBUG_TRACE, ("BSS Idx   TxCnt/DataCnt  TxByteCnt  RxCnt/DataCnt  RxByteCnt\n"));
    for (idx = 0; idx < 4; idx++) {
        DBGPRINT(RT_DEBUG_TRACE, ("%d\t 0x%x/0x%x\t 0x%x \t 0x%x/0x%x \t 0x%x\n",
                    idx, (btcr[idx] >> 16) & 0xffff, btcr[idx] & 0xffff, btbcr[idx],
                    (brcr[idx] >> 16) & 0xffff, brcr[idx] & 0xffff, brbcr[idx]));
    }


    for (idx = 0; idx < 4; idx++)
    {
        HW_IO_READ32(pAd, MIB_MB0SDR0 + idx * 0x10, &mbxsdr[idx][0]);
        HW_IO_READ32(pAd, MIB_MB0SDR1 + idx * 0x10, &mbxsdr[idx][1]);
        HW_IO_READ32(pAd, MIB_MB0SDR2 + idx * 0x10, &mbxsdr[idx][2]);
    }
    DBGPRINT(RT_DEBUG_TRACE, ("===Per-MBSS Related MIB Counters===\n"));
    DBGPRINT(RT_DEBUG_TRACE, ("BSS Idx   RTSTx/RetryCnt  BAMissCnt  AckFailCnt  FrmRetry1/2Cnt\n"));
    for (idx = 0; idx < 4; idx++) {
        DBGPRINT(RT_DEBUG_TRACE, ("%d:\t0x%x/0x%x  0x%x \t 0x%x \t  0x%x/0x%x\n",
                    idx, mbxsdr[idx][0], (mbxsdr[idx][0] >> 16) & 0xffff,
                    mbxsdr[idx][1], (mbxsdr[idx][1] >> 16) & 0xffff,
                    mbxsdr[idx][2], (mbxsdr[idx][2] >> 16) & 0xffff));
    }


    for (idx = 0; idx < 8; idx++) {
        HW_IO_READ32(pAd, WTBL_MBTCRn + idx * 4, &mbtcr[idx]);
        HW_IO_READ32(pAd, WTBL_MBRCRn + idx * 4, &mbrcr[idx]);
    }
    
    for (idx = 0; idx < 16; idx++) {
        HW_IO_READ32(pAd, WTBL_MBTBCRn + idx * 4, &mbtbcr[idx]);
        HW_IO_READ32(pAd, WTBL_MBRBCRn + idx * 4, &mbrbcr[idx]);
    }   
    DBGPRINT(RT_DEBUG_TRACE, ("===Per-MBSS Related Tx/Rx Counters===\n"));
    DBGPRINT(RT_DEBUG_TRACE, ("MBSSIdx   TxDataCnt  TxByteCnt  RxDataCnt  RxByteCnt\n"));
    for (idx = 0; idx < 16; idx++) {
        DBGPRINT(RT_DEBUG_TRACE, ("%d\t 0x%x\t 0x%x \t 0x%x \t 0x%x\n",
                    idx, 
                    ((idx % 2 == 1) ? (mbtcr[idx/2] >> 16) & 0xffff : mbtcr[idx/2] & 0xffff), 
                    mbtbcr[idx],
                    ((idx % 2 == 1)  ? (mbrcr[idx/2] >> 16) & 0xffff : mbrcr[idx/2] & 0xffff), 
                    mbrbcr[idx]));
    }
    
    return TRUE;
}


int get_low_mid_hi_index(UINT8 channel)
{
    char index = G_BAND_LOW;

    if (channel <= 14) {
        if (channel >= 1 && channel <= 5)
            index = G_BAND_LOW;
        else if (channel >= 6 && channel <= 10)
            index = G_BAND_MID;
        else if (channel >= 11 && channel <= 14)
            index = G_BAND_HI;
        else
            DBGPRINT(RT_DEBUG_ERROR, ("%s:illegal channel(%d)\n", __FUNCTION__, channel));
    } else {
        if (channel >= 184 && channel <= 188)
            index = A_BAND_LOW;
        else if (channel >= 192 && channel <= 196)
            index = A_BAND_HI;
        else if (channel >= 36 && channel <= 42)
            index = A_BAND_LOW;
        else if (channel >= 44 && channel <= 48)
            index = A_BAND_HI;
        else if (channel >= 52 && channel <= 56)
            index = A_BAND_LOW;
        else if (channel >= 58 && channel <= 64)
            index = A_BAND_HI;
        else if (channel >= 98 && channel <= 104)
            index = A_BAND_LOW;
        else if (channel >= 106 && channel <= 114)
            index = A_BAND_HI;
        else if (channel >= 116 && channel <= 128)
            index = A_BAND_LOW;
        else if (channel >= 130 && channel <= 144)
            index = A_BAND_HI;
        else if (channel >= 149 && channel <= 156)
            index = A_BAND_LOW;
        else if (channel >= 157 && channel <= 165)
            index = A_BAND_HI;
        else
            DBGPRINT(RT_DEBUG_ERROR, ("%s:illegal channel(%d)\n", __FUNCTION__, channel));
    }

    return index;
}

void mt7603_tx_pwr_gain(MP_ADAPTER *pAd, UINT8 channel)
{
    UINT32 value;
    CHAR tx_0_pwr;
    struct MT_TX_PWR_CAP cap;

//Eddy ADD
//    cap.tx_0_target_pwr_g_band = TX_TARGET_PWR_DEFAULT_VALUE;
      cap.tx_0_target_pwr_g_band = 0x9;
      //cap->tx_0_target_pwr_g_band = 34;
      cap.tx_0_chl_pwr_delta_g_band[get_low_mid_hi_index(channel)] = 0;
      cap.tx_pwr_g_band_ofdm_6_9 = 0;
      cap.tx_pwr_g_band_ofdm_12_18 = 0;
      cap.tx_pwr_g_band_ofdm_24_36 = 0;
      cap.tx_pwr_g_band_ofdm_48_54 = 0;
      cap.tx_pwr_ht_bpsk_mcs_0_8 = 0;
      cap.tx_pwr_ht_qpsk_mcs_1_2_9_10 = 0;
      cap.tx_pwr_ht_16qam_mcs_3_4_11_12 = 0;
      cap.tx_pwr_ht_64qam_mcs_5_13 = 0;
      cap.tx_pwr_ht_64qam_mcs_6_14 = 0;
      cap.tx_pwr_ht_64qam_mcs_7_15 = 0;
      cap.tx_pwr_cck_1_2 = 0;
      cap.tx_pwr_cck_5_11 = 0;
      cap.delta_tx_pwr_bw40_g_band = 0; 
 //Eddy ADD_END

    tx_0_pwr = cap.tx_0_target_pwr_g_band;
    tx_0_pwr += cap.tx_0_chl_pwr_delta_g_band[get_low_mid_hi_index(channel)];

    HW_IO_READ32(pAd, TMAC_FP0R0, &value);
    
    value &= ~LG_OFDM0_FRAME_POWER0_DBM_MASK;
    value |= LG_OFDM0_FRAME_POWER0_DBM(tx_0_pwr + cap.tx_pwr_g_band_ofdm_6_9);
    
    value &= ~LG_OFDM1_FRAME_POWER0_DBM_MASK;
    value |= LG_OFDM1_FRAME_POWER0_DBM(tx_0_pwr + cap.tx_pwr_g_band_ofdm_12_18);
    
    value &= ~LG_OFDM2_FRAME_POWER0_DBM_MASK;
    value |= LG_OFDM2_FRAME_POWER0_DBM(tx_0_pwr + cap.tx_pwr_g_band_ofdm_24_36);
    
    value &= ~LG_OFDM3_FRAME_POWER0_DBM_MASK;
    value |= LG_OFDM3_FRAME_POWER0_DBM(tx_0_pwr + cap.tx_pwr_g_band_ofdm_48_54);
    
    HW_IO_WRITE32(pAd, TMAC_FP0R0, value);

    HW_IO_READ32(pAd, TMAC_FP0R1, &value);
    
    value &= ~HT20_0_FRAME_POWER0_DBM_MASK;
    value |= HT20_0_FRAME_POWER0_DBM(tx_0_pwr + cap.tx_pwr_ht_bpsk_mcs_0_8);
    
    value &= ~HT20_1_FRAME_POWER0_DBM_MASK;
    value |= HT20_1_FRAME_POWER0_DBM(tx_0_pwr + cap.tx_pwr_ht_qpsk_mcs_1_2_9_10);
    
    value &= ~HT20_2_FRAME_POWER0_DBM_MASK;
    value |= HT20_2_FRAME_POWER0_DBM(tx_0_pwr + cap.tx_pwr_ht_16qam_mcs_3_4_11_12);
    
    value &= ~HT20_3_FRAME_POWER0_DBM_MASK;
    value |= HT20_3_FRAME_POWER0_DBM(tx_0_pwr + cap.tx_pwr_ht_64qam_mcs_5_13);
    
    HW_IO_WRITE32(pAd, TMAC_FP0R1, value);
    
    HW_IO_READ32(pAd, TMAC_FP0R2, &value);
    
    value &= ~HT40_0_FRAME_POWER0_DBM_MASK;
    value |= HT40_0_FRAME_POWER0_DBM(tx_0_pwr + cap.tx_pwr_ht_bpsk_mcs_0_8 
                                            + cap.delta_tx_pwr_bw40_g_band);
    
    value &= ~HT40_1_FRAME_POWER0_DBM_MASK;
    value |= HT40_1_FRAME_POWER0_DBM(tx_0_pwr + cap.tx_pwr_ht_qpsk_mcs_1_2_9_10 
                                            + cap.delta_tx_pwr_bw40_g_band);
    
    value &= ~HT40_2_FRAME_POWER0_DBM_MASK;
    value |= HT40_2_FRAME_POWER0_DBM(tx_0_pwr + cap.tx_pwr_ht_16qam_mcs_3_4_11_12
                                            + cap.delta_tx_pwr_bw40_g_band);
    
    value &= ~HT40_3_FRAME_POWER0_DBM_MASK;
    value |= HT40_3_FRAME_POWER0_DBM(tx_0_pwr + cap.tx_pwr_ht_64qam_mcs_5_13
                                            + cap.delta_tx_pwr_bw40_g_band);
    
    HW_IO_WRITE32(pAd, TMAC_FP0R2, value);
    
    HW_IO_READ32(pAd, TMAC_FP0R3, &value);
    
    value &= ~CCK0_FRAME_POWER0_DBM_MASK;
    value |= CCK0_FRAME_POWER0_DBM(tx_0_pwr + cap.tx_pwr_cck_1_2);
    
    value &= ~LG_OFDM4_FRAME_POWER0_DBM_MASK;
    value |= LG_OFDM4_FRAME_POWER0_DBM(tx_0_pwr + cap.tx_pwr_g_band_ofdm_48_54);    
    
    value &= ~CCK1_FRAME_POWER0_DBM_MASK;
    value |= CCK1_FRAME_POWER0_DBM(tx_0_pwr + cap.tx_pwr_cck_5_11);
    
    value &= ~HT40_6_FRAME_POWER0_DBM_MASK;
    value |= HT40_6_FRAME_POWER0_DBM(tx_0_pwr + 0 + cap.delta_tx_pwr_bw40_g_band);
    
    HW_IO_WRITE32(pAd, TMAC_FP0R3, value);
    
    HW_IO_READ32(pAd, TMAC_FP0R4, &value);
    
    value &= ~HT20_4_FRAME_POWER0_DBM_MASK;
    value |= HT20_4_FRAME_POWER0_DBM(tx_0_pwr + cap.tx_pwr_ht_64qam_mcs_6_14);
    
    value &= ~HT20_5_FRAME_POWER0_DBM_MASK;
    value |= HT20_5_FRAME_POWER0_DBM(tx_0_pwr + cap.tx_pwr_ht_64qam_mcs_7_15);  
    
    value &= ~HT40_4_FRAME_POWER0_DBM_MASK;
    value |= HT40_4_FRAME_POWER0_DBM(tx_0_pwr + cap.tx_pwr_ht_64qam_mcs_6_14
                                                + cap.delta_tx_pwr_bw40_g_band);
    
    value &= ~HT40_5_FRAME_POWER0_DBM_MASK;
    value |= HT40_5_FRAME_POWER0_DBM(tx_0_pwr + cap.tx_pwr_ht_64qam_mcs_7_15
                                                + cap.delta_tx_pwr_bw40_g_band);
    
    HW_IO_WRITE32(pAd, TMAC_FP0R4, value);
}

VOID AsicAddWAPIKey(
    IN PMP_ADAPTER pAd, 
    IN USHORT WCID, 
    IN PUCHAR pWPIEK, 
    IN UCHAR EKKeyLen, 
    IN PUCHAR pWPICK, 
    IN UCHAR CKKeyLen, 
    IN PUCHAR pPN, 
    IN UCHAR PNLen, 
    IN UCHAR KeyID, 
    IN BOOLEAN bPNIncrement, 
    IN BOOLEAN bUsePairewiseKeyTable)
{
    ULONG offset, WCIDAttri;
    RX_PN_PADDING_CFG_STRUC RxPnPadCfg = {0};

    DBGPRINT(RT_DEBUG_TRACE, ("AsicAddWAPIKey (WCID=%d) (EKKeyLen/CKKeyLen=%d/%d) (PNLen =%d) (KeyID=%d )(bPNIncrement=%d) \n", WCID, EKKeyLen,CKKeyLen, PNLen, KeyID, bPNIncrement));

    DumpFrameMessage(pWPIEK, EKKeyLen, ("WAPI pWPIEK"));
    DumpFrameMessage(pWPICK, CKKeyLen, ("WAPI pWPICK"));    
    DumpFrameMessage(pPN, PNLen, ("WAPI PN"));  

    // WPI key
    if(bUsePairewiseKeyTable == 0)
    {
        SHAREDKEY_MODE_STRUC csr1 = {0};
        
        //Share key table
        offset = SHARED_KEY_TABLE_BASE;
        RTUSBMultiWrite(pAd, (USHORT)(offset), pWPIEK, EKKeyLen);

        offset += EKKeyLen;
        RTUSBMultiWrite(pAd, (USHORT)(offset), pWPICK, CKKeyLen);

        //Share key mode
        RTUSBReadMACRegister(pAd, SHARED_KEY_MODE_BASE, &csr1.word);
        csr1.field.Bss0Key0CipherAlg = CIPHER_WAPI;
        RTUSBWriteMACRegister(pAd, SHARED_KEY_MODE_BASE, csr1.word);    
    }
    else
    {
        offset = PAIRWISE_KEY_TABLE_BASE + (WCID * HW_KEY_ENTRY_SIZE);
        RTUSBMultiWrite(pAd, (USHORT)(offset), pWPIEK, EKKeyLen);

        offset += EKKeyLen;
        RTUSBMultiWrite(pAd, (USHORT)(offset), pWPICK, CKKeyLen);
    }   

    // PN = {WAPI_PN_MSB(8) + EIV(4) + IV(4)}
    if (PNLen == 16)
    {
        offset = MAC_IVEIV_TABLE_BASE + (WCID * HW_IVEIV_ENTRY_SIZE);
        RTUSBMultiWrite(pAd, (USHORT)(offset), pPN, HW_IVEIV_ENTRY_SIZE);

        offset = MAC_WAPI_PN_TABLE_BASE + (WCID * HW_WAPI_PN_ENTRY_SIZE);
        RTUSBMultiWrite(pAd, (USHORT)(offset), (PUCHAR)(&pPN[HW_IVEIV_ENTRY_SIZE]), HW_WAPI_PN_ENTRY_SIZE);
    }

    //Attribure
    offset = MAC_WCID_ATTRIBUTE_BASE + (WCID * HW_WCID_ATTRI_SIZE);
    WCIDAttri = 0;
    if(bUsePairewiseKeyTable)
    {
        WCIDAttri = (bPNIncrement << 15) | (KeyID << 24) | (1 << 10) | bUsePairewiseKeyTable; // SMS4 on (bit 10)
    }
    else
    {
        WCIDAttri = (bPNIncrement << 15) | (KeyID << 24);
    }
    RTUSBWriteMACRegister(pAd, (USHORT)(offset), WCIDAttri);    
}
    
/*
    ========================================================================

    Routine Description:
        Set Cipher Key, Cipher algorithm, IV/EIV to Asic

    Arguments:
        pAd                     Pointer to our adapter
        WCID                    WCID Entry number.
        BssIndex                BSSID index, station or none multiple BSSID support 
                                this value should be 0.
        KeyIdx                  This KeyIdx will set to IV's KeyID if bTxKey enabled
        pCipherKey              Pointer to Cipher Key.
        bUsePairewiseKeyTable   TRUE means saved the key in SharedKey table, 
                                otherwise PairewiseKey table
        bTxKey                  This is the transmit key if enabled.

    Return Value:
        None 

    Note:
        This routine will set the relative key stuff to Asic including WCID attribute,
        Cipher Key, Cipher algorithm and IV/EIV.

        IV/EIV will be update if this CipherKey is the transmission key because 
        ASIC will base on IV's KeyID value to select Cipher Key.

        If bTxKey sets to FALSE, this is not the TX key, but it could be 
        RX key

        For AP mode bTxKey must be always set to TRUE.
    ========================================================================
*/
VOID AsicAddKeyEntry(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN UCHAR       WCID,
    IN UCHAR        BssIndex,
    IN UCHAR        KeyIdx,
    IN PCIPHER_KEY  pCipherKey, 
    IN BOOLEAN      bUsePairewiseKeyTable,
    IN BOOLEAN      bTxKey)
{
#if 1
    PMAC_TABLE_ENTRY pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, WCID);  
    
    DBGPRINT(RT_DEBUG_TRACE, ("==> AsicAddKeyEntry port(%d) (WCID=%d, KeyIdx=%d, bUsePairewiseKeyTable=%d, bTxKey=%d)\n", pPort->PortNumber,WCID, KeyIdx, bUsePairewiseKeyTable, bTxKey));
    DBGPRINT(RT_DEBUG_TRACE, ("%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x\n",
                                        pCipherKey->Key[0],
                                        pCipherKey->Key[1],
                                        pCipherKey->Key[2],
                                        pCipherKey->Key[3],
                                        pCipherKey->Key[4],
                                        pCipherKey->Key[5],
                                        pCipherKey->Key[6],
                                        pCipherKey->Key[7],
                                        pCipherKey->Key[8],
                                        pCipherKey->Key[9],
                                        pCipherKey->Key[10],
                                        pCipherKey->Key[11],
                                        pCipherKey->Key[12],
                                        pCipherKey->Key[13],
                                        pCipherKey->Key[14],
                                        pCipherKey->Key[15]
                                        ));

    DBGPRINT(RT_DEBUG_TRACE, ("AP AsicAddKeyEntry : BssIndex = %d KeyIdx = %d WCID = %d\n", BssIndex, KeyIdx, WCID));
    if (pPort->PortNumber == PORT_1)
    {
        if(pWcidMacTabEntry == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, WCID));
            return;
        }    
        
        if (bUsePairewiseKeyTable)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("AP AsicAddKeyEntry : Pairewise Key, KeyIdx = %d\n", KeyIdx));
            CmdProcAddRemoveKey(pAd, 0, BssIndex, KeyIdx, (UCHAR)WCID, PAIRWISEKEYTABLE, pCipherKey, pWcidMacTabEntry->Addr);
        }
        else if (pCipherKey->CipherAlg == CIPHER_WEP64 || pCipherKey->CipherAlg == CIPHER_WEP128)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("AP AsicAddKeyEntry : SHAREDKEYTABLE....WEP\n"));
            CmdProcAddRemoveKey(pAd, 0, BssIndex, KeyIdx, (UCHAR)WCID, SHAREDKEYTABLE, pCipherKey, pWcidMacTabEntry->Addr);
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("AP AsicAddKeyEntry : SHAREDKEYTABLE....Group Key, KeyIdx = %d\n", KeyIdx));
            CmdProcAddRemoveKey(pAd, 0, BssIndex, KeyIdx, WCID, SHAREDKEYTABLE, pCipherKey, BROADCAST_ADDR);
        }
    }
    else
    {
        if (bUsePairewiseKeyTable)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("AsicAddKeyEntry : Pairewise Key\n"));
            CmdProcAddRemoveKey(pAd, 0, BssIndex, KeyIdx, (UCHAR)WCID, PAIRWISEKEYTABLE, pCipherKey, pPort->PortCfg.Bssid);
        }
        else if (pCipherKey->CipherAlg == CIPHER_WEP64 || pCipherKey->CipherAlg == CIPHER_WEP128)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("AsicAddKeyEntry : SHAREDKEYTABLE....WEP\n"));
            CmdProcAddRemoveKey(pAd, 0, BssIndex, KeyIdx, (UCHAR)WCID, SHAREDKEYTABLE, pCipherKey, pPort->PortCfg.Bssid);
        }
        else
        {
            UCHAR Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_MBCAST);
            DBGPRINT(RT_DEBUG_TRACE, ("AsicAddKeyEntry : SHAREDKEYTABLE....Group Key, KeyIdx = %d\n", KeyIdx));
            CmdProcAddRemoveKey(pAd, 0, BssIndex, KeyIdx /*(KeyIdx == 1 ? 2 : 1)*/, Wcid, SHAREDKEYTABLE, pCipherKey, BROADCAST_ADDR);
        }
    }


#else
    ULONG   offset;
    UCHAR   IvEIV[8];
    PUCHAR  pKey = pCipherKey->Key;
    PUCHAR  pTxMic = pCipherKey->TxMic;
    PUCHAR  pRxMic = pCipherKey->RxMic;
    PUCHAR  pTxtsc = pCipherKey->TxTsc;
    UCHAR   CipherAlg = pCipherKey->CipherAlg;
    SHAREDKEY_MODE_STRUC csr1;

//  ASSERT(KeyLen <= MAX_LEN_OF_PEER_KEY);

    DBGPRINT(RT_DEBUG_TRACE, ("==> AsicAddKeyEntry (WCID=%d, KeyIdx=%d, bUsePairewiseKeyTable=%d, bTxKey=%d)\n", WCID, KeyIdx, bUsePairewiseKeyTable, bTxKey));
    DBGPRINT(RT_DEBUG_TRACE, ("%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x\n",
        pCipherKey->Key[0],
        pCipherKey->Key[1],
        pCipherKey->Key[2],
        pCipherKey->Key[3],
        pCipherKey->Key[4],
        pCipherKey->Key[5],
        pCipherKey->Key[6],
        pCipherKey->Key[7],
        pCipherKey->Key[8],
        pCipherKey->Key[9],
        pCipherKey->Key[10],
        pCipherKey->Key[11],
        pCipherKey->Key[12],
        pCipherKey->Key[13],
        pCipherKey->Key[14],
        pCipherKey->Key[15]
        ));

    //
    // 1.) decide key table offset
    //
    if (bUsePairewiseKeyTable)
        offset = PAIRWISE_KEY_TABLE_BASE + (WCID * HW_KEY_ENTRY_SIZE);
    else
        offset = SHARED_KEY_TABLE_BASE + (4 * BssIndex + KeyIdx) * HW_KEY_ENTRY_SIZE;

    //
    // 2.) Set Key to Asic
    //  
    RTUSBMultiWrite(pAd, (USHORT) offset, pKey, MAX_LEN_OF_PEER_KEY);
    offset += MAX_LEN_OF_PEER_KEY;

    //
    // 3.) Set MIC key if available
    //
    if (pTxMic)
    {       
        RTUSBMultiWrite(pAd, (USHORT) offset, pTxMic, 8);
    }
    offset += LEN_TKIP_TXMICK;
        
    if (pRxMic)
    {
        RTUSBMultiWrite(pAd, (USHORT) offset, pRxMic, 8);
    }

    //
    // 4.) Modify IV/EIV if needs
    //     This will force Asic to use this key ID by setting IV.
    //
    if (bTxKey)
    {
        offset = PAIRWISE_IVEIV_TABLE_BASE + (WCID * HW_IVEIV_ENTRY_SIZE);
        PlatformZeroMemory(IvEIV,8);        
        //
        // Write IV/EIV
        //  

        // IEEE 802.11i Standard, Section 8.3.3.4.3 PN and replay dection, Page 62
        // The PN shall be implemented as a 48-bit monotonically incrementing non-negative integer,
        // initialized to 1 when the corresponding temporal key is initialize or refreshed.
        IvEIV[0] = pTxtsc[0];   
        IvEIV[3] = (KeyIdx << 6);
        if ((CipherAlg == CIPHER_TKIP) || (CipherAlg == CIPHER_TKIP_NO_MIC) ||(CipherAlg == CIPHER_AES))
            IvEIV[3] |= 0x20;   // turn on extension bit means EIV existence

        RTUSBMultiWrite(pAd, (USHORT) offset, IvEIV,8);
        AsicUpdateWCIDAttribute(pAd,pPort, WCID, BssIndex, CipherAlg, bUsePairewiseKeyTable);
    }

    if (!bUsePairewiseKeyTable)
    {
        //
        // Only update the shared key security mode
        //
        RTUSBReadMACRegister(pAd, SHARED_KEY_MODE_BASE + 4 * (BssIndex / 2), &csr1.word);
        if ((BssIndex % 2) == 0)
        {
            if (KeyIdx == 0)
                csr1.field.Bss0Key0CipherAlg = CipherAlg;
            else if (KeyIdx == 1)
                csr1.field.Bss0Key1CipherAlg = CipherAlg;
            else if (KeyIdx == 2)
                csr1.field.Bss0Key2CipherAlg = CipherAlg;
            else
                csr1.field.Bss0Key3CipherAlg = CipherAlg;
        }
        else
        {
            if (KeyIdx == 0)
                csr1.field.Bss1Key0CipherAlg = CipherAlg;
            else if (KeyIdx == 1)
                csr1.field.Bss1Key1CipherAlg = CipherAlg;
            else if (KeyIdx == 2)
                csr1.field.Bss1Key2CipherAlg = CipherAlg;
            else
                csr1.field.Bss1Key3CipherAlg = CipherAlg;
        }
        RTUSBWriteMACRegister(pAd, SHARED_KEY_MODE_BASE + 4 * (BssIndex / 2), csr1.word);           
    }
#endif

    DBGPRINT(RT_DEBUG_TRACE, ("<== AsicAddKeyEntry\n"));
}

//  IRQL = DISPATCH_LEVEL
VOID AsicAddSharedKeyEntry(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN UCHAR         BssIndex,
    IN UCHAR         KeyIdx,
    IN UCHAR         CipherAlg,
    IN PUCHAR        pKey,
    IN PUCHAR        pTxMic,
    IN PUCHAR        pRxMic)
{
    INT   i;
    ULONG offset;
    SHAREDKEY_MODE_STRUC csr1;
    UCHAR       KeyBuffer[MAX_LEN_OF_SHARE_KEY + (MIC_KEY_LEN * 2)];
    USHORT      KeyLen = 0;
    ULONG       WCIDAttri;
    ULONG       Value;

    DBGPRINT(RT_DEBUG_ERROR,("AsicAddSharedKeyEntry: %s key #%d\n", DecodeCipherName(CipherAlg), BssIndex*4 + KeyIdx));
    DBGPRINT_RAW(RT_DEBUG_TRACE, ("     Key ="));
    for (i = 0; i < 16; i++)
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x:", pKey[i]));
    }
    DBGPRINT_RAW(RT_DEBUG_TRACE, ("\n"));
    if (pRxMic)
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("     Rx MIC Key = "));
        for (i = 0; i < 8; i++)
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x:", pRxMic[i]));
        }
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("\n"));
    }
    if (pTxMic)
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("     Tx MIC Key = "));
        for (i = 0; i < 8; i++)
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%02x:", pTxMic[i]));
        }
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("\n"));
    }


    //
    // fill key material - key + TX MIC + RX MIC
    //
    offset = SHARED_KEY_TABLE_BASE + (4*BssIndex + KeyIdx)*HW_KEY_ENTRY_SIZE;

    PlatformMoveMemory(KeyBuffer, pPort->SharedKey[BSS0][KeyIdx].Key, MAX_LEN_OF_SHARE_KEY);
    KeyLen = MAX_LEN_OF_SHARE_KEY;

    if (pTxMic)
    {
        PlatformMoveMemory(KeyBuffer + KeyLen, pPort->SharedKey[BSS0][KeyIdx].TxMic, MIC_KEY_LEN);
        KeyLen += MIC_KEY_LEN;
    }

    if (pRxMic)
    {
        PlatformMoveMemory(KeyBuffer + KeyLen, pPort->SharedKey[BSS0][KeyIdx].RxMic, MIC_KEY_LEN);
        KeyLen += MIC_KEY_LEN;
    }

    RTUSBMultiWrite(pAd, (USHORT) offset, KeyBuffer, KeyLen);


    // WCID Attribute UDF:3, BSSIdx:3, Alg:3, Keytable:1=PAIRWISE KEY, BSSIdx is 0
    if ((CipherAlg == CIPHER_TKIP) || (CipherAlg == CIPHER_TKIP_NO_MIC) || (CipherAlg == CIPHER_AES))
        WCIDAttri = (CipherAlg<<1)|SHAREDKEYTABLE;
    else
        WCIDAttri = (CipherAlg<<1)|SHAREDKEYTABLE;
    
    offset = MAC_WCID_ATTRIBUTE_BASE;
    RTUSBWriteMACRegister(pAd, (USHORT)offset, WCIDAttri);
    RTUSBReadMACRegister(pAd, (USHORT)offset, &Value);
    DBGPRINT(RT_DEBUG_TRACE, ("MCAST_WCID : offset = %x, WCIDAttri = %x\n", offset, WCIDAttri));

    //
    // Update cipher algorithm. WSTA always use BSS0
    //
    RTUSBReadMACRegister(pAd, SHARED_KEY_MODE_BASE+4*(BssIndex/2), &csr1.word);
    if ((BssIndex%2) == 0)
    {
        if (KeyIdx == 0)
            csr1.field.Bss0Key0CipherAlg = CipherAlg;
        else if (KeyIdx == 1)
            csr1.field.Bss0Key1CipherAlg = CipherAlg;
        else if (KeyIdx == 2)
            csr1.field.Bss0Key2CipherAlg = CipherAlg;
        else
            csr1.field.Bss0Key3CipherAlg = CipherAlg;
    }
    else
    {
        if (KeyIdx == 0)
            csr1.field.Bss1Key0CipherAlg = CipherAlg;
        else if (KeyIdx == 1)
            csr1.field.Bss1Key1CipherAlg = CipherAlg;
        else if (KeyIdx == 2)
            csr1.field.Bss1Key2CipherAlg = CipherAlg;
        else
            csr1.field.Bss1Key3CipherAlg = CipherAlg;
    }
    RTUSBWriteMACRegister(pAd, SHARED_KEY_MODE_BASE+4*(BssIndex/2), csr1.word);

}


VOID AsicRemoveSharedKeyMode(
    IN PMP_ADAPTER pAd,
    IN UCHAR         BssIndex,
    IN UCHAR         KeyIdx)
{
    SHAREDKEY_MODE_STRUC csr1;
    USHORT                offset;

    ASSERT(BssIndex < 8);       // RT2860 support 8 multiple BSSID
    ASSERT(KeyIdx < 4);

    DBGPRINT(RT_DEBUG_TRACE,("AsicRemoveSharedKeyMode: BssIndex=%d, #%d \n", BssIndex, BssIndex * 4 + KeyIdx));

    offset = SHARED_KEY_MODE_BASE + 4 * (BssIndex / 2);

    RTUSBReadMACRegister(pAd, offset, &csr1.word);
    if ((BssIndex % 2) == 0)
    {
        if (KeyIdx == 0)
            csr1.field.Bss0Key0CipherAlg = 0;
        else if (KeyIdx == 1)
            csr1.field.Bss0Key1CipherAlg = 0;
        else if (KeyIdx == 2)
            csr1.field.Bss0Key2CipherAlg = 0;
        else
            csr1.field.Bss0Key3CipherAlg = 0;
    }
    else
    {
        if (KeyIdx == 0)
            csr1.field.Bss1Key0CipherAlg = 0;
        else if (KeyIdx == 1)
            csr1.field.Bss1Key1CipherAlg = 0;
        else if (KeyIdx == 2)
            csr1.field.Bss1Key2CipherAlg = 0;
        else
            csr1.field.Bss1Key3CipherAlg = 0;
    }
    RTUSBWriteMACRegister(pAd, offset, csr1.word);
}


VOID AsicRemoveKeyEntry(
    IN PMP_ADAPTER pAd,
    IN USHORT        WCID,
    IN UCHAR        KeyIdx,
    IN BOOLEAN      bUsePairewiseKeyTable    
    )
{
#if 0
    PMAC_TABLE_ENTRY pWcidMacTabEntry = &pAd->MacTabPoolArray[WCID];
    UCHAR BssIndex = Ndis6CommonGetBssidIndex(pAd, pWcidMacTabEntry->pPort, MULTI_BSSID_MODE);

    DBGPRINT(RT_DEBUG_TRACE,("AsicRemoveKeyEntry: #%d \n", WCID));

    if (bUsePairewiseKeyTable == TRUE)
    {
        CmdProcAddRemoveKey(pAd, 1, BssIndex, KeyIdx, pWcidMacTabEntry->wcid, PAIRWISEKEYTABLE, &pWcidMacTabEntry->PairwiseKey, pWcidMacTabEntry->Addr);
    }
    else
    {
        CmdProcAddRemoveKey(pAd, 1, BssIndex, KeyIdx, pWcidMacTabEntry->wcid, SHAREDKEYTABLE, &pWcidMacTabEntry->PairwiseKey, pWcidMacTabEntry->Addr);
    }

    CmdProcAddRemoveKey(pAd, 1, BssIndex, KeyIdx, pWcidMacTabEntry->wcid, SHAREDKEYTABLE, &pWcidMacTabEntry->PairwiseKey, BROADCAST_ADDR);
#endif    
}



//  IRQL = DISPATCH_LEVEL
VOID AsicRemoveSharedKeyEntry(
    IN PMP_ADAPTER pAd,
    IN UCHAR         BssIndex,
    IN UCHAR         KeyIdx)
{
    SHAREDKEY_MODE_STRUC csr1;

    DBGPRINT(RT_DEBUG_TRACE,("AsicRemoveSharedKeyEntry: #%d \n", BssIndex*4 + KeyIdx));

    RTUSBReadMACRegister(pAd, SHARED_KEY_MODE_BASE+4*(BssIndex/2), &csr1.word);
    if ((BssIndex%2) == 0)
    {
        if (KeyIdx == 0)
            csr1.field.Bss0Key0CipherAlg = 0;
        else if (KeyIdx == 1)
            csr1.field.Bss0Key1CipherAlg = 0;
        else if (KeyIdx == 2)
            csr1.field.Bss0Key2CipherAlg = 0;
        else
            csr1.field.Bss0Key3CipherAlg = 0;
    }
    else
    {
        if (KeyIdx == 0)
            csr1.field.Bss1Key0CipherAlg = 0;
        else if (KeyIdx == 1)
            csr1.field.Bss1Key1CipherAlg = 0;
        else if (KeyIdx == 2)
            csr1.field.Bss1Key2CipherAlg = 0;
        else
            csr1.field.Bss1Key3CipherAlg = 0;
    }
    DBGPRINT(RT_DEBUG_TRACE,("Write: SHARED_KEY_MODE_BASE at this Bss[%d] = 0x%x \n", BssIndex, csr1.word));
    RTUSBWriteMACRegister(pAd, SHARED_KEY_MODE_BASE+4*(BssIndex/2), csr1.word);
    ASSERT(BssIndex < 8);
    ASSERT(KeyIdx < 4);

}

//  IRQL = DISPATCH_LEVEL
VOID AsicRemovePairwiseKeyEntry(
    IN PMP_ADAPTER pAd,
    IN UCHAR         KeyIdx)
{
}

BOOLEAN AsicSendCommanToMcu(
    IN PMP_ADAPTER pAd,
    IN UCHAR         Command,
    IN UCHAR         Token,
    IN UCHAR         Arg0,
    IN UCHAR         Arg1)
{
    HOST_CMD_CSR_STRUC  H2MCmd;
    H2M_MAILBOX_STRUC   H2MMailbox;
    ULONG               i = 0;
    ULONG               Value = 0;
    NTSTATUS    Status = STATUS_SUCCESS;

    //2 TODO: New MCU command method
    
    if(pAd->bInShutdown && (pAd->Shutdowncnt == 0))
        return FALSE;


    do
    {
        RTUSBReadMACRegister(pAd, H2M_MAILBOX_CSR, &H2MMailbox.word);
        if (H2MMailbox.field.Owner == 0)
            break;
        if(pAd->bInShutdown && (pAd->Shutdowncnt == 0))
            return FALSE;
        Delay_us(2);
        DBGPRINT(RT_DEBUG_INFO, ("AsicSendCommanToMcu::Mail box is busy\n"));
    } while(i++ < 100);

    if (i >= 100)
    {
        DBGPRINT_ERR(("H2M_MAILBOX still hold by MCU. command fail\n"));
        return FALSE;
    }

    H2MMailbox.field.Owner    = 1;     // pass ownership to MCU
    H2MMailbox.field.CmdToken = Token;
    H2MMailbox.field.HighByte = Arg1;
    H2MMailbox.field.LowByte  = Arg0;
    RTUSBWriteMACRegister(pAd, H2M_MAILBOX_CSR, H2MMailbox.word);

    H2MCmd.word               = 0;
    H2MCmd.field.HostCommand  = Command;

    Status = RTUSBWriteMACRegister(pAd, HOST_CMD_CSR, H2MCmd.word);
 
    //vendor command fail retry again
    if(!NT_SUCCESS(Status)) 
    {
        DBGPRINT_ERR(("HOST_CMD_CSR write fail . retry again\n"));
        NdisCommonGenericDelay(10);

        Status = RTUSBWriteMACRegister(pAd, HOST_CMD_CSR, H2MCmd.word);
    }

    if (Command != 0x80)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("SW interrupt MCU (cmd=0x%02x, token=0x%02x, arg1,arg0=0x%02x,0x%02x)\n",
            H2MCmd.field.HostCommand, Token, Arg1, Arg0));
    }

    // Wait for MCU WakeUp cmd succeed. (from 2860 driver)
    if (Command == WAKE_MCU_CMD)
    {
            Delay_us(2000);
    }
    else if (Command == RFTUNE_MCU_CMD)
    {
        Delay_us(200);
    }
    else if (Command == 0x68)
        {
        do
        {
            RTUSBReadMACRegister(pAd, 0x7070, &Value);
            if ((Value & 0xff) == 0x78)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("0x68 command sent Sucessful, Value= 0x%x\n", Value));
                break;
            }
            if(pAd->bInShutdown && (pAd->Shutdowncnt == 0))
                return FALSE;
            Delay_us(2);
            DBGPRINT(RT_DEBUG_TRACE, ("AsicSendCommanToMcu::Mail box is busy, %d\n", i));
        } while(i++ < 100);

        if (i >= 100)
        {
            DBGPRINT_ERR(("0x68 command fail, H2MMailbox.word= 0x%x\n", H2MMailbox.word));
            return FALSE;
        }       
        }

    return TRUE;
}

/*
    ========================================================================

    Routine Description:
        Verify the support rate for different PHY type

    Arguments:
        pAd                 Pointer to our adapter

    Return Value:
        None

    IRQL = PASSIVE_LEVEL

    ========================================================================
*/
VOID    RTMPCheckRates(
    IN      PMP_ADAPTER   pAd,
    IN OUT  UCHAR           SupRate[],
    IN OUT  UCHAR           *SupRateLen)
{
    UCHAR   RateIdx, i, j;
    UCHAR   NewRate[12], NewRateLen;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    
    NewRateLen = 0;
    
    if ((pPort->CommonCfg.PhyMode == PHY_11B) || 
        ((pAd->StaCfg.BssType == BSS_ADHOC) && (pAd->StaCfg.AdhocMode == ADHOC_11B) && (pPort->ScaningChannel <= 14)))
        RateIdx = 4;
    else
        RateIdx = 12;

    // Check for support rates exclude basic rate bit   
    for (i = 0; i < *SupRateLen; i++)
        for (j = 0; j < RateIdx; j++)
            if ((SupRate[i] & 0x7f) == RateIdTo500Kbps[j])
                NewRate[NewRateLen++] = SupRate[i];
            
    *SupRateLen = NewRateLen;
#pragma prefast(suppress: __WARNING_USING_UNINIT_VAR, "NewRate already be prepared above")          
    PlatformMoveMemory(SupRate, NewRate, NewRateLen);
}

BOOLEAN RTMPCheckChannel(
    IN PMP_ADAPTER pAd,
    IN UCHAR        CentralChannel,
    IN UCHAR        Channel)
{
    UCHAR       k;
    UCHAR       UpperChannel = 0, LowerChannel = 0;
    BOOLEAN     bWideBand = FALSE;
    UCHAR       NoEffectChannelinList = 0;
    
    // Find upper and lower channel according to 40MHz current operation. 
    if (CentralChannel < Channel)
    {
        UpperChannel = Channel;
        if (CentralChannel > 2)
            LowerChannel = CentralChannel - 2;
    }
    else if (CentralChannel > Channel)
    {
        UpperChannel = CentralChannel + 2;
        LowerChannel = Channel;
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("LinkUP 20MHz . No Effected Channel \n"));
        // Now operating in 20MHz, doesn't find 40MHz effected channels
        return bWideBand;
    }

    for (k = 0;k < pAd->HwCfg.ChannelListNum;k++)
    {
        if (pAd->HwCfg.ChannelList[k].Channel == UpperChannel)
        {
            DBGPRINT(RT_DEBUG_TRACE,("UpperChannel=%d  is in Channel List[%d]\n", UpperChannel, k ));
            NoEffectChannelinList ++;
        }
        if (pAd->HwCfg.ChannelList[k].Channel == LowerChannel)
        {
            DBGPRINT(RT_DEBUG_TRACE,("LowerChannel=%d  is in Channel List[%d]\n", LowerChannel, k ));
            NoEffectChannelinList ++;
        }
    }

    DBGPRINT(RT_DEBUG_TRACE,("Total Channel in Channel List = [%d]\n", NoEffectChannelinList));
    if (NoEffectChannelinList == 2)
        bWideBand = TRUE;

    return bWideBand;
}

/*
    ========================================================================

    Routine Description:
        Verify the support rate for HT phy type

    Arguments:
        pAd                 Pointer to our adapter

    Return Value:
        FALSE if pPort->CommonCfg.SupportedHtPhy doesn't accept the pHtCapability.  (AP Mode)

    IRQL = PASSIVE_LEVEL

    ========================================================================
*/
BOOLEAN     RTMPCheckHt(
    IN      PMP_ADAPTER   pAd,
    IN      PMP_PORT      pPort,
    IN      UCHAR   Wcid,
    IN OUT PHT_CAPABILITY_IE pHtCapability, 
    IN OUT PADD_HT_INFO_IE pAddHtInfo, 
    IN BOOLEAN bVhtCapable, 
    IN PVHT_CAP_IE pVhtCapability)
{
    BOOLEAN     bWideBand;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid);  
    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
        return FALSE;
    }

    if (Wcid >= MAX_LEN_OF_MAC_TABLE)
        return FALSE;

    // If use AMSDU, set flag.
    if (pPort->CommonCfg.DesiredHtPhy.AmsduEnable)
        CLIENT_STATUS_SET_FLAG(pWcidMacTabEntry, fCLIENT_STATUS_AMSDU_INUSED);
    // Save Peer Capability
    if (pHtCapability->HtCapInfo.ShortGIfor20)
        CLIENT_STATUS_SET_FLAG(pWcidMacTabEntry, fCLIENT_STATUS_SGI20_CAPABLE);
    if (pHtCapability->HtCapInfo.ShortGIfor40)
        CLIENT_STATUS_SET_FLAG(pWcidMacTabEntry, fCLIENT_STATUS_SGI40_CAPABLE);
    if (pHtCapability->HtCapInfo.TxSTBC)
        CLIENT_STATUS_SET_FLAG(pWcidMacTabEntry, fCLIENT_STATUS_HT_TX_STBC_CAPABLE);
    if (pHtCapability->HtCapInfo.RxSTBC)
        CLIENT_STATUS_SET_FLAG(pWcidMacTabEntry, fCLIENT_STATUS_HT_RX_STBC_CAPABLE);
    if (pPort->CommonCfg.bRdg && pHtCapability->ExtHtCapInfo.RDGSupport)
    {
            CLIENT_STATUS_SET_FLAG(pWcidMacTabEntry, fCLIENT_STATUS_RDG_CAPABLE);
    }
    
    if (Wcid < MAX_LEN_OF_MAC_TABLE)
    {
        //
        // SVD team: Atheros AP/station will advertise 0 in HT capability.
        // For IOT issue, MPDU density will not set to less than 5(4us).
        //
        pWcidMacTabEntry->MpduDensity = max(5, pHtCapability->HtCapParm.MpduDensity);
        DBGPRINT(RT_DEBUG_TRACE, ("%s, AP advertises MPDU density %d, %d will be used.\n",
            __FUNCTION__,
            pHtCapability->HtCapParm.MpduDensity,
            pWcidMacTabEntry->MpduDensity));
    }
    
    if (((pPort->PortSubtype == PORTSUBTYPE_STA) && (MlmeSyncGetRoleTypeByWlanIdx(pPort, Wcid) == ROLE_WLANIDX_BSSID)) || (pPort->PortSubtype == PORTSUBTYPE_P2PClient))
    {
        if (BaSizeArray[pHtCapability->HtCapParm.MaxRAmpduFactor] > pPort->CommonCfg.BACapability.field.TxBAWinLimit)
            pWcidMacTabEntry->BaSizeInUse = (UCHAR)pPort->CommonCfg.BACapability.field.TxBAWinLimit;
        else
            pWcidMacTabEntry->BaSizeInUse = BaSizeArray[pHtCapability->HtCapParm.MaxRAmpduFactor];
    }

    // This parameter is used to tell AP what we support in Rx path (fix bug when 1T2R)
    pAd->MlmeAux.HtCapability.MCSSet[0] = 0xff;
    pAd->MlmeAux.HtCapability.MCSSet[2] = 0x00;
    pAd->MlmeAux.HtCapability.MCSSet[4] = 0x1;
    if (pAd->HwCfg.Antenna.field.RxPath >= 2) // 3: 3Rx   2: 2Rx   1: 1Rx
    {
        if (pAd->CommonCfg.IOTestParm.AtherosDIR855)
        {
            pAd->MlmeAux.HtCapability.MCSSet[1] = 0x3f; // MCS-13
        }
        else if (pAd->CommonCfg.RxStream == 3)
        {
            pAd->MlmeAux.HtCapability.MCSSet[1] = 0xff;
            pAd->MlmeAux.HtCapability.MCSSet[2] = 0xff;
        }                   
        else
        {
            pAd->MlmeAux.HtCapability.MCSSet[1] = 0xff;
        }
    }
    else
    {
        pAd->MlmeAux.HtCapability.MCSSet[1] = 0x00;
    }

    // For WTE STA CLI inteface
    if (pPort->CommonCfg.TgnControl.field.UseWTESupportedMCS == TRUE)
    {
        pAd->MlmeAux.HtCapability.MCSSet[0] =  (UCHAR)(pPort->CommonCfg.WTESupportedMCS&0x00FF);
        
        if (pAd->HwCfg.Antenna.field.RxPath >= 2)
        {
            pAd->MlmeAux.HtCapability.MCSSet[1] =  (UCHAR)((pPort->CommonCfg.WTESupportedMCS&0xFF00)>>8);
        }
        
        if (pAd->HwCfg.Antenna.field.RxPath >= 3) 
        {
            pAd->MlmeAux.HtCapability.MCSSet[2] =  (UCHAR)((pPort->CommonCfg.WTESupportedMCS&0xFF0000)>>16);
        }

        DBGPRINT(RT_DEBUG_TRACE, ("%s:WTESupportedMCS 0x%08x MCSSet[0]=0x%02x, MCSSet[1]=0x%02x\n",__FUNCTION__,pPort->CommonCfg.WTESupportedMCS,pAd->MlmeAux.HtCapability.MCSSet[0],pAd->MlmeAux.HtCapability.MCSSet[1]));           
    }

    if(pPort->CommonCfg.TgnControl.field.DisableSupportMCS32 == TRUE)     
    {
        pAd->MlmeAux.HtCapability.MCSSet[4] =  0;
    }

    //
    // decide bandwidth
    //  
    if (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
    {
        if (pPort->Channel > 14)
            pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = pAddHtInfo->AddHtInfo.RecomWidth & pPort->CommonCfg.DesiredHtPhy.HtChannelWidth & !pAd->HwCfg.NicConfig2.field.bDisableBW40ForA;
        else
            pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = pAddHtInfo->AddHtInfo.RecomWidth & pPort->CommonCfg.DesiredHtPhy.HtChannelWidth & !pAd->HwCfg.NicConfig2.field.bDisableBW40ForG;
    }
    else
    {
        pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = pAddHtInfo->AddHtInfo.RecomWidth & pPort->CommonCfg.DesiredHtPhy.HtChannelWidth;
    }
    DBGPRINT(RT_DEBUG_TRACE,("%s(): pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = %d, pAddHtInfo->AddHtInfo.RecomWidth = %d, pPort->CommonCfg.DesiredHtPhy.HtChannelWidth = %d, pAd->HwCfg.NicConfig2.field.bDisableBW40ForG = %d\n",
            __FUNCTION__,
            pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth,
            pAddHtInfo->AddHtInfo.RecomWidth,
            pPort->CommonCfg.DesiredHtPhy.HtChannelWidth,
            pAd->HwCfg.NicConfig2.field.bDisableBW40ForG));   
    

    if (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40)
    {
        // If both station and AP use 40MHz, still need to check if the 40MHZ band's legality in my country region
        bWideBand = RTMPCheckChannel(pAd, pPort->CentralChannel, pPort->Channel);
        // If this 40MHz wideband is not allowed in my country list, use bandwidth 20MHZ instead,
        // If this 40MHz wideband is not allowed in my country list, use bandwidth 20MHZ instead,
        if (bWideBand == FALSE)
        {
            pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = BW_20;
            pPort->CommonCfg.DesiredHtPhy.HtChannelWidth = BW_20;
        }
    }
    DBGPRINT(RT_DEBUG_TRACE, ("%s:: HtCapInfo.ChannelWidth=%d, RecomWidth=%d, DesiredHtPhy.HtChannelWidth=%d, BW40MAvailForA/G=%d/%d, PhyMode=%d \n",
        __FUNCTION__,
        pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth,
        pAddHtInfo->AddHtInfo.RecomWidth,
        pPort->CommonCfg.DesiredHtPhy.HtChannelWidth,
        pAd->HwCfg.NicConfig2.field.bDisableBW40ForA,
        pAd->HwCfg.NicConfig2.field.bDisableBW40ForG,
        pPort->CommonCfg.PhyMode));
    
    pAd->MlmeAux.HtCapability.HtCapInfo.GF =  pHtCapability->HtCapInfo.GF &pPort->CommonCfg.DesiredHtPhy.GF;

    // Send Assoc Req with my HT capability.
    pAd->MlmeAux.HtCapability.HtCapInfo.AMsduSize =  pPort->CommonCfg.DesiredHtPhy.AmsduSize;
    pAd->MlmeAux.HtCapability.HtCapInfo.MimoPs =  pPort->CommonCfg.DesiredHtPhy.MimoPs;
    pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor20 =  (pPort->CommonCfg.DesiredHtPhy.ShortGIfor20) & (pHtCapability->HtCapInfo.ShortGIfor20);
    pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor40 =  (pPort->CommonCfg.DesiredHtPhy.ShortGIfor40) & (pHtCapability->HtCapInfo.ShortGIfor40);

    if (pPort->CommonCfg.IOTestParm.AtherosDIR855)
        pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor40 = 0;   
    
    pAd->MlmeAux.HtCapability.HtCapInfo.TxSTBC =  (pPort->CommonCfg.DesiredHtPhy.TxSTBC)&(pHtCapability->HtCapInfo.RxSTBC);
    pAd->MlmeAux.HtCapability.HtCapInfo.RxSTBC =  (pPort->CommonCfg.DesiredHtPhy.RxSTBC)&(pHtCapability->HtCapInfo.TxSTBC);
    pAd->MlmeAux.HtCapability.HtCapParm.MaxRAmpduFactor = pPort->CommonCfg.DesiredHtPhy.MaxRAmpduFactor;
    pAd->MlmeAux.HtCapability.HtCapParm.MpduDensity = pPort->CommonCfg.HtCapability.HtCapParm.MpduDensity;
    pAd->MlmeAux.HtCapability.ExtHtCapInfo.PlusHTC = pHtCapability->ExtHtCapInfo.PlusHTC;
    pWcidMacTabEntry->HTCapability.ExtHtCapInfo.PlusHTC = pHtCapability->ExtHtCapInfo.PlusHTC;
    pAd->MlmeAux.HtCapability.TxBFCap = pPort->CommonCfg.HtCapability.TxBFCap; // Update the transmit beamforming capabilities
    PlatformMoveMemory(&pWcidMacTabEntry->HTCapability.TxBFCap, &pHtCapability->TxBFCap, sizeof(HT_BF_CAP)); // Update the transmit beamforming capabilities

    if (pPort->CommonCfg.bRdg)
    {
        pAd->MlmeAux.HtCapability.ExtHtCapInfo.RDGSupport = pHtCapability->ExtHtCapInfo.RDGSupport;
        if (pAd->MlmeAux.HtCapability.ExtHtCapInfo.RDGSupport == 1)
            pWcidMacTabEntry->HTCapability.ExtHtCapInfo.PlusHTC = 1;
    }
    if (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == 0)
        pAd->MlmeAux.HtCapability.MCSSet[4] = 0;
    
    COPY_AP_HTSETTINGS_FROM_BEACON(pWcidMacTabEntry , pHtCapability);
    DBGPRINT(RT_DEBUG_TRACE, ("RTMPCheckHt  : pAd->MlmeAux.HtCapability.MCSSet[2] = 0x%02X, RxPath = %d\n",pAd->MlmeAux.HtCapability.MCSSet[2],pAd->HwCfg.Antenna.field.RxPath));     

    //
    // save the VHT information of the AP(or peer STA)
    //
    if((pPort->CommonCfg.PhyMode == PHY_11VHT) && (bVhtCapable == TRUE))
    {
        pWcidMacTabEntry->VhtPeerStaCtrl.bVhtCapable = TRUE;
        PlatformMoveMemory(&pWcidMacTabEntry->VhtPeerStaCtrl.VhtCapability, pVhtCapability, SIZE_OF_VHT_CAP_IE);
    }

#if 0 // need to set ??  Tx Maximum Number Spatial Streams Support 0 : 1 stream ; 1 : 2 Streams 
    pAd->MlmeAux.HtCapability.MCSSet[14] = 0x01;
#endif

    DBGPRINT(RT_DEBUG_TRACE, ("RTMPCheckHt  : pAd->MlmeAux.HtCapability.MCSSet[2] = 0x%02X, RxPath = %d\n",pAd->MlmeAux.HtCapability.MCSSet[2],pAd->HwCfg.Antenna.field.RxPath));     
    DBGPRINT(RT_DEBUG_TRACE, ("RTMPCheckHt  : pAd->MlmeAux.HtCapability.MCSSet[1] = 0x%02X, RxPath = %d\n",pAd->MlmeAux.HtCapability.MCSSet[1], pAd->HwCfg.Antenna.field.RxPath));     
    DBGPRINT(RT_DEBUG_TRACE, ("RTMPCheckHt  : pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor40 = %d, pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor20 = %d\n", pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor40, pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor20));     

    return TRUE;
}

BOOLEAN     RTMPCheckVht(
    IN      PMP_ADAPTER  pAd,
    IN      PMP_PORT     pPort,
    IN      UCHAR          Wcid,
    IN OUT  VHT_CAP_IE     *pVhtCapability,
    IN OUT  VHT_OP_IE      *pVhtOperation)
{
    DBGPRINT(RT_DEBUG_TRACE,("%s ==>\n",__FUNCTION__));
        
    if (Wcid >= MAX_LEN_OF_MAC_TABLE)
        return FALSE;
    
    if (pPort->CommonCfg.PhyMode!=PHY_11VHT)
        return FALSE;

    pAd->MlmeAux.bVhtCapable = TRUE;
    if(pVhtCapability!=NULL)
    {       
        // collect the lower capabilities between STA and AP, need more revised
        // 
        PlatformMoveMemory(&pAd->MlmeAux.VhtCapability, &pPort->CommonCfg.DesiredVhtPhy.VhtCapability, SIZE_OF_VHT_CAP_IE);   

        if ((pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor1ss==NOT_SUPPORTED)
            || (pVhtCapability->VhtSupportMcsSet.TxMcsMap.MaxMcsFor1ss==NOT_SUPPORTED))
            pAd->MlmeAux.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor1ss = NOT_SUPPORTED;

        if ((pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor2ss==NOT_SUPPORTED)
            || (pVhtCapability->VhtSupportMcsSet.TxMcsMap.MaxMcsFor2ss==NOT_SUPPORTED))
            pAd->MlmeAux.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor2ss = NOT_SUPPORTED;

        if ((pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor3ss==NOT_SUPPORTED)
            || (pVhtCapability->VhtSupportMcsSet.TxMcsMap.MaxMcsFor3ss==NOT_SUPPORTED))
            pAd->MlmeAux.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor3ss = NOT_SUPPORTED;

        if ((pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor4ss==NOT_SUPPORTED)
            || (pVhtCapability->VhtSupportMcsSet.TxMcsMap.MaxMcsFor4ss==NOT_SUPPORTED))
            pAd->MlmeAux.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor4ss = NOT_SUPPORTED;

        if ((pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor5ss==NOT_SUPPORTED)
            || (pVhtCapability->VhtSupportMcsSet.TxMcsMap.MaxMcsFor5ss==NOT_SUPPORTED))
            pAd->MlmeAux.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor5ss = NOT_SUPPORTED;

        if ((pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor6ss==NOT_SUPPORTED)
            || (pVhtCapability->VhtSupportMcsSet.TxMcsMap.MaxMcsFor6ss==NOT_SUPPORTED))
            pAd->MlmeAux.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor6ss = NOT_SUPPORTED;

        if ((pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor7ss==NOT_SUPPORTED)
            || (pVhtCapability->VhtSupportMcsSet.TxMcsMap.MaxMcsFor7ss==NOT_SUPPORTED))
            pAd->MlmeAux.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor7ss = NOT_SUPPORTED;
        
        if ((pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor8ss==NOT_SUPPORTED)
            || (pVhtCapability->VhtSupportMcsSet.TxMcsMap.MaxMcsFor8ss==NOT_SUPPORTED))
            pAd->MlmeAux.VhtCapability.VhtSupportMcsSet.TxMcsMap.MaxMcsFor8ss = NOT_SUPPORTED;
        // ========
        if ((pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor1ss==NOT_SUPPORTED)
            || (pVhtCapability->VhtSupportMcsSet.RxMcsMap.MaxMcsFor1ss==NOT_SUPPORTED))
            pAd->MlmeAux.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor1ss = NOT_SUPPORTED;

        if ((pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor2ss==NOT_SUPPORTED)
            || (pVhtCapability->VhtSupportMcsSet.RxMcsMap.MaxMcsFor2ss==NOT_SUPPORTED))
            pAd->MlmeAux.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor2ss = NOT_SUPPORTED;

        if ((pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor3ss==NOT_SUPPORTED)
            || (pVhtCapability->VhtSupportMcsSet.RxMcsMap.MaxMcsFor3ss==NOT_SUPPORTED))
            pAd->MlmeAux.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor3ss = NOT_SUPPORTED;

        if ((pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor4ss==NOT_SUPPORTED)
            || (pVhtCapability->VhtSupportMcsSet.RxMcsMap.MaxMcsFor4ss==NOT_SUPPORTED))
            pAd->MlmeAux.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor4ss = NOT_SUPPORTED;

        if ((pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor5ss==NOT_SUPPORTED)
            || (pVhtCapability->VhtSupportMcsSet.RxMcsMap.MaxMcsFor5ss==NOT_SUPPORTED))
            pAd->MlmeAux.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor5ss = NOT_SUPPORTED;

        if ((pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor6ss==NOT_SUPPORTED)
            || (pVhtCapability->VhtSupportMcsSet.RxMcsMap.MaxMcsFor6ss==NOT_SUPPORTED))
            pAd->MlmeAux.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor6ss = NOT_SUPPORTED;

        if ((pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor7ss==NOT_SUPPORTED)
            || (pVhtCapability->VhtSupportMcsSet.RxMcsMap.MaxMcsFor7ss==NOT_SUPPORTED))
            pAd->MlmeAux.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor7ss = NOT_SUPPORTED;

        if ((pPort->CommonCfg.DesiredVhtPhy.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor8ss==NOT_SUPPORTED)
            || (pVhtCapability->VhtSupportMcsSet.RxMcsMap.MaxMcsFor8ss==NOT_SUPPORTED))
            pAd->MlmeAux.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor8ss = NOT_SUPPORTED;
    }

    if(pVhtOperation!=NULL)
    {           
        PlatformMoveMemory(&pAd->MlmeAux.VhtOperation, pVhtOperation, SIZE_OF_VHT_OP_IE);           
        
        // decide bandwidth
        if (pVhtOperation->VhtOpInfo.ChannelWidth > pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtOpInfo.ChannelWidth)
            pAd->MlmeAux.VhtOperation.VhtOpInfo.ChannelWidth = pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtOpInfo.ChannelWidth;            
        else
            pAd->MlmeAux.VhtOperation.VhtOpInfo.ChannelWidth = pVhtOperation->VhtOpInfo.ChannelWidth;

        // 80+80Mhz is not supported yet
        pAd->MlmeAux.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg2 = 0;      
    }

    // debug message
    if(pAd->MlmeAux.VhtOperation.VhtOpInfo.ChannelWidth==VHT_BW_80)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s, LINE%d, VHT BW = BW80\n",__FUNCTION__,__LINE__));
    }
    else if(pAd->MlmeAux.VhtOperation.VhtOpInfo.ChannelWidth==VHT_BW_20_40)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s, LINE%d, VHT BW = VHT_BW_20_40\n",__FUNCTION__,__LINE__));
    }
    

    DBGPRINT(RT_DEBUG_TRACE,("<== %s\n",__FUNCTION__));
    
    return TRUE;
}

VOID AsicSetRxAnt(
    IN PMP_ADAPTER    pAd,
    IN UCHAR            Ant)
{
    ULONG   Value = 0;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    

    if (((pAd->HwCfg.NicConfig2.field.AntDiv != ANT_DIV_CONFIG_ENABLE) || 
            (pPort->CommonCfg.bPPAD == TRUE)) || 
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS))  ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))   ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))          ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
    {
        return;
    }

    // Not switch Antenna if radio is Off
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
        return;

    // Not switch Antenna if scanning, keep at MainAntenna for not Combo solution
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
        return;

    // the antenna selection is through firmware and MAC register(GPIO3)
    if (Ant == 0)
    {
        // Main antenna
        AsicSendCommanToMcu(pAd, MCU_SET_ANT_DIVERSITY, 0xff, 0, 0);

        RTUSBReadMACRegister(pAd, GPIO_CTRL_CFG, &Value);
        Value &= ~(0x0808);
        Value |= 0x08;
        RTUSBWriteMACRegister(pAd, GPIO_CTRL_CFG, Value);
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("AsicSetRxAnt, switch to main antenna\n"));
    }
    else
    {
        // Aux antenna
        AsicSendCommanToMcu(pAd, MCU_SET_ANT_DIVERSITY, 0xff, 1, 0);

        RTUSBReadMACRegister(pAd, GPIO_CTRL_CFG, &Value);
        Value &= ~(0x0808);
        RTUSBWriteMACRegister(pAd, GPIO_CTRL_CFG, Value);
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("AsicSetRxAnt, switch to aux antenna\n"));
    }
}

/*
    ========================================================================

    Routine Description:
        Periodic evaluate antenna link status

    Arguments:
        pAd         - Adapter pointer

    Return Value:
        None

    ========================================================================
*/
VOID AsicEvaluateRxAnt(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort)
{
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)    ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)     ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)            ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)       ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
    {
        return;
    }

    if ((pPort->PortSubtype == PORTSUBTYPE_SoftapAP) || 
        (pPort->PortSubtype == PORTSUBTYPE_VwifiAP) ||
        (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
    {
        ApMlmeAsicEvaluateRxAnt(pAd);
        return;
    }

    // Not switch Antenna if radio is Off
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
        return;

    // Not switch Antenna if scanning, keep at MainAntenna for not Combo solution
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
        return;


    // two antenna selection mechanism- one is antenna diversity, the other is failed antenna remove
    // one is antenna diversity:there is only one antenna can rx and tx
    // the other is failed antenna remove:two physical antenna can rx and tx
    if ((pAd->HwCfg.NicConfig2.field.AntDiv == ANT_DIV_CONFIG_ENABLE) && (pPort->CommonCfg.bPPAD == FALSE))
    {
        DBGPRINT(RT_DEBUG_TRACE,("AntDiv - before evaluate Pair1-Ant (%d,%d)\n",
            pAd->HwCfg.RxAnt.Pair1PrimaryRxAnt, pAd->HwCfg.RxAnt.Pair1SecondaryRxAnt));

        AsicSetRxAnt(pAd, pAd->HwCfg.RxAnt.Pair1SecondaryRxAnt);
            
        pAd->HwCfg.RxAnt.EvaluatePeriod = 1; //1:Means switch to SecondaryRxAnt, 0:Means switch to Pair1PrimaryRxAnt
        pAd->HwCfg.RxAnt.FirstPktArrivedWhenEvaluate = FALSE;
        pAd->HwCfg.RxAnt.RcvPktNumWhenEvaluate = 0;

        // a one-shot timer to end the evalution
        // dynamic adjust antenna evaluation period according to the traffic
        if (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED))
            PlatformSetTimer(pPort, &pPort->Mlme.RxAntEvalTimer, 100);
        else
            PlatformSetTimer(pPort, &pPort->Mlme.RxAntEvalTimer, 300);
    }
    else
    {
        DBGPRINT(RT_DEBUG_INFO, ("AsicEvaluateRxAnt : RealRxPath=%d \n", pAd->Mlme.RealRxPath));
    
        if (pAd->StaCfg.Psm == PWR_SAVE)
            return;
        
        BbSetRxAntByRxPath(pAd);



        if (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED))
        {
            ULONG   TxTotalCnt = pAd->Counter.MTKCounters.OneSecTxNoRetryOkCount + 
                                pAd->Counter.MTKCounters.OneSecTxRetryOkCount + 
                                pAd->Counter.MTKCounters.OneSecTxFailCount;

            // dynamic adjust antenna evaluation period according to the traffic
            if (TxTotalCnt > 50)
            {
                PlatformSetTimer(pPort,&pPort->Mlme.RxAntEvalTimer, 20);
                pAd->Mlme.bLowThroughput = FALSE;
            }
            else
            {
                PlatformSetTimer(pPort,&pPort->Mlme.RxAntEvalTimer, 300);
                pAd->Mlme.bLowThroughput = TRUE;
            }
        }
    }
}

// this timeout routine collect AvgRssi[SecondaryRxAnt] and decide the best Ant
VOID AsicRxAntEvalTimeoutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_PERFORM_SOFT_DIVERSITY, NULL, 0);
}

VOID AsicRxAntEvalAction(
    IN PMP_ADAPTER pAd)
{
    //UCHAR         BBPR3;
    CHAR            larger = -100, rssi0, rssi1, rssi2;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    

    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)    ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)     ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)            ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
        return;

    if ((pAd->HwCfg.NicConfig2.field.AntDiv == ANT_DIV_CONFIG_ENABLE) && (pPort->CommonCfg.bPPAD == FALSE))
    {
        if ((pAd->HwCfg.RxAnt.RcvPktNumWhenEvaluate != 0) && (pAd->HwCfg.RxAnt.Pair1AvgRssi[pAd->HwCfg.RxAnt.Pair1SecondaryRxAnt] >= pAd->HwCfg.RxAnt.Pair1AvgRssi[pAd->HwCfg.RxAnt.Pair1PrimaryRxAnt]))
        {
            UCHAR           temp;
            
            //
            // select PrimaryRxAntPair
            //    Role change, Used Pair1SecondaryRxAnt as PrimaryRxAntPair.
            //    Since Pair1SecondaryRxAnt Quality good than Pair1PrimaryRxAnt
            //
            temp = pAd->HwCfg.RxAnt.Pair1PrimaryRxAnt;
            pAd->HwCfg.RxAnt.Pair1PrimaryRxAnt = pAd->HwCfg.RxAnt.Pair1SecondaryRxAnt;
            pAd->HwCfg.RxAnt.Pair1SecondaryRxAnt = temp;

            pAd->HwCfg.RxAnt.Pair1LastAvgRssi = (pAd->HwCfg.RxAnt.Pair1AvgRssi[pAd->HwCfg.RxAnt.Pair1SecondaryRxAnt] >> 3);
        }
        else
        {
            // if the evaluated antenna is not better than original, switch back to original antenna
            AsicSetRxAnt(pAd, pAd->HwCfg.RxAnt.Pair1PrimaryRxAnt);
        }
        
        pAd->HwCfg.RxAnt.EvaluatePeriod = 0; //1:Means switch to SecondaryRxAnt, 0:Means switch to Pair1PrimaryRxAnt

        DBGPRINT(RT_DEBUG_TRACE,("AsicRxAntEvalAction::After Eval(fix in #%d), <%d, %d>, RcvPktNumWhenEvaluate=%d\n",
            pAd->HwCfg.RxAnt.Pair1PrimaryRxAnt, (pAd->HwCfg.RxAnt.Pair1AvgRssi[0] >> 3), (pAd->HwCfg.RxAnt.Pair1AvgRssi[1] >> 3), pAd->HwCfg.RxAnt.RcvPktNumWhenEvaluate));
    }
    else
    {
        if (pAd->StaCfg.Psm == PWR_SAVE)
            return; 

        // if the traffic is low, use average rssi as the criteria
        if (pAd->Mlme.bLowThroughput == TRUE)
        {
            rssi0 = pAd->StaCfg.RssiSample.LastRssi[0];
            rssi1 = pAd->StaCfg.RssiSample.LastRssi[1];
            rssi2 = pAd->StaCfg.RssiSample.LastRssi[2];
        }
        else
        {
            rssi0 = pAd->StaCfg.RssiSample.AvgRssi[0];
            rssi1 = pAd->StaCfg.RssiSample.AvgRssi[1];
            rssi2 = pAd->StaCfg.RssiSample.AvgRssi[2];
        }

        //
        // Rx antenna swtich is not for 3x3 NIC
        //
        if ((pAd->HwCfg.Antenna.field.TxPath == 2) && (pAd->HwCfg.Antenna.field.RxPath == 3))
        {
            larger = max(rssi0, rssi1);

            if (larger > (rssi2 + 20))
                pAd->Mlme.RealRxPath = 2;
            else
                pAd->Mlme.RealRxPath = 3;
        }

        //
        // To fix 1T/2R issue
        // This will not update pAd->Mlme.RealRxPath
        // But make sure pAd->Mlme.RealRxPath default value set to Antenna.field.RxPath on NICInitAsicFromEEPROM
        // Otherwise pAd->Mlme.RealRxPath will not be set and keep 0.
        //
#if 0
        else if (pAd->HwCfg.Antenna.field.RxPath == 2)
        {
            if (rssi0 > (rssi1 + 20))
                pAd->Mlme.RealRxPath = 1;
            else
                pAd->Mlme.RealRxPath = 2;
        }
#endif

        BbSetRxAntByRxPath(pAd);

        DBGPRINT(RT_DEBUG_TRACE, ("AsicRxAntEvalTimeoutTimerCallback : RealRxPath=%d, AvgRssi=%d, AvgRssi2=%d, AvgRssi3=%d \n",
            pAd->Mlme.RealRxPath, rssi0, rssi1, rssi2));
    }
}

/*
    ========================================================================

    Routine Description:
        Radar detection routine

    Arguments:
        pAd     Pointer to our adapter

    Return Value:

    ========================================================================
*/
VOID RadarDetectionStart(
    IN PMP_ADAPTER    pAd)
{
    DBGPRINT(RT_DEBUG_TRACE,("RadarDetectionStart--->\n"));
}

/*
    ========================================================================

    Routine Description:
        Radar detection routine

    Arguments:
        pAd     Pointer to our adapter

    Return Value:
        TRUE    Found radar signal
        FALSE   Not found radar signal

    ========================================================================
*/
BOOLEAN RadarDetectionStop(
    IN PMP_ADAPTER    pAd)
{
    return TRUE;
}

/*
    ========================================================================

    Routine Description:
        Radar channel check routine

    Arguments:
        pAd     Pointer to our adapter

    Return Value:
        TRUE    need to do radar detect
        FALSE   need not to do radar detect

    ========================================================================
*/
#define     Channel_Count  17

BOOLEAN RadarChannelCheck(
    IN PMP_ADAPTER    pAd,
    IN UCHAR            Ch)
{
    INT     i;
    UCHAR   Channel[Channel_Count]={52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140,169,173};

    for (i = 0; i < Channel_Count; i++)
    {
        if (Ch == Channel[i])
        {
            break;
        }
    }
    
    if (i != Channel_Count)
        return TRUE;
    else
        return FALSE;
}

/*
    ==========================================================================
    Description:
        A thread for calling MlmeHandler
    Parameters:
        Context - NIC Adapter pointer
    Note:

    IRQL = PASSIVE_LEVEL
    
    ==========================================================================
 */
VOID MlmeThread(
    IN PMP_ADAPTER pAd)
{
    DBGPRINT(RT_DEBUG_TRACE, ("Mlme Thread Start\n"));

    KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);        

    while (pAd->TerminateThreads != TRUE)
    {
        PlatformWaitEventTriggeredAndReset(&(GET_THREAD_EVENT(&pAd->pHifCfg->hMLMEThread)),500);   

        DBGPRINT(RT_DEBUG_INFO, ("Mlme Thread Triggered\n"));
        MlmeHandler(pAd);
    }   
    DBGPRINT(RT_DEBUG_TRACE, ("Mlme Thread Terminated\n"));
}

/*
    ========================================================================

    Routine Description:
        Mlme free the in-used nonpaged memory,
        move it to the unused memory link list

    Arguments:
        pAd                 Pointer to our adapter
        AllocVa             Pointer to the base virtual address for free

    Return Value:
        None

    IRQL = PASSIVE_LEVEL
    IRQL = DISPATCH_LEVEL
    
    Note:

    ========================================================================
*/
VOID    MlmeFreeMemory(
    IN PMP_ADAPTER pAd,
    IN PVOID         AllocVa)
{
    PMLME_MEMORY_ELEM               pMemoryElem = NULL;            
    PLIST_ENTRY                     thisEntry = NULL;
    ULONG                           Offset = 0;

    DBGPRINT(RT_DEBUG_INFO, ("==> MlmeFreeMemory\n"));

    if (AllocVa == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("MlmeFreeMemory with NULL AllocVa! [ERROR!!!]\n"));
        return;
    }

    if (pAd->Mlme.MemHandler.bInitalized == FALSE)
    {
        DBGPRINT(RT_DEBUG_WARN,("%s: Mlme.MemHandler was either not initalized or freed. \n", __FUNCTION__));
        return;
    }

    NdisAcquireSpinLock(&pAd->Mlme.MemHandler.LockMemoryHandler);

    pMemoryElem = MLME_MEMORY_2_PTR(AllocVa, FIELD_OFFSET(MLME_MEMORY_ELEM, AllocVa));
    if (pMemoryElem->Signature != MLME_MEMORY_TAG)
    {
        NdisReleaseSpinLock(&pAd->Mlme.MemHandler.LockMemoryHandler);
        DBGPRINT(RT_DEBUG_ERROR, ("MlmeFreeMemory Invalid AllocVa! [ERROR!!!]\n"));
        return;
    }
    thisEntry = &pMemoryElem->ListEntry;
    RemoveEntryList(thisEntry);

    InsertTailList(&pAd->Mlme.MemHandler.FreeQueue, thisEntry);
    pMemoryElem->Location = 0;
    pAd->Mlme.MemHandler.nFreeCount++;
    pAd->Mlme.MemHandler.nBusyCount--;
    NdisReleaseSpinLock(&pAd->Mlme.MemHandler.LockMemoryHandler);

    DBGPRINT(RT_DEBUG_INFO, ("<== MlmeFreeMemory [IN:%d][UN:%d]\n", 
                pAd->Mlme.MemHandler.nBusyCount, pAd->Mlme.MemHandler.nFreeCount));
}

/*
    ========================================================================

    Routine Description:
        Get an unused nonpaged system-space memory for use

    Arguments:
        pAd                 Pointer to our adapter
        AllocVa             Pointer to the base virtual address for later use

    Return Value:
        NDIS_STATUS_SUCCESS
        NDIS_STATUS_FAILURE
        NDIS_STATUS_RESOURCES

    IRQL = PASSIVE_LEVEL
    IRQL = DISPATCH_LEVEL
    
    Note:

    ========================================================================
*/
NDIS_STATUS MlmeAllocateMemoryEx(
    IN PMP_ADAPTER pAd,
    IN ULONG         Location,    
    OUT PVOID        *AllocVa)
{
    NDIS_STATUS                     NdisStatus = NDIS_STATUS_SUCCESS;
    PMLME_MEMORY_ELEM               pMemoryElem = NULL;
    PLIST_ENTRY                     thisEntry = NULL;
    PMLME_MEMORY_ELEM               Current = NULL;

    DBGPRINT(RT_DEBUG_INFO, ("==> MlmeAllocateMemory\n"));

    if (pAd->Mlme.MemHandler.bInitalized == FALSE)
    {
        DBGPRINT(RT_DEBUG_WARN,("%s: Mlme.MemHandler was either not initalized or freed. (Location=0x%08x)\n", __FUNCTION__, Location));
        return NDIS_STATUS_FAILURE;
    }
    
    NdisAcquireSpinLock(&pAd->Mlme.MemHandler.LockMemoryHandler);
    if (!IsListEmpty(&pAd->Mlme.MemHandler.FreeQueue))
    {
        thisEntry = RemoveHeadList(&pAd->Mlme.MemHandler.FreeQueue);
        pMemoryElem = CONTAINING_RECORD(thisEntry, MLME_MEMORY_ELEM, ListEntry);        
        InsertTailList(&pAd->Mlme.MemHandler.BusyQueue, thisEntry);
        pAd->Mlme.MemHandler.nFreeCount--;
        pAd->Mlme.MemHandler.nBusyCount++;
    }
    else
    {
        NdisStatus = PlatformAllocateMemory(pAd, (VOID *)&Current, sizeof(MLME_MEMORY_ELEM));
        if (NdisStatus == NDIS_STATUS_SUCCESS)
        {
            pAd->Mlme.MemHandler.TotalNumber++;
            thisEntry = &Current->ListEntry;
            Current->Signature = MLME_MEMORY_TAG;
            InsertTailList(&pAd->Mlme.MemHandler.BusyQueue, thisEntry);
            pAd->Mlme.MemHandler.nBusyCount++;    
            DBGPRINT(RT_DEBUG_TRACE,("%s:No available memory, Allocate another one, Location=%d\n", __FUNCTION__, Location)); 
        }
    }
    NdisReleaseSpinLock(&pAd->Mlme.MemHandler.LockMemoryHandler);

    if (NdisStatus == NDIS_STATUS_SUCCESS)
    {
        pMemoryElem = CONTAINING_RECORD(thisEntry, MLME_MEMORY_ELEM, ListEntry);
        pMemoryElem->Location = Location;
        *AllocVa = (PVOID)pMemoryElem->AllocVa;
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s:No available memory\n", __FUNCTION__));
    }

    DBGPRINT(RT_DEBUG_INFO, ("<== MlmeAllocateMemory[IN:%d][UN:%d]\n",
                pAd->Mlme.MemHandler.nBusyCount, pAd->Mlme.MemHandler.nFreeCount));

    return NdisStatus;
}

/*
    ========================================================================

    Routine Description:
        Allocates resident (nonpaged) system-space memory for MLME send frames

    Arguments:
        pAd                 Pointer to our adapter
        Number              Total nonpaged memory for use
        Size                Each nonpaged memory size

    Return Value:
        NDIS_STATUS_SUCCESS
        NDIS_STATUS_RESOURCES

    IRQL = PASSIVE_LEVEL

    Note:

    ========================================================================
*/
NDIS_STATUS MlmeInitMemoryHandler(
    IN PMP_ADAPTER pAd,
    IN UINT  Number,
    IN UINT  Size)
{
    PMLME_MEMORY_ELEM           Current = NULL;
    NDIS_STATUS                 NdisStatus = NDIS_STATUS_SUCCESS;
    UINT                        i;

    DBGPRINT(RT_DEBUG_INFO, ("==> MlmeInitMemory\n"));

    InitializeListHead(&pAd->Mlme.MemHandler.FreeQueue);
    InitializeListHead(&pAd->Mlme.MemHandler.BusyQueue);

    NdisAllocateSpinLock(&pAd->Mlme.MemHandler.LockMemoryHandler);

    pAd->Mlme.MemHandler.TotalNumber = 0;
    pAd->Mlme.MemHandler.nFreeCount = 0;
    pAd->Mlme.MemHandler.nBusyCount = 0;

    for (i = 0; i < Number; i++)
    {
        NdisStatus = PlatformAllocateMemory(pAd, (VOID *)&Current, sizeof(MLME_MEMORY_ELEM));
        if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
            break;
        }

        Current->Signature = MLME_MEMORY_TAG;
        Current->Location = 0;
        InsertTailList(&pAd->Mlme.MemHandler.FreeQueue, &Current->ListEntry);

        pAd->Mlme.MemHandler.TotalNumber++;
        pAd->Mlme.MemHandler.nFreeCount++;
    }

    pAd->Mlme.MemHandler.bInitalized = TRUE;
    DBGPRINT(RT_DEBUG_INFO, ("<== MlmeInitMemory\n"));

    return NdisStatus;
}

/*
    ========================================================================

    Routine Description:
        Free Mlme memory handler (link list, nonpaged memory, spin lock)

    Arguments:
        pAd                 Pointer to our adapter

    Return Value:
        None

    IRQL = PASSIVE_LEVEL

    ========================================================================
*/
VOID MlmeFreeMemoryHandler(
    IN PMP_ADAPTER pAd)
{
    PLIST_ENTRY                     thisEntry = NULL;
    PMLME_MEMORY_ELEM               pMemoryElem = NULL;
    ULONG                           Count = 0;

    pAd->Mlme.MemHandler.bInitalized = FALSE;

    NdisAcquireSpinLock(&pAd->Mlme.MemHandler.LockMemoryHandler);
    if (!IsListEmpty(&pAd->Mlme.MemHandler.BusyQueue))
    {
        DBGPRINT(RT_DEBUG_WARN, ("MlmeFreeMemoryHandler::BusyQueue should be empty (=%d)\n", pAd->Mlme.MemHandler.nBusyCount));
    }

    while(!IsListEmpty(&pAd->Mlme.MemHandler.BusyQueue))
    {
        thisEntry = RemoveHeadList(&pAd->Mlme.MemHandler.BusyQueue);
        InsertTailList(&pAd->Mlme.MemHandler.FreeQueue, thisEntry);
        pAd->Mlme.MemHandler.nFreeCount++;
        pAd->Mlme.MemHandler.nBusyCount--;
    }

    //
    // Free memory at FreeQueue
    //
    while(!IsListEmpty(&pAd->Mlme.MemHandler.FreeQueue))
    {
        thisEntry = RemoveHeadList(&pAd->Mlme.MemHandler.FreeQueue);
        pAd->Mlme.MemHandler.nFreeCount--;
        pMemoryElem = CONTAINING_RECORD(thisEntry, MLME_MEMORY_ELEM, ListEntry);

        if (pMemoryElem->Signature == MLME_MEMORY_TAG)
        {
            NdisFreeMemory(pMemoryElem, sizeof(MLME_MEMORY_ELEM), 0);
            Count++;
        }           
    }
    NdisReleaseSpinLock(&pAd->Mlme.MemHandler.LockMemoryHandler);

    NdisFreeSpinLock(&pAd->Mlme.MemHandler.LockMemoryHandler);

    if (Count != pAd->Mlme.MemHandler.TotalNumber)
    {
        DBGPRINT(RT_DEBUG_WARN, ("MlmeFreeMemoryHandler::Free memory mismatch (%d, %d)\n", Count, pAd->Mlme.MemHandler.TotalNumber));
    }
}
/*
    ========================================================================

    Routine Description:
        Verify the support rate for different PHY type

    Arguments:
        pAd                 Pointer to our adapter

    Return Value:
        None

    IRQL = PASSIVE_LEVEL

    ========================================================================
*/
VOID RTMPUpdateMlmeRate(
    IN PMP_ADAPTER    pAd,
    UCHAR   Channel)
{
    UCHAR   MinimumRate;
    UCHAR   ProperMlmeRate = RATE_54;
    UCHAR   i, j, RateIdx = 12; //1, 2, 5.5, 11, 6, 9, 12, 18, 24, 36, 48, 54
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    PMAC_TABLE_ENTRY pMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_MBCAST);

    if(pMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
        return;
    }

    switch (pPort->CommonCfg.PhyMode) 
    {
        case PHY_11BG_MIXED:
        case PHY_11B:
        case PHY_11BGN_MIXED:
            MinimumRate = RATE_1;
            break;
        case PHY_11A:
        case PHY_11N:   // rt2860 need to check mlmerate for 802.11n
        case PHY_11G:   // rt2860 need to check mlmerate for 802.11n
        case PHY_11GN_MIXED:
        case PHY_11AN_MIXED:
        case PHY_11AGN_MIXED:
        case PHY_11VHT:
            MinimumRate = RATE_6;
            break;
        case PHY_11ABGN_MIXED:
        case PHY_11ABG_MIXED:
            if (Channel <= 14)
            {
               MinimumRate = RATE_1;
            }
            else
            {
                MinimumRate = RATE_6;
            }
            break;
        default: // error
            MinimumRate = RATE_1;
            break;
    }

    for (i = 0; i < pAd->MlmeAux.SupRateLen; i++)
    {
        for (j = 0; j < RateIdx; j++)
        {
            if ((pAd->MlmeAux.SupRate[i] & 0x7f) == RateIdTo500Kbps[j])
            {
                if (j < ProperMlmeRate)
                    ProperMlmeRate = j;  //Zero base.
                else if (j == MinimumRate)
                    ProperMlmeRate = MinimumRate;
            }           
        }

        if (ProperMlmeRate == MinimumRate)
            break;
    }

    for (i = 0; i < pAd->MlmeAux.ExtRateLen; i++)
    {
        for (j = 0; j < RateIdx; j++)
        {
            if ((pAd->MlmeAux.ExtRate[i] & 0x7f) == RateIdTo500Kbps[j])
            {
                if (j < (ProperMlmeRate))
                    ProperMlmeRate = j;  //Zero base.
                else if (j == MinimumRate)
                    ProperMlmeRate = MinimumRate;
            }               
        }
        
        if (ProperMlmeRate == MinimumRate)
            break;      
    }

    pPort->CommonCfg.MlmeRate = ProperMlmeRate;
    //pPort->CommonCfg.RtsRate = ProperMlmeRate;
    if (pPort->CommonCfg.MlmeRate >= RATE_6)
    {
        WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.MgmtPhyCfg, MODE_OFDM);
        WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MgmtPhyCfg, OfdmRateToRxwiMCS[pPort->CommonCfg.MlmeRate]);
    }
    else
    {
        WRITE_PHY_CFG_MODE(pAd, &pPort->CommonCfg.MgmtPhyCfg, MODE_CCK);
        WRITE_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MgmtPhyCfg, pPort->CommonCfg.MlmeRate);
    }
    
    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pMacTabEntry->TxPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.MgmtPhyCfg));
    
    if (READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.MgmtPhyCfg) == MODE_OFDM)
    {
        WRITE_PHY_CFG_MCS(pAd, &pMacTabEntry->TxPhyCfg, OfdmRateToRxwiMCS[RATE_6]);
    }
    else
    {
        WRITE_PHY_CFG_MCS(pAd, &pMacTabEntry->TxPhyCfg, 0);
    }
}

CHAR RTMPMaxRssi(
    IN PMP_ADAPTER    pAd,
    IN CHAR             Rssi0,
    IN CHAR             Rssi1,
    IN CHAR             Rssi2)
{
    CHAR    larger = -100;
    
    if (pAd->HwCfg.Antenna.field.RxPath == 1)
    {
        larger = Rssi0;
    }

    if (pAd->HwCfg.Antenna.field.RxPath >= 2)
    {
        larger = max(Rssi0, Rssi1);
    }
    
    if (pAd->HwCfg.Antenna.field.RxPath == 3)
    {
        larger = max(larger, Rssi2);
    }
    return larger;
}

VOID RTMPSetAGCInitValue(
    IN PMP_ADAPTER    pAd,
    IN UCHAR            BandWidth)
{
    //TODO: Add
}

VOID AsicSendBA(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN UCHAR        Channel,
    IN ULONG        Wcid)
{
    FRAME_BA Frame;
    TXWI_STRUC      TxWI;
    PUCHAR          ptr;
    USHORT i;
    ULONG       longptr;
    ULONG           Value;
    //HTTRANSMIT_SETTING    HTTransmit;
    PHY_CFG BaPhyCfg = {0};
    //UCHAR         BBPValue;
    PMAC_TABLE_ENTRY pMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, (USHORT)Wcid);
    if(pMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
        return;
    }
    
    PlatformZeroMemory(&Frame, sizeof(FRAME_BA));
    Frame.FC.Type = BTYPE_CNTL;
    Frame.FC.SubType = SUBTYPE_BLOCK_ACK;
    COPY_MAC_ADDR(&Frame.Addr1, (ADHOC_ON(pPort) || ( INFRA_ON(pPort))) ? (pMacTabEntry->Addr) : (pPort->PortCfg.Bssid)); // Addr1 = DA
    COPY_MAC_ADDR(&Frame.Addr2, pPort->CurrentAddress);
    Frame.BarControl.ACKPolicy = 1;
    Frame.BarControl.Compressed = 1;
    Frame.StartingSeq.field.StartSeq = pMacTabEntry->BARStartSeq;

    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &BaPhyCfg, 0);
    if (Channel > 14)
    {
        WRITE_PHY_CFG_MODE(pAd, &BaPhyCfg, MODE_OFDM);
        WRITE_PHY_CFG_MCS(pAd, &BaPhyCfg, OfdmRateToRxwiMCS[RATE_6]);
    }
    else
    {
        WRITE_PHY_CFG_DOUBLE_WORD(pAd, &BaPhyCfg, 0);
    }

    XmitWriteTxWI(pAd, pPort, &TxWI,  FALSE, FALSE, FALSE, TRUE, FALSE, 0, (UCHAR)Wcid, sizeof(FRAME_BA),
        0, 0, IFS_HTTXOP, FALSE, BaPhyCfg, FALSE, TRUE, FALSE);

    // move BA Frame TXD and frame content to on-chip memory
    ptr = (PUCHAR)&TxWI;
    for (i = 0; i < pAd->HwCfg.TXWI_Length;)  
    {
        longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
        RTUSBWriteMACRegister(pAd, HW_NULL2_BASE + i, longptr);
        ptr += 4;
        i+=4;
    }

    // start right after the 24-byte TXINFO field. last 4 bytes should be zero. request by MAX.
    ptr = (PUCHAR)&Frame;
    for (i=0; i< READ_TXWI_MPDU_TOTAL_BYTE_COUNT(pAd, &TxWI);)
    {
        longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
        RTUSBWriteMACRegister(pAd, HW_NULL2_BASE + pAd->HwCfg.TXWI_Length + i, longptr);
        ptr += 4;
        i+=4;
    }

    if ((pPort->BBPCurrentBW >= BW_40) && (INFRA_ON(pPort) || ADHOC_ON(pPort)))
    {
        //AsicSwitchChannel(pAd, pPort->Channel, FALSE);
        AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
    }

    //
    // kick NULL frame #1
    //
    RTUSBWriteMACRegister(pAd, PBF_CTRL, KICK_NULL1_FRAME);

    i = 0;
    do
    {
        i++;
        RTUSBReadMACRegister(pAd, PBF_CTRL, &Value);
        DBGPRINT(RT_DEBUG_TRACE, ("Wait [%d] Kick NULL frame #1 to Send BA=%d\n", i,pMacTabEntry->BARStartSeq));

        if ((Value & 0x40) != 0)
            Delay_us(1000); //Delay while not success.
        else
            break;
    } while (i < 20);   

    if ((pPort->BBPCurrentBW >= BW_40) && (INFRA_ON(pPort) || ADHOC_ON(pPort))) 
    {
        //AsicSwitchChannel(pAd, pPort->CentralChannel, FALSE);
        AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
    }
}

VOID AsicSendNullFrame(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN UCHAR        Channel,
    IN UCHAR        PwrMgmt) 
{
    PUCHAR          ptr;
    USHORT          i;
    ULONG           longptr;
    ULONG           Value = 0;
    TXWI_STRUC      NullFrameTxWI;
    PTXWI_STRUC     pTxWI;
    HEADER_802_11   NullFrame;
    //HTTRANSMIT_SETTING    HTTransmit;
    PHY_CFG NullPhyCfg = {0};
    UCHAR           PortSecured = pAd->StaCfg.PortSecured;
    UCHAR           Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;

    // Use P2P's PortSecured in concurrent Client
    if (IS_P2P_CON_CLI(pAd, pPort) ||  IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))
    {
        Wcid = P2pGetClientWcid(pAd, pPort);
        pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid);  

        if(pWcidMacTabEntry == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
            return;
        }
        
        PortSecured = (UCHAR)pWcidMacTabEntry->PortSecured;
    }
    
    if ((MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
    {
        return;
    }
    
    // WPA 802.1x secured port control
    if (((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA) || 
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPAPSK) ||
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) ||
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK) ||
        (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)) &&
        (PortSecured == WPA_802_1X_PORT_NOT_SECURED))
    {
        return;
    }   
    


    i = 0;
    do
    {
        i++;
        RTUSBReadMACRegister(pAd, TXRXQ_PCNT, &Value);
        DBGPRINT(RT_DEBUG_TRACE, ("Wait [%d] Polling TXQ Empty\n", i));

        if ((Value & 0xff0000) != 0)
            Delay_us(5000);  //Delay while TXQ not empty.
        else
            break;

    } while (i < 20);

    pTxWI = &NullFrameTxWI;
    //PlatformZeroMemory(&NullFrameTxWI, sizeof(TXWI_STRUC));
    PlatformZeroMemory(&NullFrameTxWI, pAd->HwCfg.TXWI_Length);
    PlatformZeroMemory(&NullFrame, sizeof(HEADER_802_11));
    NullFrame.FC.Type = BTYPE_DATA;
    NullFrame.FC.SubType = SUBTYPE_NULL_FUNC;
    if (pPort->CommonCfg.bAPSDForcePowerSave)
        NullFrame.FC.PwrMgmt = PWR_SAVE;
    else
        NullFrame.FC.PwrMgmt = (PwrMgmt == PWR_SAVE) ? 1: 0;
    NullFrame.FC.ToDs = 1;
    COPY_MAC_ADDR(NullFrame.Addr1, pPort->PortCfg.Bssid);
    COPY_MAC_ADDR(NullFrame.Addr2, pPort->CurrentAddress);
    COPY_MAC_ADDR(NullFrame.Addr3, pPort->PortCfg.Bssid);

    pAd->pTxCfg->Sequence       = ((pAd->pTxCfg->Sequence) + 1) & (MAX_SEQ_NUMBER);
    NullFrame.Sequence = pAd->pTxCfg->Sequence;

    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &NullPhyCfg, 0);
    if (Channel > 14)
    {
        WRITE_PHY_CFG_MODE(pAd, &NullPhyCfg, MODE_OFDM);
        WRITE_PHY_CFG_MCS(pAd, &NullPhyCfg, OfdmRateToRxwiMCS[RATE_6]);
    
        NullFrame.Duration = XmitCalcUsbDuration(pAd, RATE_6, 14);
    }
    else
    {
        WRITE_PHY_CFG_MODE(pAd, &NullPhyCfg, MODE_CCK);
        WRITE_PHY_CFG_MCS(pAd, &NullPhyCfg, OfdmRateToRxwiMCS[RATE_1]);

        NullFrame.Duration = XmitCalcUsbDuration(pAd, RATE_1, 14);                    
    }

    XmitWriteTxWI(pAd, pPort, pTxWI, FALSE, FALSE, FALSE, FALSE, FALSE, 0, Wcid, sizeof(HEADER_802_11),
        0, 0, IFS_HTTXOP, FALSE, NullPhyCfg, FALSE, TRUE, FALSE);

    // move NULLFrame TXD and frame content to on-chip memory
    ptr = (PUCHAR)&NullFrameTxWI;
    for (i = 0; i < pAd->HwCfg.TXWI_Length; )  
    {
        longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
        RTUSBWriteMACRegister(pAd, HW_NULL2_BASE + i, longptr);
        ptr += 4;
        i+=4;
    }

    // start right after the 24-byte TXINFO field. last 4 bytes should be zero. request by MAX.
    ptr = (PUCHAR)&NullFrame;
    
    for (i=0; i< (READ_TXWI_MPDU_TOTAL_BYTE_COUNT(pAd, &NullFrameTxWI));)
    {
        longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
        RTUSBWriteMACRegister(pAd, HW_NULL2_BASE + pAd->HwCfg.TXWI_Length + i, longptr);
        ptr += 4;
        i+=4;
    }

    //
    // kick NULL frame #1
    //
    RTUSBWriteMACRegister(pAd, PBF_CTRL, KICK_NULL1_FRAME);

    i = 0;
    do
    {
        i++;
        RTUSBReadMACRegister(pAd, PBF_CTRL, &Value);
        DBGPRINT(RT_DEBUG_TRACE, ("Wait [%d] Kick NULL frame #1 PwrMgmt=%d\n", i, PwrMgmt));

        if ((Value & 0x40) != 0)
            Delay_us(1000); //Delay while not success.
        else
            break;
    } while (i < 20);
}

VOID CarrierDetectTimeoutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
}

VOID CarrierDetectTimeoutAction(
    IN PMP_ADAPTER pAd) 
{
    RX_STA_CNT1_STRUC   RxStaCnt1;
    ULONG               IntSrc;
    TX_STA_FIFO_STRUC   StaFifo;
    TX_STAT_FIFO_EXT_STRUC TxStatFifoExt = {0};
    UCHAR TxPktId = 0;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

    ULONG               i = 0;
    ULONG               data;
    UCHAR               pid = 0, wcid = 0;

    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS) ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
    {
        return;
    }

    if (!pPort->CommonCfg.bCarrierDetect)
        return;

    // monitor txdone status to start carrier detection
    do
    {
        RTUSBReadMACRegister(pAd, TX_STA_FIFO, &StaFifo.word);
        pid = (UCHAR)StaFifo.field.PidType;
        wcid = (UCHAR)StaFifo.field.wcid;

        TxPktId = (UCHAR)(StaFifo.field.PidType);

        if (StaFifo.field.bValid == 0)
        {
            break;
        }
        
        if ((TxPktId == PID_CTS_TO_SELF) && (StaFifo.field.TxSuccess))
        {
            DBGPRINT(RT_DEBUG_INFO, ("CarrierDetectTimeoutAction::MCU has sent CTS-To-Self, start detection \n"));
            RTUSBReadMACRegister(pAd, RX_STA_CNT1, &data);      // read and clear
            AsicSendCommanToMcu(pAd, 0x62, 0xff, 1, 0);
        }
        DBGPRINT(RT_DEBUG_INFO, (">>>CarrierDetectTimeoutAction<<< word = 0x%x \n", StaFifo.word));

        i++;
        // ASIC store 16 stack
    }while (i < 16);

    //Read MCU to Host Interrupt Source and clear it
    RTUSBReadMACRegister(pAd, H2M_INT_SRC, &IntSrc);
    RTUSBWriteMACRegister(pAd, H2M_INT_SRC, 0);

    if (IntSrc & 0x4)
    {
        RTUSBReadMACRegister(pAd, RX_STA_CNT1, &RxStaCnt1.word);
        pAd->Counter.MTKCounters.OneSecFalseCCACnt += RxStaCnt1.field.FalseCca;
        DBGPRINT(RT_DEBUG_TRACE, ("CarrierDetectTimeoutAction:: carrier detect (IntSrc=%x, FalseCCA=%d, HitCount=%d, bCarrierAssert=%d)\n",
            IntSrc, RxStaCnt1.field.FalseCca, pPort->CommonCfg.HitCount, pPort->CommonCfg.bCarrierAssert));

        // If FalseCca is larger or equal threshold, it means carrier assertion.
        // Otherwise, no carrier assertion
        if (RxStaCnt1.field.FalseCca >= CARRIER_FALSECCA_THRES)
        {
            if (pPort->CommonCfg.HitCount < CARRIER_HIT_THRES)
            {
                pPort->CommonCfg.HitCount ++;
            }

            if ((pPort->CommonCfg.bCarrierAssert == FALSE) && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) && (pPort->CommonCfg.HitCount >= CARRIER_HIT_THRES))
            {
                pPort->CommonCfg.bCarrierAssert = TRUE;

                // Carrier assertion, driver can not send anything, change detection with no cts-to-self
                AsicSetCarrierDetectAction(pAd, CARRIER_ACTION_NO_CTS, 1000);
                pPort->CommonCfg.bCarrierDetectPhase = TRUE;
                
                if (pAd->OpMode == OPMODE_STA)
                {
                    // MlmeCntLinkDown(pAd, FALSE);
                }
                else if (pAd->OpMode == OPMODE_AP)
                {
                    MtAsicDisableSync(pAd);
                }
            }
        }
        else
        {
            // No carrier assertion, change detection with one cts-to-self
            if (pPort->CommonCfg.HitCount == 1)
            {
                pPort->CommonCfg.bCarrierAssert = FALSE;

                if (pAd->OpMode == OPMODE_STA)
                {
                    // STA will reconnect (triggered by periodic timer)
                }
                else if (pAd->OpMode == OPMODE_AP)
                {
                    MtAsicEnableBssSync(pPort->pAd, pPort->CommonCfg.BeaconPeriod);
                    AsicSetCarrierDetectAction(pAd, CARRIER_ACTION_ONE_CTS, 1000);
                    pPort->CommonCfg.bCarrierDetectPhase = TRUE;
                }
            }

            if (pPort->CommonCfg.HitCount > 0)
            {
                pPort->CommonCfg.HitCount --;
            }
        }
    }
    
    if (IntSrc & 0x200)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("RTMPHandleMCUInterrupt-CTS can't be sent within 20ms (IntSrc=0x%x)\n", IntSrc));
    }
}


//
// Initiate the VCO calibration
//

VOID FullVCOCalibration(
    IN PMP_ADAPTER pAd)
{
    UCHAR RfValue = 0;
    DBGPRINT(RT_DEBUG_TRACE, ("%s-->\n",__FUNCTION__));
    RT30xxReadRFRegister(pAd, RF_R03, &RfValue);
    RfValue = ((RfValue & ~0x80) | 0x80);
    RT30xxWriteRFRegister(pAd, RF_R03, RfValue);
    DBGPRINT(RT_DEBUG_TRACE, ("<--%s\n",__FUNCTION__));
}

VOID IncrementalVCOCalibration(
    IN PMP_ADAPTER pAd)
{
    UCHAR RfValue = 0;
    DBGPRINT(RT_DEBUG_TRACE, ("%s-->\n",__FUNCTION__));
    RT30xxReadRFRegister(pAd, RF_R05, &RfValue);
    RfValue = ((RfValue & ~0x80) | 0x80);
    RT30xxWriteRFRegister(pAd, RF_R05, RfValue);
    DBGPRINT(RT_DEBUG_TRACE, ("<--%s\n",__FUNCTION__));
}

//
// Initiate the VCO calibration
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value:
//  None
//
VOID AsicVCOCalibration(
    IN PMP_ADAPTER pAd)
{
    UCHAR RfValue = 0;

    DBGPRINT(RT_DEBUG_INFO, ("---> %s\n", __FUNCTION__));
    
    DBGPRINT(RT_DEBUG_INFO, ("<--- %s\n", __FUNCTION__));
}

BOOLEAN AsicCheckCommandOk(
    IN PMP_ADAPTER pAd,
    IN UCHAR         Command)
{
    ULONG   CmdStatus, CID, i;
    ULONG   ThisCIDMask = 0;
    
    i = 0;
    do
    {
        RTUSBReadMACRegister(pAd, H2M_MAILBOX_CID, &CID);
        if ((CID & CID0MASK) == Command)
        {
            ThisCIDMask = CID0MASK;
            break;
        }
        else if ((((CID & CID1MASK)>>8) & 0xff) == Command)
        {
            ThisCIDMask = CID1MASK;
            break;
        }
        else if ((((CID & CID2MASK)>>16) & 0xff) == Command)
        {
            ThisCIDMask = CID2MASK;
            break;
        }
        else if ((((CID & CID3MASK)>>24) & 0xff) == Command)
        {
            ThisCIDMask = CID3MASK;
            break;
        }

        Delay_us(100);
        i++;
    }while (i < 200);


    RTUSBReadMACRegister(pAd, H2M_MAILBOX_STATUS, &CmdStatus);
    if (i < 200)
    {
        if (((CmdStatus & ThisCIDMask) == 0x1) || ((CmdStatus & ThisCIDMask) == 0x100) 
            || ((CmdStatus & ThisCIDMask) == 0x10000) || ((CmdStatus & ThisCIDMask) == 0x1000000))
        {
            DBGPRINT(RT_DEBUG_INFO, ("PSM : --> AsicCheckCommandOk CID = 0x%x, CmdStatus= 0x%x \n", CID, CmdStatus));
            RTUSBWriteMACRegister(pAd, H2M_MAILBOX_STATUS, 0xffffffff);
            RTUSBWriteMACRegister(pAd, H2M_MAILBOX_CID, 0xffffffff);
            return TRUE;
        }
        DBGPRINT(RT_DEBUG_TRACE, ("--> AsicCheckCommanFail1 CID = 0x%x, CmdStatus= 0x%x \n", CID, CmdStatus));
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("--> AsicCheckCommanFail2 Timeout Command = %d, CmdStatus= 0x%x \n", Command, CmdStatus));
    }
    RTUSBWriteMACRegister(pAd, H2M_MAILBOX_STATUS, 0xffffffff);
    RTUSBWriteMACRegister(pAd, H2M_MAILBOX_CID, 0xffffffff);

    return FALSE;

}

// Initialize the frequency calibration
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value:
//  None
//
VOID InitFrequencyCalibration(
    IN PMP_ADAPTER pAd)
{
    BBP_R179_STRUC BbpR179 = {0};
    BBP_R180_STRUC BbpR180 = {0};
    BBP_R182_STRUC BbpR182 = {0};
    UCHAR   RfValueTemp = 0;

    if (pAd->HwCfg.FreqCalibrationCtrl.bEnableFrequencyCalibration == TRUE)
    {   
        DBGPRINT(RT_DEBUG_TRACE, ("---> %s\n", __FUNCTION__));

        StopFrequencyCalibration(pAd);

        DBGPRINT(RT_DEBUG_TRACE, ("%s: frequency offset in the EEPROM = %d\n", 
            __FUNCTION__, 
            pAd->HwCfg.RfFreqOffset));

        DBGPRINT(RT_DEBUG_TRACE, ("<--- %s\n", __FUNCTION__));
    }
}

//
// To stop the frequency calibration algorithm
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value:
//  None
//
VOID StopFrequencyCalibration(
    IN PMP_ADAPTER pAd)
{
    if (pAd->HwCfg.FreqCalibrationCtrl.bEnableFrequencyCalibration == TRUE)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("---> %s\n", __FUNCTION__));

        // Base on the frequency offset of the EEPROM
        pAd->HwCfg.FreqCalibrationCtrl.AdaptiveFreqOffset = (CHAR)(pAd->HwCfg.RfFreqOffset); // C1 value control

        pAd->HwCfg.FreqCalibrationCtrl.LatestFreqOffsetOverBeacon = INVALID_FREQUENCY_OFFSET;

        pAd->HwCfg.FreqCalibrationCtrl.bSkipFirstFrequencyCalibration = TRUE;

        pAd->HwCfg.FreqCalibrationCtrl.bApproachFrequency = FALSE;

        DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->HwCfg.FreqCalibrationCtrl.AdaptiveFreqOffset = 0x%X\n", 
            __FUNCTION__, 
            pAd->HwCfg.FreqCalibrationCtrl.AdaptiveFreqOffset));
        
        DBGPRINT(RT_DEBUG_TRACE, ("<--- %s\n", __FUNCTION__));
    }
}

//
// The frequency calibration algorithm
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value:
//  None
//
VOID FrequencyCalibration(
    IN PMP_ADAPTER pAd)
{
    UCHAR RFValue = 0;
    CHAR HighFreqTriggerPoint = 0, LowFreqTriggerPoint = 0;
    CHAR DecreaseFreqOffset = 0, IncreaseFreqOffset = 0;
    UCHAR PreRFValue = 0;
    
    if (pAd->HwCfg.FreqCalibrationCtrl.bEnableFrequencyCalibration == FALSE)
        return;

    //
    // Frequency calibration period: 
    // a) 10 seconds: Check the reported frequency offset
    // b) 1 second: Update the RF frequency if possible
    //
    if ((((pAd->HwCfg.FreqCalibrationCtrl.bApproachFrequency == FALSE) && ((pAd->Mlme.PeriodicRound % FREQUENCY_CALIBRATION_PERIOD) == 0)) || 
           ((pAd->HwCfg.FreqCalibrationCtrl.bApproachFrequency == TRUE) && ((pAd->Mlme.PeriodicRound % (FREQUENCY_CALIBRATION_PERIOD / 10)) == 0))))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("---> %s\n", __FUNCTION__));

        if (pAd->HwCfg.FreqCalibrationCtrl.bSkipFirstFrequencyCalibration == TRUE)
        {
            pAd->HwCfg.FreqCalibrationCtrl.bSkipFirstFrequencyCalibration = FALSE;

            DBGPRINT(RT_DEBUG_TRACE, ("%s: Skip cuurent frequency calibration (avoid calibrating frequency at the time the STA is just link-up)\n", __FUNCTION__));
        }
        else
        {
            if (pAd->HwCfg.FreqCalibrationCtrl.LatestFreqOffsetOverBeacon != INVALID_FREQUENCY_OFFSET)
            {   
                //
                // Sync the thresholds
                //
                if (pAd->HwCfg.FreqCalibrationCtrl.BeaconPhyMode == MODE_CCK) // CCK
                {
                    HighFreqTriggerPoint = HIGH_FREQUENCY_TRIGGER_POINT_CCK;
                    LowFreqTriggerPoint = LOW_FREQUENCY_TRIGGER_POINT_CCK;

                    DecreaseFreqOffset = DECREASE_FREQUENCY_OFFSET_CCK;
                    IncreaseFreqOffset = INCREASE_FREQUENCY_OFFSET_CCK;
                }
                else // OFDM
                {
                    HighFreqTriggerPoint = HIGH_FREQUENCY_TRIGGER_POINT_OFDM;
                    LowFreqTriggerPoint = LOW_FREQUENCY_TRIGGER_POINT_OFDM;

                    DecreaseFreqOffset = DECREASE_FREQUENCY_OFFSET_OFDM;
                    IncreaseFreqOffset = INCREASE_FREQUENCY_OFFSET_OFDM;
                }

                if ((pAd->HwCfg.FreqCalibrationCtrl.LatestFreqOffsetOverBeacon >= HighFreqTriggerPoint) || 
                     (pAd->HwCfg.FreqCalibrationCtrl.LatestFreqOffsetOverBeacon <= LowFreqTriggerPoint))
                {
                    pAd->HwCfg.FreqCalibrationCtrl.bApproachFrequency = TRUE;
                }

                if (pAd->HwCfg.FreqCalibrationCtrl.bApproachFrequency == TRUE)
                {
                    if ((pAd->HwCfg.FreqCalibrationCtrl.LatestFreqOffsetOverBeacon <= DecreaseFreqOffset) && 
                          (pAd->HwCfg.FreqCalibrationCtrl.LatestFreqOffsetOverBeacon >= IncreaseFreqOffset))
                    {
                        pAd->HwCfg.FreqCalibrationCtrl.bApproachFrequency = FALSE; // Stop approaching frquency if -10 < reported frequency offset < 10
                    }
                    else if (pAd->HwCfg.FreqCalibrationCtrl.LatestFreqOffsetOverBeacon > DecreaseFreqOffset)
                {
                        if ( pAd->HwCfg.FreqCalibrationCtrl.AdaptiveFreqOffset > 0 )
                            pAd->HwCfg.FreqCalibrationCtrl.AdaptiveFreqOffset--;
                    
                        DBGPRINT(RT_DEBUG_TRACE, ("%s: -- frequency offset = 0x%X\n", __FUNCTION__, pAd->HwCfg.FreqCalibrationCtrl.AdaptiveFreqOffset));
                    }
                    else if (pAd->HwCfg.FreqCalibrationCtrl.LatestFreqOffsetOverBeacon < IncreaseFreqOffset)
                    {
                        if ( pAd->HwCfg.FreqCalibrationCtrl.AdaptiveFreqOffset < 0xFF )
                            pAd->HwCfg.FreqCalibrationCtrl.AdaptiveFreqOffset++;

                        DBGPRINT(RT_DEBUG_TRACE, ("%s: ++ frequency offset = 0x%X\n", __FUNCTION__, pAd->HwCfg.FreqCalibrationCtrl.AdaptiveFreqOffset));
                    }
                }

                DBGPRINT(RT_DEBUG_TRACE, ("%s: AdaptiveFreqOffset = %d, LatestFreqOffsetOverBeacon = %d, bApproachFrequency = %d\n", 
                    __FUNCTION__, 
                    pAd->HwCfg.FreqCalibrationCtrl.AdaptiveFreqOffset, 
                    pAd->HwCfg.FreqCalibrationCtrl.LatestFreqOffsetOverBeacon, 
                    pAd->HwCfg.FreqCalibrationCtrl.bApproachFrequency));
            }
        }
        
        DBGPRINT(RT_DEBUG_TRACE, ("<--- %s\n", __FUNCTION__));
    }
}

//
// Get the frequency offset
//
// Parameters
//  pAd: The adapter data structure
//  pRxWI: Point to the RxWI structure
//
// Return Value:
//  Frequency offset
//
UCHAR GetFrequencyOffset(
    IN PMP_ADAPTER pAd, 
    IN PRXWI_STRUC pRxWI)
{
    CHAR FreqOffset = 0;
    
    if (pAd->HwCfg.FreqCalibrationCtrl.bEnableFrequencyCalibration == TRUE)
    {   
        DBGPRINT(RT_DEBUG_INFO, ("---> %s\n", __FUNCTION__));

        FreqOffset = (CHAR)(READ_RXWI_FREQ_OFFSET(pAd, pRxWI));

        if ((FreqOffset < LOWERBOUND_OF_FREQUENCY_OFFSET) || 
             (FreqOffset > UPPERBOUND_OF_FREQUENCY_OFFSET))
        {
            FreqOffset = INVALID_FREQUENCY_OFFSET;

            DBGPRINT(RT_DEBUG_ERROR, ("%s: (out-of-range) FreqOffset = %d\n", 
                __FUNCTION__, 
                FreqOffset));
        }

        DBGPRINT(RT_DEBUG_INFO, ("%s: FreqOffset = %d\n", 
            __FUNCTION__, 
            FreqOffset));

        DBGPRINT(RT_DEBUG_INFO, ("<--- %s\n", __FUNCTION__));
    }

    return FreqOffset;
}

//
// Initialize PPAD (per-packet antenna diversity)
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value:
//  None
//
VOID InitPPAD(
    IN PMP_ADAPTER pAd)
{
    UCHAR BbpValue = 0, RfValue = 0;
    ULONG ulBbpValue = 0;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    DBGPRINT(RT_DEBUG_TRACE, ("---> %s\n", __FUNCTION__));

    DBGPRINT(RT_DEBUG_TRACE, ("%s[Before]: MAC 0x1000 = 0x%X, bPPAD = %d, AntDiv = %d\n", 
        __FUNCTION__, 
        pAd->HwCfg.MACVersion, 
        pPort->CommonCfg.bPPAD, 
        pAd->HwCfg.NicConfig2.field.AntDiv));
  
    {
        pPort->CommonCfg.bPPAD = FALSE; // Disable PPAD by default

        // Unify ToDo?
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s[After]: MAC 0x1000 = 0x%X, bPPAD = %d, AntDiv = %d\n", 
        __FUNCTION__, 
        pAd->HwCfg.MACVersion, 
        pPort->CommonCfg.bPPAD, 
        pAd->HwCfg.NicConfig2.field.AntDiv));

    DBGPRINT(RT_DEBUG_TRACE, ("<--- %s\n", __FUNCTION__));
}

//
// Rounding to integer
// e.g., +16.9 ~= 17 and -16.9 ~= -17
//
// Parameters
//  pAd: The adapter data structure
//  Integer: Integer part
//  Fraction: Fraction part
//  DenominatorOfTssiRatio: The denominator of the TSSI ratio
//
// Return Value:
//  Rounding result
//
LONG Rounding(
    IN PMP_ADAPTER pAd, 
    IN LONG Integer, 
    IN LONG Fraction, 
    IN LONG DenominatorOfTssiRatio)
{
    LONG temp = 0;

    DBGPRINT(RT_DEBUG_INFO, ("%s: Integer = %d, Fraction = %d, DenominatorOfTssiRatio = %d\n", 
        __FUNCTION__, 
        Integer, 
        Fraction, 
        DenominatorOfTssiRatio));

    if (Fraction >= 0)
    {
        if (Fraction < (DenominatorOfTssiRatio / 10))
        {
            return Integer; // e.g., 32.08059 ~= 32
        }
    }
    else
    {
        if (-Fraction < (DenominatorOfTssiRatio / 10))
        {
            return Integer; // e.g., -32.08059 ~= -32
        }
    }

    if (Integer >= 0)
    {
        if (Fraction == 0)
        {
            return Integer;
        }
        else
        {
            do {
                if (Fraction == 0)
                {
                    break;
                }
                else
                {
                    temp = Fraction / 10;
                    if (temp == 0)
                    {
                        break;
                    }
                    else
                    {
                        Fraction = temp;
                    }
                }
            } while (1);

            DBGPRINT(RT_DEBUG_INFO, ("%s: [+] temp = %d, Fraction = %d\n", __FUNCTION__, temp, Fraction));

            if (Fraction >= 5)
            {
                return (Integer + 1);
            }
            else
            {
                return Integer;
            }
        }
    }
    else
    {
        if (Fraction == 0)
        {
            return Integer;
        }
        else
        {
            do {
                if (Fraction == 0)
                {
                    break;
                }
                else
                {
                    temp = Fraction / 10;
                    if (temp == 0)
                    {
                        break;
                    }
                    else
                    {
                        Fraction = temp;
                    }
                }
            } while (1);

            DBGPRINT(RT_DEBUG_INFO, ("%s: [-] temp = %d, Fraction = %d\n", __FUNCTION__, temp, Fraction));

            if (Fraction <= -5)
            {
                return (Integer - 1);
            }
            else
            {
                return Integer;
            }
        }
    }
}


//
// Initialize the 802.11k noise histogram measurement (NHM) control
//
// Parameters
//  pAd: The adapter data structure
//  NHMExternGain: Extern LNA + channel filter loss + antenna switch loss, -31 dB ~ +31 dB
//  NHMPeriod: Noise histogram measurement period (0: 1us, 1: 2us, 2: 3us, 3: 4us)
//  IPIMsrnPeriod: Measurement period of idel power indicator (IPI), in unit of 1.024 ms
//
// Return Value:
//  None
//
VOID RRMInitNHMCtrl(
    IN PMP_ADAPTER pAd, 
    IN CHAR NHMExternGain, 
    IN UCHAR NHMPeriod, 
    IN ULONG IPIMsrnPeriod)
{
    BBP_R144_STRUC BbpR144 = {0};
    BBP_IPI_TIMER_STRUC BbpIPITimer = {0};
    
    DBGPRINT(RT_DEBUG_TRACE, ("dot11k: ---> %s\n", __FUNCTION__));

    // Initialize the NHM control
    BbpR144.field.NHMExternGain = NHMExternGain;
    BbpR144.field.NHMPeriod = NHMPeriod;
    RTUSBWriteBBPRegister(pAd, BBP_R144, BbpR144.byte);
    DBGPRINT(RT_DEBUG_TRACE, ("dot11k: %s: BbpR144.byte = 0x%X\n", __FUNCTION__, BbpR144.byte));

    // Initialize the BBP IPI timer
    BbpIPITimer.field.BBP_IPI_Period = IPIMsrnPeriod;
    BbpIPITimer.field.BBP_IPI_KICK = 1; // kick-off
    RTUSBWriteMACRegister(pAd, BBP_IPI_TIMER, BbpIPITimer.word);
    DBGPRINT(RT_DEBUG_TRACE, ("dot11k: %s: BbpIPITimer.word = 0x%X\n", __FUNCTION__, BbpIPITimer.word));    

    DBGPRINT(RT_DEBUG_TRACE, ("dot11k: <--- %s\n", __FUNCTION__));
}

//
// Get the IPI densities after performing the noise histogram measurement (NHM)
//
// Parameters
//  pAd: The adapter data structure
//  pIPPDensityArray: Point to a caller-supplied arrary in which stores the IPI densities (IPI0 density, IPI1 density, ..., IPI10 density)
//
// Return Value:
//  None
//
VOID RRMGetIPIDensities(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR pIPIDensityArray)
{
    UCHAR i = 0, retryCnt = 0;
    UCHAR BbpR145 = 0;
    BBP_IPI_TIMER_STRUC BbpIPITimer = {0};
    ULONG IPIValueArray[NUM_OF_IPI_VALUE] = {0}, idleTime = 0;
    UCHAR byte0 = 0, byte1 = 0, byte2 = 0, byte3 = 0;
    ULONG IPIDensity = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("dot11k: ---> %s\n", __FUNCTION__));

    do
    {
        ++retryCnt;
        
        BbpIPITimer.word = 0;
        RTUSBReadMACRegister(pAd, BBP_IPI_TIMER, &BbpIPITimer.word);

        if (BbpIPITimer.field.BBP_IPI_KICK == 0)
        {
            break;
        }
        else
        {
            Delay_us(10000); // 10 ms
        }

        DBGPRINT(RT_DEBUG_TRACE, ("dot11k: %s: BbpIPITimer.field.BBP_IPI_KICK = %d\n", __FUNCTION__, BbpIPITimer.field.BBP_IPI_KICK));
    } while ((BbpIPITimer.field.BBP_IPI_KICK == 1) && (retryCnt < 10));

    // Reset BBP R145
    BbpR145 = 0;
    RTUSBWriteBBPRegister(pAd, BBP_R145, BbpR145);

    // Get the IPI values
    for (i = 0; i < (NUM_OF_IPI_VALUE + NUM_OF_IDLE_TIME); i++)
    {
        RTUSBReadBBPRegister(pAd, BBP_R145, &BbpR145);
        byte0 = BbpR145; // byte0

        RTUSBReadBBPRegister(pAd, BBP_R145, &BbpR145);
        byte1 = BbpR145; // byte1

        RTUSBReadBBPRegister(pAd, BBP_R145, &BbpR145);
        byte2 = BbpR145; // byte2

        RTUSBReadBBPRegister(pAd, BBP_R145, &BbpR145);
        byte3 = BbpR145; // byte3

        if ((i >= 0) && (i < NUM_OF_IPI_VALUE)) // IPI
        {
#pragma prefast(suppress: 6201, "IPIValueArray will not over range")
            IPIValueArray[i] = (byte3 << 24) | (byte2 << 16) | (byte1 << 8) | (byte0);
#pragma prefast(suppress: 6201, "IPIValueArray will not over range")            
            DBGPRINT(RT_DEBUG_TRACE, ("dot11k: %s: IPIValueArray[%d] = (0x%X << 24) | (0x%X << 16) | (0x%X << 8) | (0x%X); = 0x%X\n", 
                __FUNCTION__, 
                i, 
                byte3, 
                byte2, 
                byte1, 
                byte0, 
                IPIValueArray[i]));
        }
        else // Idle time
        {
            idleTime = (byte3 << 24) | (byte2 << 16) | (byte1 << 8) | (byte0);

            DBGPRINT(RT_DEBUG_TRACE, ("dot11k: %s: idleTime = (0x%X << 24) | (0x%X << 16) | (0x%X << 8) | (0x%X) = 0x%X\n", 
                __FUNCTION__, 
                byte3, 
                byte2, 
                byte1, 
                byte0, 
                idleTime));
        }       
    }

    if ((retryCnt == 100) || (idleTime == 0)) // ASIC not ready or divid-by-zero
    {
        for (i = 0; i < NUM_OF_IPI_VALUE; i++)
        {
            pIPIDensityArray[i] = RandomByte(pAd); // Random IPI density

            DBGPRINT(RT_DEBUG_TRACE, ("dot11k: %s: (ASIC not ready or divid-by-zero): pIPIDensityArray[%d] = %d\n", __FUNCTION__, i, pIPIDensityArray[i]));
        }
    }
    else // Normal case
    {
        //
        // IPI density calculation
        //
        // IPI Density = Integer(255 * ([Duration receiving at IPI value (microseconds)] / ((1024 * [Measure-
        //  ment Duration (TU)]) - [NAVBUSY (microseconds)] - [Ttx (microseconds)] - [Trx (microsec-
        //  onds)]))).
        //
        // where
        //  Ttx is the frame transmission time during the Measurement Duration,
        //  Trx is the frame reception time during the Measurement Duration, and
        //  NAVBUSY is the total time in microseconds that NAV is non-zero during the Measurement Duration.
        //
        for (i = 0; i < NUM_OF_IPI_VALUE; i++)
        {
            IPIValueArray[i] *= 255;
            IPIDensity = IPIValueArray[i] / idleTime;

            if (IPIDensity > 255) // Boundary verification
            {
                pIPIDensityArray[i] = 255; // 255
            }
            else
            {
                pIPIDensityArray[i] = (UCHAR)(IPIDensity); // 0~254
            }
            
            DBGPRINT(RT_DEBUG_TRACE, ("dot11k: %s: pIPIDensityArray[%d] = IPIValueArray[%d] / idleTime = %d / %d = %d\n", 
                __FUNCTION__, i, i, IPIValueArray[i],  idleTime, pIPIDensityArray[i]));
        }
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("dot11k: <--- %s\n", __FUNCTION__));
}

#define LENGTH_OF_MPDU_WITH_NDP_ANNOUNCEMENT    (sizeof(HEADER_802_11) + sizeof(QOS_CONTROL) + sizeof(HT_CONTROL))

//
// Initialize the transmit beamforming control
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value
//  None
//
VOID InitTxBfCtrl(
    IN PMP_ADAPTER pAd)
{
    PTRANSMIT_BEAMFORMING_CONTROL pTxBfCtrl = &pAd->HwCfg.TxBfCtrl;

    if (!isEnableETxBf(pAd))
    {
        return;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("TxBf: ---> %s\n", __FUNCTION__));

    //
    // NIC control
    //
    RTUSBWriteMACRegister(pAd, TX_TXBF_CFG_0, 0x8000FC21);
    RTUSBWriteMACRegister(pAd, TX_TXBF_CFG_3, 0x00009C40);

    TxBfOidSetETxEn(pAd, &pTxBfCtrl->ETxBfEn, sizeof(pTxBfCtrl->ETxBfEn));

    TxBfOidSetETxBfTimeout(pAd, &pTxBfCtrl->ETxBfTimeout, sizeof(pTxBfCtrl->ETxBfTimeout));

    UpdateTxBfCapabilitiesField(pAd); // Update the TxBf capabilities field

    DBGPRINT(RT_DEBUG_TRACE, ("TxBf: <--- %s\n", __FUNCTION__));
}

//
// Update the TxBf capabilities field
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value
//  None
//
VOID UpdateTxBfCapabilitiesField(
    IN PMP_ADAPTER pAd)
{
}

//
// Enable/disable the explicit TxBf
//
// Parameters
//  pAd: The adapter data structure
//  pInfoBuf: Pointer to a caller-supplied variable in which points to the information buffer
//  InfoBufLength: The length of the information buffer
//
// Return Value
//  NDIS_STATUS_INVALID_LENGTH: Incorrect length of the information buffer
//  NDIS_STATUS_INVALID_DATA: Incorrect parameter
//
NDIS_STATUS TxBfOidSetETxEn(
    IN PMP_ADAPTER pAd, 
    IN PVOID pInfoBuf, 
    IN ULONG InfoBufLength)
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    ULONG Enable =*((ULONG*)pInfoBuf);
    ULONG value = 0;
    UCHAR RFValue = 0;
    
    if (InfoBufLength != sizeof(ULONG))
    {
        Status = NDIS_STATUS_INVALID_LENGTH;
        
        DBGPRINT(RT_DEBUG_ERROR, ("TxBf: %s: Incorrect InfoBufLength (= %d)\n", 
            __FUNCTION__, 
            InfoBufLength));

        return Status;
    }
    
    if ((Enable != 1) && (Enable != 0))
    {
        Status = NDIS_STATUS_INVALID_DATA;
        
        DBGPRINT(RT_DEBUG_ERROR, ("TxBf: %s: Incorrect parameter (= %d)\n", 
            __FUNCTION__, 
            Enable));

        return Status;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("TxBf: ---> %s\n", __FUNCTION__));

    DBGPRINT(RT_DEBUG_TRACE, ("TxBf: %s: Enable = %d\n", __FUNCTION__, Enable));

    RTUSBReadMACRegister(pAd, TX_TXBF_CFG_0, &value);

    if (Enable == 1)
    {
        value = (value & ~0x00000020) | 0x00000020; // Enable the explicit TxBf
    }
    else if (Enable == 0)
    {
        value = (value & ~0x00000020) | 0x00000000; // Disable the explicit TxBf
    }

    RTUSBWriteMACRegister(pAd, TX_TXBF_CFG_0, value);

    pAd->HwCfg.TxBfCtrl.ETxBfEn = Enable;

    if (pAd->HwCfg.TxBfCtrl.ETxBfEn == 1)
    {
        RT30xxReadRFRegister(pAd, RF_R39, &RFValue);
        RFValue = ((RFValue & ~0x40) | 0x40); // Rx divider
        RT30xxWriteRFRegister(pAd, RF_R39, RFValue);

        RT30xxReadRFRegister(pAd, RF_R49, &RFValue);
        RFValue = ((RFValue & ~0x20) | 0x20); // Tx divider
        RT30xxWriteRFRegister(pAd, RF_R49, RFValue);
    }
    else
    {
        RT30xxReadRFRegister(pAd, RF_R39, &RFValue);
        RFValue = ((RFValue & ~0x40) | 0x00); // Rx divider
        RT30xxWriteRFRegister(pAd, RF_R39, RFValue);

        RT30xxReadRFRegister(pAd, RF_R49, &RFValue);
        RFValue = ((RFValue & ~0x20) | 0x00); // Tx divider
        RT30xxWriteRFRegister(pAd, RF_R49, RFValue);
    }

    UpdateTxBfCapabilitiesField(pAd); // Update the TxBf capabilities field

    if (isEnableETxBf(pAd))
    {
        RTUSBWriteBBPRegister(pAd, BBP_REG_BF, 0xBD); // Enable the profile saving of the explicit TxBf and the implicit TxBf
    }
    else
    {
        RTUSBWriteBBPRegister(pAd, BBP_REG_BF, 0x9D); // Disable the profile saving of the explicit TxBf and the implicit TxBf
    }

    DBGPRINT(RT_DEBUG_TRACE, ("TxBf: <--- %s\n", __FUNCTION__));

    return Status;
}

//
// Set the explicit TxBf timeout value
//
// Parameters
//  pAd: The adapter data structure
//  pInfoBuf: Pointer to a caller-supplied variable in which points to the information buffer
//  InfoBufLength: The length of the information buffer
//
// Return Value
//  NDIS_STATUS_SUCCESS: Success
//  NDIS_STATUS_INVALID_LENGTH: Incorrect length of the information buffer
//  NDIS_STATUS_INVALID_DATA: The explicit TxBf timeout value is out of range.
//
NDIS_STATUS TxBfOidSetETxBfTimeout(
    IN PMP_ADAPTER pAd, 
    IN PVOID pInfoBuf, 
    IN ULONG InfoBufLength)
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    LONG ETxBfTimeout = (*(PLONG)(pInfoBuf));
    
    if (InfoBufLength != sizeof(LONG))
    {
        Status = NDIS_STATUS_INVALID_LENGTH;
        
        DBGPRINT(RT_DEBUG_ERROR, ("TxBf: %s: Incorrect InfoBufLength (= %d)\n", 
            __FUNCTION__, 
            InfoBufLength));

        return Status;
    }
    
    if ((ETxBfTimeout < ETX_BF_TIMEOUT_LOWERBOUND) || (ETxBfTimeout > ETX_BF_TIMEOUT_UPPERBOUND))
    {
        Status = NDIS_STATUS_INVALID_DATA;
        
        DBGPRINT(RT_DEBUG_ERROR, ("TxBf: %s: Incorrect ETxBfTimeout (= %d)\n", 
            __FUNCTION__, 
            ETxBfTimeout));

        return Status;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("TxBf: ---> %s\n", __FUNCTION__));

    DBGPRINT(RT_DEBUG_TRACE, ("TxBf: %s: ETxBfTimeout = %d\n", 
        __FUNCTION__, 
        ETxBfTimeout));

    pAd->HwCfg.TxBfCtrl.ETxBfTimeout = ETxBfTimeout;

    //
    // Set the timeout value of the explicit TxBf
    //
    RTUSBWriteMACRegister(pAd, TX_TXBF_CFG_3, pAd->HwCfg.TxBfCtrl.ETxBfTimeout);

    DBGPRINT(RT_DEBUG_TRACE, ("TxBf: <--- %s\n", __FUNCTION__));

    return Status;
}

//
// Select the non-compressed beamforming feedback matrix or the compressed beamforming feedback matrix
//
// Parameters
//  pAd: The adapter data structure
//  pInfoBuf: Pointer to a caller-supplied variable in which points to the information buffer
//  InfoBufLength: The length of the information buffer
//
// Return Value
//  NDIS_STATUS_SUCCESS: Success
//  NDIS_STATUS_INVALID_LENGTH: Incorrect length of the information buffer
//  NDIS_STATUS_INVALID_DATA: The explicit TxBf timeout value is out of range.
//
NDIS_STATUS TxBfOidSetETxBfNonCompressedFeedback(
    IN PMP_ADAPTER pAd, 
    IN PVOID pInfoBuf, 
    IN ULONG InfoBufLength)
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    LONG ETxBfNonCompressedFeedback = (*(PLONG)(pInfoBuf));
    
    if (InfoBufLength != sizeof(LONG))
    {
        Status = NDIS_STATUS_INVALID_LENGTH;
        
        DBGPRINT(RT_DEBUG_ERROR, ("TxBf: %s: Incorrect InfoBufLength (= %d)\n", 
            __FUNCTION__, 
            InfoBufLength));

        return Status;
    }
    
    if ((ETxBfNonCompressedFeedback != 1) && (ETxBfNonCompressedFeedback != 0))
    {
        Status = NDIS_STATUS_INVALID_DATA;
        
        DBGPRINT(RT_DEBUG_ERROR, ("TxBf: %s: Incorrect parameter (= %d)\n", 
            __FUNCTION__, 
            ETxBfNonCompressedFeedback));

        return Status;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("TxBf: ---> %s\n", __FUNCTION__));

    DBGPRINT(RT_DEBUG_TRACE, ("TxBf: %s: ETxBfNonCompressedFeedback = %d\n", 
        __FUNCTION__, 
        ETxBfNonCompressedFeedback));

    pAd->HwCfg.TxBfCtrl.ETxBfNonCompressedFeedback = ETxBfNonCompressedFeedback;

    UpdateTxBfCapabilitiesField(pAd); // Update the TxBf capabilities field

    DBGPRINT(RT_DEBUG_TRACE, ("TxBf: <--- %s\n", __FUNCTION__));

    return Status;
}

//
// Send a +HTC MPDU with the NDP Announcement set to 1
//
// Parameters
//  pAd: The adapter data structure
//  QueIdx: Queue index
//  pPacket: Pointer to a caller-supplied variable in which points to the packet
//  Length: Packet length
//  pHTTxRate: Pointer to a caller-supplied variable in which points to the HTTRANSMIT_SETTING
//
// Return Value
//  None
//
NDIS_STATUS TxBfNDPRequest(
    IN PMP_ADAPTER pAd,  
    IN  PVOID   pBuffer, 
    IN UINT Length, 
    //IN PHTTRANSMIT_SETTING pHTTxRate
    IN PPHY_CFG pNdpPhyCfg)
{
    ULONG           EncryptionOverhead = 0; 
    PHEADER_802_11  pHeader_802_11 = (PHEADER_802_11)pBuffer;
    BOOLEAN bNDPAnnouncement = FALSE;
    PHT_CONTROL pHTCtrl = NULL;
    PMGMT_STRUC pMgmt;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    DBGPRINT_RAW(RT_DEBUG_INFO, ("---> TxBfNDPRequest\n"));
    

    // Check management ring free avaliability
    NdisAcquireSpinLock(&pAd->pHifCfg->MLMEWaitQueueLock);
    pMgmt = (PMGMT_STRUC)&pAd->pHifCfg->MgmtRing[pAd->pHifCfg->PushMgmtIndex];
    // This management cell has been occupied
    if (pMgmt->Valid == TRUE)
    {
        NdisReleaseSpinLock(&pAd->pHifCfg->MLMEWaitQueueLock);
        MlmeFreeMemory(pAd, pBuffer);
        pAd->Counter.MTKCounters.MgmtRingFullCount++;
        DBGPRINT_RAW(RT_DEBUG_WARN, ("NdisCommonMiniportMMRequest (error:: MgmtRing full)\n"));
    }
    // Insert this request into software managemnet ring
    else
    {
        
        pMgmt->pBuffer = pBuffer;
        pMgmt->Length  = Length + EncryptionOverhead;
        if (EncryptionOverhead > 0)
            PlatformZeroMemory(pMgmt->pBuffer + Length, EncryptionOverhead);
        
        pMgmt->bNDPAnnouncement = FALSE;
        pMgmt->pMgmtPhyCfg = pNdpPhyCfg;

        pMgmt->Valid   = TRUE;
        if (isEnableETxBf(pAd))
        {
            if (Length == LENGTH_OF_MPDU_WITH_NDP_ANNOUNCEMENT)
            {
                pHTCtrl = ((PHT_CONTROL)((PUCHAR)pBuffer + sizeof(HEADER_802_11) + sizeof(QOS_CONTROL) ));
                if ((pHeader_802_11->FC.Type == BTYPE_DATA) && 
                     (pHeader_802_11->FC.SubType == SUBTYPE_QDATA) && 
                     (pHeader_802_11->FC.Order == 1) && 
                     (pHTCtrl->NDPAnnouncement == 1))
                {
                    pMgmt->bNDPAnnouncement = TRUE;

                    DBGPRINT(RT_DEBUG_TRACE, ("TxBf: %s: +HTC MPDU with NDP Announcement set to 1\n", __FUNCTION__));
                }
            }
        }

        
        pAd->pHifCfg->PushMgmtIndex++;
        DbgPrint("%s, pAd->pHifCfg->PushMgmtIndex++    %d\n",__FUNCTION__,pAd->pHifCfg->PushMgmtIndex);
        InterlockedIncrement(&pAd->pHifCfg->MgmtQueueSize);
        if (pAd->pHifCfg->PushMgmtIndex >= MGMT_RING_SIZE)
        {
            pAd->pHifCfg->PushMgmtIndex = 0;
        }
        NdisReleaseSpinLock(&pAd->pHifCfg->MLMEWaitQueueLock);
    }

    NdisCommonDequeueMLMEPacket(pAd);

    // If pAd->pHifCfg->PrioRingTxCnt is larger than 0, this means that prio_ring have something to transmit.
    // Then call KickBulkOut to transmit it
    if (pAd->pHifCfg->PrioRingTxCnt > 0)
    {
        if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
            AsicForceWakeup(pAd);
        if(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
        {
            AsicRadioOn(pAd);
        }
        N6USBKickBulkOut(pAd);
    }
    DBGPRINT(RT_DEBUG_TRACE, ("TxBf: <--- %s\n", __FUNCTION__));

    return Status;
}

//
// Send a +HTC MPDU with the NDP Announcement set to 1
//
// Parameters
//  pAd: The adapter data structure
//  pHTTxRate: Pointer to a caller-supplied variable in which points to the HTTRANSMIT_SETTING
//  bFromAP: Send from an AP
//  pAddress1: Pointer to a caller-supplied variable in which points to the Address 1
//  pAddress2: Pointer to a caller-supplied variable in which points to the Address 2
//  pAddress3: Pointer to a caller-supplied variable in which points to the Address 3
//
// Return Value
//  None
//
VOID    TxBfSendNDPAnnouncement(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT       pPort,
    //IN PHTTRANSMIT_SETTING pHTTxRate,
    IN PPHY_CFG pNdpPhyCfg, 
    IN BOOLEAN bFromAP, 
    IN PUCHAR pAddress1, 
    IN PUCHAR pAddress2, 
    IN PUCHAR pAddress3)
{
    UCHAR NullFrame[48] = {0};
    ULONG Length = 0;
    PHEADER_802_11 pHeader_802_11 = NULL;
    PHT_CONTROL pHTCtrl = NULL;
    UCHAR       PortSecured = pAd->StaCfg.PortSecured;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, P2pGetClientWcid(pAd, pPort));      
    if (pAd->StaCfg.bFastRoamingScan)
    {
        return;
    }

    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS) || 
         MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) || 
         MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
    {
        return;
    }

    // Use P2P's PortSecured in concurrent Client
    if (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))
    {
        if(pWcidMacTabEntry != NULL)
        {
            PortSecured = (UCHAR)pWcidMacTabEntry->PortSecured;
        }
    }


    // WPA 802.1x secured port control
    if (((pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA) || 
            (pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPAPSK) || 
            (pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2) || 
            (pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2PSK) || 
            (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)) && 
            (PortSecured == WPA_802_1X_PORT_NOT_SECURED))
    {
        return;
    }

    // Not send NULL data for CCX
    if ((pAd->StaCfg.CCXControl.field.VoiceRate) && 
         (pAd->Counter.MTKCounters.LastOneSecTotalTxCount > 3))
    {
        return;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("TxBf: ---> %s\n", __FUNCTION__));
    
    PlatformZeroMemory(NullFrame, 48);
    Length = sizeof(HEADER_802_11);

    pHeader_802_11 = ((PHEADER_802_11)(NullFrame));
    
    pHeader_802_11->FC.Type = BTYPE_DATA;
    pHeader_802_11->FC.SubType = SUBTYPE_QDATA;
    COPY_MAC_ADDR(pHeader_802_11->Addr1, pAddress1);
    COPY_MAC_ADDR(pHeader_802_11->Addr2, pAddress2);
    COPY_MAC_ADDR(pHeader_802_11->Addr3, pAddress3);

    if (bFromAP == TRUE) // AP ==> STA
    {
        pHeader_802_11->FC.ToDs = 0;
        pHeader_802_11->FC.FrDs = 1;
    }
    else // STA ==> AP
    {
        pHeader_802_11->FC.ToDs = 1;
        pHeader_802_11->FC.FrDs = 0;
    }

    if (pPort->CommonCfg.bAPSDForcePowerSave)
    {
        pHeader_802_11->FC.PwrMgmt = PWR_SAVE;
    }
    else
    {
        pHeader_802_11->FC.PwrMgmt = (pAd->StaCfg.Psm == PWR_SAVE) ? 1 : 0;
    }
    
    pHeader_802_11->Duration = pPort->CommonCfg.Dsifs + XmitCalcDuration(pAd, pPort->CommonCfg.TxRate, 14);

    // Sequence is increased in the MlmeHardTransmit
    pHeader_802_11->Sequence = pAd->pTxCfg->Sequence;

    // QoS Control field
    NullFrame[Length] =  0;
    NullFrame[Length + 1] =  0;
    Length += sizeof(QOS_CONTROL);

    // HT Control field
    pHeader_802_11->FC.Order = 1;
    pHTCtrl = ((PHT_CONTROL)(&NullFrame[Length]));
    pHTCtrl->NDPAnnouncement = 1;

    if (pPort->CommonCfg.TxBfCtrl.ETxBfNonCompressedFeedback == 1)
    {
        pHTCtrl->CsiSteering = 2; // Non-compressed beamforming

        DBGPRINT(RT_DEBUG_TRACE, ("TxBf: %s: Non-compressed beamforming\n", __FUNCTION__));
    }
    else
    {
        pHTCtrl->CsiSteering = 3; // Compressed beamforming

        DBGPRINT(RT_DEBUG_TRACE, ("TxBf: %s: Compressed beamforming\n", __FUNCTION__));
    }

    Length += sizeof(HT_CONTROL);

    TxBfNDPRequest(pAd, NullFrame, Length, pNdpPhyCfg);

    DBGPRINT(RT_DEBUG_TRACE, ("TxBf: <--- %s\n", __FUNCTION__));
}

//
// The generation of +HTC MPDU(s) with the NDP Announcement set to 1
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value
//  None
//
VOID    TxBfNDPAnnouncementGeneration(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT       pPort)
{
    ULONG i = 0;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;      
    PQUEUE_HEADER pHeader;    
    BOOLEAN bPeerSTATxBfCapable = FALSE;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, P2pGetClientWcid(pAd, pPort));      
    DBGPRINT(RT_DEBUG_TRACE, ("TxBf: ---> %s\n", __FUNCTION__));

    if (((pPort->PortSubtype == PORTSUBTYPE_SoftapAP) || (pPort->PortSubtype == PORTSUBTYPE_VwifiAP) && (pPort->SoftAP.bAPStart)) ||
        ((pPort->PortSubtype == PORTSUBTYPE_P2PGO) && (pPort->P2PCfg.bGOStart)))
    {
        //
        // Send a +HTC MPDU with the NDP Announcement set to 1 for all asssociated STAs.
        //
        pHeader = &pPort->MacTab.MacTabList;
        pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
        while (pNextMacEntry != NULL)
        {
            pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
            
            if(pMacEntry == NULL)
            {
                break; 
            }
            
            if(pMacEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST)
            {
                pNextMacEntry = pNextMacEntry->Next;   
                pMacEntry = NULL;
                continue; 
            }

           if ((pMacEntry->ValidAsCLI == FALSE) || (pMacEntry->Sst != SST_ASSOC))
            {
                pNextMacEntry = pNextMacEntry->Next;   
                pMacEntry = NULL;
                continue; 
            }
            
            if (pPort->PortSubtype == PORTSUBTYPE_P2PGO)
            {
                if ((pMacEntry->ValidAsP2P == TRUE)
                    && (IS_P2P_MS_CLI_WCID(pPort, i))
                    && ((pMacEntry->P2pInfo.P2pClientState == P2PSTATE_GO_ASSOC)
                    || (pMacEntry->P2pInfo.P2pClientState == P2PSTATE_GO_OPERATING)))
                pNextMacEntry = pNextMacEntry->Next;   
                pMacEntry = NULL;
                continue;    // Skip P2P Clients in wcid '22' and '23'
            }

            bPeerSTATxBfCapable = FALSE;
            if ((isEnableETxBf(pAd)) && (pMacEntry->HTCapability.TxBFCap.RxNDPCapable == 1))
            {
                if (pPort->CommonCfg.TxBfCtrl.ETxBfNonCompressedFeedback == 1) // Non-compressed beamforming feedback
                {
                    if ((pMacEntry->HTCapability.TxBFCap.ETxBfNonCompressedFeedbackCapable == ETX_BF_IMMEDIATE_FEEDBACK) || 
                        (pMacEntry->HTCapability.TxBFCap.ETxBfNonCompressedFeedbackCapable == ETX_BF_DELAYED_AND_IMMEDIATE_FEEDBACK))
                    {
                        bPeerSTATxBfCapable = TRUE;
                    }
                }
                else // Compressed beamforming feedback
                {
                    if ((pMacEntry->HTCapability.TxBFCap.ETxBfCompressedFeedbackCapable == ETX_BF_IMMEDIATE_FEEDBACK) || 
                        (pMacEntry->HTCapability.TxBFCap.ETxBfCompressedFeedbackCapable == ETX_BF_DELAYED_AND_IMMEDIATE_FEEDBACK))
                    {
                        bPeerSTATxBfCapable = TRUE;
                    }
                }

                if (bPeerSTATxBfCapable == TRUE)
                {
                    TxBfSendNDPAnnouncement(pAd, pPort, &pMacEntry->TxPhyCfg, TRUE, 
                        pMacEntry->Addr, pPort->PortCfg.Bssid, pPort->PortCfg.Bssid);
                }
            }
            
            pNextMacEntry = pNextMacEntry->Next;   
            pMacEntry = NULL;
        }
        
    }


    //
    // STA or P2PClient
    //
    // TODO: BT Entry??
    if ((pPort->PortSubtype == PORTSUBTYPE_STA) || (pPort->PortSubtype == PORTSUBTYPE_P2PClient))
    {
        //
        // Send a +HTC MPDU with the NDP Announcement set to 1 for the associated AP only.
        //
        if (INFRA_ON(pPort) && (!DLS_ON(pAd)) && 
            (!TDLS_ON(pAd)) && (!ADHOC_ON(pPort)) && 
            (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED)))
        {
            if (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))
            {
                pMacEntry = pWcidMacTabEntry;
            }
            else
            {
                pMacEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
            }

            if(pMacEntry == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
                return;
            }
        
            bPeerSTATxBfCapable = FALSE;
            
            if ((isEnableETxBf(pAd)) && (pMacEntry->HTCapability.TxBFCap.RxNDPCapable == 1))
            {
                if (pPort->CommonCfg.TxBfCtrl.ETxBfNonCompressedFeedback == 1) // Non-compressed beamforming feedback
                {
                    if ((pMacEntry->HTCapability.TxBFCap.ETxBfNonCompressedFeedbackCapable == ETX_BF_IMMEDIATE_FEEDBACK) || 
                        (pMacEntry->HTCapability.TxBFCap.ETxBfNonCompressedFeedbackCapable == ETX_BF_DELAYED_AND_IMMEDIATE_FEEDBACK))
                    {
                        bPeerSTATxBfCapable = TRUE;
                    }
                }
                else // Compressed beamforming feedback
                {
                    if ((pMacEntry->HTCapability.TxBFCap.ETxBfCompressedFeedbackCapable == ETX_BF_IMMEDIATE_FEEDBACK) || 
                        (pMacEntry->HTCapability.TxBFCap.ETxBfCompressedFeedbackCapable == ETX_BF_DELAYED_AND_IMMEDIATE_FEEDBACK))
                    {
                        bPeerSTATxBfCapable = TRUE;
                    }
                }
            }

            if (bPeerSTATxBfCapable == TRUE)
            {
                TxBfSendNDPAnnouncement(pAd, pPort, &pMacEntry->TxPhyCfg, FALSE, 
                                    pPort->PortCfg.Bssid, pPort->CurrentAddress, pPort->PortCfg.Bssid);
            }
        }
    }

    DBGPRINT(RT_DEBUG_TRACE, ("TxBf: <--- %s\n", __FUNCTION__));
}

//
// Determine that the packets should apply the beamforming matrix of the implicit/explicit TxBf, or not
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value
//  None
//
// Note:
//  a) The NIC preferes the explicit TxBf.
//
VOID    TxBfApplyMatrixToPacket(
    IN PMP_ADAPTER pAd)
{
    ULONG i = 0;
    PPHY_CFG pTxBfPhyCfg = NULL;
    CHAR RSSI = 0;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;  
    PQUEUE_HEADER pHeader;
    PMP_PORT  pPort = MlmeSyncGetActivePort(pAd);

    DBGPRINT(RT_DEBUG_INFO, ("TxBf: ---> %s\n", __FUNCTION__));

    // TODO: Microsoft VWiFi SoftAP or P2PGO ??
    // SoftAP
    if (pAd->OpMode == OPMODE_AP)
    {
        if (pAd->HwCfg.Antenna.field.RxPath == 3)
        {
            RSSI = ((pPort->SoftAP.ApCfg.RssiSample.AvgRssi[0] + pPort->SoftAP.ApCfg.RssiSample.AvgRssi[1] + pPort->SoftAP.ApCfg.RssiSample.AvgRssi[2]) / 3);
        }
        else if (pAd->HwCfg.Antenna.field.RxPath == 2)
        {
            RSSI = ((pPort->SoftAP.ApCfg.RssiSample.AvgRssi[0] + pPort->SoftAP.ApCfg.RssiSample.AvgRssi[1]) >> 1);
        }
        else
        {
            RSSI = pPort->SoftAP.ApCfg.RssiSample.AvgRssi[0];
        }
        pHeader = &pPort->MacTab.MacTabList;
        pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
        while (pNextMacEntry != NULL)
        {
            pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;

            if(pMacEntry == NULL)
            {
                break; 
            }
            
            if(pMacEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST)
            {
                pNextMacEntry = pNextMacEntry->Next;   
                pMacEntry = NULL;
                continue; 
            }

            if (pMacEntry->ValidAsCLI == FALSE)
            {
                pNextMacEntry = pNextMacEntry->Next;   
                pMacEntry = NULL;
                continue; 
            }

            if (RSSI > RSSI_OF_APPLYING_TXBF_MATRIX)
            {
                pNextMacEntry = pNextMacEntry->Next;   
                pMacEntry = NULL;
                continue; 
            }

            pTxBfPhyCfg = &pMacEntry->TxPhyCfg;
            WRITE_PHY_CFG_ITX_BF_PACKET(pAd, pTxBfPhyCfg, 0);
            WRITE_PHY_CFG_ETX_BF_PACKET(pAd, pTxBfPhyCfg, 0);

            if (isEnableETxBf(pAd))
            {
                if (pPort->CommonCfg.TxBfCtrl.ETxBfNonCompressedFeedback == 1) // Non-compressed beamforming feedback
                {
                    if ((pMacEntry->HTCapability.TxBFCap.ETxBfNonCompressedFeedbackCapable == ETX_BF_IMMEDIATE_FEEDBACK) || 
                        (pMacEntry->HTCapability.TxBFCap.ETxBfNonCompressedFeedbackCapable == ETX_BF_DELAYED_AND_IMMEDIATE_FEEDBACK))
                    {
                        WRITE_PHY_CFG_ETX_BF_PACKET(pAd, pTxBfPhyCfg, 1);
                    }
                }
                else // Compressed beamforming feedback
                {
                    if ((pMacEntry->HTCapability.TxBFCap.ETxBfCompressedFeedbackCapable == ETX_BF_IMMEDIATE_FEEDBACK) || 
                        (pMacEntry->HTCapability.TxBFCap.ETxBfCompressedFeedbackCapable == ETX_BF_DELAYED_AND_IMMEDIATE_FEEDBACK))
                    {
                        WRITE_PHY_CFG_ETX_BF_PACKET(pAd, pTxBfPhyCfg, 1);
                    }
                }
            }
            
            pNextMacEntry = pNextMacEntry->Next;   
            pMacEntry = NULL;
        }

    }

    // STA or P2P Client
    if (((pAd->OpMode == OPMODE_STA) || (pAd->OpMode == OPMODE_STAP2P)) && (pPort->SoftAP.bAPStart == FALSE) && (pPort->P2PCfg.bGOStart == FALSE))
    {
        if (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))
        {
            pMacEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, P2pGetClientWcid(pAd, pPort)); // For the associated GO
        }
        else
        {
            pMacEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID); // For the associated AP only.
        }

        if(pMacEntry == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
            return;
        }

        if ((INFRA_ON(pPort)) && (!DLS_ON(pAd)) && (!TDLS_ON(pAd)) && (!ADHOC_ON(pPort)))
        {
            pTxBfPhyCfg = &pMacEntry->TxPhyCfg;
            WRITE_PHY_CFG_ITX_BF_PACKET(pAd, pTxBfPhyCfg, 0);
            WRITE_PHY_CFG_ETX_BF_PACKET(pAd, pTxBfPhyCfg, 0);

            if (pAd->HwCfg.Antenna.field.RxPath == 3)
            {
                RSSI = ((pAd->StaCfg.RssiSample.AvgRssi[0] + pAd->StaCfg.RssiSample.AvgRssi[1] + pAd->StaCfg.RssiSample.AvgRssi[2]) / 3);
            }
            else if (pAd->HwCfg.Antenna.field.RxPath == 2)
            {
                RSSI = ((pAd->StaCfg.RssiSample.AvgRssi[0] + pAd->StaCfg.RssiSample.AvgRssi[1]) >> 1);
            }
            else
            {
                RSSI = pAd->StaCfg.RssiSample.AvgRssi[0];
            }

            DBGPRINT(RT_DEBUG_INFO, ("TxBf: %s: ETxBfNonCompressedFeedbackCapable = %d, ETxBfCompressedFeedbackCapable = %d\n", 
                __FUNCTION__, 
                pMacEntry->HTCapability.TxBFCap.ETxBfNonCompressedFeedbackCapable, 
                pMacEntry->HTCapability.TxBFCap.ETxBfCompressedFeedbackCapable));

            if (RSSI > RSSI_OF_APPLYING_TXBF_MATRIX)
            {
                // Skip TxBf for good signal
            }
            else
            {
                if (isEnableETxBf(pAd))
                {           
                    if (pPort->CommonCfg.TxBfCtrl.ETxBfNonCompressedFeedback == 1) // Non-compressed beamforming feedback
                    {
                        if ((pMacEntry->HTCapability.TxBFCap.ETxBfNonCompressedFeedbackCapable == ETX_BF_IMMEDIATE_FEEDBACK) || 
                            (pMacEntry->HTCapability.TxBFCap.ETxBfNonCompressedFeedbackCapable == ETX_BF_DELAYED_AND_IMMEDIATE_FEEDBACK))
                        {
                            WRITE_PHY_CFG_ETX_BF_PACKET(pAd, pTxBfPhyCfg, 1);
                        }
                    }
                    else // Compressed beamforming feedback
                    {
                        if ((pMacEntry->HTCapability.TxBFCap.ETxBfCompressedFeedbackCapable == ETX_BF_IMMEDIATE_FEEDBACK) || 
                            (pMacEntry->HTCapability.TxBFCap.ETxBfCompressedFeedbackCapable == ETX_BF_DELAYED_AND_IMMEDIATE_FEEDBACK))
                        {
                            WRITE_PHY_CFG_ETX_BF_PACKET(pAd, pTxBfPhyCfg, 1);
                        }
                    }
                }
            }
        }
    }

    DBGPRINT(RT_DEBUG_INFO, ("TxBf: <--- %s\n", __FUNCTION__));
}


#if 0
VOID MlmeUpdateTxStatistic(
    IN PMP_ADAPTER pAd,
    OUT TX_STA_CNT0_STRUC *pTxStaCnt0,
    OUT TX_STA_CNT1_STRUC *pStaTx1)
{
    pTxStaCnt0->word = 0;
    pStaTx1->word = 0;

    // Update statistic counter
    RTUSBReadMACRegister(pAd, TX_STA_CNT0, &pTxStaCnt0->word);
    RTUSBReadMACRegister(pAd, TX_STA_CNT1, &pStaTx1->word);
}

VOID MlmeUpdateDRSTimeAndStatistic(
    IN PMP_ADAPTER pAd,
    OUT TX_STA_CNT0_STRUC *pTxStaCnt0,
    OUT TX_STA_CNT1_STRUC *pStaTx1)
{
    
    ULONGLONG  CurrentTime = 0;

    //Get Current time to calculate ratio
    //NdisGetSystemUpTime(&CurrentTime);
    NdisGetCurrentSystemTime((PLARGE_INTEGER)&CurrentTime);
    
    if(CurrentTime == pAd->HwCfg.LastDRSTime)
    {
        DBGPRINT(RT_DEBUG_INFO ,(" Time Period too short skip this time\n"));
    }
    else
    {
        // Update Tx Statsistic Counter
        MlmeUpdateTxStatistic(pAd, pTxStaCnt0, pStaTx1);
        
        if (pAd->HwCfg.LastDRSTime == 0)
        {
            pAd->HwCfg.LastDRSTime = CurrentTime;
            pAd->HwCfg.CurrentDRSDeltaTime = 1;
            pAd->HwCfg.LastDRSDeltaTime = 1;
        }
        else
        {
            //Calculate Last DRStime and Current DRStime
            pAd->HwCfg.LastDRSDeltaTime = pAd->HwCfg.CurrentDRSDeltaTime;
            pAd->HwCfg.CurrentDRSDeltaTime = CurrentTime - pAd->HwCfg.LastDRSTime;
            DBGPRINT(RT_DEBUG_INFO,(" The Last Delta Time = %u \n", pAd->HwCfg.LastDRSDeltaTime));
            DBGPRINT(RT_DEBUG_INFO,(" Current Delta time = %u   \n", pAd->HwCfg.CurrentDRSDeltaTime));
            pAd->HwCfg.LastDRSTime = CurrentTime;
        }   
    }
}
#endif


VOID MlmeCancelAllTimers(
    IN PMP_ADAPTER pAd)
{
    PMP_PORT pPort = NULL;
    ULONG i;
    BOOLEAN Cancelled = FALSE;

    for(i = 0; i < RTMP_MAX_NUMBER_OF_PORT;i++)
    {
        pPort = pAd->PortList[i];
        if(pPort->bActive ==FALSE)
            continue;
        
        PlatformCancelTimer(&pPort->Mlme.AssocTimer,       &Cancelled);
        PlatformCancelTimer(&pPort->Mlme.ReassocTimer,     &Cancelled);
        PlatformCancelTimer(&pPort->Mlme.DisassocTimer,    &Cancelled);
        PlatformCancelTimer(&pPort->Mlme.AuthTimer,        &Cancelled);
        PlatformCancelTimer(&pPort->Mlme.ReSendBulkinIRPsTimer,   &Cancelled);
        PlatformCancelTimer(&pPort->Mlme.ForceSleepTimer,   &Cancelled);
        PlatformCancelTimer(&pPort->Mlme.BeaconTimer,      &Cancelled);
        PlatformCancelTimer(&pPort->Mlme.ScanTimer,        &Cancelled);
        PlatformCancelTimer(&pPort->Mlme.ChannelTimer,     &Cancelled);
        PlatformCancelTimer(&pPort->Mlme.MlmeCntLinkUpTimer,         &Cancelled);
        PlatformCancelTimer(&pPort->Mlme.MlmeCntLinkDownTimer,       &Cancelled);
        PlatformCancelTimer(&pPort->Mlme.RxAntEvalTimer,      &Cancelled);
        PlatformCancelTimer(&pPort->Mlme.CarrierDetectTimer,  &Cancelled);
        PlatformCancelTimer(&pPort->Mlme.WakeTimer,           &Cancelled);
        PlatformCancelTimer(&pPort->Mlme.TBTTTimer, &Cancelled);
        PlatformCancelTimer(&pPort->Mlme.SwTbttTimer, &Cancelled);    
        PlatformCancelTimer(&pPort->P2PCfg.P2pStopGoTimer, &Cancelled);
        PlatformCancelTimer(&pPort->P2PCfg.P2pCTWindowTimer, &Cancelled);
        PlatformCancelTimer(&pPort->P2PCfg.P2pStopClientTimer, &Cancelled);
        PlatformCancelTimer(&pPort->P2PCfg.P2pStopWpsTimer, &Cancelled);
        PlatformCancelTimer(&pPort->P2PCfg.P2pSwNoATimer, &Cancelled);
        PlatformCancelTimer(&pPort->P2PCfg.P2pPreAbsenTimer, &Cancelled);
        
    if (pAd->LogoTestCfg.OnTestingWHQL == TRUE)
    {
            PlatformCancelTimer(&pPort->Mlme.WHCKCPUUtilCheckTimer, &Cancelled);
    }

            PlatformCancelTimer(&pPort->Mlme.PeriodicTimer,       &Cancelled);

            PlatformCancelTimer(&pPort->Mlme.MlmeCntLinkUpTimer, &Cancelled);
            PlatformCancelTimer(&pPort->Mlme.MlmeCntLinkDownTimer, &Cancelled);
            PlatformCancelTimer(&pPort->Mlme.RxAntEvalTimer, &Cancelled);

    // Clear timer for Wsc
        WscStop(pAd, pPort);

    for (i = 0; i < MAX_NUM_OF_DLS_ENTRY; i++)
    {
            PlatformCancelTimer(&pPort->StaCfg.DLSEntry[i].Timer, &Cancelled);
    }   
    for (i = 0; i < MAX_NUM_OF_TDLS_ENTRY; i++)
    {
            PlatformCancelTimer(&pPort->StaCfg.TDLSEntry[i].Timer, &Cancelled);
        }  
    }   
}

VOID WhckCPUCheckTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    DBGPRINT(RT_DEBUG_TRACE, ("===> WhckCPUCheckTimerCallback\n"));

    DBGPRINT(RT_DEBUG_TRACE, ("WhckDataCntForPast40Sec = %d, WhckDataCntForPast60Sec = %d\n",pAd->LogoTestCfg.WhckDataCntForPast40Sec, pAd->LogoTestCfg.WhckDataCntForPast60Sec));
    if (pAd->LogoTestCfg.WhckFirst40Secs == TRUE)
    {
        pAd->LogoTestCfg.WhckDataCntForPast40Sec = pAd->LogoTestCfg.WhckDataCntForPast60Sec;
        pAd->LogoTestCfg.WhckFirst40Secs = FALSE;
        DBGPRINT(RT_DEBUG_TRACE, ("This is the first 40 seconds\n"));

        // measure for next 20 seconds for decide if current situation is under WHCK CPU Utilization
        PlatformSetTimer(pPort, &pPort->Mlme.WHCKCPUUtilCheckTimer, 20000); 
    }
    else if (pAd->LogoTestCfg.WhckCpuUtilTestRunning == TRUE)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("WHCK CPU Utilization is finished, set flag back to normal\n"));
        pAd->LogoTestCfg.WhckCpuUtilTestRunning = FALSE;
        
        RTUSBWriteMACRegister(pAd, RX_FILTR_CFG, STANORMAL);
        DBGPRINT(RT_DEBUG_TRACE, ("Set RxFilterCfg as 0x%x\n", STANORMAL));
        MtAsicEnableBssSync(pPort->pAd, pPort->CommonCfg.BeaconPeriod);
    }
    else
    {
        if (((((pAd->LogoTestCfg.WhckDataCntForPast60Sec - pAd->LogoTestCfg.WhckDataCntForPast40Sec) < 70 ) && (pAd->LogoTestCfg.WhckDataCntForPast40Sec > 150)) ||
            (pAd->LogoTestCfg.OnTestingCPUUtil == TRUE)) &&
            (!P2P_ON(pPort)))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Set pAd->LogoTestCfg.WhckCpuUtilTestRunning = TRUE\n"));

            pAd->LogoTestCfg.WhckCpuUtilTestRunning = TRUE;

            
            if ((pAd->LogoTestCfg.OnTestingWHQL == TRUE) && INFRA_ON(pPort) )
            {
                RTUSBWriteMACRegister(pAd, RX_FILTR_CFG, 0x1ffff);
                DBGPRINT(RT_DEBUG_TRACE, ("Set RxFilterCfg as 0x1ffff\n"));
            }
            MtAsicEnableBssSync(pPort->pAd, pPort->CommonCfg.BeaconPeriod);
            PlatformSetTimer(pPort, &pPort->Mlme.WHCKCPUUtilCheckTimer, 765000); // set timer for WHCK CPU Utilization test end

        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Set pAd->LogoTestCfg.WhckCpuUtilTestRunning = FALSE\n"));
            pAd->LogoTestCfg.WhckCpuUtilTestRunning = FALSE;
        }
    }
    DBGPRINT(RT_DEBUG_TRACE, ("<=== WhckCPUCheckTimerCallback\n"));

}

