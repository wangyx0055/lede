/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2010, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    p2p.h
    
    Abstract:
    Peer to peer is also called Wifi Direct. P2P is a Task Group of WFA. P2P header file.

    Revision History:
    Who              When               What
    --------    ----------    ----------------------------------------------
    Jan Lee         2009-10-05    created for Peer-to-Peer(Wifi Direct)
*/
#ifndef __P2P_H__
#define __P2P_H__

#define P2P_OUI                 0x099a6f50
#define P2P_RANDOM_BASE         25      //; 2.5 second.
#define P2P_RANDOM_WPS_BASE     10      //; 5 second.
#define P2P_RANDOM_BIAS         10      //; 2 second.
#define P2P_RANDOM_PERST_BIAS   60      //; 2 second.
#define P2P_SCAN_CHANNEL_STEP   11      //
#define P2P_SCAN_LONGPERIOD     3000    // unit : 100ms. 5 MIN
#define P2P_SCAN_PERIOD         150     // unit : 100ms. Scan period how long ? 2min ?
#define P2P_SCAN_FOR_CONNECTION     1200 //  unit : 100ms.
#define P2P_SCAN_SHORTPERIOD    200     // unit : 100ms. Scan period how long ? 2min ?
#define P2P_CONNECTION_FAIL         1200
#define P2P_SCAN_EARLYSTAGE     40  // unit : 100ms. In first 20 seconds.. only scan social channesl. 
#define P2P_EXT_LISTEN_INTERVAL 4000    // unit is 1ms
#define P2P_EXT_LISTEN_PERIOD   1000    // unit is 1ms

#define P2P_CHECK_GO_TIMER      200000  // 3 minutes
#define P2P_CHECK_CLIENT_TIMER   120000 // 30 seconds
#define P2P_CHECK_STARTGROUPFORM_TIMER      120000  // 2 minutes

#define ONETU                       100
#define SUBID_P2P_STATUS            0
#define SUBID_P2P_MINOR_REASON      1
#define SUBID_P2P_CAP               2
#define SUBID_P2P_DEVICE_ID         3
#define SUBID_P2P_OWNER_INTENT      4
#define SUBID_P2P_CONFIG_TIMEOUT    5
#define SUBID_P2P_LISTEN_CHANNEL    6
#define SUBID_P2P_GROUP_BSSID       7
#define SUBID_P2P_EXT_LISTEN_TIMING 8
#define SUBID_P2P_INTERFACE_ADDR    9
#define SUBID_P2P_MANAGEABILITY     10
#define SUBID_P2P_CHANNEL_LIST      11
#define SUBID_P2P_NOA               12
#define SUBID_P2P_DEVICE_INFO       13
#define SUBID_P2P_GROUP_INFO        14
#define SUBID_P2P_GROUP_ID          15
#define SUBID_P2P_INTERFACE         16
#define SUBID_P2P_OP_CHANNEL        17
#define SUBID_P2P_INVITE_FLAG       18
#define SUBID_P2P_VENDOR            221

#define MINOR_REASON_SUCCESS                0
#define MINOR_REASON_CROSS_CONNECT          1
#define MINOR_REASON_BCZ_MANAGED_BIT_ZERO   2
#define MINOR_REASON_COEXIST_PARM_BAD       3
#define MINOR_REASON_OUTSIDE_IT_DEFINED     4


//bitmap of P2P Capability/ 1st byte is DEVCAP, 2nd byte is GRPCAP
#define DEVCAP_SD                   0x1 //Service discovery
#define DEVCAP_CLIENT_DISCOVER      0x2
#define DEVCAP_CLIENT_CONCURRENT    0x4
#define DEVCAP_INFRA_MANAGED        0x8
#define DEVCAP_DEVICE_LIMIT         0x10
#define DEVCAP_INVITE               0x20
#define GRPCAP_OWNER                0x1
#define GRPCAP_PERSISTENT           0x2
#define GRPCAP_LIMIT                0x4
#define GRPCAP_INTRA_BSS            0x8
#define GRPCAP_CROSS_CONNECT        0x10
#define GRPCAP_PERSISTENT_RECONNECT 0x20
#define GRPCAP_GROUP_FORMING        0x40
// Status
#define P2PSTATUS_SUCCESS           0
#define P2PSTATUS_PASSED            1
#define P2PSTATUS_IMCOMPA_PARM      2
#define P2PSTATUS_LIMIT             3
#define P2PSTATUS_INVALID_PARM      4
#define P2PSTATUS_UNABLE            5
#define P2PSTATUS_NO_CHANNEL        7
#define P2PSTATUS_UNKNOWN_GROUP     8
#define P2PSTATUS_BOTH_INTENT15     9
#define P2PSTATUS_INCOMPATIBLE_PROVISION    10
#define P2PSTATUS_REJECT_BY_USER            11

// P2P PUBLIC Action Frame Subtype(Signal)
#define GO_NEGOCIATION_REQ      0
#define GO_NEGOCIATION_RSP      1
#define GO_NEGOCIATION_CONFIRM  2
#define P2P_INVITE_REQ          3
#define P2P_INVITE_RSP          4
#define P2P_DEV_DIS_REQ         5
#define P2P_DEV_DIS_RSP         6
#define P2P_PROVISION_REQ       7
#define P2P_PROVISION_RSP       8
#define P2P_ACTION_SEND_COMPLETE        9

#ifdef WFD_NEW_PUBLIC_ACTION
// P2pMs PUBLIC Action Frame additional Signal
#define P2P_GOT_ACK                 100

// P2pMs PUBLIC Action Frame State machine
#define P2P_NULL_STATE              (P2P_GOT_ACK + 1)
#define P2P_SEND_STATE              (P2P_GOT_ACK + 2)
#define P2P_WAIT_ACK_STATE      (P2P_GOT_ACK + 3)
#define P2P_WAIT_RSP_STATE      (P2P_GOT_ACK + 4)

#define WFD_PUBLIC_ACTION_FRAME_RETRY_COUNT_MAX 15 
#define WFD_WAIT_ACK_TIMEOUT                            100 // unit: ms --> 100 msec
#define WFD_WAIT_RSP_TIMEOUT                            100 // unit: ms --> 100 msec

#endif /*WFD_NEW_PUBLIC_ACTION*/

// Public action frame address3 
#define USE_P2P_SPEC                                    1

// Timeout definitions
#define WFD_COMPLETE_SEND_TIMEOUT                   10  // unit: 100 ms, value >= (WFD_CHANNEL_SWITCH_TIMEOUT+WFD_PEER_CHANNEL_ACTION_TIMEOUT)
#define WFD_BLOCKING_SCAN_TIMEOUT                   300//600    // unit: 100 ms
#define WFD_CHANNEL_SWITCH_TIMEOUT                  1   // unit: 100 ms
#define WFD_PEER_CHANNEL_ACTION_TIMEOUT             5   // unit: 100 ms

#define MULTI_CHANNEL_NEW_CONNECTION_TIMEOUT        150 // unit: 100ms --> 15 sec
#define MULTI_CHANNEL_STOP_CONNECTION_TIMEOUT       50  // unit: 100ms --> 5 sec

// P2P ACTION Frame Subtype
#define P2PACT_NOA              0
#define P2PACT_PRESENCE_REQ     1
#define P2PACT_PRESENCE_RSP     2
#define P2PACT_GO_DISCOVER_REQ  3


