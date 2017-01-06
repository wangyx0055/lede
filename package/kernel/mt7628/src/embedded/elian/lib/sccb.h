#ifndef __MSC_LIB_H__
#define __MSC_LIB_H__

//#define BIT(n) (1UL << (n%(8*sizeof(unsigned long))))

#define ERR_NOTSUPPORT		-1
#define ERR_TIMEDOUT		-2
#define ERR_INVALIDPARAM	-3

#define ELIAN_MONITOR_MODE_OFF  0
#define ELIAN_MONITOR_MODE_FULL 2


/* for 7620 rx filer */
#define  ELIAN_RXFILTER_NORMAL	      0x00015f97  /*0x1400 default value*/
#define  ELIAN_RXFILTER_DROP_CRC_ERR    (1 << 0)   /* Drops frames with CRC errors. Initial Value: 0x1 */
#define  ELIAN_RXFILTER_DROP_PHY_ERR    (1 << 1)   /* Drops frames with physical errors. Initial Value: 0x1 */
#define  ELIAN_RXFILTER_DROP_UC_NOME    (1 << 2)   /* Drops not to me unicast frames. Initial Value: 0x1 */
#define  ELIAN_RXFILTER_DROP_NOT_MYBSS  (1 << 3)   /* Drops frames that are not my BSSID. Initial Value: 0x1 */
#define  ELIAN_RXFILTER_DROP_VER_ERR    (1 << 4)   /* Drops frames with 802.11 version errors. Initial Value: 0x1 */
#define  ELIAN_RXFILTER_DROP_MC         (1 << 5)   /* Drops multicast frames. Initial Value: 0x0 */
#define  ELIAN_RXFILTER_DROP_BC         (1 << 6)   /* Drops broadcast frames. Initial Value: 0x0 */
#define  ELIAN_RXFILTER_DROP_DUPL       (1 << 7)   /* Drops duplicated frames. Initial Value: 0x1 */
#define  ELIAN_RXFILTER_DROP_CFACL      (1 << 8)   /* Drops CF-END + CF-ACK frames. Initial Value: 0x1  */
#define  ELIAN_RXFILTER_DROP_CFEND      (1 << 9)   /* Drops CF-END frames. Initial Value: 0x1 */
#define  ELIAN_RXFILTER_DROP_ACK        (1 << 10)  /* Drops ACK frames. Initial Value: 0x1 */
#define  ELIAN_RXFILTER_DROP_CTS        (1 << 11)  /* Drops CTS frames. Initial Value: 0x1 */
#define  ELIAN_RXFILTER_DROP_RTS        (1 << 12)  /* Drops RTS frames. Initial Value: 0x1 */
#define  ELIAN_RXFILTER_DROP_PSPOLL     (1 << 13)  /* Drops PS-Poll frames. Initial Value: 0x0 */
#define  ELIAN_RXFILTER_DROP_BA         (1 << 14)  /* Drops BA frames. Initial Value: 0x1*/
#define  ELIAN_RXFILTER_DROP_BAR        (1 << 15)  /* Drops BAR frames. Initial Value: 0x0*/ 
#define  ELIAN_RXFILTER_DROP_CTRL_RSV   (1 << 16)  /* Drops reserve control subtype. Initial Value: 0x1 */


//#define MSC_DROP_DATA BIT(0)
//#define MSC_DROP_MGMT BIT(1)
//#define MSC_DROP_MULTICAST BIT(2)
//#define MSC_DROP_BROADCAST BIT(3)
#if 0 
enum sc_ctrl_id {
	MSC_CTRLID_ENABLE_MONITOR_MODE = 0x00,
	MSC_CTRLID_DISABLE_MONITOR_MODE,
	MSC_CTRLID_GET_MODE,
	MSC_CTRLID_SET_CHANNEL,
	MSC_CTRLID_GET_CHANNEL,
	MSC_CTRLID_SET_FILTER,
	MSC_CTRLID_GET_FILTER,
	MSC_CTRLID_USE_ELIAN,
	MSC_CTRLID_MAX
};
enum msc_band {
	MSC_BAND_2GHZ,
	MSC_BAND_5GHZ,
	MSC_BAND_60GHZ,
	MSC_BAND_NUM
};
#endif

enum msc_chan_width {
	MSC_CHAN_WIDTH_20_NOHT,
	MSC_CHAN_WIDTH_20,
	MSC_CHAN_WIDTH_40_PLUS,
	MSC_CHAN_WIDTH_80,
	MSC_CHAN_WIDTH_80P80,
	MSC_CHAN_WIDTH_160,
	MSC_CHAN_WIDTH_5,
	MSC_CHAN_WIDTH_10,
	MSC_CHAN_WIDTH_40_MINUS,
};

enum msc_iftype {
	MSC_IFTYPE_UNSPECIFIED,
	MSC_IFTYPE_ADHOC,
	MSC_IFTYPE_STATION,
	MSC_IFTYPE_AP,
	MSC_IFTYPE_AP_VLAN,
	MSC_IFTYPE_WDS,
	MSC_IFTYPE_MONITOR,
	MSC_IFTYPE_MESH_POINT,
	MSC_IFTYPE_P2P_CLIENT,
	MSC_IFTYPE_P2P_GO,
	MSC_IFTYPE_P2P_DEVICE,

	MSC_IFTYPE_IFTYPES,
};

struct chan_info {
	int chan_id;
	enum msc_chan_width width;
	unsigned int flags;
};

struct monitor_info {
	//enum msc_iftype iftype;
	unsigned int filter;
	struct chan_info chl_info;
	void *priv;
};

typedef int (*sc_input_func) (char *, int);
typedef int (*sc_sw_filter) (char *, int);

int sccb_init(sc_input_func input_func, sc_sw_filter filter_func);
int sccb_deinit(void);
int sccb_enter_monitor_mode(struct monitor_info *info, void *priv);
int sccb_leave_monitor_mode(void *priv);
int sccb_set_monitor_chan(struct chan_info *chl_info, void *priv);
int sccb_rx(char *data, unsigned int data_len);
int sccb_enable_input(void);
int sccb_disable_input(void);
#endif

