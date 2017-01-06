
#ifndef _LOOPBACK_H_
#define _LOOPBACK_H_

#if 1//def RTMP_USB_SUPPORT
void LoopBack_Start(RTMP_ADAPTER *pAd, LOOPBACK_SETTING *pSetting);
void LoopBack_Stop(RTMP_ADAPTER *pAd);
void LoopBack_Status(RTMP_ADAPTER *pAd, LOOPBACK_RESULT *pResult);
void LoopBack_RawData(RTMP_ADAPTER *pAd, UINT32 *pLength, BOOLEAN IsTx, UINT8 *pRawData);
void LoopBack_ExpectRx(RTMP_ADAPTER *pAd, UINT32 Length, UINT8 *pRawData);
void LoopBack_ExpectTx(RTMP_ADAPTER *pAd, UINT32 Length, UINT8 *pRawData);
void LoopBack_Run(RTMP_ADAPTER *pAd, LOOPBACK_SETTING *pSetting, UINT32 length);
void LoopBack_BitTrueCheck(RTMP_ADAPTER *pAd);
void LoopBack_Fail(RTMP_ADAPTER *pAd,  LOOPBACK_FAIL FailNum);
void LoopBack_TxThread(IN OUT PVOID Context);
#endif
#endif // _LOOPBACK_H_