#define P2P_NOT_FOUND               0xFF
// Invittion Flags
#define P2P_INVITE_FLAG_REINVOKE    1

#define P2P_DEFAULT_LISTEN_CHANNEL      11

// Check for Adapter's State.
// Currently has an active P2P Profile to connect.
#define IS_SOCIAL_CHANNEL(_C)       (((_C)== 1) ||((_C)== 6) ||((_C)== 11) )
#define PORT_P2P_ON(_P)     ((((_P)->PortSubtype) == PORTSUBTYPE_P2PClient) || (((_P)->PortSubtype) == PORTSUBTYPE_P2PGO))
#define PORTV2_P2P_ON(_A, _P)   (((_P) != NULL) && (((_P)->bActive) == TRUE) && (((_P)->PortNumber) != PORT_0) /*&& (((_P)->PortNumber) == (_A)->pP2pCtrll->P2PCfg.PortNumber)*/)
#define P2P_ON(_pPort)          (((_pPort)->P2PCfg.P2PDiscoProvState) >= P2P_DISABLE)
#define P2P_OFF(_pPort)         (((_pPort)->P2PCfg.P2PDiscoProvState) < P2P_DISABLE)
#define IS_P2P_PSM(_pPort)      (((_pPort)->P2PCfg.bOppsOn) || ((_pPort)->P2PCfg.bNoAOn))
#define IS_P2P_FINDING(_pPort)      ((IS_P2P_SEARCHING(_pPort) || IS_P2P_LISTENING(_pPort)))
#define IS_P2P_LISTENING(_pPort)    ((_pPort)->P2PCfg.P2PDiscoProvState == P2P_LISTEN)
#define IS_P2P_LISTEN_IDLE(_pPort)  ((_pPort)->P2PCfg.P2PDiscoProvState == P2P_ENABLE_LISTEN_ONLY)
#define IS_P2P_SEARCHING(_pPort)    (((_pPort)->P2PCfg.P2PDiscoProvState == P2P_SEARCH))
#define IS_P2P_GO_NEG(_pPort)       (((_pPort)->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_CLIENT) || ((_pPort)->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO))

#define IS_P2P_AUTOGO(_pPort)       (((_pPort)->P2PCfg.GoIntentIdx == 16))
#define IS_P2P_NOT_AUTOGO(_pPort)       (((_pPort)->P2PCfg.GoIntentIdx != 16))
#define IS_P2P_REGISTRA(_pPort)     (((_pPort)->P2PCfg.P2PConnectState == P2P_WPS_REGISTRA))
#define IS_P2P_ENROLLEE(_pPort)     (((_pPort)->P2PCfg.P2PConnectState == P2P_DO_WPS_ENROLLEE))
#define IS_P2P_PROVISIONING(_A) (IS_P2P_ENROLLEE(_A) || IS_P2P_REGISTRA(_A))
#define IS_P2P_GO_WPA2PSKING(_pPort)        ((_pPort)->P2PCfg.P2PConnectState == P2P_GO_ASSOC_AUTH)
#define IS_P2P_GO_OP(_pPort)        (((_pPort)->P2PCfg.P2PConnectState >= P2P_I_AM_GO_OP))
#define IS_P2P_CLIENT_OP(_pPort)    (((_pPort)->P2PCfg.P2PConnectState == P2P_I_AM_CLIENT_OP))
#define IS_P2P_OP(_pPort)           (IS_P2P_GO_OP(_pPort) || IS_P2P_CLIENT_OP(_pPort))
#define IS_P2P_SUPPORT_EXT_LISTEN(_pAd, _pPort)   (((_pPort)->P2PCfg.ExtListenInterval != 0) && ((_pPort)->P2PCfg.ExtListenPeriod != 0) && ((_pAd)->CommonCfg.P2pControl.field.ExtendListen == 1))
#define IS_P2P_CON_GO(_A, _P)       ((PORTV2_P2P_ON(_A, _P)) && ((_P)->PortType < WFD_DEVICE_PORT) && ((_P)->PortSubtype == PORTSUBTYPE_P2PGO) && ((_A)->OpMode == OPMODE_STAP2P)) //concurrent GO on virtual staion port
#define IS_P2P_STA_GO(_A, _P)       (((_P)->PortSubtype == PORTSUBTYPE_P2PGO) && ((_A)->OpMode == OPMODE_STA) && ((_P)->PortType == EXTSTA_PORT) /*&&  ((_A)->pP2pCtrll->P2PCfg.PortNumber == PORT_0)*/) // GO on PORT_0
#define IS_P2P_CON_CLI(_A, _P)      ((PORTV2_P2P_ON(_A, _P)) && ((_P)->PortType < WFD_DEVICE_PORT) && ((_P)->PortSubtype == PORTSUBTYPE_P2PClient) && ((_A)->OpMode == OPMODE_STAP2P)) //concurrent CLI on virtual staion port
#define IS_P2P_MS_DEV(_A, _P)       (((_P) != NULL) && (((_P)->bActive) == TRUE)  && ((_P)->PortType == WFD_DEVICE_PORT) /*&& ((_A)->pP2pCtrll->P2PCfg.PortNumber != PORT_0)*/) //Win8 MS Device
#define IS_P2P_MS_GO(_A, _P)        (((_P) != NULL) && (((_P)->bActive) == TRUE)  && ((_P)->PortType == WFD_GO_PORT)) //Win8 MS GO
#define IS_P2P_MS_CLI(_A, _P)       (((_P) != NULL) && (((_P)->bActive) == TRUE) &&  ((_P)->PortType == WFD_CLIENT_PORT))// Win8 MS Client#1
#define IS_P2P_MS_CLI2(_A, _P)      (((_P) != NULL) && (((_P)->bActive) == TRUE) && (((_P)->PortNumber) == (_P)->P2PCfg.Client2PortNumber) && ((_P)->PortType == WFD_CLIENT_PORT) /*&& ((_A)->pP2pCtrll->P2PCfg.Client2PortNumber != PORT_0)*/)// Win8 MS Client#2
#define IS_P2P_MS_CLI_WCID(_P, _IDX)      (((_IDX) == (USHORT)MlmeSyncGetWlanIdxByPort(_P, ROLE_WLANIDX_P2P_CLIENT)) )// Win8 MS Client
#define IS_P2P_CONNECT_IDLE(_pPort)     (((_pPort)->P2PCfg.P2PConnectState) == P2P_CONNECT_IDLE || ((_pPort)->P2PCfg.P2PConnectState) == P2P_NEGOTIATION)
#define IS_P2P_CONNECTING(_pPort)       ((((_pPort)->P2PCfg.P2PConnectState) > P2P_INVITE) && (((_pPort)->P2PCfg.P2PConnectState) < P2P_I_AM_CLIENT_OP))
#define IS_P2P_GO_NEGOING(_pPort)       (((_pPort)->P2PCfg.P2PConnectState < P2P_DO_GO_SCAN_BEGIN) &&  ((_pPort)->P2PCfg.P2PConnectState > P2P_CONNECT_IDLE))
#define IS_P2P_GROUP_FORMING(_pPort)    (((_pPort)->P2PCfg.P2PConnectState <= P2P_WPS_REGISTRA) &&  ((_pPort)->P2PCfg.P2PConnectState > P2P_CONNECT_IDLE))
#define IS_P2P_INVITING(_pPort)         (((_pPort)->P2PCfg.P2PConnectState == P2P_INVITE))
// Check for Peer's State.
#define IS_P2P_PEER_CLIENT_OP(_C)       (((_C)->P2pClientState >= P2PSTATE_CLIENT_OPERATING))
#define IS_P2P_PEER_PROVISIONING(_C)        (((_C)->P2pClientState == P2PSTATE_CLIENT_WPS) || ((_C)->P2pClientState == P2PSTATE_GO_WPS))
#define IS_P2P_PEER_WPAPSK(_C)      (((_C)->P2pClientState <= P2PSTATE_CLIENT_ASSOC)&& ((_C)->P2pClientState >= P2PSTATE_CLIENT_AUTH))
#define IS_P2P_PEER_GO_OP(_C)       (((_C)->P2pClientState >= P2PSTATE_GO_OPERATING))
#define IS_P2P_PEER_OP(_C)          ((IS_P2P_PEER_CLIENT_OP(_C)) || (IS_P2P_PEER_GO_OP(_C)))
#define IS_P2P_PEER_DISCOVERY(_C)       (((_C)->P2pClientState <= P2PSTATE_DISCOVERY_CLIENT)&& ((_C)->P2pClientState >= P2PSTATE_DISCOVERY))
#define IS_P2PPEER_CLIENT_GO_FORM(_C)   (((_C)->P2pClientState <= P2PSTATE_GO_COMFIRM_ACK_SUCCESS)&& ((_C)->P2pClientState >= P2PSTATE_DISCOVERY))

