
// NDIS 6.3 
#define OID_DOT11_WFD_DISCOVER_REQUEST                  0x0E050105	
#define OID_DOT11_WFD_ENUM_DEVICE_LIST                  0x0E050106	
#define OID_DOT11_WFD_SEND_GO_NEGOTIATION_RESPONSE      0x0E05010B	
#define OID_DOT11_WFD_DEVICE_CAPABILITY                 0x0E050101	
#define OID_DOT11_WFD_SEND_INVITATION_RESPONSE          0x0E05010E	
#define OID_DOT11_WFD_LISTEN_STATE_DISCOVERABILITY      0x0E050107	
#define OID_DOT11_WFD_GROUP_OWNER_CAPABILITY            0x0E050102	
#define OID_DOT11_WFD_SEND_PROVISION_DISCOVERY_REQUEST  0x0E05010F	
#define OID_DOT11_WFD_SECONDARY_DEVICE_TYPE_LIST        0x0E050104	
#define OID_DOT11_WFD_GROUP_JOIN_PARAMETERS             0x0E060106	
#define OID_DOT11_WFD_ADDITIONAL_IE                     0x0E050108	
#define OID_DOT11_WFD_CONNECT_TO_GROUP_REQUEST          0x0E060104	
#define OID_DOT11_WFD_GROUP_START_PARAMETERS            0x0E060103	
#define OID_DOT11_WFD_SEND_GO_NEGOTIATION_CONFIRMATION  0x0E05010C	
#define OID_DOT11_WFD_SEND_INVITATION_REQUEST           0x0E05010D	
#define OID_DOT11_WFD_SEND_GO_NEGOTIATION_REQUEST       0x0E05010A	
#define OID_DOT11_WFD_FLUSH_DEVICE_LIST                 0x0E050109	
#define OID_DOT11_WFD_DEVICE_INFO                       0x0E050103	
#define OID_DOT11_WFD_DESIRED_GROUP_ID                  0x0E060101	
#define OID_DOT11_WFD_DISCONNECT_FROM_GROUP_REQUEST     0x0E060105	
#define OID_DOT11_WFD_START_GO_REQUEST                  0x0E060102	
#define OID_DOT11_WFD_SEND_PROVISION_DISCOVERY_RESPONSE 0x0E050110	
#define OID_DOT11_WFD_STOP_DISCOVERY                    0x0E050112	
#define OID_DOT11_WFD_GET_DIALOG_TOKEN                  0x0E050111	
#define OID_DOT11_POWER_MGMT_MODE_AUTO_ENABLED          0x0E070101	
#define OID_DOT11_POWER_MGMT_MODE_STATUS                0x0E070102	
#define OID_DOT11_OFFLOAD_NETWORK_LIST                  0x0E070103	


