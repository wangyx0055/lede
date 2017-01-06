
#ifndef _TXBF_H_
#define _TXBF_H_

//CR offset
#define CR_BF_CTRL0			WIFI_PHY_BASE + 0x00004800
#define CR_BF_CTRL1			WIFI_PHY_BASE + 0x00004808
#define CR_BF_PFILE_ACCESS	WIFI_PHY_BASE + 0x0000480C

#define CR_BF_PFILE_RDATA0	WIFI_PHY_BASE + 0x00004828
#define CR_BF_PFILE_RDATA1 WIFI_PHY_BASE + 0x0000482C
#define CR_BF_PFILE_RDATA2 WIFI_PHY_BASE + 0x00004830
#define CR_BF_PFILE_RDATA3 WIFI_PHY_BASE + 0x00004834
#define CR_BF_PFILE_RDATA4 WIFI_PHY_BASE + 0x00004838

#define CR_BF_PFILE_WDATA0 WIFI_PHY_BASE + 0x00004810
#define CR_BF_PFILE_WDATA1 WIFI_PHY_BASE + 0x00004814
#define CR_BF_PFILE_WDATA2 WIFI_PHY_BASE + 0x00004818
#define CR_BF_PFILE_WDATA3 WIFI_PHY_BASE + 0x0000481C
#define CR_BF_PFILE_WDATA4 WIFI_PHY_BASE + 0x00004820

#define CR_BF_CSD_1			 WIFI_PHY_BASE + 0x00004884



// Divider phase calibration closed loop definition
#define RX0TX0     0
#define RX1TX1     5

#define ADC0_RX0_2R   8
#define ADC1_RX1_2R   8


//#define MRQ_FORCE_TX		//Force MRQ regardless the capability of the station


// TxSndgPkt Sounding type definitions
#define SNDG_TYPE_DISABLE	0
#define SNDG_TYPE_SOUNDING	1
#define SNDG_TYPE_NDP		2

// Explicit TxBF feedback mechanism
#define ETXBF_FB_DISABLE	0
#define ETXBF_FB_CSI		1
#define ETXBF_FB_NONCOMP	2
#define ETXBF_FB_COMP		4

// MCS FB definitions
#define MSI_TOGGLE_BF		6
#define TOGGLE_BF_PKTS		5// the number of packets with inverted BF status

// TXBF State definitions
#define READY_FOR_SNDG0		0//jump to WAIT_SNDG_FB0 when channel change or periodically
#define WAIT_SNDG_FB0		1//jump to WAIT_SNDG_FB1 when bf report0 is received
#define WAIT_SNDG_FB1		2
#define WAIT_MFB			3
#define WAIT_USELESS_RSP	4
#define WAIT_BEST_SNDG		5

#define NO_SNDG_CNT_THRD	0//send sndg packet if there is no sounding for (NO_SNDG_CNT_THRD+1)*500msec. If this =0, bf matrix is updated at each call of APMlmeDynamicTxRateSwitchingAdapt()


// ------------ BEAMFORMING PROFILE HANDLING ------------

#define IMP_MAX_BYTES		14		// Implicit: 14 bytes per subcarrier
#define IMP_MAX_BYTES_ONE_COL	7	// Implicit: 7 bytes per subcarrier, when reading first column
#define EXP_MAX_BYTES		18		// Explicit: 18 bytes per subcarrier

#define MAX_BYTES            2      // 2 bytes per subcarrier for implicit and explicit TxBf

#define IMP_COEFF_SIZE		 9		// 9 bits/coeff
#define IMP_COEFF_MASK		0x1FF

#define PROFILE_MAX_CARRIERS_20		56		// Number of subcarriers in 20 MHz mode
#define PROFILE_MAX_CARRIERS_40		114		// Number of subcarriers in 40 MHz mode

#define PROFILE_MAX_CARRIERS_80		242		// Number of subcarriers in 80 MHz mode

#define NUM_CHAIN			 3
#if 1
// Indices of valid rows in Implicit and Explicit profiles for 20 and 40 MHz
typedef struct _SC_TABLE_ENTRY{
	int lwb1, upb1;
	int lwb2, upb2;
} SC_TABLE_ENTRY;