#define IS_P2P_ABSENCE(_pPort)  (((_pPort)->P2PCfg.bPreKeepSlient == TRUE) || ((_pPort)->P2PCfg.bKeepSlient == TRUE))
#define IS_PERSISTENT_ON(_A)    (_A->CommonCfg.P2pControl.field.EnablePresistent == 1)
#define IS_P2P_SIGMA_OFF(_pPort)        (_pPort->P2PCfg.LastConfigMode < CONFIG_MODE_SIGMA_P2P_RESET_DEFAULT)
#define IS_P2P_SIGMA_ON(_pPort)         (_pPort->P2PCfg.LastConfigMode >= CONFIG_MODE_SIGMA_P2P_RESET_DEFAULT)

#define CONFIG_MODE_DISABLE_P2P_WPSE        0   // Uncheck "enable WFD feature" #define CONFIG_MODE_DISABLE_WIFI_DIRECT_WPSE     0
#define CONFIG_MODE_ENABLE_P2P              1   // set device name to driver when check "enable WFD feature"
#define CONFIG_MODE_P2P_SCAN                2   // set device name to driver when press "Scan"
#define CONFIG_MODE_CONNECT_P2P             3   // set all config (RTMP_WIFI_DIRECT_CONFIG) to driver #define CONFIG_MODE_ACTIVATE_WIFI_DIRECT           1
#define CONFIG_MODE_ACTIVATE_WPSE           4   // #define CONFIG_MODE_ACTIVATE_WPSE                 2
#define CONFIG_MODE_DELETE_ACTIVE_P2P       5   // set to actived config (RTMP_WIFI_DIRECT_CONFIG) to driver (driver will send disconnect event and clean persistent table ) #define CONFIG_MODE_DELETE_ACTIVED_WIFI_DIRECT    3
#define CONFIG_MODE_DISCONNECT_P2P          6   // do nothing in UI when press "Disconnect" #define CONFIG_MODE_DISCONNECT_WIFI_DIRECT         4
#define CONFIG_MODE_DELETE_PERST_P2P        7   // set inactive but persistent config (RTMP_WIFI_DIRECT_CONFIG) to driver when delete perseistent profile from profile list #define CONFIG_MODE_DELETE_PERST_WIFI_DIRECT    5
#define CONFIG_MODE_PROVISION_THIS          8   //  
#define CONFIG_MODE_SET_GO_WPS              9   //  When I am GO. Set WPS for indivial client.
#define CONFIG_MODE_SERVICE_DISCOVERY       10  //  Start SErvice Discovery
#define CONFIG_MODE_NOTIFY_ICS_IS_ENABLED   11  // GUI notify he enables windows ICS.
#define CONFIG_MODE_NOTIFY_ICS_IS_DISABLED  12  // GUI notify he disables windows ICS.

/*** P2P Configuration Mode for SIGMA Control Agent ***/
#define CONFIG_MODE_SIGMA_P2P_RESET_DEFAULT             0x20    // Reset P2P to device defaults
#define CONFIG_MODE_SIGMA_SET_P2P_LISTEN                0x21    // Set P2P mode to Listen(Enable)
#define CONFIG_MODE_SIGMA_SET_P2P_DISCOVERY             0x22    // Set P2P mode to Discovery(Scan)
#define CONFIG_MODE_SIGMA_SET_P2P_DISABLE               0x23    // Set P2P mode to Disable
#define CONFIG_MODE_SIGMA_START_AUTONOMOUS_GO           0x24    // Start Autonomous GO
#define CONFIG_MODE_SIGMA_START_GROUP_FORMATION_REQ     0x25    // Start group formation request
#define CONFIG_MODE_SIGMA_WAIT_GROUP_FORMATION_REQ      0x26    // Accept group formation request and wait for it
#define CONFIG_MODE_SIGMA_P2P_CONNECT                   0x27    // Select GO to connect as a P2P Client, no group formation
#define CONFIG_MODE_SIGMA_P2P_DISSOLVE                  0x28    // Disconnect/Dissolve existing P2P group(s)
#define CONFIG_MODE_SIGMA_SEND_P2P_INVITATION_REQ       0x29    // Send the P2P invitation request. Invite a persisten P2P group or invite a P2P device to join a P2P group
#define CONFIG_MODE_SIGMA_ACCEPT_P2P_INVITATION_REQ     0x2A    // Trigger device accept the P2P invitation request if device needs user intervention
#define CONFIG_MODE_SIGMA_SEND_P2P_PROVISION_DIS_REQ    0x2B    // Send P2P provision discovery request
#define CONFIG_MODE_SIGMA_SET_WPS                       0x2C    // Set WPS Mode
#define CONFIG_MODE_SIGMA_P2P_SWITCH_OP_STA             0x2D    // Notify Driver sends a event to GUI for switching to STA mode
#define CONFIG_MODE_SIGMA_P2P_SWITCH_OP_STAP2P          0x2E    // Nnform Driver sends a event to GUI for switching to concurrent mode
#define CONFIG_MODE_SIGMA_SEND_P2P_PRESENCE_REQ         0x2F    // Send P2P presence request
#define CONFIG_MODE_SIGMA_CHANGE_P2PCTRL_SETTING        0x30    // Sigma change setting. Ask driver to upate parameters.
#define CONFIG_MODE_SIGMA_SET_P2P_IDLE                  0x31
#define CONFIG_MODE_SIGMA_SEND_P2P_SRVDISCO_REQ         0x32    // Send P2P Service Discovery request

#define P2P_SIGMA_OFF   0
#define P2P_SIGMA_ON    1

