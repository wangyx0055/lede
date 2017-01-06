/*
 ***************************************************************************
 * MediaTek Inc.
 * 5F, No.5, Tai-Yuan 1st St., 
 * Chupei City, Hsinchu County 30265, Taiwan, R.O.C.
 *
 * (c) Copyright 2014-2016, MediaTek, Inc.
 *
 * All rights reserved. MediaTek's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

    Module Name:
    Led.c

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */
    
#include    "MtConfig.h"
#include    "..\..\tables\LED.h"

/*
    ========================================================================
    
    Routine Description:
        Set LED Status

    Arguments:
        pAd                     Pointer to our adapter
        Status                  LED Status

    Return Value:
        None

    IRQL = PASSIVE_LEVEL
    IRQL = DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/
VOID LedCtrlSetLed(
    IN PMP_ADAPTER    pAd, 
    IN UCHAR            Status)
{
    UCHAR           HighByte = 0; 
    UCHAR           LowByte;


    //
    // Normal led mode
    //

    SetLEDByStatus(pAd, Status);

    //
    // Keep LED status for LED SiteSurvey mode.
    // After SiteSurvey, we will set the LED mode to previous status.
    //

    if ((Status != LED_ON_SITE_SURVEY) && (Status != LED_POWER_UP) && (Status != LED_WPS_TURN_LED_OFF))
        pAd->HwCfg.LedStatus = Status;

    
    LowByte = pAd->HwCfg.LedCntl.field.LedMode&0x7f; // LED mode.
    
    DBGPRINT(RT_DEBUG_INFO, ("LedCtrlSetLed::Mode=%d,HighByte=0x%02x,LowByte=0x%02x\n", pAd->HwCfg.LedCntl.field.LedMode, HighByte, LowByte));
}

VOID SetLEDByStatus(
    IN PMP_ADAPTER      pAd, 
    IN UCHAR                Status
    )
{
    ULONG           LEDModeFromEEPROM = 0; 
    ULONG           LEDBehavior = 0xFF;
    ULONG           LEDNumber = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("%s, Status %d\n", __FUNCTION__, Status));

    //Retrun all Led function, due to led function not ready.
#if 0


    switch (Status)
    {
        case LED_LINK_DOWN:
            DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_LINK_DOWN\n", __FUNCTION__));
            break;

        case LED_LINK_UP:
            if (pPort->Channel > 14)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_LINK_UP, CH > 14\n", __FUNCTION__));
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_LINK_UP, CH < 14\n", __FUNCTION__));
            }
            break;

        case LED_RADIO_OFF:
            DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_RADIO_OFF\n", __FUNCTION__));
            break;

        case LED_RADIO_ON:
            DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_RADIO_ON\n", __FUNCTION__));
            break;

        case LED_HALT: 
            DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_HALT\n", __FUNCTION__));
            break;

        case LED_WPS:
            if((pPort->P2PCfg.P2PConnectState > P2P_ANY_IN_FORMATION_AS_GO) &&
                (pPort->P2PCfg.P2PConnectState < P2P_I_AM_CLIENT_OP)) 
            {
                DBGPRINT(RT_DEBUG_TRACE, ("In p2p process LED turn off "));
                return;
            }
            else
                DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS\n", __FUNCTION__));
            break;

        case LED_ON_SITE_SURVEY:
            if(((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_6)
                &&(pPort->StaCfg.WscControl.WscState > WSC_STATE_START))
            {
                //special order???
                //LEDModeFromEEPROM = 0x1;
                //LEDBehavior = xxx;
                //return;

                //no need to change during site survey
                return;
            }
            else if ((((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_14))&&(pAd->HwCfg.FirmwareVersion == 222))
            {
                //special order???
                //LEDModeFromEEPROM = 0x1;
                //LEDBehavior = xxx;
                //return;

                //no need to change during site survey
                return;
            }
            else if ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_15)
            {
                //Alternating green and blue: USB Adapter is scanning for a network

                //no need to change during site survey
                return;
            }

            DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_ON_SITE_SURVEY\n", __FUNCTION__));
            break;

        case LED_POWER_UP:
            DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_POWER_UP\n", __FUNCTION__));
            break;

        case LED_WPS_IN_PROCESS:
            if((pPort->P2PCfg.P2PConnectState > P2P_ANY_IN_FORMATION_AS_GO) &&
                (pPort->P2PCfg.P2PConnectState < P2P_I_AM_CLIENT_OP)) 
            {
                DBGPRINT(RT_DEBUG_TRACE, ("In p2p process LED turn off "));
                return;
            }
            else //if (WscSupportWPSLEDMode(pAd))
            {
                pPort->StaCfg.WscControl.WscLEDMode = LED_WPS_IN_PROCESS;
                DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_IN_PROCESS\n", __FUNCTION__));
            }
            break;

        case LED_WPS_ERROR:
            if((pPort->P2PCfg.P2PConnectState > P2P_ANY_IN_FORMATION_AS_GO) &&
                (pPort->P2PCfg.P2PConnectState < P2P_I_AM_CLIENT_OP)) 
            {
                DBGPRINT(RT_DEBUG_TRACE, ("In p2p process LED turn off "));
                return;
            }
            else //if (WscSupportWPSLEDMode(pAd))
            {
                // In the case of LED mode 9, the error LED should be turned on only after WPS walk time expiration.
                if ((pPort->StaCfg.WscControl.bWPSWalkTimeExpiration == FALSE) && 
                    (pPort->StaCfg.WscControl.bErrDeteWPSTermination == FALSE) &&
                    ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_9))
                {
                    // do nothing.
                    return;
                }
                else
                {                   
                    //HighByte = LINK_STATUS_WPS_ERROR;
                    //AsicSendCommanToMcu(pAd, MCU_SET_WPS_LED_MODE, 0xff, LowByte, HighByte);
                }
                
                pPort->StaCfg.WscControl.WscLEDMode = LED_WPS_ERROR;
                pPort->StaCfg.WscControl.WscLastWarningLEDMode = LED_WPS_ERROR; // Update for walk time expiration.
                if ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_15)
                {
                    // Turn off the WPS successful LED pattern after 120 seconds.
                    PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscLEDTimer, WSC_REG_SESSION_TIMEOUT);
                }
                DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_ERROR\n", __FUNCTION__));
            }
            break;

        case LED_WPS_SESSION_OVERLAP_DETECTED:
            if((pPort->P2PCfg.P2PConnectState > P2P_ANY_IN_FORMATION_AS_GO) &&
                (pPort->P2PCfg.P2PConnectState < P2P_I_AM_CLIENT_OP)) 
            {
                DBGPRINT(RT_DEBUG_TRACE, ("In p2p process LED turn off "));
                return;
            }
            else //if (WscSupportWPSLEDMode(pAd))
            {
                //HighByte = LINK_STATUS_WPS_SESSION_OVERLAP_DETECTED;
                //AsicSendCommanToMcu(pAd, MCU_SET_WPS_LED_MODE, 0xff, LowByte, HighByte);
                pPort->StaCfg.WscControl.WscLEDMode = LED_WPS_SESSION_OVERLAP_DETECTED;
                pPort->StaCfg.WscControl.WscLastWarningLEDMode = LED_WPS_SESSION_OVERLAP_DETECTED;        
                if ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_15)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("LED4200 check: %s %d \n",__FUNCTION__,__LINE__));
                    // Turn off the WPS successful LED pattern after 10 seconds.
                    PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscLEDTimer, 10000);
                } 
            }
            DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_SESSION_OVERLAP_DETECTED\n", __FUNCTION__));
            break;

        case LED_WPS_SUCCESS:
            if((pPort->P2PCfg.P2PConnectState > P2P_ANY_IN_FORMATION_AS_GO) &&
                (pPort->P2PCfg.P2PConnectState < P2P_I_AM_CLIENT_OP)) 
            {
                DBGPRINT(RT_DEBUG_TRACE, ("In p2p process LED turn off "));
                return;
            }
            else //if (WscSupportWPSLEDMode(pAd))
            {
                if (((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_7) || // The WPS LED mode 7
                    ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_12))
                {
                    // In the WPS LED mode 7, the blue LED would last 300 seconds regardless of the AP's security settings.
                    pPort->StaCfg.WscControl.WscLEDMode = LED_WPS_SUCCESS;
                    // Turn off the WPS successful LED pattern after 300 seconds.
                    PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);
                    pPort->StaCfg.WscControl.bSkipWPSTurnOffLED = TRUE;
                    PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscSkipTurnOffLEDTimer, WSC_WPS_SKIP_TURN_OFF_LED_TIMEOUT);

                    DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_SUCCESS (LINK_STATUS_WPS_SUCCESS_WITH_SECURITY)\n", __FUNCTION__));
                }
                else if ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_8) // The WPS LED mode 8
                {
                    if (WscAPHasSecuritySetting(pAd)) // The WPS AP has the security setting.
                    {
                        pPort->StaCfg.WscControl.WscLEDMode = LED_WPS_SUCCESS;

                        // Turn off the WPS successful LED pattern after 300 seconds.
                        PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);

                        DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_SUCCESS (LINK_STATUS_WPS_SUCCESS_WITH_SECURITY)\n", __FUNCTION__));
                    }
                    else // The WPS AP does not have the secuirty setting.
                    {
                        pPort->StaCfg.WscControl.WscLEDMode = LED_WPS_SUCCESS;

                        // Turn off the WPS successful LED pattern after 300 seconds.
                        PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);

                        DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_SUCCESS (LINK_STATUS_WPS_SUCCESS_WITHOUT_SECURITY)\n", __FUNCTION__));
                    }
                    pPort->StaCfg.WscControl.bSkipWPSTurnOffLED = TRUE;
                    PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscSkipTurnOffLEDTimer, WSC_WPS_SKIP_TURN_OFF_LED_TIMEOUT);
                }
                else if ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_9) // The WPS LED mode 9.
                {
                    // Always turn on the WPS blue LED for 300 seconds.
                    //HighByte = LINK_STATUS_WPS_BLUE_LED;
                    //AsicSendCommanToMcu(pAd, MCU_SET_WPS_LED_MODE, 0xff, LowByte, HighByte);

                    pPort->StaCfg.WscControl.WscLEDMode = LED_WPS_SUCCESS;

                    if(pPort->StaCfg.WscControl.bWPSLEDSetTimer)
                    {
                        pPort->StaCfg.WscControl.bWPSLEDSetTimer = FALSE;

                        pPort->StaCfg.WscControl.bIndicateConnNotTurnOff = TRUE;

                        // Turn off the WPS successful LED pattern after 300 seconds.
                        PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);

                        pPort->StaCfg.WscControl.bSkipWPSTurnOffLEDAfterSuccess = TRUE;
                        PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscSkipTurnOffLEDTimer, WSC_WPS_SKIP_TURN_OFF_LED_TIMEOUT_AFTER_SUCCESS);
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_SUCCESS (LINK_STATUS_WPS_BLUE_LED)\n", __FUNCTION__));
                }
                else if ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_15) 
                {
                    pPort->StaCfg.WscControl.WscLEDMode = LED_WPS_SUCCESS;

                    // Turn off the WPS successful LED pattern after 300 seconds.
                    PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);

                    pPort->StaCfg.WscControl.bSkipWPSTurnOffLED = TRUE;
                    PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscSkipTurnOffLEDTimer, WSC_WPS_SKIP_TURN_OFF_LED_TIMEOUT);

                    DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_SUCCESS (LINK_STATUS_WPS_SUCCESS)\n", __FUNCTION__));
                }
                else
                {
                    //pPort->StaCfg.WscControl.WscLEDMode = LED_WPS_SUCCESS;

                    // Turn off the WPS successful LED pattern after 300 seconds.
                    //RTMPSetTimer(pAd, &pPort->StaCfg.WscControl.WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);

                    pPort->StaCfg.WscControl.bSkipWPSLEDCMD = TRUE;
                    PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscSkipTurnOffLEDTimer, 6000);
                        
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_SUCCESS (LINK_STATUS_WPS_SUCCESS)\n", __FUNCTION__));
                }
/*              
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_SUCCESS (Incorrect LED mode = %d)\n", 
                        __FUNCTION__, (pAd->HwCfg.LedCntl.field.LedMode & LED_MODE)));
                    ASSERT(FALSE);
                }
*/
            }
            pPort->StaCfg.WscControl.bWPSLEDSetTimer = FALSE;
            break;

        case LED_WPS_TURN_LED_OFF:
            if((pPort->P2PCfg.P2PConnectState > P2P_ANY_IN_FORMATION_AS_GO) &&
                (pPort->P2PCfg.P2PConnectState < P2P_I_AM_CLIENT_OP)) 
            {
                DBGPRINT(RT_DEBUG_TRACE, ("In p2p process LED turn off "));
                return;
            }
            else //if (WscSupportWPSLEDMode(pAd))
            {
                //HighByte = LINK_STATUS_WPS_TURN_LED_OFF;
                //AsicSendCommanToMcu(pAd, MCU_SET_WPS_LED_MODE, 0xff, LowByte, HighByte);
                pPort->StaCfg.WscControl.WscLEDMode = LED_WPS_TURN_LED_OFF;
                DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_TURN_LED_OFF\n", __FUNCTION__));
                return; //currently return since we don't have LED WPS LED
            }
            break;

        case LED_WPS_TURN_ON_BLUE_LED:
            if((pPort->P2PCfg.P2PConnectState > P2P_ANY_IN_FORMATION_AS_GO) &&
                (pPort->P2PCfg.P2PConnectState < P2P_I_AM_CLIENT_OP)) 
            {
                DBGPRINT(RT_DEBUG_TRACE, ("In p2p process LED turn off "));
                return;
            }
            else //if (WscSupportWPSLEDMode(pAd))
            {
                pPort->StaCfg.WscControl.WscLEDMode = LED_WPS_SUCCESS;
                DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_TURN_ON_BLUE_LED\n", __FUNCTION__));
            }
            break;

        case LED_NORMAL_CONNECTION_WITHOUT_SECURITY:
            if((pPort->P2PCfg.P2PConnectState > P2P_ANY_IN_FORMATION_AS_GO) &&
                (pPort->P2PCfg.P2PConnectState < P2P_I_AM_CLIENT_OP)) 
            {
                DBGPRINT(RT_DEBUG_TRACE, ("In p2p process LED turn off "));
                return;
            }
            else //if (WscSupportWPSLEDMode(pAd))
            {
                pPort->StaCfg.WscControl.WscLEDMode = LED_WPS_SUCCESS;
                DBGPRINT(RT_DEBUG_TRACE, ("%s: LINK_STATUS_NORMAL_CONNECTION_WITHOUT_SECURITY\n", __FUNCTION__));
            }
            break;

        case LED_NORMAL_CONNECTION_WITH_SECURITY:
            //if (WscSupportWPSLEDMode(pAd))
            {
                pPort->StaCfg.WscControl.WscLEDMode = LED_WPS_SUCCESS;
                DBGPRINT(RT_DEBUG_TRACE, ("%s: LINK_STATUS_NORMAL_CONNECTION_WITH_SECURITY\n", __FUNCTION__));
            }
            break;

        //WPS LED MODE 10
        case LED_WPS_MODE10_TURN_ON:
            if((pPort->P2PCfg.P2PConnectState > P2P_ANY_IN_FORMATION_AS_GO) &&
                (pPort->P2PCfg.P2PConnectState < P2P_I_AM_CLIENT_OP)) 
            {
                DBGPRINT(RT_DEBUG_TRACE, ("In p2p process LED turn off "));
                return;
            }
            else //if (WscSupportWPSLEDMode10(pAd)) //let UI set LED anyway to reduce the config
            {
                //make it same as Radio ON
                Status = LED_RADIO_ON;
                DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_MODE10_TURN_ON\n", __FUNCTION__));
            }
            break;

        case LED_WPS_MODE10_FLASH:
            if((pPort->P2PCfg.P2PConnectState > P2P_ANY_IN_FORMATION_AS_GO) &&
                (pPort->P2PCfg.P2PConnectState < P2P_I_AM_CLIENT_OP)) 
            {
                DBGPRINT(RT_DEBUG_TRACE, ("In p2p process LED turn off "));
                return;
            }
            else //if (WscSupportWPSLEDMode10(pAd)) //let UI set LED anyway to reduce the config
            {
                //make it same as WPS BLINK
                Status = LED_WPS;
                DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_MODE10_FLASH\n", __FUNCTION__));
            }
            break;

        case LED_WPS_MODE10_TURN_OFF:
            if((pPort->P2PCfg.P2PConnectState > P2P_ANY_IN_FORMATION_AS_GO) &&
                (pPort->P2PCfg.P2PConnectState < P2P_I_AM_CLIENT_OP)) 
            {
                DBGPRINT(RT_DEBUG_TRACE, ("In p2p process LED turn off "));
                return;
            }
            else //if (WscSupportWPSLEDMode10(pAd)) //let UI set LED anyway to reduce the config
            {
                //make it same as Radio OFF
                Status = LED_RADIO_OFF;
                DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_MODE10_TURN_OFF\n", __FUNCTION__));
            }
            break;

        default:
            DBGPRINT(RT_DEBUG_WARN, ("RTMPSetLED::Unknown Status %d\n", Status));
            break;
    }

    // need to wait FW ready, temprary hard code 0xA (D-link)