// From WinDDK\7600.16385.0\inc\api\Windot11.h
#define OID_DOT11_OFFLOAD_CAPABILITY                    0x0D010300
#define OID_DOT11_CURRENT_OFFLOAD_CAPABILITY            0x0D010301
#define OID_DOT11_WEP_OFFLOAD                           0x0D010302
#define OID_DOT11_WEP_UPLOAD                            0x0D010303
#define OID_DOT11_DEFAULT_WEP_OFFLOAD                   0x0D010304
#define OID_DOT11_DEFAULT_WEP_UPLOAD                    0x0D010305
#define OID_DOT11_MPDU_MAX_LENGTH                       0x0D010306
#define OID_DOT11_OPERATION_MODE_CAPABILITY             0x0D010307
#define OID_DOT11_CURRENT_OPERATION_MODE                0x0D010308
#define OID_DOT11_CURRENT_PACKET_FILTER                 0x0D010309
#define OID_DOT11_ATIM_WINDOW                           0x0D01030A
#define OID_DOT11_SCAN_REQUEST                          0x0D01030B
#define OID_DOT11_CURRENT_PHY_TYPE                      0x0D01030C
#define OID_DOT11_JOIN_REQUEST                          0x0D01030D
#define OID_DOT11_START_REQUEST                         0x0D01030E
#define OID_DOT11_UPDATE_IE                             0x0D01030F
#define OID_DOT11_RESET_REQUEST                         0x0D010310
#define OID_DOT11_NIC_POWER_STATE                       0x0D010311
#define OID_DOT11_OPTIONAL_CAPABILITY                   0x0D010312
#define OID_DOT11_CURRENT_OPTIONAL_CAPABILITY           0x0D010313
#define OID_DOT11_STATION_ID                            0x0D010314
#define OID_DOT11_MEDIUM_OCCUPANCY_LIMIT                0x0D010315
#define OID_DOT11_CF_POLLABLE                           0x0D010316
#define OID_DOT11_CFP_PERIOD                            0x0D010317
#define OID_DOT11_CFP_MAX_DURATION                      0x0D010318
#define OID_DOT11_POWER_MGMT_MODE                       0x0D010319
#define OID_DOT11_OPERATIONAL_RATE_SET                  0x0D01031A
#define OID_DOT11_BEACON_PERIOD                         0x0D01031B
#define OID_DOT11_DTIM_PERIOD                           0x0D01031C
#define OID_DOT11_WEP_ICV_ERROR_COUNT                   0x0D01031D
#define OID_DOT11_MAC_ADDRESS                           0x0D01031E
#define OID_DOT11_RTS_THRESHOLD                         0x0D01031F
#define OID_DOT11_SHORT_RETRY_LIMIT                     0x0D010320
#define OID_DOT11_LONG_RETRY_LIMIT                      0x0D010321
#define OID_DOT11_FRAGMENTATION_THRESHOLD               0x0D010322
#define OID_DOT11_MAX_TRANSMIT_MSDU_LIFETIME            0x0D010323
#define OID_DOT11_MAX_RECEIVE_LIFETIME                  0x0D010324
#define OID_DOT11_COUNTERS_ENTRY                        0x0D010325
#define OID_DOT11_SUPPORTED_PHY_TYPES                   0x0D010326
#define OID_DOT11_CURRENT_REG_DOMAIN                    0x0D010327
#define OID_DOT11_TEMP_TYPE                             0x0D010328
#define OID_DOT11_CURRENT_TX_ANTENNA                    0x0D010329
#define OID_DOT11_DIVERSITY_SUPPORT                     0x0D01032A
#define OID_DOT11_CURRENT_RX_ANTENNA                    0x0D01032B
#define OID_DOT11_SUPPORTED_POWER_LEVELS                0x0D01032C
#define OID_DOT11_CURRENT_TX_POWER_LEVEL                0x0D01032D
#define OID_DOT11_HOP_TIME                              0x0D01032E
#define OID_DOT11_CURRENT_CHANNEL_NUMBER                0x0D01032F
#define OID_DOT11_MAX_DWELL_TIME                        0x0D010330
#define OID_DOT11_CURRENT_DWELL_TIME                    0x0D010331
#define OID_DOT11_CURRENT_SET                           0x0D010332
#define OID_DOT11_CURRENT_PATTERN                       0x0D010333
#define OID_DOT11_CURRENT_INDEX                         0x0D010334
#define OID_DOT11_CURRENT_CHANNEL                       0x0D010335
#define OID_DOT11_CCA_MODE_SUPPORTED                    0x0D010336
#define OID_DOT11_CURRENT_CCA_MODE                      0x0D010337
#define OID_DOT11_ED_THRESHOLD                          0x0D010338
#define OID_DOT11_CCA_WATCHDOG_TIMER_MAX                0x0D010339
#define OID_DOT11_CCA_WATCHDOG_COUNT_MAX                0x0D01033A
#define OID_DOT11_CCA_WATCHDOG_TIMER_MIN                0x0D01033B
#define OID_DOT11_CCA_WATCHDOG_COUNT_MIN                0x0D01033C
#define OID_DOT11_REG_DOMAINS_SUPPORT_VALUE             0x0D01033D
#define OID_DOT11_SUPPORTED_TX_ANTENNA                  0x0D01033E
#define OID_DOT11_SUPPORTED_RX_ANTENNA                  0x0D01033F
#define OID_DOT11_DIVERSITY_SELECTION_RX                0x0D010340
#define OID_DOT11_SUPPORTED_DATA_RATES_VALUE            0x0D010341
#define OID_DOT11_CURRENT_FREQUENCY                     0x0D010342
#define OID_DOT11_TI_THRESHOLD                          0x0D010343
#define OID_DOT11_FREQUENCY_BANDS_SUPPORTED             0x0D010344
#define OID_DOT11_SHORT_PREAMBLE_OPTION_IMPLEMENTED     0x0D010345
#define OID_DOT11_PBCC_OPTION_IMPLEMENTED               0x0D010346
#define OID_DOT11_CHANNEL_AGILITY_PRESENT               0x0D010347
#define OID_DOT11_CHANNEL_AGILITY_ENABLED               0x0D010348
#define OID_DOT11_HR_CCA_MODE_SUPPORTED                 0x0D010349
#define OID_DOT11_MULTI_DOMAIN_CAPABILITY_IMPLEMENTED   0x0D01034A
#define OID_DOT11_MULTI_DOMAIN_CAPABILITY_ENABLED       0x0D01034B
#define OID_DOT11_COUNTRY_STRING                        0x0D01034C
#define OID_DOT11_MULTI_DOMAIN_CAPABILITY               0x0D01034D
#define OID_DOT11_EHCC_PRIME_RADIX                      0x0D01034E
#define OID_DOT11_EHCC_NUMBER_OF_CHANNELS_FAMILY_INDEX  0x0D01034F
#define OID_DOT11_EHCC_CAPABILITY_IMPLEMENTED           0x0D010350
#define OID_DOT11_EHCC_CAPABILITY_ENABLED               0x0D010351
#define OID_DOT11_HOP_ALGORITHM_ADOPTED                 0x0D010352
#define OID_DOT11_RANDOM_TABLE_FLAG                     0x0D010353
#define OID_DOT11_NUMBER_OF_HOPPING_SETS                0x0D010354
#define OID_DOT11_HOP_MODULUS                           0x0D010355
#define OID_DOT11_HOP_OFFSET                            0x0D010356
#define OID_DOT11_HOPPING_PATTERN                       0x0D010357
#define OID_DOT11_RANDOM_TABLE_FIELD_NUMBER             0x0D010358
#define OID_DOT11_WPA_TSC                               0x0D010359
#define OID_DOT11_RSSI_RANGE                            0x0D01035A
#define OID_DOT11_RF_USAGE                              0x0D01035B
#define OID_DOT11_NIC_SPECIFIC_EXTENSION                0x0D01035C
#define OID_DOT11_AP_JOIN_REQUEST                       0x0D01035D
#define OID_DOT11_ERP_PBCC_OPTION_IMPLEMENTED           0x0D01035E
#define OID_DOT11_ERP_PBCC_OPTION_ENABLED               0x0D01035F
#define OID_DOT11_DSSS_OFDM_OPTION_IMPLEMENTED          0x0D010360
#define OID_DOT11_DSSS_OFDM_OPTION_ENABLED              0x0D010361
#define OID_DOT11_SHORT_SLOT_TIME_OPTION_IMPLEMENTED    0x0D010362
#define OID_DOT11_SHORT_SLOT_TIME_OPTION_ENABLED        0x0D010363
#define OID_DOT11_MAX_MAC_ADDRESS_STATES                0x0D010364
#define OID_DOT11_RECV_SENSITIVITY_LIST                 0x0D010365
#define OID_DOT11_WME_IMPLEMENTED                       0x0D010366
#define OID_DOT11_WME_ENABLED                           0x0D010367
#define OID_DOT11_WME_AC_PARAMETERS                     0x0D010368
#define OID_DOT11_WME_UPDATE_IE                         0x0D010369
#define OID_DOT11_QOS_TX_QUEUES_SUPPORTED               0x0D01036A
#define OID_DOT11_QOS_TX_DURATION                       0x0D01036B
#define OID_DOT11_QOS_TX_MEDIUM_TIME                    0x0D01036C
#define OID_DOT11_SUPPORTED_OFDM_FREQUENCY_LIST         0x0D01036D
#define OID_DOT11_SUPPORTED_DSSS_CHANNEL_LIST           0x0D01036E

