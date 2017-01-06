/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    aironet.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Paul Lin    06-08-08        Initial
*/

#ifndef __WSC_H__
#define __WSC_H__


//Messages for the WSC state machine,
#define WSC_MACHINE_BASE    34
#define WSC_EAP_REQ_MSG     34
#define WSC_EAP_RSP_MSG     35
#define WSC_EAP_FAIL_MSG    36

#define MAX_WSC_MSG         5

#define PBC_ENR_TAB_SIZE    8

// WSC OUI SMI
#define WSC_OUI             0x0050f204
#define WSC_SMI             0x00372A
#define WSC_VENDOR_TYPE     0x00000001

//8021X PACKET TYPE 
#define EAPOL_TYPE_PACKET           0
#define EAPOL_TYPE_START            1

// EAP code
#define EAP_CODE_REQ        0x01
#define EAP_CODE_RSP        0x02
#define EAP_CODE_FAIL       0x04
#define EAP_TYPE_ID         0x01
#define EAP_TYPE_NOTIFY     0x02
#define EAP_TYPE_WSC        0xfe

// WSC Opcode
#define WSC_OPCODE_START        0x01
#define WSC_OPCODE_ACK          0x02
#define WSC_OPCODE_NACK         0x03
#define WSC_OPCODE_MSG          0x04
#define WSC_OPCODE_DONE         0x05
#define WSC_OPCODE_FRAG_ACK     0x06

// Wsc EAP Messges type received from AP
#define WSC_MSG_EAP_OL_START        0x20    
#define WSC_MSG_EAP_REQ_ID          0x21    
#define WSC_MSG_EAP_REQ_START       0x22    
#define WSC_MSG_EAP_FAIL            0x23
#define WSC_MSG_EAP_RSP_ID          0x24
#define WSC_MSG_BEACON              0x01    // Use the definition in WSC_IE_MSG_TYPE
#define WSC_MSG_PROB_REQ            0x02
#define WSC_MSG_PROB_RSP            0x03
#define WSC_MSG_M1                  0x04
#define WSC_MSG_M2                  0x05
#define WSC_MSG_M2D                 0x06
#define WSC_MSG_M3                  0x07
#define WSC_MSG_M4                  0x08
#define WSC_MSG_M5                  0x09
#define WSC_MSG_M6                  0x0A
#define WSC_MSG_M7                  0x0B
#define WSC_MSG_M8                  0x0C
#define WSC_MSG_WSC_ACK             0x0D
#define WSC_MSG_WSC_NACK            0x0E
#define WSC_MSG_WSC_DONE            0x0F
#define WSC_MSG_FRAG_ACK            0x10
#define WSC_MSG_UNKNOWN             0xff

// WSC connection mode
#define WSC_PIN_MODE                0x10000000  // Default PIN
#define WSC_PBC_MODE                0x20000000
#define WSC_PIN_MODE_USER_SPEC      0x40000000
#define WSC_PIN_MODE_REGISTRA_SPEC  0x80000000

