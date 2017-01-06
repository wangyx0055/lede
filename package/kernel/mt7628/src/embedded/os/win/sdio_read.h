#ifndef _SDIO_READ_H_
#define _SDIO_READ_H_
//why 0x04: sdio header is 4 bytes, each event data have one sdio header
#define SDIO_HEADER_LEN   0X04

//why 0xFFFF: pkt len in CHISR is unsigned short. Greater thant this have no meaning
#define SDIO_MAX_PACKET_LEN  0x0000FFFF


//IN byte mode,  the align size is 4 bytes
#define BYTE_ALIGN  4

VOID EvtIoCanceledOnQueue(IN WDFQUEUE  Queue,IN WDFREQUEST  Request);

VOID ReadRx(RTMP_ADAPTER *pAdapter, size_t RxPktLen);

VOID ReadRxIdx(RTMP_ADAPTER *pAdapter, size_t RxPktLen, UINT32 Idx);

//VOID DelayToReadRx(RTMP_ADAPTER *pAdapter);

#endif  // _SDIO_READ_H_