#define P2P_PHYMODE_LEGACY_ONLY     0
#define P2P_PHYMODE_ENABLE_11N_20   1
#define P2P_PHYMODE_ENABLE_11N_40   2

#define P2P_GROUP_MODE_TEMP         1
#define P2P_GROUP_MODE_PERSISTENT   2

#define    P2P_WPSE_PIN_METHOD      0x10000000
#define    P2P_WPSE_PBC_METHOD      0x20000000
#define    P2P_WPSE_SMPBC_METHOD    0x30000000

#define WMM_USP_STOP_PRIOR_TO_NOA_MARGIN        3000    // 3 msec

#define COUNTRY_USA         1
#define COUNTRY_EU          2
#define COUNTRY_JP          3
#define COUNTRY_GLOBAL          4

#define DEFAULT_HOWMANY_BI      5

typedef struct _RTMP_OID_SET_P2P_CONFIG 
{
    UCHAR   ConfigMode; // Disable, activate p2p, or activate WPSE, or delete p2p profile.
    ULONG   WscMode;    // Method : PIN or PBC or SMPBC
    UCHAR   PinCode[8];
    UCHAR   DeviceName[32];
    ULONG   DeviceNameLen;
    UCHAR   SSID[32];
    UCHAR   SSIDLen;
    UCHAR   P2PGroupMode; // temporary or persistent. See definition.
    UCHAR   GoIntentIdx;    // Value = 0~15. Intent to be a GO in P2P
    UCHAR   ConnectingMAC[MAX_P2P_GROUP_SIZE][6];  // Specify MAC address want to connect. Set to all 0xff or all 0x0 if not specified.
    UCHAR   ConnectingDeviceName[MAX_P2P_GROUP_SIZE][32];  // Specify the Device Name that want to connect. Set to all 0xff or all 0x0 if not specified.
    UCHAR   ListenChannel;
    UCHAR   OperatinChannel;
} RT_OID_SET_P2P_STRUCT, *PRT_OID_SET_P2P_STRUCT;


// Packet Format.
#define IE_P2P  0xdd
#pragma pack(1)

typedef struct {
    UCHAR       OUI[3];
    UCHAR       OUIType;
    UCHAR       Octet[1];
} P2P_IE, *PP2P_IE;

#define SIZE_OF_FIXED_CLIENT_INFO_DESC  25
typedef struct {
    UCHAR       Length;
    UCHAR       DevAddr[MAC_ADDR_LEN];
    UCHAR       InterfaceAddr[MAC_ADDR_LEN];
    UCHAR       Capability;
    UCHAR       ConfigMethod[2];
    UCHAR       PrimaryDevType[P2P_DEVICE_TYPE_LEN];
    UCHAR       NumSecondaryType;
    UCHAR       Octet[1];
} P2P_CLIENT_INFO_DESC, *PP2P_CLIENT_INFO_DESC;

typedef struct  _P2P_PUBLIC_FRAME   {
    HEADER_802_11   p80211Header;
    UCHAR           Category;
    UCHAR           Action;
    UCHAR           OUI[3];
    UCHAR           OUIType;
    UCHAR           Subtype;
    UCHAR           Token;
    UCHAR           ElementID;
    UCHAR           Length;
    UCHAR           OUI2[3];
    UCHAR           OUIType2;
    UCHAR           Octet[1];
}   P2P_PUBLIC_FRAME, *PP2P_PUBLIC_FRAME;

typedef struct  _P2P_ACTION_FRAME   {
    HEADER_802_11   p80211Header;
    UCHAR           Category;
    UCHAR           OUI[3];
    UCHAR           OUIType;
    UCHAR           Subtype;
    UCHAR           Token;
    UCHAR           Octet[1];
}   P2P_ACTION_FRAME, *PP2P_ACTION_FRAME;

typedef struct _FRAME_P2P_ACTION {
    HEADER_802_11   Hdr;
    UCHAR   Category;
    UCHAR   OUI[3];
    UCHAR   OUIType;    // 1
    UCHAR   OUISubType; // 1
    UCHAR   Token;  // 1
}   FRAME_P2P_ACTION, *PFRAME_P2P_ACTION;
#pragma pack()

typedef struct _MLME_P2P_ACTION_STRUCT {
    UCHAR      TabIndex;    // sometimes it's Mactable index, sometime it's P2P table index. depend on the command.
    UCHAR      Addr[MAC_ADDR_LEN];
} MLME_P2P_ACTION_STRUCT, *PMLME_P2P_ACTION_STRUCT;

typedef struct _P2pMs_ACTION_STRUCT {
    IN PMP_ADAPTER    pAd;
    IN PMP_PORT       pPort;
    UCHAR               Signal;
} P2pMs_ACTION_STRUCT, *PP2pMs_ACTION_STRUCT;

#ifdef MULTI_CHANNEL_SUPPORT
typedef struct _MULTI_CH_DEFER_OID_STRUCT {
    UCHAR           Type;
    NDIS_STATUS     ndisStatus;
} MULTI_CH_DEFER_OID_STRUCT, *PMULTI_CH_DEFER_OID_STRUCT;
#endif /*#ifdef MULTI_CHANNEL_SUPPORT*/

// Query via OID

VOID P2pActionStateMachineInit(
    IN  PMP_ADAPTER   pAd, 
    IN  STATE_MACHINE *S, 
    OUT STATE_MACHINE_FUNC Trans[]);

VOID P2pActionOneTimeNoAOffChannel(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN ULONG        Duration);

BOOLEAN P2pActionMlmeCntlOidScanProc(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN MLME_SCAN_REQ_STRUCT *pScanReq);

VOID P2pActionGasIntialReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID P2pActionGasIntialRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID P2pActionGasComebackReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID P2pActionGasComebackRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID P2pActionNoaAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID P2pActionPresRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID P2pActionGoDiscoverAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID P2pActionPresReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID P2pActionPeerNoaAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID P2pActionPeerPresRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID P2pActionPeerGoDiscoverAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID P2pActionPeerPresReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID P2pActionSendServiceReqCmd(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN UCHAR    p2pindex,
    IN PUCHAR   Addr);

VOID P2pActionSendComebackReq(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN UCHAR        p2pindex,
    IN PUCHAR   Addr);

VOID P2pActionSendPresenceReqCmd(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN UCHAR     macindex);

VOID P2pGOStartNoA(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN BOOLEAN      bOffChannel);

VOID P2pStopNoA(
    IN PMP_ADAPTER pAd, 
    IN PMAC_TABLE_ENTRY pMacClient);