// WSC IE definitions
#define WSC_IE_AP_CHANNEL           0x1001
#define WSC_IE_ASSOC_STATE          0x1002
#define WSC_IE_AUTH_TYPE            0x1003
#define WSC_IE_AUTH_TYPE_FLAG       0x1004
#define WSC_IE_AUTHENTICATOR        0x1005
#define WSC_IE_CONF_MTHD            0x1008
#define WSC_IE_CONF_ERROR           0x1009
#define WSC_IE_CONF_URL4            0x100A
#define WSC_IE_CONF_URL6            0x100B
#define WSC_IE_CONN_TYPE            0x100C
#define WSC_IE_CONN_TYPE_FLAG       0x100D
#define WSC_IE_CREDENTIAL           0x100E
#define WSC_IE_ENCR_TYPE            0x100F
#define WSC_IE_ENCR_TYPE_FLAG       0x1010
#define WSC_IE_DEV_NAME             0x1011
#define WSC_IE_DEV_PASS_ID          0x1012
#define WSC_IE_E_HASH_1             0x1014
#define WSC_IE_E_HASH_2             0x1015
#define WSC_IE_E_SNONCE_1           0x1016
#define WSC_IE_E_SNONCE_2           0x1017
#define WSC_IE_ENCR_SETTING         0x1018
#define WSC_IE_EN_NONCE             0x101A
#define WSC_IE_FEATURE_ID           0x101B
#define WSC_IE_ID                   0x101C
#define WSC_IE_ID_PROOF             0x101D
#define WSC_IE_KWRAP_AUTH           0x101E
#define WSC_IE_KEY_ID               0x101F
#define WSC_IE_MAC_ADDR             0x1020
#define WSC_IE_MANUFACTURER         0x1021
#define WSC_IE_MSG_TYPE             0x1022
#define WSC_IE_MODEL_NAME           0x1023
#define WSC_IE_MODEL_NO             0x1024
#define WSC_IE_NETWORK_IDX          0x1026
#define WSC_IE_NETWORK_KEY          0x1027
#define WSC_IE_NETWORK_KEY_IDX      0x1028
#define WSC_IE_NEW_DEV_NAME         0x1029
#define WSC_IE_NEW_PASSWD           0x102A
#define WSC_IE_OOB_DEV_PASSWD       0x102C
#define WSC_IE_OS_VER               0x102D
#define WSC_IE_POWER_LVL            0x102F
#define WSC_IE_PSK_CURRENT          0x1030
#define WSC_IE_PSK_MAX              0x1031
#define WSC_IE_PUBLIC_KEY           0x1032
#define WSC_IE_RADIO_ENABLE         0x1033
#define WSC_IE_REBOOT               0x1034
#define WSC_IE_REG_CURRENT          0x1035
#define WSC_IE_REG_EST              0x1036
#define WSC_IE_REG_LIST             0x1037
#define WSC_IE_REG_MAX              0x1038
#define WSC_IE_REG_NONCE            0x1039
#define WSC_IE_REQ_TYPE             0x103A
#define WSC_IE_RSP_TYPE             0x103B
#define WSC_IE_RF_BAND              0x103C
#define WSC_IE_R_HASH_1             0x103D
#define WSC_IE_R_HASH_2             0x103E
#define WSC_IE_R_SNONCE_1           0x103F
#define WSC_IE_R_SNONCE_2           0x1040
#define WSC_IE_SEL_REG              0x1041
#define WSC_IE_SERIAL               0x1042
#define WSC_IE_STATE                0x1044
#define WSC_IE_SSID                 0x1045
#define WSC_IE_TOTAL_NET            0x1046
#define WSC_IE_UUID_E               0x1047
#define WSC_IE_UUID_R               0x1048
#define WSC_IE_VENDOR_EXT           0x1049
#define WSC_IE_VERSION              0x104A
#define WSC_IE_X_509_CERT_REQ       0x104B
#define WSC_IE_X_509_CERT           0x104C
#define WSC_IE_EAP_ID               0x104D
#define WSC_IE_MSG_COUNTER          0x104E
#define WSC_IE_PUBLIC_KEY_HASH      0x104F
#define WSC_IE_REKEY_KEY            0x1050
#define WSC_IE_KEY_LIFETIME         0x1051
#define WSC_IE_PERM_CFG_MTHD        0x1052
#define WSC_IE_REG_CFG_MTHD         0x1053
#define WSC_IE_PRI_DEV_TYPE         0x1054
#define WSC_IE_SEC_DEV_TYPE_LIST    0x1055
#define WSC_IE_PORTABLE_DEV         0x1056
#define WSC_IE_AP_SETUP_LOCK        0x1057
#define WSC_IE_APP_LIST             0x1058
#define WSC_IE_EAP_TYPE             0x1059
#define WSC_IE_INIT_VECTOR          0x1060
#define WSC_IE_KEY_PRO_AUTO         0x1061
#define WSC_IE_WSC_802_1X_ENABLE    0x1062
#define WSC_IE_APP_SESSION_KEY      0x1063
#define WSC_IE_WEP_TRANSMIT_KEY     0x1064
#define WSC_IE_SETTINGS_DELAY_TIME      0x04                //0x1065                //2.0 added
#define WSC_IE_NETWORK_KEY_SHAREABLE    0x02            //0x1066                //2.0 added
#define WSC_IE_VERSION2                 0x00                //0x1067                //2.0 added
#define WSC_IE_REQUEST_ENROLL           0x03                //0x1068                //2.0 added
#define WSC_IE_AUTHORIZED_MACS          0x01                //0x1069                //2.0 added
#define WSC_IE_REQUESTED_DEVICE_TYPE    0x106A


// Value of WSC_IE_DEV_PASS_ID 0x1012
#define DEV_PASS_ID_PIN             0x0000
#define DEV_PASS_ID_USER            0x0001
#define DEV_PASS_ID_MACHINE         0x0002
#define DEV_PASS_ID_REKEY           0x0003
#define DEV_PASS_ID_PBC             0x0004
#define DEV_PASS_ID_REG             0x0005
#define DEV_PASS_ID_NOSPEC          0xffff

// Common definition
#define WSC_VERSION                 0x10
#define WSC_VERSION2                0x20