#define OID_DOT11_CURRENT_ADDRESS                       0x0D010702
#define OID_DOT11_PERMANENT_ADDRESS                     0x0D010703
#define OID_DOT11_MULTICAST_LIST                        0x0D010704
#define OID_DOT11_MAXIMUM_LIST_SIZE                     0x0D010705

#define	OID_DOT11_AUTO_CONFIG_ENABLED                   0x0E010178
#define	OID_DOT11_ENUM_BSS_LIST                         0x0E010179
#define	OID_DOT11_FLUSH_BSS_LIST                        0x0E01017A
#define	OID_DOT11_POWER_MGMT_REQUEST                    0x0E01017B
#define	OID_DOT11_DESIRED_SSID_LIST                     0x0E01017C
#define	OID_DOT11_EXCLUDED_MAC_ADDRESS_LIST             0x0E01017D
#define	OID_DOT11_DESIRED_BSSID_LIST                    0x0E01017E
#define	OID_DOT11_DESIRED_BSS_TYPE                      0x0E01017F
#define	OID_DOT11_PMKID_LIST                            0x0E010280
#define	OID_DOT11_CONNECT_REQUEST                       0x0E010181
#define	OID_DOT11_EXCLUDE_UNENCRYPTED                   0x0E010182
#define	OID_DOT11_STATISTICS                            0x0E020183
#define	OID_DOT11_PRIVACY_EXEMPTION_LIST                0x0E010184
#define	OID_DOT11_ENABLED_AUTHENTICATION_ALGORITHM      0x0E010185
#define	OID_DOT11_SUPPORTED_UNICAST_ALGORITHM_PAIR      0x0E010186
#define	OID_DOT11_ENABLED_UNICAST_CIPHER_ALGORITHM      0x0E010187
#define	OID_DOT11_SUPPORTED_MULTICAST_ALGORITHM_PAIR    0x0E010188
#define	OID_DOT11_ENABLED_MULTICAST_CIPHER_ALGORITHM    0x0E010189
#define	OID_DOT11_CIPHER_DEFAULT_KEY_ID                 0x0E01018A
#define	OID_DOT11_CIPHER_DEFAULT_KEY                    0x0E01018B
#define	OID_DOT11_CIPHER_KEY_MAPPING_KEY                0x0E01018C
#define	OID_DOT11_ENUM_ASSOCIATION_INFO                 0x0E01018D
#define	OID_DOT11_DISCONNECT_REQUEST                    0x0E01018E
#define	OID_DOT11_UNICAST_USE_GROUP_ENABLED             0x0E01018F
#define	OID_DOT11_HARDWARE_PHY_STATE                    0x0E010190
#define	OID_DOT11_DESIRED_PHY_LIST                      0x0E010191
#define	OID_DOT11_CURRENT_PHY_ID                        0x0E010192
#define	OID_DOT11_MEDIA_STREAMING_ENABLED               0x0E010193
#define	OID_DOT11_UNREACHABLE_DETECTION_THRESHOLD       0x0E010194
#define	OID_DOT11_ACTIVE_PHY_LIST                       0x0E010195
#define	OID_DOT11_EXTSTA_CAPABILITY                     0x0E010196
#define	OID_DOT11_DATA_RATE_MAPPING_TABLE               0x0E010197
#define	OID_DOT11_SUPPORTED_COUNTRY_OR_REGION_STRING    0x0E010198
#define	OID_DOT11_DESIRED_COUNTRY_OR_REGION_STRING      0x0E010199
#define	OID_DOT11_PORT_STATE_NOTIFICATION               0x0E01029A
#define	OID_DOT11_IBSS_PARAMS                           0x0E01019B
#define	OID_DOT11_QOS_PARAMS                            0x0E01029C
#define	OID_DOT11_SAFE_MODE_ENABLED                     0x0E01019D
#define	OID_DOT11_HIDDEN_NETWORK_ENABLED                0x0E01019E
#define	OID_DOT11_ASSOCIATION_PARAMS                    0x0E01019F
#define	OID_DOT11_WPS_ENABLED                           0x0E030101
#define	OID_DOT11_START_AP_REQUEST                      0x0E030102
#define	OID_DOT11_AVAILABLE_CHANNEL_LIST                0x0E030103
#define	OID_DOT11_AVAILABLE_FREQUENCY_LIST              0x0E030104
#define	OID_DOT11_DISASSOCIATE_PEER_REQUEST             0x0E030105
#define	OID_DOT11_INCOMING_ASSOCIATION_DECISION         0x0E030106
#define	OID_DOT11_ADDITIONAL_IE                         0x0E030107
#define	OID_DOT11_ENUM_PEER_INFO                        0x0E030108
#define	OID_DOT11_CREATE_MAC                            0x0E040101
#define	OID_DOT11_DELETE_MAC                            0x0E040102
#define	OID_DOT11_PREFERRED_MAC                         0x0E040103
#define	OID_DOT11_VIRTUAL_STATION_CAPABILITY            0x0E040204


// From WinDDK\7600.16385.0\inc\api\Ntddndis.h
//
// General Objects
//