/*  
    if(Status <= LED_POWER_UP)
    {
        LEDModeFromEEPROM = pAd->HwCfg.LedCntl.field.LedMode&0x7;
        LEDBehavior = LED_MODE_STATUS_NUMBER_NORMAL[LEDModeFromEEPROM][Status];
    }
    else if (Status >= LED_WPS_MODE10_TURN_ON)
    {
        LEDModeFromEEPROM = pAd->HwCfg.LedCntl.field.LedMode&0x7;
        LEDBehavior = LED_MODE_STATUS_NUMBER_NORMAL[LEDModeFromEEPROM][Status];
    }
    else
    {
        LEDModeFromEEPROM = (pAd->HwCfg.LedCntl.field.LedMode&0x78 >> 3);
        LEDBehavior = LED_MODE_STATUS_NUMBER_WPS[LEDModeFromEEPROM][Status - 7]; //WPS Status starts from 8, -7 to make status meet array number.
    }
*/

    if(Status < LED_MAX_STATE)
    {
        LEDModeFromEEPROM = pAd->HwCfg.LedCntl.field.LedMode&0x7F;
        
        LEDBehavior = LED_MODE_STATUS_NUMBER[LEDModeFromEEPROM][Status];
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s::Mode:%d, Behavior:%d\n", __FUNCTION__, LEDModeFromEEPROM, LEDBehavior));

    if(LEDBehavior == 0xFF)
    {
        DBGPRINT(RT_DEBUG_WARN, ("%s::Un defined LED behavior, skip it (Mode:%d, Status:%d)\n", LEDModeFromEEPROM, Status));
        return;
    }

    //LEDModeFromEEPROM = 0xA; //no need to be used in the future since SendLEDCmd will be changed.

    SendLEDCmd(pAd, LEDNumber, LEDBehavior); //LED ON
#endif
}