#define CONFIG_METHOD_USBA          0x0001
#define CONFIG_METHOD_ETHERNET      0x0002
#define CONFIG_METHOD_LABEL         0x0004
#define CONFIG_METHOD_DISPLAY       0x0008
#define CONFIG_METHOD_EXT_NFC_TOKEN 0x0010
#define CONFIG_METHOD_INT_NFC_TOKEN 0x0020
#define CONFIG_METHOD_NFC_INT       0x0040
#define CONFIG_METHOD_PUSHBUTTON    0x0080
#define CONFIG_METHOD_KEYPAD        0x0100

/* ******************************************
 *[Enrollee registration]
 *  EAPOL-START TX --> EAP-Req(ID) RX --> EAP-Rsp(ID) TX --> EAP-Req(Start) RX -->  
 *  M1 TX --> M2 RX --> M3 TX --> M4 RX--> M5 TX --> M6 RX --> M7 TX --> M8 RX -->
 *  EAP-Rsp(Done) TX --> EAP-Fail RX
 *
 *[Registrar registration with Inb-EAP transport]
 *  EAPOL-START TX --> EAP-Req(ID) RX --> EAP-Rsp(ID) TX                       -->  
 *  M1 RX --> M2 TX --> M3 RX --> M4 TX--> M5 RX --> M6 TX --> M7 RX --> M8 TX -->
 *  EAP-Req(Done) RX --> EAP-Rsp(ACK) TX --> EAP-Fail RX
 *
 * *******************************************
 */
// Wsc status code
// 1. common
#define STATUS_WSC_NOTUSED                      0x0000      // Not used
#define STATUS_WSC_IDLE                         0x0001
#define STATUS_WSC_SCAN_AP                      0x0002      // Scanning AP
#define STATUS_WSC_START_ASSOC                  0x0003      // Begin associating to WPS AP
#define STATUS_WSC_LINK_UP                      0x0004      // Associated to WPS AP
#define STATUS_WSC_EAPOL_START_SENT             0x0005      // Sending EAPOL-Start
#define STATUS_WSC_EAP_RSP_ID_SENT              0x0006      // Sending EAP-Rsp(ID)
#define STATUS_WSC_EAP_RSP_ACK_SENT             0x0007      // Sending EAP-Rsp (ACK)
#define STATUS_WSC_EAP_RSP_NACK_SENT            0x0008      // Sending EAP-Rsp (NACK)
#define STATUS_WSC_EAP_REQ_NACK_RECEIVED        0x0009      // Receive EAP-Req (NACK)
#define STATUS_WSC_EAP_CONFIGURED               0x000F      // Configured
// 2. for enrollee
#define STATUS_WSC_EAP_REQ_START_RECEIVED       0x0011      // Receive EAP-Req (Start)
#define STATUS_WSC_EAP_M1_SENT                  0x0012      // Sending M1
#define STATUS_WSC_EAP_M2_RECEIVED              0x0013      // Received M2
#define STATUS_WSC_EAP_M2D_RECEIVED             0x0014      // Received M2D
#define STATUS_WSC_EAP_M3_SENT                  0x0015      // Sending M3
#define STATUS_WSC_EAP_M4_RECEIVED              0x0016      // Received M4
#define STATUS_WSC_EAP_M5_SENT                  0x0017      // Sending M5
#define STATUS_WSC_EAP_M6_RECEIVED              0x0018      // Received M6
#define STATUS_WSC_EAP_M7_SENT                  0x0019      // Sending M7
#define STATUS_WSC_EAP_M8_RECEIVED              0x001A      // Received M8
#define STATUS_WSC_EAP_RSP_DONE_SENT            0x001B      // Sending EAP-Rsp (Done)
// 3. for registrar
#define STATUS_WSC_EAP_M1_RECEIVED              0x0022      // Received M1
#define STATUS_WSC_EAP_M2_SENT                  0x0023      // Sending M2
#define STATUS_WSC_EAP_M2D_SENT                 0x0024      // Sending M2D
#define STATUS_WSC_EAP_M3_RECEIVED              0x0025      // Received M3
#define STATUS_WSC_EAP_M4_SENT                  0x0026      // Sending M4
#define STATUS_WSC_EAP_M5_RECEIVED              0x0027      // Received M5
#define STATUS_WSC_EAP_M6_SENT                  0x0028      // Sending M6
#define STATUS_WSC_EAP_M7_RECEIVED              0x0029      // Received M7
#define STATUS_WSC_EAP_M8_SENT                  0x002A      // Sending M8
#define STATUS_WSC_EAP_REQ_DONE_RECEIVED        0x002B      // Received EAP-Req (Done)
// 4. for External Registrar ...
#define STATUS_WSC_EXTREG_SCAN_ENROLLEE         0x002C      // As external registrar scan enrollees by probe request
#define STATUS_WSC_EXTREG_SCAN_ONE_ENROLLEE     0x002D      // Get one enrollee as external registrar