//
//  Required OIDs
//
#define OID_GEN_SUPPORTED_LIST                  0x00010101
#define OID_GEN_HARDWARE_STATUS                 0x00010102
#define OID_GEN_MEDIA_SUPPORTED                 0x00010103
#define OID_GEN_MEDIA_IN_USE                    0x00010104
#define OID_GEN_MAXIMUM_LOOKAHEAD               0x00010105
#define OID_GEN_MAXIMUM_FRAME_SIZE              0x00010106
#define OID_GEN_LINK_SPEED                      0x00010107
#define OID_GEN_TRANSMIT_BUFFER_SPACE           0x00010108
#define OID_GEN_RECEIVE_BUFFER_SPACE            0x00010109
#define OID_GEN_TRANSMIT_BLOCK_SIZE             0x0001010A
#define OID_GEN_RECEIVE_BLOCK_SIZE              0x0001010B
#define OID_GEN_VENDOR_ID                       0x0001010C
#define OID_GEN_VENDOR_DESCRIPTION              0x0001010D
#define OID_GEN_CURRENT_PACKET_FILTER           0x0001010E
#define OID_GEN_CURRENT_LOOKAHEAD               0x0001010F
#define OID_GEN_DRIVER_VERSION                  0x00010110
#define OID_GEN_MAXIMUM_TOTAL_SIZE              0x00010111
#define OID_GEN_PROTOCOL_OPTIONS                0x00010112
#define OID_GEN_MAC_OPTIONS                     0x00010113
#define OID_GEN_MEDIA_CONNECT_STATUS            0x00010114
#define OID_GEN_MAXIMUM_SEND_PACKETS            0x00010115

//
//  Optional OIDs
//
#define OID_GEN_VENDOR_DRIVER_VERSION           0x00010116
#define OID_GEN_SUPPORTED_GUIDS                 0x00010117
#define OID_GEN_NETWORK_LAYER_ADDRESSES         0x00010118  // Set only
#define OID_GEN_TRANSPORT_HEADER_OFFSET         0x00010119  // Set only
#define OID_GEN_MEDIA_CAPABILITIES              0x00010201
#define OID_GEN_PHYSICAL_MEDIUM                 0x00010202

//
// new optional for NDIS 6.0
//
#define OID_GEN_RECEIVE_SCALE_CAPABILITIES      0x00010203  // query only
#define OID_GEN_RECEIVE_SCALE_PARAMETERS        0x00010204  // query and set

//
// new for NDIS 6.0. NDIS will handle on behalf of the miniports
//
#define OID_GEN_MAC_ADDRESS                     0x00010205  // query and set
#define OID_GEN_MAX_LINK_SPEED                  0x00010206  // query only
#define OID_GEN_LINK_STATE                      0x00010207  // query only

//
// new and required for NDIS 6 miniports
//
#define OID_GEN_LINK_PARAMETERS                 0x00010208  // set only
#define OID_GEN_INTERRUPT_MODERATION            0x00010209  // query and set
#define OID_GEN_NDIS_RESERVED_3                 0x0001020A
#define OID_GEN_NDIS_RESERVED_4                 0x0001020B
#define OID_GEN_NDIS_RESERVED_5                 0x0001020C


//
// Port related OIDs
//
#define OID_GEN_ENUMERATE_PORTS                 0x0001020D  // query only, handled by NDIS
#define OID_GEN_PORT_STATE                      0x0001020E  // query only, handled by NDIS
#define OID_GEN_PORT_AUTHENTICATION_PARAMETERS  0x0001020F  // Set only

//
// optional OID for NDIS 6 miniports
//
#define OID_GEN_TIMEOUT_DPC_REQUEST_CAPABILITIES 0x00010210 // query only

//
// this OID is handled by NDIS for PCI devices
//
#define OID_GEN_PCI_DEVICE_CUSTOM_PROPERTIES    0x00010211  // query only
#define OID_GEN_NDIS_RESERVED_6                 0x00010212
#define OID_GEN_PHYSICAL_MEDIUM_EX              0x00010213  // query only


#endif // (NTDDI_VERSION >= NTDDI_VISTA)

#define OID_GEN_MACHINE_NAME                    0x0001021A  // set only
#define OID_GEN_RNDIS_CONFIG_PARAMETER          0x0001021B  // Set only
#define OID_GEN_VLAN_ID                         0x0001021C

#define OID_GEN_RECEIVE_HASH                    0x0001021F  // query and set
#define OID_GEN_MINIPORT_RESTART_ATTRIBUTES     0x0001021D

//
// Optional OID for header data/split
//
#define OID_GEN_HD_SPLIT_PARAMETERS             0x0001021E  // Set only
#define OID_GEN_HD_SPLIT_CURRENT_CONFIG         0x00010220  // Query only

//
// the following OIDs are used in querying interfaces
//
#define OID_GEN_PROMISCUOUS_MODE                0x00010280  // used in querying interfaces
#define OID_GEN_LAST_CHANGE                     0x00010281  // used in querying interfaces
#define OID_GEN_DISCONTINUITY_TIME              0x00010282  // used in querying interfaces
#define OID_GEN_OPERATIONAL_STATUS              0x00010283  // used in querying interfaces
#define OID_GEN_XMIT_LINK_SPEED                 0x00010284  // used in querying interfaces
#define OID_GEN_RCV_LINK_SPEED                  0x00010285  // used in querying interfaces
#define OID_GEN_UNKNOWN_PROTOS                  0x00010286  // used in querying interfaces
#define OID_GEN_INTERFACE_INFO                  0x00010287  // used in querying interfaces
#define OID_GEN_ADMIN_STATUS                    0x00010288  // used in querying interfaces
#define OID_GEN_ALIAS                           0x00010289  // used in querying interfaces
#define OID_GEN_MEDIA_CONNECT_STATUS_EX         0x0001028A  // used in querying interfaces
#define OID_GEN_LINK_SPEED_EX                   0x0001028B  // used in querying interfaces
#define OID_GEN_MEDIA_DUPLEX_STATE              0x0001028C  // used in querying interfaces
#define OID_GEN_IP_OPER_STATUS                  0x0001028D  // used in querying interfaces


//
//  Required statistics
//
#define OID_GEN_XMIT_OK                         0x00020101
#define OID_GEN_RCV_OK                          0x00020102
#define OID_GEN_XMIT_ERROR                      0x00020103
#define OID_GEN_RCV_ERROR                       0x00020104
#define OID_GEN_RCV_NO_BUFFER                   0x00020105