/*
    ========================================================================
    
    Routine Description:
        Set LED Signal Stregth 

    Arguments:
        pAd                     Pointer to our adapter
        Dbm                     Signal Stregth

    Return Value:
        None

    IRQL = PASSIVE_LEVEL
    
    Note:
        Can be run on any IRQL level. 

        According to Microsoft Zero Config Wireless Signal Stregth definition as belows.
        <= -90  No Signal
        <= -81  Very Low
        <= -71  Low
        <= -67  Good
        <= -57  Very Good
         > -57  Excellent       
    ========================================================================
*/
VOID LedCtrlSetSignalLed(
    IN PMP_ADAPTER    pAd, 
    IN NDIS_802_11_RSSI Dbm)
{
    UCHAR       nLed = 0;

    if(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
        return;

    if (Dbm <= -90)
        nLed = 0;
    else if (Dbm <= -81)
        nLed = 1;
    else if (Dbm <= -71)
        nLed = 3;
    else if (Dbm <= -67)
        nLed = 7;
    else if (Dbm <= -57)
        nLed = 15;
    else 
        nLed = 31;

    //
    // Update Signal Stregth to if changed.
    //
    if ((pAd->HwCfg.LedIndicatorStregth != nLed) && 
        (pAd->HwCfg.LedCntl.field.LedMode == LED_MODE_SIGNAL_STREGTH))
    {
        pAd->HwCfg.LedIndicatorStregth = nLed;
        AsicSendCommanToMcu(pAd, 0x51, 0xff, nLed, pAd->HwCfg.LedCntl.field.Polarity);
    }
}

VOID LedCtrlBlinkThread(
    IN PMP_ADAPTER       pAd
    )
{
    UCHAR WPSLEDStatus = 0;
    FUNC_ENTER;
    
    KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);        

    while (pAd->TerminateThreads != TRUE)
    {
        PlatformWaitEventTriggeredAndReset(&(GET_THREAD_EVENT(&pAd->HwCfg.hLedThread)),0);   
        DBGPRINT(RT_DEBUG_INFO, ("LedBlinkThread Thread Triggered\n"));
        
        WPSLEDStatus = pAd->HwCfg.SetLedStatus;
       
        LedCtrlSetLed(pAd, WPSLEDStatus);
    }   
    
    FUNC_LEAVE;
}