VOID P2pStartOpPS(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID P2pStopOpPS(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID P2pPreAbsenTimeOutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

VOID P2pSwNoATimeOutTimerCallback(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3);

VOID SwTBTTTimeOutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

VOID P2pStopConnectAction(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);
    
BOOLEAN P2pHandleNoAAttri(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN PMAC_TABLE_ENTRY pMacClient,
    IN PUCHAR pData);

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
VOID P2pSendProReqTimeOutTimerCallback(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3);
#endif

BOOLEAN     P2pResetNoATimer(
    IN PMP_ADAPTER pAd,
    IN  ULONG   DiffTimeInus,
    IN BOOLEAN      bOffChannel);

BOOLEAN     P2pSetGP(
    IN PMP_ADAPTER pAd,
    IN  ULONG   DiffTimeInus);

BOOLEAN     P2pAdjustSwNoATimer(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN ULONG    CurrentTimeStamp, 
    IN ULONG    NextTimePoint);

VOID        P2pGPTimeOutHandle(
    IN PMP_ADAPTER pAd);

VOID        RTMPInitP2P(
    IN PMP_ADAPTER pAd);

VOID PeerGASIntialReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID PeerGASIntialRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID PeerGASComebackReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID PeerGASComebackRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID P2PPublicAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID P2pResumeMsduAction(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID P2pCopyPerstParmToCfg(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT       pPort,
    IN UCHAR        Perstindex);

VOID P2pSetPerstTable(
    IN PMP_ADAPTER pAd, 
    IN PVOID pInformationBuffer);

VOID P2pSetProfileParm(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN PRT_OID_SET_P2P_STRUCT pP2pStruc);

VOID P2pGOSetProfileParm(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN PRT_OID_SET_P2P_STRUCT pP2pStruc);

NDIS_STATUS P2pSetProfile(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN ULONG InformationBufferLength,
    IN PVOID pInformationBuffer); 

VOID P2pSetGoWps(
    IN PMP_ADAPTER pAd,
    IN PRT_OID_SET_P2P_STRUCT       pP2pStruc);

VOID P2pEnable(
    IN PMP_ADAPTER pAd);

VOID P2pMlmeCntLinkUp(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort);

VOID P2pMlmeCntLinkDown(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort,
    IN BOOLEAN          IsReqFromAP);

VOID P2pEdcaDefault(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort);

VOID P2PDefaultConfigM(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID P2pDefaultListenChannel(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT      pPort);

BOOLEAN P2pScanNextchannel(
    IN PMP_ADAPTER pAd);

VOID P2pScanChannelDefault(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID P2pScanChannelUpdate(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID P2pSetDefaultGOHt(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID P2pSetGOTxRateSwitch(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort);

VOID P2pDecidePhyMode(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

BOOLEAN P2pDecideHtBw(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort);

VOID P2pPeriodicExec(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort);

VOID P2pRefreshTableThenScan(
    IN PMP_ADAPTER pAd,
    IN ULONG         CounterAftrScanButton);

VOID P2pScan(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort);

VOID P2pStopScan(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort);

VOID P2pCheckInviteReq(
    IN PMP_ADAPTER pAd,
    IN BOOLEAN      bIAmGO,
    IN UCHAR        index,
    IN PUCHAR   ChannelList,
    IN PUCHAR   BssidAddr,
    IN UCHAR        OpChannel,
    IN PUCHAR   Ssid,
    IN UCHAR    SsidLen,
    IN UCHAR    *pRspStatus);

VOID P2pCheckInviteReqFromExisting(
    IN PMP_ADAPTER pAd,
    IN PUCHAR   ChannelList,
    IN PUCHAR   BssidAddr,
    IN UCHAR        OpChannel,
    IN PUCHAR   Ssid,
    IN UCHAR    SsidLen,
    IN UCHAR    *pRspStatus);

UCHAR ChannelToClass(
    IN UCHAR        Channel,
    IN UCHAR        Country);

BOOLEAN P2pCheckChannelList(
    IN PMP_ADAPTER pAd,
    IN PUCHAR   pChannelList);

VOID P2PMakeFakeNoATlv(
    IN PMP_ADAPTER pAd,
     IN  PMP_PORT        pPort,
    IN ULONG     StartTime,
    IN PUCHAR       pOutBuffer);

ULONG InsertP2PGroupInfoTlv(
    IN PMP_ADAPTER pAd,
    IN PUCHAR       pOutBuffer);

// P2P Subelement was renamed to P2P attribute in final verison spec. 
// At first, it's called Subelement.
// So the function is named to InsertP2PSubelmtTlv.  but I prefer still use InsertP2PSubelmtTlv
ULONG InsertP2PSubelmtTlv(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN UCHAR            SubId,
    IN PUCHAR       pInBuffer,
    IN PUCHAR       pOutBuffer);

VOID InsertP2pChannelList(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,  
    IN PUCHAR    ChannelList,
    OUT ULONG    *ChannelListLen,
    OUT PUCHAR  pDest);

BOOLEAN P2PParseServiceDiscoReq(
    IN PMP_ADAPTER pAd, 
    IN PFRAME_802_11    pFrame, 
    IN UCHAR        *ServiceTransaction);

BOOLEAN P2PParseServiceDiscoRsp(
    IN PMP_ADAPTER pAd, 
    IN  PMP_PORT        pPort,
    IN PRT_P2P_CLIENT_ENTRY pP2pEntry,
    IN UCHAR        pP2pidx,
    IN PFRAME_802_11    pFrame, 
    IN UCHAR        ServiceTransaction);

BOOLEAN P2PParseComebackReq(
    IN PMP_ADAPTER pAd, 
    IN PFRAME_802_11    pFrame, 
    IN UCHAR        *ServiceTransaction);

BOOLEAN P2PParseComebackRsp(
    IN PMP_ADAPTER pAd, 
    IN PRT_P2P_CLIENT_ENTRY pP2pEntry,
    IN PFRAME_802_11    pFrame, 
    IN UCHAR        *ServiceTransaction);

BOOLEAN P2pParseGroupInfoAttribute(
    IN PMP_ADAPTER pAd, 
    IN  PMP_PORT        pPort,
    IN UCHAR P2pindex, 
    IN VOID *Msg, 
    IN ULONG MsgLen);

VOID P2pParseNoASubElmt(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    IN UCHAR  p2pindex);

VOID P2pParseExtListenSubElmt(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT USHORT *ExtListenPeriod,
    OUT USHORT *ExtListenInterval);

VOID P2pParseManageSubElmt(
    IN PMP_ADAPTER pAd, 
    IN  PMP_PORT        pPort,
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT UCHAR *pChannel,
    OUT UCHAR *pNumOfP2pOtherAttribute,
    OUT UCHAR *pTotalNumOfP2pAttribute,
    OUT UCHAR *pMamageablity,
    OUT UCHAR *pMinorReason);

VOID P2pParseSubElmt(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    IN BOOLEAN  bBeacon, 
    OUT USHORT *pDpid,
    OUT UCHAR *pGroupCap,
    OUT UCHAR *pDeviceCap,
    OUT UCHAR *pDeviceName,
    OUT UCHAR *pDeviceNameLen,
    OUT UCHAR *pDevAddr,
    OUT UCHAR *pInterFAddr,
    OUT UCHAR *pBssidAddr,
    OUT UCHAR *pSsidLen,
    OUT UCHAR *pSsid,
    OUT USHORT *pConfigMethod,
    OUT USHORT *pWpsConfigMethod,
    OUT UCHAR *pDevType,
    OUT UCHAR *pNumSecondaryType,
    OUT UCHAR *pSecondarDevTypes,   
    OUT UCHAR *pListenChannel,
    OUT UCHAR *pOpChannel,
    OUT UCHAR *pChannelList,
    OUT UCHAR *pIntent,
    OUT UCHAR *pStatusCode,
    OUT UCHAR *pInviteFlag);

VOID P2pReceGoNegoConfirmAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem); 

VOID P2pReceGoNegoRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem); 

VOID P2pSendProbeReq(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT      pPort,
    IN MP_PORT_SUBTYPE      PortSubtype);

VOID P2pReceGoNegoReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem); 

VOID P2pReceDevDisReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID P2pReceDevDisRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID P2pReceInviteRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID P2pReceInviteReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID P2pReceProvisionReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID P2pReceProvisionRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);


VOID P2PMakeGoNegoConfirm(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN PUCHAR       Addr1,
    IN UCHAR            Token,
    IN PUCHAR       pOutBuffer,
    OUT PULONG      pTotalFrameLen);

VOID P2PSendGoNegoConfirm(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN UCHAR            Token,
    IN UCHAR            idx,
    IN PUCHAR       Addr1,
    IN UCHAR        StatusCode);

VOID P2PSendDevDisReq(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN PUCHAR       Addr1,
    IN PUCHAR       Bssid,
    IN PUCHAR       ClientAddr1,
    OUT PULONG      pTotalFrameLen);

VOID P2PSendDevDisRsp(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN UCHAR        RspStatus,
    IN UCHAR            Token,
    IN PUCHAR       Addr1,
    OUT PULONG      pTotalFrameLen);

VOID P2PSendProvisionReq(
    IN PMP_ADAPTER pAd,
    IN USHORT       ConfigMethod,
    IN UCHAR            Token,
    IN PUCHAR       Addr1,
    OUT PULONG      pTotalFrameLen);

VOID P2PSendProvisionRsp(
    IN PMP_ADAPTER pAd,
    IN USHORT       ConfigMethod,
    IN UCHAR            Token,
    IN PUCHAR       Addr1,
    OUT PULONG      pTotalFrameLen);

VOID P2PMakeGoNegoRsp(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN PUCHAR       Addr1,
    IN USHORT           ReceDpid,
    IN UCHAR            Token,
    IN UCHAR            TempIntent,
    IN UCHAR            Channel,
    IN UCHAR            Status,
    IN PUCHAR       pOutBuffer,
    OUT PULONG      pTotalFrameLen);

VOID P2PMakeGoNegoReq(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN UCHAR            index,
    IN PUCHAR       Addr1,
    IN PUCHAR       pOutBuffer,
    OUT PULONG      pTotalFrameLen);

VOID P2PMakeInviteReq(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN UCHAR            MyRule,
    IN UCHAR            InviteFlag,
    IN PUCHAR       Addr1,
    IN PUCHAR       Bssid,
    OUT PULONG      pTotalFrameLen);

VOID P2PMakeInviteRsp(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN UCHAR        MyRule,
    IN UCHAR        Token,
    IN PUCHAR       Addr1,
    IN PUCHAR       Bssid,
    IN PUCHAR       OpChannel,
    IN PUCHAR       Status,
    OUT PULONG      pTotalFrameLen);

VOID P2pAckRequiredCheck(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN PP2P_PUBLIC_FRAME    pFrame,
    OUT         UCHAR   *TempPid);

VOID P2pCopySettingsWhenLinkup(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort);

VOID P2pCalculateChannelQuality(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN ULONGLONG Now64);

VOID P2pClientStateUpdate(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

BOOLEAN IsP2pFirstMacSmaller(
    IN PUCHAR       Firststaddr,
    IN PUCHAR       SecondAddr);

BOOLEAN P2PIsScanAllowed (
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT     pPort);

VOID P2pSetListenIntBias(
    IN PMP_ADAPTER pAd,
    IN UCHAR        Bias);

VOID P2pSetRule(
    IN PMP_ADAPTER pAd,
    IN UCHAR        Index,
    IN PMP_PORT   pPort,
    IN PUCHAR       PeerBssid,
    IN UCHAR        PeerGOIntent,
    IN UCHAR        Channel,
    IN PUCHAR       PeerDevAddr);

VOID P2pEventMaintain(
    IN PMP_ADAPTER pAd);
    
VOID P2pGroupMaintain(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort);

VOID P2pCopyP2PTabtoMacTab(
    IN PMP_ADAPTER pAd,
    IN UCHAR        P2pindex,
    IN UCHAR        Macindex);

VOID P2pGroupTabInit(
    IN PMP_ADAPTER pAd);

VOID P2pGroupMacTabDisconnect(
    IN  PMP_ADAPTER   pAd,
    IN PMP_PORT     pPort,
    IN  UCHAR           DeleteClass);

UCHAR P2pGroupTabInsert(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR    Addr,
    IN P2P_CLIENT_STATE State,
    IN CHAR Ssid[], 
    IN UCHAR SsidLen);

UCHAR P2pGroupTabDelete(
    IN PMP_ADAPTER pAd, 
    IN UCHAR    p2pindex, 
    IN PUCHAR    Addr);

UCHAR P2pGroupTabSearch(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR    Addr); 

VOID P2pPerstTabClean(
    IN PMP_ADAPTER pAd);

UCHAR P2pPerstTabInsert(
    IN PMP_ADAPTER pAd,
    IN PUCHAR   pAddr);

UCHAR P2pPerstTabDelete(
    IN PMP_ADAPTER pAd,
    IN UCHAR    *pMacList);

UCHAR P2pPerstTabSearch(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR    Addr);

VOID P2pCrednTabClean(
    IN PMP_ADAPTER pAd);

VOID P2pCrednTabInsert(
    IN PMP_ADAPTER pAd,
    IN PUCHAR   pAddr,
    IN WSC_CREDENTIAL   *pProfile);

BOOLEAN P2pCrednEntrySearch(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR    Addr,
    IN PUCHAR   ResultIndex);

// For Win8 WFD
VOID P2pDiscoTabInit(
    IN PMP_ADAPTER pAd);

UCHAR P2pDiscoTabSearch(
    IN PMP_ADAPTER    pAd, 
    IN PUCHAR           TxAddr,
    IN PUCHAR           DevAddr,
    OUT UCHAR           DevIndexArray[]);

UCHAR P2pDiscoTabDelete(
    IN PMP_ADAPTER pAd, 
    IN UCHAR        p2pindex, 
    IN PUCHAR       Addr);

UCHAR P2pDiscoTabInsert(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR    TxAddr,
    IN PUCHAR    DevAddr,
    IN UCHAR Ssid[], 
    IN UCHAR SsidLen);

BOOLEAN P2pStartGroupForm(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN PUCHAR   Addr,
    IN UCHAR        idx);

BOOLEAN P2pProvision(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN PUCHAR   Addr);

VOID P2pConnect(
    IN PMP_ADAPTER pAd);

VOID P2pConnectForward(
    IN PMP_ADAPTER pAd);

BOOLEAN P2pConnectAfterScan(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN BOOLEAN  bBeacon,
    IN UCHAR        idx);

VOID P2pConnectAction(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN BOOLEAN  bBeacon,
    IN UCHAR        index);

VOID P2pConnectP2pClient(
    IN PMP_ADAPTER pAd, 
    IN UCHAR        GrpIndex);

BOOLEAN P2pConnectP2pGo(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN UCHAR        idx);

NDIS_STATUS P2pDisconnectOneConnection(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN ULONG InformationBufferLength,
    IN PVOID pInformationBuffer);

VOID P2pAutoReconnectPerstClient(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR       Addr,
    IN ULONG        CounterAutoReconnectForP2pClient);

BOOLEAN P2pClientDiscovery(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN PUCHAR   Addr,
    IN UCHAR        GoP2pTabIdx);

BOOLEAN P2pInviteAsRule(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN UCHAR        MyRule,
    IN UCHAR        P2pTabIdx);

BOOLEAN P2pInvite(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN PUCHAR   Addr,
    IN UCHAR        PersistentTabIdx, 
    IN UCHAR        P2pTabIdx);

BOOLEAN PeerP2pProbeReqSanity(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT   pPort,
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT CHAR Ssid[], 
    OUT UCHAR *pSsidLen, 
//  OUT RALINKIP_IE *RequestIp,
    OUT ULONG *P2PSubelementLen, 
    OUT PUCHAR pP2pSubelement, 
//  OUT USHORT *WSCInfoAtBeaconsLen,
//  OUT PUCHAR WSCInfoAtBeacons,
//  OUT USHORT *WSCInfoAtProbeRspLen,
//  OUT PUCHAR WSCInfoAtProbeRsp,
    OUT ULONG *WpsIELen, 
    OUT PUCHAR pWpsIE);


BOOLEAN PeerP2pBeaconSanity(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT   pPort,
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT CHAR Ssid[], 
    OUT UCHAR *pSsidLen, 
    OUT ULONG *Peerip,
    OUT ULONG *P2PSubelementLen, 
    OUT PUCHAR pP2pSubelement);

BOOLEAN PeerP2pProbeRspSanity(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT   pPort,
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT CHAR Ssid[], 
    OUT UCHAR *pSsidLen, 
    OUT ULONG *Peerip,
    OUT ULONG *P2PSubelementLen, 
    OUT PUCHAR pP2pSubelement);

BOOLEAN P2pMlmeSyncMlmeSyncPeerBeaconAtJoinAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem,
    IN PUCHAR       Bssid,
    IN UCHAR        SelectedRegistrar);

BOOLEAN PeerP2pBeaconProbeRspAtScan(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT pPort,
    IN MLME_QUEUE_ELEM *Elem,
    IN USHORT CapabilityInfo,
    IN UCHAR    WorkingChannel,
    OUT PRT_P2P_CLIENT_ENTRY *pP2PBssEntry,
    OUT PRT_P2P_DISCOVERY_ENTRY *pP2PDiscoEntry);

VOID PeerP2pBeacon(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT pPort,
    IN PUCHAR   pAddr2,
    IN MLME_QUEUE_ELEM *Elem);

BOOLEAN MlmeSyncPeerBeaconParseRalinkIE( 
        IN PMP_ADAPTER pAd, 
        IN PMP_PORT   pPort,
        IN VOID *Msg, 
        IN ULONG MsgLen, 
//      OUT RALINKIP_IE             *pRalinkIE,
//      OUT RALINKMBRIP_ELEM                *pMemberip,
        OUT ULONG *pPeerip);
#if DBG
VOID PeerP2pProbeReq(
    IN PMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem,
    IN BOOLEAN  bSendRsp,
    ULONG Line);
#else
VOID PeerP2pProbeReq(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem,
    IN BOOLEAN  bSendRsp);
#endif

BOOLEAN P2pGoPeerAuthAtAuthRspIdleAction(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR       Addr2);

BOOLEAN P2PParseWPSIE(
    IN PUCHAR   pWpsData,
    IN USHORT       WpsLen,
    OUT PUSHORT Dpid,
    OUT PUSHORT ConfigMethod,
    OUT PUCHAR  DeviceName,
    OUT UCHAR   *DeviceNameLen,
    OUT PUCHAR  pSelectedRegistrar,
    OUT PUCHAR  pNumRequestedType,  
    OUT PUCHAR  pDevType);

BOOLEAN P2PDeviceMatch(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR   Addr,
    IN PUCHAR   DeviceName,
    IN ULONG        DeviceNameLen);

VOID P2PMakeProbe(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT   pPort,
    IN MLME_QUEUE_ELEM *Elem, 
    IN MP_PORT_SUBTYPE      PortSubtype,
    IN UCHAR        DsChannel,
    IN USHORT   SubType,
    OUT PUCHAR pDest,
    OUT ULONG *pFrameLen);

VOID P2pMakeP2pIE(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,  
    IN  MP_PORT_SUBTYPE     PortSubtype,
    IN  UCHAR           PacketType,
    OUT PUCHAR          pOutBuf,
    OUT PULONG          pIeLen);

BOOLEAN P2pSetEvent(
    IN  PMP_ADAPTER   pAd, 
    IN PMP_PORT     pPort,
    IN    UCHAR     Status);

VOID P2pSetWps(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT  pPort);

VOID P2pGotoIdle(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT    pPort);

VOID P2pGotoScan(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN P2P_DISCOVERY_TYPE ScanType,
    IN ULONG StartScanRound);

VOID P2pGroupFormFailHandle(
    IN PMP_ADAPTER pAd);

VOID P2pGoNegoDone(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT       pPort,
    IN PRT_P2P_CLIENT_ENTRY pP2pEntry);

VOID P2pWpsDone(
    IN PMP_ADAPTER pAd,
    IN PUCHAR       pAddr,
    IN PMP_PORT       pPort);

VOID P2pCopyMacTabtoP2PTab(
    IN PMP_ADAPTER pAd,
    IN UCHAR        P2pindex,
    IN UCHAR        Macindex);

VOID P2pConnectStateUpdate(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT      pPort);

VOID P2pWPADone(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN BOOLEAN      bGO);

VOID P2pReceiveEapNack(
    IN PMP_ADAPTER pAd,
    IN  PMLME_QUEUE_ELEM    pElem);

VOID P2pIndicateGo(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort);

VOID P2pMakeProbeRspWSCIE(
    IN  PMP_ADAPTER   pAd,
    IN PMP_PORT       pPort,
    IN USHORT           SubType,
    OUT PUCHAR          pOutBuf,
    OUT PULONG          pIeLen);

VOID P2pMakeBeaconWSCIE(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT        pPort,
    OUT PUCHAR          pOutBuf,
    OUT PULONG          pIeLen);

VOID P2PResumeGroup(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,   
    IN UCHAR        MyRule,
    IN BOOLEAN      AutoSelect);

VOID P2pStartAutoGo(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID P2pStartGo(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort);

VOID P2pBackToOpChannel(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID P2pGoSwitchChannel(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort);

VOID P2pStopGo(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort);

VOID P2PStopClientTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3);

VOID P2PStopWpsTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3);

BOOLEAN P2pCheckBssSync(
    IN PMP_ADAPTER pAd);

VOID P2PCTWindowTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3);

VOID P2PStopGoTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3);

VOID GoPeerDisassocReq(
    IN PMP_ADAPTER pAd,
    IN PMAC_TABLE_ENTRY pEntry,
    IN PUCHAR   Addr2);

VOID P2pConnectRequest(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT      pPort);

ULONG P2pUpdateGroupBeacon(
    IN PMP_ADAPTER pAd,
    IN ULONG    StartPosition);

ULONG P2pUpdateNoABeacon(
    IN PMP_ADAPTER pAd,
    IN ULONG    StartPosition);

ULONG P2pUpdateNoAProbeRsp(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT        pPort,
    IN PUCHAR   pInbuffer);

VOID P2pUpdateBeaconP2pCap(
    IN PMP_ADAPTER pAd,
    IN  PUCHAR   pCapability);

VOID P2pMakeBssBeacon(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort);

VOID GoIndicateConnectStatus(
    IN PMP_ADAPTER        pAd);

VOID GoIndicateAssociationStatus(
    IN PMP_ADAPTER    pAd,
    IN PUCHAR           pAddr,
    IN UCHAR            p2pidx);

VOID
P2pShutdown(
    IN PMP_ADAPTER pAd
    );

NDIS_STATUS
    P2pDown(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort
    );

VOID    P2PSetSigmaStatus(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort,
    IN UCHAR            Rule);

VOID P2PInitSigmaParam(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort);

VOID P2pCheckAndUpdateGroupChannel(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT         pPort,
    IN PUCHAR           ChannelList);

VOID P2PUpdateCapability(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT         pPort);

VOID P2PSetHwReg(
    IN  PMP_ADAPTER pAd,
    IN  PRT_802_11_HARDWARE_REGISTER pHardwareRegister);

VOID GOUpdateBeaconFrame(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort);

UCHAR P2pGetClientWcid(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort);

LPSTR decodeDpid (USHORT dpid);
LPSTR decodeP2PRule (USHORT Rule);
LPSTR decodeP2PErrCode (ULONG Code);
LPSTR decodeConfigMethod (USHORT ConfigMethos);
LPSTR decodeP2PState (UCHAR P2pState);
LPSTR decodeP2PClientState (P2P_CLIENT_STATE P2pClientState);
#ifdef MULTI_CHANNEL_SUPPORT
LPSTR decodeMultiChannelMode (USHORT MultiChannelCurrentMode);
#endif /*MULTI_CHANNEL_SUPPORT*/

VOID P2PPrintPort(
    IN PMP_ADAPTER pAd);

VOID P2PPrintMac(
    IN PMP_ADAPTER pAd,
    IN UCHAR    macindex);

VOID P2PPrintP2PEntry(
    IN PMP_ADAPTER pAd,
    IN UCHAR        p2pindex);

VOID P2PPrintDiscoEntry(
    IN PMP_ADAPTER pAd,
    IN UCHAR        p2pindex);

VOID P2PPrintP2PPerstEntry(
    IN PMP_ADAPTER pAd,
    IN UCHAR        p2pindex);

VOID P2PPrintState(
    IN PMP_ADAPTER pAd);

VOID P2PPrintStateOnly(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID P2pStopConnectThis(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID P2pSigmaSetProfile(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN ULONG InformationBufferLength,
    IN PVOID pInformationBuffer);

VOID P2PDebugUseCmd(
    IN  PMP_ADAPTER pAd,
    IN  PRT_802_11_HARDWARE_REGISTER pHardwareRegister);

VOID P2PExitWhckCpuCheck(
    IN PMP_ADAPTER pAd);

#ifdef MULTI_CHANNEL_SUPPORT
VOID MultiChannelSwitchExec(
    IN PMP_PORT pPort,
    IN PUCHAR pInBuffer);

VOID MultiChannelSwitchTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

VOID MultiChannelSwitchKernel(
    IN PMP_ADAPTER pAd); 

VOID MultiChannelSwicthHwQAndCh(
    IN PMP_ADAPTER pAd,
    IN ULONG HwQSel,
    IN UCHAR Channel,
    IN UCHAR ExtChanOffset);

VOID MultiChannelGOStop(
    IN PMP_ADAPTER pAd);

VOID MultiChannelCLIStop(
    IN PMP_ADAPTER pAd);

VOID MultiChannelSTAStop(
    IN PMP_ADAPTER pAd);

VOID MultiChannelGOStartNewConnection(
    IN PMP_ADAPTER pAd);

VOID MultiChannelCLIOutStartNewConnection(
    IN PMP_PORT pPort);

VOID MultiChannelCLIInStartNewConnection(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID MultiChannelSTAStartNewConnection(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID MultiChannelSwitchStop(
    IN PMP_ADAPTER pAd);

VOID MultiChannelEnabeAllHwQ(
    IN PMP_ADAPTER pAd);

VOID MultiChannelNewP2pConnectionFail(
    IN PMP_ADAPTER pAd);  

NDIS_STATUS MultiChannelDeferDisconnectRequest( 
    IN PMP_ADAPTER pAd, 
    IN  PMP_PORT  pPort);

NDIS_STATUS MultiChannelDeferStartGoRequest( 
    IN PMP_ADAPTER pAd, 
    IN  PMP_PORT  pPort);

NDIS_STATUS MultiChannelDeferScanRequest( 
    IN PMP_ADAPTER pAd, 
    IN  PMP_PORT  pPort);

NDIS_STATUS MultiChannelDeferStopGoRequest( 
    IN PMP_ADAPTER pAd, 
    IN  PMP_PORT  pPort);

NDIS_STATUS MultiChannelDeferResetRequest( 
    IN PMP_ADAPTER pAd, 
    IN  PMP_PORT  pPort);

VOID MultiChannelDeferRequestIndication( 
    IN PMP_ADAPTER pAd, 
    IN PUCHAR   pInBuffer);

VOID MultiChannelDecideGOOpCh(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID MultiChannelWaitHwQEmpty(
    IN PMP_ADAPTER pAd,
    IN UCHAR            QueIdx,
    IN UINT32           NewI);

VOID MultiChannelWaitTxRingEmpty(
    IN PMP_ADAPTER    pAd);

UCHAR   MultiChannelGetSwQ(
    IN UCHAR    HwQ);

UCHAR   MultiChannelGetHwQ(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT  pPort);

VOID MultiChannelSetDeafultHwQ(
     IN PMP_ADAPTER pAd,
    IN  PMP_PORT  pPort);

VOID MultiChannelSuspendMsduTransmission(
    IN  PMP_ADAPTER   pAd);

VOID MultiChannelResumeMsduTransmission(
    IN  PMP_ADAPTER   pAd);

VOID MultiChannelGetHwQAndCh(
    IN PMP_ADAPTER pAd,
    IN PULONG pHwQSel,
    IN PUCHAR pChannel,
    IN PUCHAR pExtChanOffset);

VOID RtmpSetSwTSF(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    ULONGLONG     TimeStamp);

VOID RtmpGetSwTSF(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN ULONGLONG *pTSF);

BOOLEAN MultiChannelDecision(
    IN PMP_PORT pPort,
    IN UCHAR PrimaryCh1,
    IN UCHAR PrimaryCh2,
    IN UCHAR CentralCh1,
    IN UCHAR ExtChanOffset1,
    IN UCHAR CentralCh2,
    IN UCHAR ExtChanOffset2);

VOID MultiChannelResumeNextMode(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID
MultiChannelCmdThread(
    IN PMP_ADAPTER pAd
    );

#endif /* MULTI_CHANNEL_SUPPORT */

VOID MultiChannelSetCentralCh(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN UCHAR Channel,
    IN UCHAR ExtChanOffset,
    IN USHORT ChannelWidth);

VOID
P2PCmdThread(
    IN PMP_ADAPTER pAd
    );
#endif  // __P2P_H__