//
// mandatory for NDIS 6.0 and higher miniports
//
#define OID_GEN_STATISTICS                      0x00020106


//
//  Optional statistics
//
#define OID_GEN_DIRECTED_BYTES_XMIT             0x00020201
#define OID_GEN_DIRECTED_FRAMES_XMIT            0x00020202
#define OID_GEN_MULTICAST_BYTES_XMIT            0x00020203
#define OID_GEN_MULTICAST_FRAMES_XMIT           0x00020204
#define OID_GEN_BROADCAST_BYTES_XMIT            0x00020205
#define OID_GEN_BROADCAST_FRAMES_XMIT           0x00020206
#define OID_GEN_DIRECTED_BYTES_RCV              0x00020207
#define OID_GEN_DIRECTED_FRAMES_RCV             0x00020208
#define OID_GEN_MULTICAST_BYTES_RCV             0x00020209
#define OID_GEN_MULTICAST_FRAMES_RCV            0x0002020A
#define OID_GEN_BROADCAST_BYTES_RCV             0x0002020B
#define OID_GEN_BROADCAST_FRAMES_RCV            0x0002020C
#define OID_GEN_RCV_CRC_ERROR                   0x0002020D
#define OID_GEN_TRANSMIT_QUEUE_LENGTH           0x0002020E

#define OID_GEN_GET_TIME_CAPS                   0x0002020F
#define OID_GEN_GET_NETCARD_TIME                0x00020210
#define OID_GEN_NETCARD_LOAD                    0x00020211
#define OID_GEN_DEVICE_PROFILE                  0x00020212

//
// The following are exported by NDIS itself and are only queryable. 
//

//
// the time in milliseconds a driver took to initialize.
//
#define OID_GEN_INIT_TIME_MS                    0x00020213

//
// the number of times the miniport adapter was reset
//
#define OID_GEN_RESET_COUNTS                    0x00020214

//
// the number of timer the miniport reported a media state change
//
#define OID_GEN_MEDIA_SENSE_COUNTS              0x00020215

//
// the friendly name of the adapter
//
#define OID_GEN_FRIENDLY_NAME                   0x00020216

//
// returns miniport information such as whether the driver is serialized or not
// if it supports sending multiple packets, etc. refer to NDIS_MINIPORT_XXX flags
//
#define OID_GEN_NDIS_RESERVED_1                 0x00020217

//
// handled by NDIS to reset the test verification paramters on the
// miniport
//
#define OID_GEN_NDIS_RESERVED_2                 0x00020218
#define OID_GEN_NDIS_RESERVED_5                 0x0001020C

//
// more optional OIDs new for NDIS 6.0
//
#define OID_GEN_BYTES_RCV                       0x00020219
#define OID_GEN_BYTES_XMIT                      0x0002021A
#define OID_GEN_RCV_DISCARDS                    0x0002021B
#define OID_GEN_XMIT_DISCARDS                   0x0002021C

//
// 802.3 Objects (Ethernet)
//
#define OID_802_3_PERMANENT_ADDRESS             0x01010101
#define OID_802_3_CURRENT_ADDRESS               0x01010102
#define OID_802_3_MULTICAST_LIST                0x01010103
#define OID_802_3_MAXIMUM_LIST_SIZE             0x01010104
//
// This OID has been deprecated for NDIS 6 drivers. 
//
#define OID_802_3_MAC_OPTIONS                   0x01010105  // deprecated

#define OID_802_3_RCV_ERROR_ALIGNMENT           0x01020101
#define OID_802_3_XMIT_ONE_COLLISION            0x01020102
#define OID_802_3_XMIT_MORE_COLLISIONS          0x01020103

#define OID_802_3_XMIT_DEFERRED                 0x01020201
#define OID_802_3_XMIT_MAX_COLLISIONS           0x01020202
#define OID_802_3_RCV_OVERRUN                   0x01020203
#define OID_802_3_XMIT_UNDERRUN                 0x01020204
#define OID_802_3_XMIT_HEARTBEAT_FAILURE        0x01020205
#define OID_802_3_XMIT_TIMES_CRS_LOST           0x01020206
#define OID_802_3_XMIT_LATE_COLLISIONS          0x01020207

//
// new for NDIS 6
//
#define OID_802_3_ADD_MULTICAST_ADDRESS         0x01010208
#define OID_802_3_DELETE_MULTICAST_ADDRESS      0x01010209


//
// IEEE 802.11 OIDs
//
#define OID_802_11_BSSID                        0x0D010101
#define OID_802_11_SSID                         0x0D010102
#define OID_802_11_NETWORK_TYPES_SUPPORTED      0x0D010203
#define OID_802_11_NETWORK_TYPE_IN_USE          0x0D010204
#define OID_802_11_TX_POWER_LEVEL               0x0D010205
#define OID_802_11_RSSI                         0x0D010206
#define OID_802_11_RSSI_TRIGGER                 0x0D010207
#define OID_802_11_INFRASTRUCTURE_MODE          0x0D010108
#define OID_802_11_FRAGMENTATION_THRESHOLD      0x0D010209
#define OID_802_11_RTS_THRESHOLD                0x0D01020A
#define OID_802_11_NUMBER_OF_ANTENNAS           0x0D01020B
#define OID_802_11_RX_ANTENNA_SELECTED          0x0D01020C
#define OID_802_11_TX_ANTENNA_SELECTED          0x0D01020D
#define OID_802_11_SUPPORTED_RATES              0x0D01020E
#define OID_802_11_DESIRED_RATES                0x0D010210
#define OID_802_11_CONFIGURATION                0x0D010211
#define OID_802_11_STATISTICS                   0x0D020212
#define OID_802_11_ADD_WEP                      0x0D010113
#define OID_802_11_REMOVE_WEP                   0x0D010114
#define OID_802_11_DISASSOCIATE                 0x0D010115
#define OID_802_11_POWER_MODE                   0x0D010216
#define OID_802_11_BSSID_LIST                   0x0D010217
#define OID_802_11_AUTHENTICATION_MODE          0x0D010118
#define OID_802_11_PRIVACY_FILTER               0x0D010119
#define OID_802_11_BSSID_LIST_SCAN              0x0D01011A
#define OID_802_11_WEP_STATUS                   0x0D01011B
// Renamed to reflect better the extended set of encryption status
#define OID_802_11_ENCRYPTION_STATUS            0x0D01011B
#define OID_802_11_RELOAD_DEFAULTS              0x0D01011C
// Added to allow key mapping and default keys
#define OID_802_11_ADD_KEY                      0x0D01011D
#define OID_802_11_REMOVE_KEY                   0x0D01011E
#define OID_802_11_ASSOCIATION_INFORMATION      0x0D01011F
#define OID_802_11_TEST                         0x0D010120
#define OID_802_11_MEDIA_STREAM_MODE            0x0D010121
#define OID_802_11_CAPABILITY                   0x0D010122
#define OID_802_11_PMKID                        0x0D010123
#define OID_802_11_NON_BCAST_SSID_LIST          0x0D010124
#define OID_802_11_RADIO_STATUS                 0x0D010125