VOID 
LedCtrlSetBlinkMode(
    IN PMP_ADAPTER       pAd,
    IN UCHAR                WPSLedStatus
    )
{
    pAd->HwCfg.SetLedStatus = WPSLedStatus;
    NdisSetEvent(&(GET_THREAD_EVENT(&pAd->HwCfg.hLedThread)));
}

//
// LED indication for normal connection start.
//
VOID
LedCtrlConnectionStart(
    IN PMP_ADAPTER pAd
    )
{
    PMP_PORT pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];
    // LED indication.
    if (pPort->StaCfg.WscControl.bWPSSession == FALSE)
    {
        if ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_9) // LED mode 9.
        {
            
            // The AP uses OPEN-NONE.
            if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeOpen) && (pPort->PortCfg.WepStatus == Ralink802_11EncryptionDisabled))
            {
                if(pPort->StaCfg.WscControl.bIndicateConnNotTurnOff == FALSE)
                {
                    LedCtrlSetBlinkMode(pAd, LED_WPS_TURN_LED_OFF);
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_TURN_LED_OFF\n", __FUNCTION__));
                }
                else
                {
                    pPort->StaCfg.WscControl.bIndicateConnNotTurnOff = FALSE;
                }
            }
            else // The AP uses an encryption algorithm.
            {
                UCHAR   i;
                BOOLEAN WepStatusEqualAP = FALSE;
                for (i = 0; i < pAd->ScanTab.BssNr; i++) 
                {
                    BSS_ENTRY *pInBss = &pAd->ScanTab.BssEntry[i];
                    // pick up the first available vacancy
                    if ((MAC_ADDR_EQUAL(pInBss->Bssid, pPort->PortCfg.Bssid)) && (pInBss->WepStatus == pPort->PortCfg.WepStatus))
                    {
                        WepStatusEqualAP = TRUE;
                        break;
                    }
                }
                if((pPort->StaCfg.WscControl.bWPSMODE9SKIPProgress == FALSE)&&WepStatusEqualAP)
                {
                    LedCtrlSetBlinkMode(pAd, LED_WPS_IN_PROCESS);
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_IN_PROCESS\n", __FUNCTION__));
                }
                else
                {
                    pPort->StaCfg.WscControl.bWPSMODE9SKIPProgress = FALSE;
                }
            }
        }
    }
}

