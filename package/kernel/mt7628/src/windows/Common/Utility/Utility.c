#include    "MtConfig.h"


extern ULONG   RTDebugLevel ;
/*
    ========================================================================

    Routine Description:
        Implement this function to replace "DBGDumpHex" routine
        in Ndis driver

    Arguments:
        DebugLevel  - Debug level.
        length      - Length of dump buffer.
        buffer      - point to dump buffer
        AsciiDump   - True for Hex and Ascii dump, False for only Hex dump

    Return Value:

    Note:


    ========================================================================
*/
VOID
DBGDumpHex(
    ULONG DebugLevel,
    const char *title, 
    UINT length,
    PUCHAR buffer,
    BOOLEAN AsciiDump)
{
    int i,count,index;
    char rgbDigits[]="0123456789abcdef";
    char rgbLine[1024];
    char cbLine;

    if (DebugLevel > RTDebugLevel)
        return; // skip
    DBGPRINT(DebugLevel, ("%s - %s(len=%lu):\n", title, (AsciiDump ? "DBGDumpHex_Ascii" : "DBGDumpHex"), length));
    
    for (index = 0; length; length -= count, buffer += count, index += count)
    {
        count = (length > 16) ? 16:length;
        //RtlStringCbPrintfA(rgbLine, 4, "%4.4x  ", index);
        //sprintf(rgbLine, "%4.4x  ",index);
        //cbLine = 6;
        cbLine = 0;
        for (i=0;i<count;i++)
        {
            rgbLine[cbLine++] = rgbDigits[buffer[i] >> 4];
            rgbLine[cbLine++] = rgbDigits[buffer[i] & 0x0f];
            if (i == 7)
            {
                rgbLine[cbLine++] = ' ';
                rgbLine[cbLine++] = ':';
                rgbLine[cbLine++] = ' ';
            }
            else
            {
                rgbLine[cbLine++] = ' ';
            }
        }
        for (; i < 16; i++)
        {
            rgbLine[cbLine++] = ' ';
            rgbLine[cbLine++] = ' ';
            rgbLine[cbLine++] = ' ';
        }
        
        rgbLine[cbLine++] = ' ';

        if (AsciiDump)
        {
            for (i = 0; i < count; i++)
            {
                if (buffer[i] < 32 || buffer[i] > 126 || buffer[i] == '%')
                {
                    rgbLine[cbLine++] = '.';
                }
                else
                {
                    rgbLine[cbLine++] = buffer[i];
                }
            }
        }
        rgbLine[cbLine++] = 0;
        DBGPRINT(DebugLevel, ("%s\n",rgbLine)); 
    }
}// End of DBGDumpHex()

//===========================================================================
    LPSTR decodeCipherAlgorithm (ULONG algorithm)
//===========================================================================
// 
// Description: 
//    
//      Inputs: 
//    
//     Returns: 
//    
//---------------------------------------------------------------------------
{
    LPSTR retval = "";
    switch (algorithm) {
        case DOT11_CIPHER_ALGO_NONE:          retval = "DOT11_CIPHER_ALGO_NONE";          break;
        case DOT11_CIPHER_ALGO_WEP40:         retval = "DOT11_CIPHER_ALGO_WEP40";         break;
        case DOT11_CIPHER_ALGO_TKIP:          retval = "DOT11_CIPHER_ALGO_TKIP";          break;
        case DOT11_CIPHER_ALGO_CCMP:          retval = "DOT11_CIPHER_ALGO_CCMP";          break;
        case DOT11_CIPHER_ALGO_BIP:           retval = "DOT11_CIPHER_ALGO_BIP";          break;
        case DOT11_CIPHER_ALGO_WEP104:        retval = "DOT11_CIPHER_ALGO_WEP104";        break;
        case DOT11_CIPHER_ALGO_WEP:           retval = "DOT11_CIPHER_ALGO_WEP";           break;
        case DOT11_CIPHER_ALGO_IHV_START:     retval = "DOT11_CIPHER_ALGO_IHV_START";     break;
        case DOT11_CIPHER_ALGO_IHV_END:       retval = "DOT11_CIPHER_ALGO_IHV_END";       break;
//        case DOT11_CIPHER_ALGO_MFPTKIP:       retval = "DOT11_CIPHER_ALGO_MFPTKIP";       break;
//        case DOT11_CIPHER_ALGO_MFPCCMP:       retval = "DOT11_CIPHER_ALGO_MFPCCMP";       break;
//        case Ralink802_11EncryptionWPI_SMS4:  retval = "WPI_SMS4";                        break;        

        default:
            retval = "***UNKNOWN Cipher***";
    }
    return(retval);
}