// All error message dtarting from 0x0100
#define STATUS_WSC_PBC_TOO_MANY_AP              0x0101      // Too many PBC AP avaliable
#define STATUS_WSC_PBC_NO_AP                    0x0102      // No PBC AP avaliable
#define STATUS_WSC_EAP_FAIL_RECEIVED            0x0103      // Received EAP-FAIL
#define STATUS_WSC_EAP_NONCE_MISMATCH           0x0104      // Receive Message with wrong NONCE
#define STATUS_WSC_EAP_INVALID_DATA             0x0105      // Receive Message without integrity
#define STATUS_WSC_PASSWORD_MISMATCH            0x0106      // Error PIN Code
#define STATUS_WSC_EXTREG_M6_FAIL               0x0107      // M6 fail as External Registrar, UI must pop message with the reuse of PIN number.
#define STATUS_WSC_M8_PROFILE_MAC_ERR           0x0108      // MAC in Credential of M8 is not eaqul to AP BSSID.
#define STATUS_WSC_EXTREG_MULTI_PBC_SESSIONS_DETECT 0x0109      // External registrar get UUID-E of M1 is different from Probe req.
#define STATUS_WSC_EXTREG_PBC_NO_ENROLLEE       0x010A      // External registrar get no enrollee in PBC.
#define STATUS_WSC_EXTREG_PBC_TOO_MANY_ENROLLEE 0x010B      // External registrar get more than one enrollee in PBC.


// Error message, UI stop querying
#define STATUS_WSC_INVALID_SSID                 0x0201      // Invalid SSID/BSSID in PIN mode
#define STATUS_WSC_EAP_REQ_WRONG_SMI            0x0202      // Receive EAP-Req with wrong WPS SMI Vendor Id
#define STATUS_WSC_EAP_REQ_WRONG_VENDOR_TYPE    0x0203      // Receive EAPReq with wrong WPS Vendor Type
#define STATUS_WSC_EAP_FAILED                   0x0204      // WPS EAP process failed


// WSC Processing Timer
#define WSC_REG_SESSION_TIMEOUT                 120000      // 2 min
#define WSC_PER_MSG_TIMEOUT                     15000       // 15 sec
#define WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT      300000      // 300 seconds
#define WSC_WPS_FAIL_LED_PATTERN_TIMEOUT            15000       // 15 seconds.
#define WSC_WPS_SESSION_OVERLAP_DETECTED_TIMEOUT    15000       // 15 seconds.
#define WSC_WPS_SKIP_TURN_OFF_LED_TIMEOUT           2000            // 2 seconds.
#define WSC_WPS_TURN_OFF_LED_TIMEOUT                1000            // 1 second.
#define WSC_SKIP_TURN_LED_OFF_AFTER_GETPROFILE      800     //0.8 second.
#define WSC_WPS_SKIP_TURN_OFF_LED_AFTER_SESSION_OVERLAP 500 //0.5 second.
#define WSC_WPS_SKIP_TURN_OFF_LED_TIMEOUT_AFTER_SUCCESS     4500    //4.5 seconds.


// WSC error messages
#define WSC_ERROR_NO_ERROR                      0x0000
#define WSC_ERROR_OOB_INT_READ_ERR              0x0001
#define WSC_ERROR_DECRYPT_CRC_FAIL              0x0002
#define WSC_ERROR_CHAN24_NOT_SUPP               0x0003
#define WSC_ERROR_CHAN50_NOT_SUPP               0x0004
#define WSC_ERROR_SIGNAL_WEAK                   0x0005
#define WSC_ERROR_NW_AUTH_FAIL                  0x0006
#define WSC_ERROR_NW_ASSOC_FAIL                 0x0007
#define WSC_ERROR_NO_DHCP_RESP                  0x0008
#define WSC_ERROR_FAILED_DHCP_CONF              0x0009
#define WSC_ERROR_IP_ADDR_CONFLICT              0x000A
#define WSC_ERROR_FAIL_CONN_REGISTRAR           0x000B
#define WSC_ERROR_MULTI_PBC_DETECTED            0x000C
#define WSC_ERROR_ROGUE_SUSPECTED               0x000D
#define WSC_ERROR_DEVICE_BUSY                   0x000E
#define WSC_ERROR_SETUP_LOCKED                  0x000F
#define WSC_ERROR_MSG_TIMEOUT                   0x0010
#define WSC_ERROR_REG_SESSION_TIMEOUT           0x0011
#define WSC_ERROR_DEV_PWD_AUTH_FAIL             0x0012