//
// LED indication for normal connection completion.
//
VOID
LedCtrlConnectionCompletion(
    IN PMP_ADAPTER pAd, 
    IN  PMP_PORT pPort,
    IN BOOLEAN bSuccess)
{
    // LED indication.
    if (pPort->StaCfg.WscControl.bWPSSession == FALSE)
    {
        if ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_9) // LED mode 9.
        {
            
            if (bSuccess == TRUE) // Successful connenction.
            {   
                if(INFRA_ON(pPort))
                {
                    if ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_9) // LED mode 9.
                    {           
                        // The AP uses OPEN-NONE.
                        if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeOpen) && (pPort->PortCfg.WepStatus == Ralink802_11EncryptionDisabled))
                        {
                            LedCtrlSetBlinkMode(pAd, LED_NORMAL_CONNECTION_WITHOUT_SECURITY);
                            DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_NORMAL_CONNECTION_WITHOUT_SECURITY\n", __FUNCTION__));
                        }
                        else // The AP uses an encryption algorithm.
                        {
                            LedCtrlSetBlinkMode(pAd, LED_NORMAL_CONNECTION_WITH_SECURITY);
                            DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_NORMAL_CONNECTION_WITH_SECURITY\n", __FUNCTION__));
                        }
                    }
                }
            }
            else // Connection failure.
            {
                LedCtrlSetBlinkMode(pAd, LED_WPS_TURN_LED_OFF);
                DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_WPS_TURN_LED_OFF\n", __FUNCTION__));
            }
        }
    }
}