typedef struct _PROFILE_DATA{
	BOOLEAN impProfile;
	BOOLEAN fortyMHz;
	int rows, columns;
	int grouping;
	UCHAR tag[EXP_MAX_BYTES];
	UCHAR data[PROFILE_MAX_CARRIERS_40][EXP_MAX_BYTES];
} PROFILE_DATA;

extern PROFILE_DATA profData;

typedef
struct _ITXBF_PHASE_PARAMS{
	UCHAR E1gBeg;
	UCHAR E1gEnd;
	UCHAR E1aHighBeg;
	UCHAR E1aHighEnd;
	UCHAR E1aLowBeg;
	UCHAR E1aLowEnd;
	UCHAR E1aMidBeg;
	UCHAR E1aMidMid;
	UCHAR E1aMidEnd;	
} ITXBF_PHASE_PARAMS;			// ITxBF BBP reg phase calibration parameters

typedef
struct _ITXBF_LNA_PARAMS{
	UCHAR E1gBeg[3];
	UCHAR E1gEnd[3];
	UCHAR E1aHighBeg[3];
	UCHAR E1aHighEnd[3];
	UCHAR E1aLowBeg[3];
	UCHAR E1aLowEnd[3];
	UCHAR E1aMidBeg[3];
	UCHAR E1aMidMid[3];
	UCHAR E1aMidEnd[3];
} ITXBF_LNA_PARAMS;			// ITxBF BBP reg LNA calibration parameters

typedef
struct _ITXBF_DIV_PARAMS{
	UCHAR E1gBeg;
	UCHAR E1gEnd;
	UCHAR E1aHighBeg;
	UCHAR E1aHighEnd;
	UCHAR E1aLowBeg;
	UCHAR E1aLowEnd;
	UCHAR E1aMidBeg;
	UCHAR E1aMidMid;
	UCHAR E1aMidEnd;
} ITXBF_DIV_PARAMS;				// ITxBF Divider Calibration parameters
#endif
#if 0
void ITxBFGetEEPROM(
	IN RTMP_ADAPTER *pAd,
	IN ITXBF_PHASE_PARAMS *phaseParams,
	IN ITXBF_LNA_PARAMS *lnaParams,
	IN ITXBF_DIV_PARAMS *divParams);
*/
INT8 ITxBFDividerCalibrationStartUp(
	IN RTMP_ADAPTER *pAd,
	IN int calFunction,
	IN int calMethod,
	OUT UCHAR *divPhase);

INT8 ITxBFDividerCalibration(
	IN RTMP_ADAPTER *pAd,
	IN int calFunction,
	IN int calMethod,
	OUT UCHAR *divPhase);

VOID ITxBFLoadLNAComp(
	IN RTMP_ADAPTER *pAd);

INT8 ITxBFLNACalibration(
	IN RTMP_ADAPTER *pAd,
	IN int calFunction,
	IN int calMethod,
	IN BOOLEAN gBand);

INT8 mt76x2_ITxBFDividerCalibration(
	IN RTMP_ADAPTER *pAd,
	IN int calFunction,
	IN int calMethod,
	OUT UCHAR *divPhase);

VOID mt76x2_ITxBFLoadLNAComp(
	IN RTMP_ADAPTER *pAd);

int mt76x2_ITxBFLNACalibration(
	IN RTMP_ADAPTER *pAd,
	IN int calFunction,
	IN int calMethod,
	IN BOOLEAN gBand);


INT8 ITxBFLNACalibrationStartUp(
	IN RTMP_ADAPTER *pAd,
	IN INT calFunction,
	IN INT calMethod,
	IN BOOLEAN gBand);

INT8 ITxBFPhaseCalibrationStartUp(
	IN RTMP_ADAPTER *pAd,
	IN INT calFunction);

INT8 ITxBFPhaseCalibration(
	IN RTMP_ADAPTER *pAd,
	IN INT calFunction);

void Read_TxBfProfile(
	IN	RTMP_ADAPTER	*pAd, 
	IN	PROFILE_DATA	*prof,
	IN	int				profileNum,
	IN	BOOLEAN			implicitProfile);

void Write_TxBfProfile(
	IN	RTMP_ADAPTER	*pAd, 
	IN	PROFILE_DATA	*prof,
	IN	int				profileNum);

void Read_TagField(
	IN	PRTMP_ADAPTER	pAd, 
	IN  UCHAR	*row,
	IN  int		profileNum);
	