#define WSC_MAX_RX_BUFFER           2048

// WSC Configured Mode
#define WSC_ConfMode_DISABLE        0x00
#define WSC_ConfMode_ENROLLEE       0x01
#define WSC_ConfMode_REGISTRAR      0x02
#define WSC_ConfMode_REGISTRAR_UPNP 0x03

// WSC Setup State
#define WSC_APSTATE_UNCONFIGURED    0x01
#define WSC_APSTATE_CONFIGURED      0x02

// Device request/response type
#define WSC_MSGTYPE_ENROLLEE_INFO_ONLY      0x00
#define WSC_MSGTYPE_ENROLLEE_OPEN_8021X     0x01
#define WSC_MSGTYPE_REGISTRAR               0x02
#define WSC_MSGTYPE_AP_WLAN_MGR             0x03

// Pack struct to align at byte
#pragma pack(1)

// 802.1x authentication format
typedef struct  _IEEE8021X_FRAME    {
    UCHAR   Version;                    // 1.0
    UCHAR   Type;                       // 0 = EAP Packet
    USHORT  Length;
}   IEEE8021X_FRAME, *PIEEE8021X_FRAME;

// EAP frame format
typedef struct  _EAP_FRAME  {
    UCHAR   Code;                       // 1 = Request, 2 = Response
    UCHAR   Id;
    USHORT  Length;
    UCHAR   Type;                       // 1 = Identity, 0xfe = reserved, used by WSC
}   EAP_FRAME, *PEAP_FRAME;

// WSC fixed information within EAP
typedef struct  _WSC_FRAME  {
    UCHAR   SMI[3];
    ULONG   VendorType;
    UCHAR   OpCode;
    UCHAR   Flags;
}   WSC_FRAME, *PWSC_FRAME;

typedef struct _EXTENSIBLE_AUTH{
    UCHAR   Code;                   //code 4 = Failure
    UCHAR   Id;
    USHORT  Length;
}   EXTENSIBLE_AUTH, *PEXTENSIBLE_AUTH;

// WSC IE structure
typedef struct  _WSC_IE
{
    USHORT  Type;
    USHORT  Length;
    UCHAR   Data[1];    // variable length data
}   WSC_IE, *PWSC_IE;

#pragma pack()

// For WSC state machine states.
// We simplified it
typedef enum    _WscState
{
    WSC_STATE_OFF,
    WSC_STATE_INIT,     
    WSC_STATE_START,
    WSC_STATE_CONN_FAIL,
    WSC_STATE_FAIL, 
    WSC_STATE_LINK_UP,
    WSC_STATE_WAIT_START,
    WSC_STATE_WAIT_M1,
    WSC_STATE_WAIT_M2,
    WSC_STATE_RX_M2D,
    WSC_STATE_WAIT_M3,
    WSC_STATE_WAIT_M4,
    WSC_STATE_WAIT_M5,
    WSC_STATE_WAIT_M6,
    WSC_STATE_WAIT_M7,
    WSC_STATE_WAIT_M8,
    WSC_STATE_WAIT_DONE,
    WSC_STATE_WAIT_EAPFAIL,
    WSC_STATE_WAIT_DISCONN,
    WSC_STATE_CONFIGURED,   
}   WSC_STATE;

// WSC set mode
typedef struct  _WSC_MODE
{
    ULONG               WscMode;        // predefined WSC mode, 1: PIN, 2: PBC
    UCHAR               PIN[8];         // 8 bytes PIN nunmber
    NDIS_802_11_SSID    WscSSID;
    NDIS_802_11_MAC_ADDRESS Bssid;
    UCHAR               Channel;
    UCHAR               WscConfigMode;  // 1: As an Enrollee, 2: As a Registrar, configure a wireless network/AP
                                        // 3: As a Registrar, add a new device via UPnP
}   WSC_MODE, *PWSC_MODE;

typedef struct _WSC_EXTREG_MSG
{
    BOOLEAN         bProbReq;
    UCHAR           EnrolleeMAC[6];
    UCHAR           UserSelEnrMAC[6];    //For PIN
    UCHAR           MsgData[1024];
    USHORT          Length;
} WSC_EXTREG_MSG, *PWSC_EXTREG_MSG;

typedef struct _EXTREG_PBC_ENRO_MAC