//
//  PnP and PM OIDs
//
#define OID_PNP_CAPABILITIES                    0xFD010100
#define OID_PNP_SET_POWER                       0xFD010101
#define OID_PNP_QUERY_POWER                     0xFD010102
#define OID_PNP_ADD_WAKE_UP_PATTERN             0xFD010103
#define OID_PNP_REMOVE_WAKE_UP_PATTERN          0xFD010104
#define OID_PNP_WAKE_UP_PATTERN_LIST            0xFD010105
#define OID_PNP_ENABLE_WAKE_UP                  0xFD010106

//
//  PnP/PM Statistics (Optional).
//
#define OID_PNP_WAKE_UP_OK                      0xFD020200
#define OID_PNP_WAKE_UP_ERROR                   0xFD020201

//
// new power management OIDs for NDIS 6.20 drivers
//
#define OID_PM_CURRENT_CAPABILITIES             0xFD010107
#define OID_PM_HARDWARE_CAPABILITIES            0xFD010108
#define OID_PM_PARAMETERS                       0xFD010109
#define OID_PM_ADD_WOL_PATTERN                  0xFD01010A
#define OID_PM_REMOVE_WOL_PATTERN               0xFD01010B
#define OID_PM_WOL_PATTERN_LIST                 0xFD01010C
#define OID_PM_ADD_PROTOCOL_OFFLOAD             0xFD01010D 
#define OID_PM_GET_PROTOCOL_OFFLOAD             0xFD01010E 
#define OID_PM_REMOVE_PROTOCOL_OFFLOAD          0xFD01010F
#define OID_PM_PROTOCOL_OFFLOAD_LIST            0xFD010110