// Write_TagField - write a profile tagfield
void Write_TagField(
	IN	RTMP_ADAPTER *pAd, 
	IN  UCHAR	*row,
	IN  int		profileNum);
#endif

INT8 TxBfProfileTagRead(
	IN RTMP_ADAPTER		*pAd,
	IN UCHAR			profileIdx,
	OUT PFMU_PROFILE	*prof);

INT8 TxBfProfileTagWrite(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			profileIdx,	
	IN PFMU_PROFILE		*prof);

INT8 TxBfProfileDataRead(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			profileIdx,
	IN UCHAR			SubcarrierIdx,
	OUT PFMU_DATA		*pPfmuD);

INT8 TxBfProfileDataWrite(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			profileIdx,
	IN UCHAR			SubcarrierIdx,
	IN PFMU_DATA 		*pPfmuD);

INT8 TxBfProfileTagValid(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			validFlg, 
	IN UCHAR			profileIdx);
#if 0
	
// displayTagfield - display one tagfield
void displayTagfield(
	IN	RTMP_ADAPTER *pAd, 
	IN	int		profileNum,
	IN	BOOLEAN implicitProfile);
	
// Unpack an ITxBF matrix element from a row of bytes
INT8 Unpack_IBFValue(
	IN UCHAR *row,
	IN int elemNum);

INT8 iCalcCalibration(
	IN RTMP_ADAPTER *pAd, 
	IN int calParams[2], 
	IN int profileNum);

void ITxBFSetEEPROM(
	IN RTMP_ADAPTER *pAd,
	IN ITXBF_PHASE_PARAMS *phaseParams,
	IN ITXBF_LNA_PARAMS *lnaParams,
	IN ITXBF_DIV_PARAMS *divParams);
#endif

INT8 TxBfProfileDataWriteAll(
         IN RTMP_ADAPTER *pAd, 
         IN UCHAR TxBfType,
         IN UCHAR profileIdx,
         IN UCHAR BW,
         IN PFMU_DATA *pPfmuD);
INT8 TxBfProfileDataReadAll(
         IN RTMP_ADAPTER *pAd, 
         IN UCHAR TxBfType,
         IN UCHAR profileIdx,
         IN UCHAR BW,
         OUT PFMU_DATA *pPfmuD,
         OUT UINT32 *size);
INT8 TxBfProfileTag_MCS(
         IN RTMP_ADAPTER *pAd, 
         IN UCHAR MCS_ub,
         IN UCHAR MCS_lb);
INT8 TxBfProfileTag_Matrix(
         IN RTMP_ADAPTER *pAd, 
         IN UCHAR ng,
         IN UCHAR cw,
         IN UCHAR nrow,
         IN UCHAR ncol,
         IN UCHAR LM);
INT8 TxBfProfileTag_SNR(
         IN RTMP_ADAPTER *pAd, 
         IN UCHAR StsSnr0,
         IN UCHAR StsSnr1);
INT8 TxBfProfileTag_TxScale(
         IN RTMP_ADAPTER *pAd, 
         IN UCHAR Tx1_scale_2ss,
         IN UCHAR Tx0_scale_2ss,
         IN UCHAR Tx1_scale_1ss,
         IN UCHAR Tx0_scale_1ss);
INT8 TxBfProfileTag_MAC(
         IN RTMP_ADAPTER *pAd, 
         IN UCHAR *pMac);
INT8 TxBfProfileTag_Flg(
         IN RTMP_ADAPTER *pAd, 
         IN UCHAR LD,
         IN UCHAR EO,
         IN UCHAR IO,
         IN UCHAR IE);
INT8 TxBfBypassMacAddress(
         IN RTMP_ADAPTER *pAd, 
         IN BOOLEAN IsEnable);

void TxBFPowerNormalize(IN RTMP_ADAPTER *pAd, IN BOOLEAN IsEnable);

void MT_SetATETxBfProc(RTMP_ADAPTER *pAd, UINT8 TxBfEnFlg,  UINT32 BW);

void IsPfmuChanged(RTMP_ADAPTER *pAd, UINT32 bw, UINT32 *IsChanged);
void IsBFProfileStore(RTMP_ADAPTER *pAd, UINT32 IsStore);
#endif // _TXBF_H_