{
    USHORT      count;        //the number of MACs that in this struct
    UCHAR       *MACs;      //the point to a buffer of MACs.
} EXTREG_PBC_ENRO_MAC, *PEXTREG_PBC_ENRO_MAC;

//
// Preferred WPS AP type.
//
// a) PREFERRED_WPS_AP_PHY_TYPE_2DOT4_G_FIRST
//     Select 2.4G WPS AP first. Otherwise select 5G WPS AP.
// b) PREFERRED_WPS_AP_PHY_TYPE_5_G_FIRST
//     Select the 5G WPS AP first. Otherwise select the 2.4G WPS AP.
// c) PREFERRED_WPS_AP_PHY_TYPE_AUTO_SELECTION
//     Automactically select WPS AP.
//
typedef enum _PREFERRED_WPS_AP_PHY_TYPE
{
    PREFERRED_WPS_AP_PHY_TYPE_2DOT4_G_FIRST = 0, 
    PREFERRED_WPS_AP_PHY_TYPE_5_G_FIRST, 
    PREFERRED_WPS_AP_PHY_TYPE_AUTO_SELECTION, 
    PREFERRED_WPS_AP_PHY_TYPE_MAXIMUM, 
} PREFERRED_WPS_AP_PHY_TYPE;

// WSC saved message
typedef struct  _WSC_MESSAGE
{
    ULONG   Length;         // Length of saved message
    UCHAR   Data[WSC_MAX_RX_BUFFER];        // Contents
}   WSC_MESSAGE, *PWSC_MESSAGE;

// Data structure to hold Enrollee and Registrar information
typedef struct  _WSC_DEV_INFO
{
    UCHAR   Version;
    UCHAR   Version2;
    UCHAR   Uuid[16];
    UCHAR   MacAddr[6];
    UCHAR   DeviceName[32];
    UCHAR   DeviceNameLen;
    UCHAR   PriDeviceType[8];
    USHORT  AuthTypeFlags;
    USHORT  EncrTypeFlags;
    UCHAR   ConnTypeFlags;
    USHORT  ConfigMethods;
    UCHAR   ScState;
    UCHAR   Manufacturer[64];
    UCHAR   ModelName[32];
    UCHAR   ModelNumber[32];
    UCHAR   SerialNumber[32];
    UCHAR   RfBand;
    ULONG   OsVersion;
    ULONG   FeatureId;
    USHORT  AssocState;
    USHORT  DevPwdId;
    USHORT  ConfigError;
    UCHAR   Ssid[32];
    UCHAR   KeyMgmt[20];
    UCHAR   SelReg; // selected registrar used by AP    
}   WSC_DEV_INFO, *PWSC_DEV_INFO;

// Data structure to setup device information by customers 
typedef struct _WPS_CUSTOMIZE_DEVICE_INFO {
    UCHAR   DeviceName[32];     // Device Name(ASCII) 
    UCHAR   PriDeviceType[8];   // b0~b7: Categoty ID(2 bytes), OUI(4 bytes), Sub Category ID(2 bytes)
    UCHAR   Manufacturer[64];   // Manufacturer(ASCII)
    UCHAR   ModelName[32];      // Model Name(ASCII) 
    UCHAR   ModelNumber[32];    // Model Number     
    UCHAR   SerialNumber[32];   // Serial Number
    UCHAR   Uuid[16];           // Universally Unique Identifier
} WPS_CUSTOMIZE_DEVICE_INFO, *PWPS_CUSTOMIZE_DEVICE_INFO;

// data structure to store info of the instance of Registration protocol
typedef struct  _WSC_REG_DATA
{
    // enrollee endpoint - filled in by the Registrar, NULL for Enrollee
    WSC_DEV_INFO    EnrolleeInfo;        
    // Registrar endpoint - filled in by the Enrollee, NULL for Registrar
    WSC_DEV_INFO    RegistrarInfo;    

    //Diffie Hellman parameters
//  BIGNUM      *DH_PubKey_Peer; //peer's pub key stored in bignum format
//  DH          *DHSecret;       //local key pair in bignum format
    UCHAR       DHRandom[192];  // Saved random byte for public key generation

    
    UCHAR       Pke[192];       //enrollee's raw pub key
    UCHAR       Pkr[192];       //registrar's raw pub key

    UCHAR       SecretKey[192]; // Secret key calculated by enrollee
    
    UCHAR       StaEncrSettings[128];       // to be sent in M2/M8 by reg & M7 by enrollee
    UCHAR       ApEncrSettings[1024];   

    // Device password
    UCHAR       PIN[8];
    UCHAR       PINLen;

    // From KDF Key
    UCHAR       AuthKey[32];
    UCHAR       KeyWrapKey[16];
    UCHAR       Emsk[32];
    
    USHORT      EnrolleePwdId;
    UCHAR       EnrolleeNonce[16];      //N1
    UCHAR       RegistrarNonce[16];     //N2
    UCHAR       M2DRegistrarNonce[16];

    UCHAR       Psk1[16];
    UCHAR       Psk2[16];

    UCHAR       EHash1[32];
    UCHAR       EHash2[32];
    UCHAR       Es1[16];
    UCHAR       Es2[16];

    UCHAR       RHash1[32];
    UCHAR       RHash2[32];
    UCHAR       Rs1[16];
    UCHAR       Rs2[16];
}   WSC_REG_DATA, *PWSC_REG_DATA;