LPSTR DecodePhyMode (ULONG phymode)
{
    LPSTR   retval = "";
    switch (phymode) 
    {
        case PHY_11BG_MIXED      : retval = "PHY_11BG_MIXED";          break;
        case PHY_11B             : retval = "PHY_11B";                 break;
        case PHY_11A             : retval = "PHY_11A";                 break;
        case PHY_11ABG_MIXED     : retval = "PHY_11ABG_MIXED";         break;
        case PHY_11G             : retval = "PHY_11G";                 break;
        case PHY_11ABGN_MIXED    : retval = "PHY_11ABGN_MIXED";        break;
        case PHY_11N             : retval = "PHY_11N";                 break;
        case PHY_11GN_MIXED      : retval = "PHY_11GN_MIXED";          break;
        case PHY_11AN_MIXED      : retval = "PHY_11AN_MIXED";          break;
        case PHY_11BGN_MIXED     : retval = "PHY_11BGN_MIXED";         break;
        case PHY_11AGN_MIXED     : retval = "PHY_11AGN_MIXED";         break;
        case PHY_11VHT           : retval = "PHY_11VHT";               break;
        
        default:
            DBGPRINT(RT_DEBUG_ERROR, ("**********UNKNOWN Phymode (=0x%x)************\n", phymode));
    }
    return(retval);
}

//===========================================================================
    LPSTR decodeAuthAlgorithm (ULONG algorithm)
//===========================================================================
// 
// Description: 
//    
//      Inputs: 
//    
//     Returns: 
//    
//---------------------------------------------------------------------------
{
    LPSTR retval = "";
    switch (algorithm) {
        case DOT11_AUTH_ALGO_80211_OPEN:            retval = "DOT11_AUTH_ALGO_80211_OPEN";              break;
        case DOT11_AUTH_ALGO_80211_SHARED_KEY:      retval = "DOT11_AUTH_ALGO_80211_SHARED_KEY";        break;
        case DOT11_AUTH_ALGO_WPA:                   retval = "DOT11_AUTH_ALGO_WPA";                     break;
        case DOT11_AUTH_ALGO_WPA_PSK:               retval = "DOT11_AUTH_ALGO_WPA_PSK";                 break;
        case DOT11_AUTH_ALGO_RSNA:                  retval = "DOT11_AUTH_ALGO_RSNA";                    break;
        case DOT11_AUTH_ALGO_RSNA_PSK:              retval = "DOT11_AUTH_ALGO_RSNA_PSK";                break;
        case DOT11_AUTH_ALGO_IHV_START:             retval = "DOT11_AUTH_ALGO_IHV_START";               break;
        case DOT11_AUTH_ALGO_IHV_END:               retval = "DOT11_AUTH_ALGO_IHV_END";                 break;      
//        case CCX_AUTH_ALGO_CCKM:                    retval = "CCX_AUTH_ALGO_CCKM";                      break;      
//        case Ralink802_11AuthModeWAI_PSK:           retval = "WAI_PSK";                                 break;
//        case Ralink802_11AuthModeWAI_CERT:          retval = "WAI_CERT";                                break;
//        case Ralink802_11AuthModeWAI_PSKCERT:       retval = "WAI_PSK_CERT";                            break;

        default:
            DBGPRINT(RT_DEBUG_ERROR, ("**********decodeAuthAlgorithm: UNKNOWN algorithm  (algorithm=0x%x)************\n", algorithm));
            retval = "***UNKNOWN Alogorithm***";
        }
    return(retval);
}