//
// MTK OID
//
#define MTK_OID_N5_QUERY_STBC_RX_COUNT                             0x0D7101BD
#define MTK_OID_N5_QUERY_STBC_TX_COUNT                             0x0D7101BE
#define MTK_OID_N5_QUERY_SS_NUM                                    0x0D7101BF
#define MTK_OID_N5_SET_SS_NUM                                      0x0DF101BF
#define MTK_OID_N5_SET_BT_HCI_SEND_CMD                             0x0DF101C2
#define MTK_OID_N5_SET_WSC_GETPROFILE_COMPLETE                     0x0DF101C2
#define MTK_OID_N5_SET_BT_HCI_SEND_ACL_DATA                        0x0DF101C3
#define MTK_OID_N5_SET_TXBF_ETX_BF_TIMEOUT                         0x0DF10206
#define MTK_OID_N5_SET_TXBF_ETX_BF_EN                              0x0DF1020A
#define MTK_OID_N5_SET_TXBF_ETX_BF_NON_COMPRESSED_FEEDBACK         0x0DF1020B
#define MTK_OID_N6_QUERY_VENDOR                                    0xFF0C0001
#define MTK_OID_N6_QUERY_PHY_MODE                                  0xFF0C0002
#define MTK_OID_N6_QUERY_COUNTRY_REGION                            0xFF0C0003
#define MTK_OID_N6_QUERY_EEPROM_VERSION                            0xFF0C0004
#define MTK_OID_N6_QUERY_FIRMWARE_VERSION                          0xFF0C0005
#define MTK_OID_N6_QUERY_CURRENT_CHANNEL_ID                        0xFF0C0006
#define MTK_OID_N6_QUERY_TX_POWER_LEVEL_1                          0xFF0C0007
#define MTK_OID_N6_QUERY_HARDWARE_REGISTER                         0xFF0C0008
#define MTK_OID_N6_QUERY_LINK_STATUS                               0xFF0C0009
#define MTK_OID_N6_QUERY_NOISE_LEVEL                               0xFF0C000A
#define MTK_OID_N6_QUERY_STATISTICS                                0xFF0C000B
#define MTK_OID_N6_QUERY_EXTRA_INFO                                0xFF0C000D
#define MTK_OID_N6_QUERY_LAST_RX_RATE                              0xFF0C000E
#define MTK_OID_N6_QUERY_RSSI_0                                    0xFF0C000F
#define MTK_OID_N6_QUERY_RSSI_1                                    0xFF0C0010
#define MTK_OID_N6_QUERY_OP_MODE                                   0xFF0C001A
#define MTK_OID_N6_QUERY_LAST_TX_RATE                              0xFF0C0021
#define MTK_OID_N6_QUERY_RSSI_2                                    0xFF0C0022
#define MTK_OID_N6_QUERY_SNR_0                                     0xFF0C0023
#define MTK_OID_N6_QUERY_SNR_1                                     0xFF0C0024
#define MTK_OID_N6_QUERY_BATABLE                                   0xFF0C0025
#define MTK_OID_N6_QUERY_IMME_BA_CAP                               0xFF0C0026
#define MTK_OID_N6_QUERY_HT_PHYMODE                                0xFF0C0028
#define MTK_OID_N6_QUERY_MAC_TABLE                                 0xFF0C0029
#define MTK_OID_N6_QUERY_WSC_STATUS                                0xFF0C002D
#define MTK_OID_N6_QUERY_WSC_PROFILE                               0xFF0C002E
#define MTK_OID_N6_QUERY_WSC_HW_PBC                                0xFF0C002F
#define MTK_OID_N6_QUERY_AUTHENTICATION_MODE                       0xFF0C0035
#define MTK_OID_N6_QUERY_ENCRYPTION_STATUS                         0xFF0C0036
#define MTK_OID_N6_QUERY_CONFIGURATION                             0xFF0C003B
#define MTK_OID_N6_QUERY_AP_CONFIG                                 0xFF0C0040
#define MTK_OID_N6_QUERY_WPA_REKEY                                 0xFF0C0041
#define MTK_OID_N6_QUERY_EVENT_TABLE                               0xFF0C0043
#define MTK_OID_N6_SET_WLAN_WAPIMODE                               0xFF300003
#define MTK_OID_N6_SET_WAI_RESULT_STATUS                           0xFF300005
#define MTK_OID_N6_SET_WPI_KEY                                     0xFF30000A
#define MTK_OID_N6_QUERY_PREAMBLE                                  0xFF710101
#define MTK_OID_N6_QUERY_STA_CONFIG                                0xFF710111
#define MTK_OID_N6_QUERY_APSD_SETTING                              0xFF71012A
#define MTK_OID_N6_QUERY_APSD_PSM                                  0xFF71012B
#define MTK_OID_N6_QUERY_WMM                                       0xFF71012D
#define MTK_OID_N6_QUERY_WAPI_SUPPORT                              0xFF71012F
#define MTK_OID_N6_QUERY_DLS                                       0xFF710131
#define MTK_OID_N6_QUERY_DLS_PARAM                                 0xFF710132
#define MTK_OID_N6_QUERY_SUPPRESS_SCAN                             0xFF71013E
#define MTK_OID_N6_QUERY_MFP_CONTROL                               0xFF71013F
#define MTK_OID_N6_QUERY_PREFERRED_WPS_AP_PHY_TYPE                 0xFF710151
#define MTK_OID_N6_QUERY_WSC_AP_CHANNEL                            0xFF710152
#define MTK_OID_N6_QUERY_USB_VERSION                               0xFF710155
#define MTK_OID_N6_QUERY_EEPROM_BANDWIDTH                          0xFF710165
#define MTK_OID_N6_QUERY_INITIALIZATION_STATE                      0xFF710181
#define MTK_OID_N6_QUERY_MANUAL_CH_LIST                            0xFF7101BC
#define MTK_OID_N6_QUERY_SS_NUM                                    0xFF7101BF
#define MTK_OID_N6_QUERY_SNR_2                                     0xFF7101C0
#define MTK_OID_N6_QUERY_EXTREG_EVENT_MESSAGE                      0xFF7101C4
#define MTK_OID_N6_QUERY_P2P_STATUS                                0xFF7101D1
#define MTK_OID_N6_QUERY_WIFI_DIRECT_LINKSTATE                     0xFF7101D7
#define MTK_OID_N6_QUERY_P2P_PERSIST                               0xFF7101DA
#define MTK_OID_N6_QUERY_PORT_USAGE                                0xFF7101DF
#define MTK_OID_N6_QUERY_BALD_EAGLE_SSID                           0xFF7101E1
#define MTK_OID_N6_QUERY_WSC_VERSION                               0xFF7101E2
#define MTK_OID_N6_QUERY_WSC_EXTREG_PBC_ENRO_MAC                   0xFF7101E3
#define MTK_OID_N6_QUERY_WSC20_TESTBED                             0xFF7101E7
#define MTK_OID_N6_QUERY_WSC_SSID                                  0xFF7101E8
#define MTK_OID_N6_QUERY_WSC_CUSTOMIZED_IE_PARAM                   0xFF7101EA
#define MTK_OID_N6_QUERY_SIGMA_P2P_STATUS                          0xFF710221
#define MTK_OID_N6_QUERY_SIGMA_WPS_PINCODE                         0xFF710222
#define MTK_OID_N6_QUERY_SIGMA_PSK                                 0xFF710223
#define MTK_OID_N6_QUERY_HW_CAPABILITIES                           0xFF710335
#define MTK_OID_N6_QUERY_P2P_MAC_INFO                              0xFF710351
#define MTK_OID_N6_QUERY_WTBL_INFO                                 0xFF710501
#define MTK_OID_N6_QUERY_MIB                                       0xFF710504
#define MTK_OID_N6_QUERY_HW_ARCHITECTURE_VERSION                   0xFF7C0050
#define MTK_OID_N6_SET_FLUSH_BSS_LIST                              0xFF81017A
#define MTK_OID_N6_SET_POWER_MGMT_REQUEST                          0xFF81017B
#define MTK_OID_N6_SET_DESIRED_BSSID_LIST                          0xFF81017E
#define MTK_OID_N6_SET_CIPHER_DEFAULT_KEY                          0xFF81018B
#define MTK_OID_N6_SET_CIPHER_KEY_MAPPING_KEY                      0xFF81018C
#define MTK_OID_N6_SET_DISCONNECT_REQUEST                          0xFF81018E
#define MTK_OID_N6_SET_RTS_THRESHOLD                               0xFF81031F
#define MTK_OID_N6_SET_FRAGMENTATION_THRESHOLD                     0xFF810322
#define MTK_OID_N6_SET_PHY_MODE                                    0xFF8C0002
#define MTK_OID_N6_SET_COUNTRY_REGION                              0xFF8C0003
#define MTK_OID_N6_SET_TX_POWER_LEVEL_1                            0xFF8C0007
#define MTK_OID_N6_SET_HARDWARE_REGISTER                           0xFF8C0008
#define MTK_OID_N6_SET_RESET_COUNTERS                              0xFF8C000C
#define MTK_OID_N6_SET_OP_MODE                                     0xFF8C001A
#define MTK_OID_N6_SET_ADD_IMME_BA                                 0xFF8C0025
#define MTK_OID_N6_SET_IMME_BA_CAP                                 0xFF8C0026
#define MTK_OID_N6_SET_TEAR_IMME_BA                                0xFF8C0027
#define MTK_OID_N6_SET_HT_PHYMODE                                  0xFF8C0028
#define MTK_OID_N6_SET_WSC_MODE                                    0xFF8C002A
#define MTK_OID_N6_SET_WSC_ASSOCIATE_IE                            0xFF8C002B
#define MTK_OID_N6_SET_WSC_PROBEREQ_IE                             0xFF8C002C
#define MTK_OID_N6_SET_AUTHENTICATION_MODE                         0xFF8C0035
#define MTK_OID_N6_SET_ENCRYPTION_STATUS                           0xFF8C0036
#define MTK_OID_N6_SET_ADD_KEY                                     0xFF8C0038
#define MTK_OID_N6_SET_REMOVE_KEY                                  0xFF8C0039
#define MTK_OID_N6_SET_CONFIGURATION                               0xFF8C003B
#define MTK_OID_N6_SET_AP_CONFIG                                   0xFF8C0040
#define MTK_OID_N6_SET_WPA_REKEY                                   0xFF8C0041
#define MTK_OID_N6_SET_ACL                                         0xFF8C0046
#define MTK_OID_N6_SET_WSC_PROFILE                                 0xFF8C0049
#define MTK_OID_N6_SET_WSC_ACTIVE_PROFILE                          0xFF8C004A
#define MTK_OID_N6_SET_WSC_CUSTOMIZE_DEVINFO                       0xFF8C004B
#define MTK_OID_N6_SET_WSC_PIN_CODE                                0xFF8C004D
#define MTK_OID_N6_SET_P2P_DISCONNECT_PER_CONNECTION               0xFF8C004E
#define MTK_OID_N6_SET_USB_CYCLE_PORT                              0xFF8C0060
#define MTK_OID_N6_SET_PREAMBLE                                    0xFFF10101
#define MTK_OID_N6_SET_STA_CONFIG                                  0xFFF10111
#define MTK_OID_N6_SET_CCX20_INFO                                  0xFFF1011E
#define MTK_OID_N6_SET_APSD_SETTING                                0xFFF1012A
#define MTK_OID_N6_SET_APSD_PSM                                    0xFFF1012B
#define MTK_OID_N6_SET_WMM                                         0xFFF1012D
#define MTK_OID_N6_SET_WAPI_SUPPORT                                0xFFF1012F
#define MTK_OID_N6_SET_DLS                                         0xFFF10131
#define MTK_OID_N6_SET_DLS_PARAM                                   0xFFF10132
#define MTK_OID_N6_SET_SUPPRESS_SCAN                               0xFFF1013E
#define MTK_OID_N6_SET_MFP_CONTROL                                 0xFFF1013F
#define MTK_OID_N6_SET_PREFERRED_WPS_AP_PHY_TYPE                   0xFFF10151
#define MTK_OID_N6_SET_WSC_WPS_STATE_MACHINE_TERMINATION           0xFFF101A0
#define MTK_OID_N6_SET_LED_WPS_MODE10                              0xFFF101A1
#define MTK_OID_N6_SET_WSC_HW_PBC_CUSTOMER_VENDOR_STATE_MACHINE    0xFFF101A4
#define MTK_OID_N6_SET_SS_NUM                                      0xFFF101BF
#define MTK_OID_N6_SET_WSC_EXTREG_MESSSAGE                         0xFFF101C3
#define MTK_OID_N6_SET_P2P_EVENT                                   0xFFF101D3
#define MTK_OID_N6_SET_P2P_CONFIG                                  0xFFF101D4
#define MTK_OID_N6_SET_P2P_IP                                      0xFFF101D5
#define MTK_OID_N6_SET_P2P_EVENT_READY                             0xFFF101D6
#define MTK_OID_N6_SET_P2P_PERST_TAB                               0xFFF101D8
#define MTK_OID_N6_SET_P2P_GO_PASSPHRASE                           0xFFF101D9
#define MTK_OID_N6_SET_P2P_PERSIST                                 0xFFF101DA
#define MTK_OID_N6_SET_BALD_EAGLE_SSID                             0xFFF101E1
#define MTK_OID_N6_SET_WSC_VERSION2                                0xFFF101E2
#define MTK_OID_N6_SET_USE_SEL_ENRO_MAC                            0xFFF101E5
#define MTK_OID_N6_SET_RECONFIG_AP                                 0xFFF101E6
#define MTK_OID_N6_SET_WSC20_TESTBED                               0xFFF101E7
#define MTK_OID_N6_SET_WSC_CUSTOMIZED_IE_PARAM                     0xFFF101EA
#define MTK_OID_N6_SET_DISABLE_ANTENNA_DIVERSITY                   0xFFF101EB
#define MTK_OID_N6_SET_FIXED_MAIN_ANTENNA                          0xFFF101EC
#define MTK_OID_N6_SET_FIXED_AUX_ANTENNA                           0xFFF101ED
#define MTK_OID_N6_SET_CUSTOMER_BT_RADIO_STATUS                    0xFFF10220
#define MTK_OID_N6_SET_FIXED_RATE                                  0xFFF10336
#define MTK_OID_N6_SET_TEST_TX_NULL_FRAME                          0xFFF10502
#define MTK_OID_N6_SET_RADIO                                       0xFFF10503
#define MTK_OID_N6_SET_POWER_MANAGEMENT                            0xFFF10505
#define MTK_OID_N6_SET_RF_TEST                                     0xFFF10506