// WSC configured credential
typedef struct  _WSC_CREDENTIAL
{
    NDIS_802_11_SSID    SSID;               // mandatory
    ULONG               AuthType;           // mandatory, 1: open, 2: wpa-psk, 4: shared, 8:wpa, 0x10: wpa2, 0x20: wpa2-psk
    ULONG               EncrType;           // mandatory, 1: none, 2: wep, 4: tkip, 8: aes
    ULONG               KeyIndex;           // optional, default is 1, but the value is 0
    ULONG               KeyLength;      
    UCHAR               Key[64];            // mandatory, Maximum 64 byte
    UCHAR               MacAddr[6];         // mandatory, AP MAC address
    UCHAR               NetIndex;           // optional, 1 byte, defaults to 1
    UCHAR               Rsvd[1];            // Make alignment
//  UCHAR               EapType;            // optional, 8 bytes maximum
//  UCHAR               EapIdLength;        
//  UCHAR               EapId[64];          // optional, Maximum 64 byte
//  ULONG               KeyAuto;            // optional
//  ULONG               Enable1x;           // optional
}   WSC_CREDENTIAL, *PWSC_CREDENTIAL;

// WSC configured profiles
typedef struct  _WSC_PROFILE
{
    ULONG           ProfileCnt;
    WSC_CREDENTIAL  Profile[8];             // Support up to 8 profiles
}   WSC_PROFILE, *PWSC_PROFILE;

//
// Saved Message content for authenticator calculation
//
typedef struct _SAVED_WPS_MESSAGE
{
    WSC_MESSAGE LastTx;
    WSC_MESSAGE LastRx;
    WSC_MESSAGE LastTx2;    // save last two TX WPS messages due to retransmission from the peer.
} SAVED_WPS_MESSAGE, *PSAVED_WPS_MESSAGE;

typedef struct _WSCPBCENROLLEE
{
    BOOLEAN Valid;
    UCHAR   MacAddr[6];
    UCHAR   Version;
    UCHAR   UUID_E[16];
    USHORT  DevicePassID;
    ULONGLONG   ProbReqLastTime;
} WSCPBCENROLLEE, *PWSCPBCENROLLEE;

