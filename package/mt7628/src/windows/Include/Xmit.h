
#ifndef __XMIT_H__
#define __XMIT_H__

NDIS_STATUS
XmitSendPacket(
    IN  PMP_ADAPTER       pAd,    
    IN  NDIS_PORT_NUMBER    PortNumber,
    IN  PMT_XMIT_CTRL_UNIT             pXcu
    );

NDIS_STATUS
XmitStaSendPacket(
    IN  PMP_ADAPTER       pAd,    
    IN  NDIS_PORT_NUMBER    PortNumber,
    IN  PMT_XMIT_CTRL_UNIT             pXcu,
    IN  BOOLEAN            bAironetFrame
    );

VOID
XmitSendMlmeCmdPkt(
    IN  PMP_ADAPTER       pAd,
    IN  MLME_PKT_TYPE    PktType
    );    

// MT7603 FPGA
NDIS_STATUS 
XmitHardTransmit(
    IN  PMP_ADAPTER   pAd,
    IN  PMT_XMIT_CTRL_UNIT     pXcu
    );    

NDIS_STATUS 
XmitSendBAR(
    IN  PMP_ADAPTER   pAd,
    IN  PMT_XMIT_CTRL_UNIT     pXcu,
    IN    UCHAR         QueIdx  
    );    

USHORT  
XmitCalcDuration(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           Rate,
    IN  ULONG           Size
    );    

VOID XmitSendPsPollFrame(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort
    );

BOOLEAN 
XmitCheckDHCPFrame(
    IN  PMP_ADAPTER   pAd,
    IN  PMT_XMIT_CTRL_UNIT    pXcu,
    OUT PUSHORT         Protocol,
    OUT BOOLEAN         *pbGroupMsg2,
    OUT BOOLEAN         *pDHCPv6
    );  

VOID 
XmitResumeMsduTransmission(
    IN  PMP_ADAPTER   pAd
    );

USHORT  
XmitCalcUsbDuration(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           Rate,
    IN  ULONG           Size
    );

VOID 
XmitSendNullFrameForWake(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  UCHAR           TxRate,
    IN  BOOLEAN         bQosNull
    );   

VOID 
XmitSendNullFrameForDirect(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    //IN    HTTRANSMIT_SETTING  MinTransmit,
    PHY_CFG             MinPhyCfg,
    IN  BOOLEAN         bQosNull,
    IN  PUCHAR          pDA
    ); 

VOID 
XmitSendNullFrame(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  UCHAR           TxRate,
    IN  BOOLEAN         bQosNull
    );

VOID 
XmitFirmwareFrame(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  BOOLEAN         bQosNull
    );    

#ifdef MULTI_CHANNEL_SUPPORT
VOID    
XmitSendNullFrameForChSwitch(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  UCHAR           TxRate,
    IN  BOOLEAN         bQosNull,
    IN  USHORT          PwrMgmt
    );
#endif /*MULTI_CHANNEL_SUPPORT*/

ULONG 
XmitMCSToPhyRateIn500kps(
    ULONG uMCS, 
    ULONG uMode, 
    ULONG bBW, 
    ULONG bSGI
    );

UCHAR 
XmitMulticastRateNRateToOFDMRate(
    ULONG uMCS,
    ULONG bBW
    );

UCHAR 
XmitMappingOneSpatialStreamRate(
    ULONG uMCS
    );

VOID 
XmitWriteTxWI(
    IN  PMP_ADAPTER   pAd,
    IN      PMP_PORT      pPort,
    IN  PTXWI_STRUC     pTxWI,
    IN    BOOLEAN           FRAG,   
    IN    BOOLEAN           InsTimestamp,
    IN  BOOLEAN         AMPDU,
    IN  BOOLEAN         Ack,
    IN  BOOLEAN         NSeq,       // HW new a sequence.
    IN  UCHAR           BASize,
    IN  UCHAR           WCID,
    IN  ULONG           Length,
    IN    UCHAR         PID,
    IN  UCHAR           MIMOps,
    IN  UCHAR           Txopmode,   
    IN  BOOLEAN         CfAck,  
//  IN  HTTRANSMIT_SETTING  Transmit,
    IN  PHY_CFG PacketPhyCfg, 
    IN  BOOLEAN bNDPAnnouncement, 
    IN  BOOLEAN bSkipTxBF,
    IN  BOOLEAN IsDataFrame
    );

VOID 
XmitWriteTxInfo(
    IN  PMP_ADAPTER   pAd,
    IN      PMP_PORT      pPort,
    IN  PTXINFO_STRUC   pTxInfo,
    IN    USHORT        USBDMApktLen,
    IN    BOOLEAN       bWiv,
    IN    UCHAR         QueueSel,
    IN    UCHAR         NextValid,
    IN    UCHAR         TxBurst
    );

VOID 
XmitWriteTxD(
    IN  PMP_ADAPTER           pAd,
    IN      PMP_PORT              pPort,
    IN  PTXDSCR_SHORT_STRUC pShortFormatTxD,
    IN  PTXDSCR_LONG_STRUC  pLongFormatTxD,
    IN  HARD_TRANSMIT_INFO      TxInfo
    );

 BOOLEAN 
 XmitFrameIsAggregatible(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          pPrevAddr1,
    IN  PUCHAR          p80211hdr
    );
    
#endif