// WSC control block
typedef struct  _WSC_CTRL
{
    ULONG                                   WscConfMode;    // 0: disable, 1: Enrollee, 2: External Registrar, 
                                                            // 3: External Registrar with device added (UPnP on) 
    ULONG                                   WscMode;        // Saved WscMode predefined WSC mode, 1: PIN, 2: PBC
    ULONG                                   WscStatus;      // predefined WSC connection status
    BOOLEAN                                 bConfiguredAP;  // True: AP is in the configured state. FALSE: others
    BOOLEAN                                 WscEnProfile;       // use wsc profile to do the connection
    BOOLEAN                                 WscEnAssociateIE;   // Add WSC IE on Associate frame.
    BOOLEAN                                 WscEnProbeReqIE;    // Add WSC IE on Probe-Req frame.
    PVOID                                               WscCustomizedIEData;
    ULONG                                               WscCustomizedIESize;
    WSC_STATE                               WscState;
    WSC_PROFILE                             WscProfile;     // Saved WSC profile after M8
    UCHAR                                   WscAPChannel; // Saved the WSC AP channel that the station successfully got the WSC profile from it.
    WSC_PROFILE                             WscM7Profile;   // Saved WSC profile from AP Settings in M7 
    BSS_TABLE                               WscBssTab;      // AP list for the PBC activated
    WSC_REG_DATA                            RegData;        // Registrar pair data
    MTK_TIMER_STRUCT                     WscConnectTimer, WscScanTimer, WscPBCTimer, WscPINTimer, WscEapRxTimer, WscRxTimeOuttoScanTimer; 
    MTK_TIMER_STRUCT                     WscEapM2Timer, WscM8WaitEapFailTimer;
    UCHAR                                   WscAPBssid[MAC_ADDR_LEN]; // Backup the AP's BSSID for reconnection.
    WPS_CUSTOMIZE_DEVICE_INFO               CustomizeDevInfo;
    MTK_TIMER_STRUCT                     WscExtregPBCTimer, WscExtregScanTimer;

    //
    // TRUE:    This scan request operation is used for searching WPS PBC-enabled APs.
    //          In addition, this scan request operation is sent by the miniport driver internally and 
    //          it should not indicate to Vista.
    // FALSE:   Others
    //  
    BOOLEAN                                 bScanForPBCAP;

    ULONG                                   PreferrredWPSAPPhyType; // Preferred WPS AP PHY type. Ref: PREFERRED_WPS_AP_PHY_TYPE

    ULONG           WscLEDMode; // WPS LED mode: LED_WPS_XXX definitions.
    ULONG               WscLastWarningLEDMode; // LED_WPS_ERROR or LED_WPS_SESSION_OVERLAP_DETECTED
    MTK_TIMER_STRUCT WscLEDTimer; // Control the LED feedback pattern after 2 minutes.

    BOOLEAN             bSkipWPSLEDCMD; // Skip the WPS Set LED command when WPS success for 6x9x.
    BOOLEAN             bSkipWPSTurnOffLED; // Skip the WPS turn off LED command.
    BOOLEAN             bSkipWPSTurnOffLEDAfterSuccess; 
    MTK_TIMER_STRUCT WscSkipTurnOffLEDTimer; // Allow the NIC turns off the LED again.
    BOOLEAN             bWPSSession; // WPS session.
    BOOLEAN             bWPSLEDSetTimer;    //Turn on WPS success LED, set 300 sec timer or not.
    BOOLEAN             bWPSMODE9SKIPProgress;


    // The Ralink WPS state machine can work with Ralink UI only.
    // Therefore, when the NIC runs with Windows ZeroConfigure, the HW PBC functionality should be disabled.
    // Note that the Ralink UI makes the RT_OID_802_11_WSC_QUERY_HW_PBC query periodically.
    BOOLEAN         bCheckHWPBC;
    BOOLEAN         bCheckHWPBCCustomerStateMachine;

    // This variable is TRUE after the 120 seconds WPS walk time expiration.
    // Note that in the case of LED mode 9, the error LED should be turned on only after WPS walk time expiration 
    // if the NIC cannot find any WPS PBC-enabled APs in the last scanning result.
    BOOLEAN         bWPSWalkTimeExpiration;
    //Set if it is successful getting WPS profile and link up.
    BOOLEAN         bWPSSuccessandLinkup;

    BOOLEAN         PeerDisconnect;  //When receive AP's deauth or disassoc, or receive no beacons of the AP.
    BOOLEAN         bErrDeteWPSTermination; //Gemtek UI error detection and set OID to turn on WPS error LED.
    BOOLEAN         bSessionOverlap;    //Mark of session overlap, use for Gemtek UI.
    BOOLEAN         bIndicateConnNotTurnOff;

    SAVED_WPS_MESSAGE                       SavedWPSMessage;
    
    BOOLEAN                 bQueueEAPOL;                // Queue EAPOL frame flag.
    PUCHAR                  pRxEAPOL;                   // Pointer to EAPOL frame.
    ULONG                   RxEAPOLLen;                 // Queued EAPOL frame's leng.
    RXWI_STRUC              RxWI;                       // Queued EAPOL frame's RxWI.

    UCHAR           EapId; // EAP Section Token, counting by AP as Registrar

    USHORT              ConfigMethods;

    CHAR                WscPbcEnrCount;
    WSCPBCENROLLEE      WscPbcEnrTab[PBC_ENR_TAB_SIZE];
    
    UCHAR               ExtRegEnrMAC[6];
    UCHAR               WSCRegistrarVersion;
    UCHAR               UseUserSelectEnrollee;  //UI will set this para to 1 if UI supports the list for choosing which enrollee user want to do WPS as ER.
    UCHAR               LastWSCEAPFlags;    //0x01: More fragment. 0x02: Length field. 0x04-0x08: reserved.
    ULONG               MFBufLen;   //The length of last wsc eap fragment.

    BOOLEAN             AsExtreg;

    ULONG               ReconfigAPSelbyUser;
    MLME_QUEUE_ELEM M2ElemForP2P;
    UCHAR               M2IdForP2P;
    ULONG               EapolLastSecquence;

}   WSC_CTRL, *PWSC_CTRL;

#endif  // __WSC_H__

