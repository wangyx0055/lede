#include <striptool.h>

#define BUILD_MODULE_DIRECTORY "build"
#define BUILD_CMD_DIRECTORY "build_cmd"
/*
	TDLS is general function in STA mode except DPO/STA.

	For AP release :
		DPA - basic function + WPS.
		DPB - basic function + WPS + Mesh + Gemtek proprietary function.
		DPC - basic function + WPS + Mesh.
		DPI - basic function + WPS. Exclude IDS, WAPI.

	For STA release :
		DPA: WPS + ACM + P2P
		DPAW: WPS + ACM + P2P + Intel WIDI
		DPB: Full function �V WPS + Ethernet Convert + Mesh + ACM + P2P
		DPC: WPS + Mesh + ACM + P2P
		DPD: WPS + Ethernet Convert + ACM + P2P
		DPO: Without WPS, Ethernet Convert, Mesh

	For Arch release :
		1.	AP + STA mode
		2. 	without ACM, 11R_FT, 11K_RRM.
		note : In MakeFile,
				CHIPSET = SOC
				RELEASE = ARCH

	For Samsung release :
		DPA_S: DPA + Auto Provision + Modifications for Samsung

	For Toshiba release :
		DPA_T: DPA + Modifications for Toshiba

 */

static char *RELEASE_DIRECTORY[] = {
#ifdef INCLUDE_ATE
	"ate",
	"ate/include",
	#ifdef MT_MAC
	"ate/mt_mac",
	#endif
	#if defined(RLT_MAC) || defined(RTMP_MAC)
	"ate/rt_mac",
	"ate/rt_mac/chips",
	#endif
#endif //INCLUDE_ATE
	"chips",
	"eeprom",
	"embedded",
	"embedded/common",
	"embedded/conf",
	"embedded/doc",
#ifdef CONFIG_FPGA_MODE
	"embedded/fpga",
	"embedded/include/fpga",
#endif /* CONFIG_FPGA_MODE */
	"embedded/hif",
	"embedded/include",
	"embedded/include/chip",
	"embedded/include/eeprom",
	"embedded/include/iface",
	"embedded/include/mcu",
	"embedded/include/os",
	"embedded/mcu",
	"embedded/mcu/bin",
	"embedded/mgmt",
	"embedded/naf",
	"embedded/os",
#ifdef LINUX
	"embedded/os/linux",
	"embedded/os/linux/cfg80211",
#endif
#ifdef VXWORKS
	"embedded/os/vxworks",
#endif
#ifdef ECOS
	"embedded/os/ecos",
#endif
#ifdef INCLUDE_AP
	"embedded/ap",
	"embedded/mt_wifi_ap",
#endif
#ifdef INCLUDE_STA
	"embedded/sta",
	"embedded/mt_wifi_sta",
#endif
#ifdef LINUX
	"embedded/tools",
	"embedded/tools/plug_in",
#endif
	"embedded/tx_rx",
#ifdef INCLUDE_SNIFFER
	"embedded/include/sniffer",
	"embedded/sniffer",
#endif /* INCLUDE_SNIFFER */
	"hw_ctrl",
	"include",
	"mac",
	"phy",
	"rate_ctrl",
#ifdef COMPOS_WIN
	"rate_ctrl_win",
	"windows",
#endif
	"mcu/bin",
    "include/mac",
#if defined(RLT_MAC) || defined(RTMP_MAC)
    "include/mac/mac_ral",
    "include/mac/mac_ral/nmac",
    "include/mac/mac_ral/omac",
#endif
#ifdef MT_MAC
    "include/mac/mac_mt",
    "include/mac/mac_mt/smac",
#endif
    "include/hw_ctrl",
	"include/mcu",
	"include/phy",
#ifdef INCLUDE_TRACELOG_IPARP
	"embedded/debug",
	"embedded/include/debug",
#endif /* INCLUDE_TRACELOG_IPARP */
};

/*
 * - Sort by interface
 * - Sort by mcu
 * - Sort by mac type
 * - Sort by chipset
 * - Sort by OS
 * - Sort by Function
 */
static char *EXCLUDE_FILE[] = {
/* =======================
 *	Sort by interface
 * ======================= */
#ifndef RTMP_USB_SUPPORT
	"embedded/include/iface/rtmp_usb.h",
    #ifdef LINUX
	    "embedded/os/linux/usb_main_dev.c",
    #endif // LINUX //
#endif // RTMP_USB_SUPPORT //

#ifndef RTMP_PCI_SUPPORT
	"embedded/include/iface/rtmp_pci.h",
    #ifdef LINUX
	    "embedded/os/linux/pci_main_dev.c",
    #endif // LINUX //
	    "embedded/hif/hif_pci.c",
#endif // RTMP_PCI_SUPPORT //

#ifndef RTMP_RBUS_SUPPORT
    "embedded/include/iface/rtmp_rbs.h",
#ifndef RTMP_FLASH_SUPPORT
    "embedded/common/ee_flash.c",
#endif // RTMP_FLASH_SUPPORT //
	"embedded/common/rtmp_swmcu.c",
    #ifdef LINUX
        "embedded/os/linux/rbus_main_dev.c",
    #endif // LINUX //
#endif // RTMP_RBUS_SUPPORT //

#ifndef RTMP_SDIO_SUPPORT
	"embedded/common/cmm_data_sdio.c",
	"embedded/common/cmm_mac_sdio.c",
	"embedded/common/cmm_sdio_io.c",
	"embedded/common/mtsdio_data.c",
	"embedded/common/mtsdio_io.c",
	"embedded/include/mtsdio_data.h",
	"embedded/include/mtsdio_io.h",
	"embedded/include/iface/mt_sdio.h",
	"embedded/include/iface/mt_hif_sdio.h",
	"embedded/include/mac/mac_mt/mt_mac_sdio.h",
	"embedded/include/rtsdio_io.h",
	#ifdef LINUX
		"embedded/os/linux/rt_sdio.c",
		"embedded/os/linux/sdio_main_dev.c",
	#endif // LINUX //
#endif // RTMP_SDIO_SUPPORT //

/* =======================
 *	Sort by mac type
 * ======================= */
#ifndef RTMP_MAC_USB
	"embedded/common/rt2870.bin.gpio",
	"embedded/common/rt2870.bin.host_status",
	"embedded/common/rt2870_wow.bin",
	"embedded/common/rt2870.bin",
	"embedded/common/cmm_data_usb.c",
	"embedded/common/cmm_mac_usb.c",
	"embedded/common/rtusb_dev_id.c",
	"embedded/common/rtusb_bulk.c",
	"embedded/common/rtusb_data.c",
	"embedded/common/rtusb_io.c",
	"embedded/common/mtusb_io.c",
	"embedded/include/chip/mac_usb.h",
	"embedded/include/mac_ral/mac_usb.h",
	"embedded/include/rtusb_io.h",
	"embedded/ate/common/ate_usb.c",
    #ifdef LINUX
	    "embedded/os/linux/rt_usb.c",
	    "embedded/os/linux/rt_usb_util.c",
    #endif // LINUX //
#endif // RTMP_MAC_USB //

#ifdef RTMP_MAC_USB
#ifndef INCLUDE_WOW
#ifndef INCLUDE_WOW_GPIO
	"embedded/common/rt2870.bin.gpio",
#endif // INCLUDE_WOW_GPIO //
#ifndef INCLUDE_WOW_HOST_STATUS
	"embedded/common/rt2870.bin.host_status",
#endif // INCLUDE_WOW_HOST_STATUS //
	"embedded/common/rt2870_wow.bin",
#endif // INCLUDE_WOW //
#endif // RTMP_MAC_USB //

#ifndef RTMP_MAC_PCI
	"embedded/common/rt2860.bin",
	"embedded/common/cmm_data_pci.c",
	"embedded/common/cmm_mac_pci.c",
	"embedded/include/chip/mac_pci.h",
	"embedded/include/mac/mac_ral/mac_pci.h",
    #ifdef LINUX
	    "embedded/os/linux/rt_pci_rbus.c",
	    "embedded/os/linux/rt_rbus_pci_util.c",
    #endif // LINUX //
#endif // RTMP_MAC_PCI //

/* =======================
 *	Sort by chipset
 * ======================= */
#ifndef RT30xx
	"embedded/include/chip/rt30xx.h",
	"chips/rt30xx.c",
	"ate/rt_mac/chips/rt30xx_ate.c",
#endif // RT30xx //

#ifndef RT35xx
	"embedded/include/chip/rt35xx.h",
	"chips/rt35xx.c",
	"ate/rt_mac/chips/rt35xx_ate.c",
#endif // RT35xx //

#ifndef RT305x
	"embedded/include/chip/rt305x.h",
	"chips/rt305x.c",
	"ate/rt_mac/chips/rt305x_ate.c",
#endif // RT305x //

#ifndef RT33xx
	"embedded/include/chip/rt33xx.h",
	"chips/rt33xx.c",
	"ate/rt_mac/chips/rt33xx_ate.c",
#endif // RT33xx //

#ifndef RT2860
	"embedded/include/chip/rt2860.h",
#endif // RT2860 //
#ifndef RT2870
	"embedded/include/chip/rt2870.h",
#endif // RT2870 //

#ifndef RT2880
	"embedded/include/chip/rt2880.h",
	"chips/rt2880.c",
	"embedded/os/linux/Makefile.2880.ap",
	"embedded/os/linux/Makefile.2880.sta",
	"embedded/os/linux/Makefile.release.2880",
#endif // RT2880 //

#ifndef RT2883
	"embedded/include/chip/rt2883.h",
	"chips/rt2883.c",
	"ate/rt_mac/chips/rt2883_ate.c",
#endif // RT2883 //

#ifndef RT3883
	"embedded/include/chip/rt3883.h",
	"chips/rt3883.c",
	"ate/rt_mac/chips/rt3883_ate.c",
#endif // RT3883 //

#ifndef RT3070
	"embedded/include/chip/rt3070.h",
	"chips/rt3070.c",
#endif // RT3070 //

#ifndef RT3090
	"embedded/include/chip/rt3090.h",
	"chips/rt3090.c",
#endif // RT3090 //

#ifndef RT3370
	"embedded/include/chip/rt3370.h",
	"chips/rt3370.c",
#endif // RT3370 //

#ifndef RT3390
	"embedded/include/chip/rt3390.h",
	"chips/rt3390.c",
#endif // RT3390 //

#ifndef RT3593
	"embedded/include/chip/rt3593.h",
	"chips/rt3593.c",
	"ate/rt_mac/chips/rt3593_ate.c",
#endif // RT3593 //

#ifndef RT5350
	"embedded/include/chip/rt5350.h",
	"chips/rt5350.c",
	"ate/rt_mac/chips/rt5350_ate.c",
#endif // RT5350 //

#ifndef RT3352
	"embedded/include/chip/rt3352.h",
	"chips/rt3352.c",
	"ate/rt_mac/chips/rt3352_ate.c",
#endif // RT3352 //

#if !defined(RT5370) && !defined(RT5372) && !defined(RT5390) && !defined(RT5392)
	"embedded/include/chip/rt5390.h",
	"chips/rt5390.c",
	"ate/rt_mac/chips/rt5390_ate.c",
#endif /* defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392) */

#ifndef RT5592
	"embedded/include/chip/rt5592.h",
	"chips/rt5592.c",
	"ate/rt_mac/chips/rt5592_ate.c",
#endif

#ifndef RT28xx
	"embedded/include/chip/rt28xx.h",
	"chips/rt28xx.c",
#endif

#ifndef RT3290
	"embedded/include/chip/rt3290.h",
	"chips/rt3290.c",
#endif

#ifndef RT6352
	"embedded/include/chip/rt6352.h",
	"chips/rt6352.c",
	"ate/rt_mac/chips/rt6352_ate.c",
#endif

#ifndef RT6590
	"chips/rt6592.c",
#endif

#ifndef MT7601
	"embedded/include/chip/mt7601.h",
	"chips/mt7601.c",
	"ate/rt_mac/chips/mt7601_ate.c",
	"embedded/mcu/bin/MT7601.bin",
	"embedded/include/mcu/mt7601_firmware.h",
	"embedded/include/eeprom/mt7601_e2p.h",
#endif

#ifndef RT8592
	"embedded/include/chip/rt8592.h",
	"chips/rt85592.c",
	"ate/rt_mac/chips/rt85592_ate.c",
#endif

#ifndef RT65xx
	"chips/rt65xx.c",
	"embedded/include/chip/rt65xx.h",
#endif

#ifndef MT76x0
	"embedded/include/chip/mt76x0.h",
	"chips/mt76x0.c",
#endif

#if !defined(MT7650) && !defined(MT7630)
	"embedded/include/mcu/mt7650_firmware.h",
#endif

#ifndef MT7610
	"embedded/include/mcu/mt7610_firmware.h",
#endif

#ifndef MT76x2
	"embedded/include/chip/mt76x2.h",
	"chips/mt76x2.c",
	"embedded/include/mcu/mt7662_rom_patch.h",
	"embedded/include/eeprom/mt76x2_e2p.h",
	"include/phy/mt76x2_rf.h",
	"phy/mt76x2_rf.c",
	"embedded/tools/mt7662_freq_plan.c",
	"embedded/tools/mt7662e_ap.sh",
	"embedded/tools/mt7662e_sta.sh",
	"ate/rt_mac/chips/mt76x2_ate.c",
#endif

#if !defined(MT7662) && !defined(MT7632)
	"embedded/include/mcu/mt7662_firmware.h",
#endif

#ifndef MT7612
	"embedded/include/mcu/mt7612_firmware.h",
#endif

#ifndef MT7603
	"embedded/include/chip/mt7603.h",
	"chips/mt7603.c",
	"embedded/include/mcu/mt7603_firmware.h",
	"embedded/include/mcu/mt7603_e2_firmware.h",
	"embedded/include/eeprom/mt7603_e2p.h",
#endif

#ifndef MT7615
	"embedded/include/chip/mt7615.h",
	"chips/mt7615.c",
#endif

#ifndef MT7628
	"embedded/include/chip/mt7628.h",
	"chips/mt7628.c",
	"embedded/include/mcu/mt7628_firmware.h",
	"embedded/include/mcu/mt7628_e2_firmware.h",
	"embedded/include/eeprom/mt7628_e2p.h",
#endif

#ifndef MT7636
	"embedded/include/chip/mt7636.h",
	"chips/mt7636.c",
	"embedded/include/mcu/mt7636_firmware.h",
	"embedded/include/mcu/mt7636_rom_patch.h",
	"embedded/include/eeprom/mt7636_e2p.h",
#endif

#ifndef MT_MAC
	"embedded/include/hw_ctrl/cmm_asic_mt.h",
#endif

#if !defined(RLT_MAC) && !defined(RTMP_MAC)
	"include/hw_ctrl/cmm_asic_rt.h",
#endif

#ifndef RLT_MAC
	"include/mac/mac_ral/nmac/ral_nmac.h",
	"include/mac/mac_ral/nmac/ral_nmac_pbf.h",
	"include/mac/mac_ral/nmac/ral_nmac_pci.h",
	"phy/rlt_phy.c",
	"phy/rlt_rf.c",
#endif

#ifndef RTMP_MAC
	"include/mac/mac_ral/omac/ral_omac.h",
	"include/mac/mac_ral/omac/ral_omac_pbf.h",
	"include/mac/mac_ral/omac/ral_omac_pci.h",
	"include/mac/mac_ral/omac/ral_omac_rf_ctrl.h",
	"ate/include/rt_ate.h",
	"ate/include/rt_qa.h",
	"phy/rtmp_phy.c",
	"phy/rt_rf.c",
#endif

/* =======================
 *	Sort by OS
 * ======================= */
#ifndef LINUX
	"embedded/include/os/rt_linux.h",
	"embedded/include/os/rt_linux_cmm.h",
#endif // LINUX //
#ifndef ECOS
	"embedded/include/os/rt_ecos.h",
	"embedded/include/os/rt_ecos_type.h",
#endif // ECOS //
#ifndef UCOS
	"embedded/include/os/rt_ucos.h",
#endif // UCOS //
#ifndef VXWORKS
	"embedded/include/os/rt_vxworks.h",
	"embedded/vxworks.makefile",
#endif // VXWORKS //
#ifndef THREADX
	"embedded/include/os/rt_threadx.h",
	"embedded/threadX.MakeFile",
#endif // THREADX //
	"embedded/include/os/rt_win.h",
	"embedded/include/os/rt_wince.h",

/* =======================
 *	Sort by Function
 * ======================= */
#ifndef RTMP_EFUSE_SUPPORT
	"embedded/common/ee_efuse.c",
#endif // RTMP_EFUSE_SUPPORT //

#ifndef INCLUDE_WAPI
	"embedded/include/wapi_sms4.h",
    "embedded/include/wapi_def.h",
	"embedded/include/wapi.h",
	"embedded/common/wapi.c",
	"embedded/os/linux/Makefile.libwapi.4",
	"embedded/os/linux/Makefile.libwapi.6",
#endif // INCLUDE_WAPI //
#ifndef AUTO_BUILD
    "embedded/common/wapi_sms4.c",
    "embedded/common/wapi_crypt.c",
#endif // AUTO_BUILD //
	"embedded/include/dh_key.h",
    "embedded/include/evp_enc.h",
	"embedded/common/dh_key.c",
	"embedded/common/evp_enc.c",
#ifndef INCLUDE_WPS
    "embedded/include/crypt_biginteger.h",
    "embedded/include/crypt_dh.h",
    "embedded/include/wsc_tlv.h",
    "embedded/common/crypt_biginteger.c",
    "embedded/common/crypt_dh.c",
    "embedded/common/wsc.c",
    "embedded/common/wsc_tlv.c",
#endif // INCLUDE_WPS //
#ifndef INCLUDE_WPS_V2
    "embedded/common/wsc_v2.c",
#endif // INCLUDE_WPS_V2 //

#ifndef INCLUDE_IWPS
    "embedded/sta/sta_iwsc.c",
#endif // INCLUDE_IWPS //

#ifndef INCLUDE_DFS
    "embedded/common/cmm_dfs.c",
	"embedded/include/dfs.h",
#endif // INCLUDE_DFS //

#ifndef INCLUDE_HS
    "embedded/common/hotspot.c",
	"embedded/include/hotspot.h",
#endif // INCLUDE_HS //

#ifndef INCLUDE_ADHOC_WPA2PSK
    "embedded/common/cmm_wpa_adhoc.c",
#endif // INCLUDE_ADHOC_WPA2PSK //

#ifndef INCLUDE_MAT
    "embedded/include/mat.h",
    "embedded/common/cmm_mat.c",
    "embedded/common/cmm_mat_iparp.c",
    "embedded/common/cmm_mat_ipv6.c",
    "embedded/common/cmm_mat_pppoe.c",
#endif // INCLUDE_MAT //

#ifndef INCLUDE_MESH
    "embedded/include/mesh.h",
    "embedded/include/mesh_def.h",
    "embedded/include/mesh_mlme.h",
    "embedded/include/mesh_sanity.h",
	"embedded/common/mesh.c",
    "embedded/common/mesh_bmpkt.c",
    "embedded/common/mesh_ctrl.c",
	"embedded/common/mesh_forwarding.c",
    "embedded/common/mesh_link_mng.c",
    "embedded/common/mesh_path_mng.c",
    "embedded/common/mesh_sanity.c",
    "embedded/common/mesh_tlv.c",
    "embedded/common/mesh_inf.c",
#endif // INCLUDE_MESH //

#ifndef INCLUDE_ACM
    "embedded/include/acm_comm.h",
    "embedded/include/acm_edca.h",
	"embedded/include/acm_extr.h",
    "embedded/common/acm_comm.c",
    "embedded/common/acm_edca.c",
    "embedded/common/acm_iocl.c",
#endif // INCLUDE_ACM //

#ifndef INCLUDE_11R_FT
	"embedded/include/ft.h",
	"embedded/include/ft_cmm.h",
	"embedded/common/ft.c",
	"embedded/common/ft_iocl.c",
	"embedded/common/ft_rc.c",
	"embedded/common/ft_tlv.c",
#endif // INCLUDE_11R_FT //

#ifndef DOT11_VHT_AC
	"embedded/include/vht.h",
	"embedded/include/dot11ac_vht.h",
	"embedded/mgmt/mgmt_vht.c",
	"embedded/common/vht.c",
#endif // DOT11_VHT_AC //

#ifndef INCLUDE_11K_RRM
	"embedded/include/dot11k_rrm.h",
	"embedded/include/rrm.h",
	"embedded/include/rrm_cmm.h",
	"embedded/common/rrm.c",
	"embedded/common/rrm_sanity.c",
	"embedded/common/rrm_tlv.c",
#endif // INCLUDE_11R_FT //

#ifndef INCLUDE_TDLS
	"embedded/include/dot11z_tdls.h",
	"embedded/include/tdls.h",
	"embedded/include/tdls_cmm.h",
	"embedded/include/tdls_uapsd.h",
#endif // INCLUDE_TDLS //

#if !defined(INCLUDE_TDLS) && !defined(INCLUDE_11R_FT)
	"embedded/include/dot11r_ft.h",
#endif

#ifndef INCLUDE_11W_PMF
	"embedded/include/dot11w_pmf.h",
	"embedded/include/pmf.h",
	"embedded/include/pmf_cmm.h",
	"embedded/common/pmf.c",
#endif // INCLUDE_11W_PMF //

#ifndef INCLUDE_11V_WNM_SUPPORT
	"embedded/common/wnm.c",
	"embedded/common/wnm_tlv.c",
	"embedded/include/dot11v_wnm.h",
	"embedded/include/wnm.h",
	"embedded/include/wnm_cmm.h",
#endif // INCLUDE_11V_WNM_SUPPORT //

#ifndef INCLUDE_P2P
	"embedded/common/p2p_inf.c",
	"embedded/common/p2p_dbg.c",
	"embedded/common/p2p.c",
	"embedded/common/p2p_packet.c",
	"embedded/common/p2p_action.c",
	"embedded/common/p2p_table.c",
	"embedded/common/p2p_ctrl.c",
	"embedded/common/p2p_disc_mng.c",
	"embedded/common/p2p_nego_mng.c",
	"embedded/common/p2p_dbg.c",
	"embedded/common/p2p.c",
	"embedded/common/p2p_packet.c",
	"embedded/common/p2p_action.c",
	"embedded/common/p2p_table.c",
	"embedded/common/p2p_ctrl.c",
	"embedded/common/p2p_disc_mng.c",
	"embedded/common/p2p_nego_mng.c",
	"embedded/common/p2p_cfg.c",
	"embedded/sta/p2pcli.c",
	"embedded/sta/p2pcli_ctrl.c",
	"embedded/sta/p2pcli_sync.c",
	"embedded/sta/p2pcli_auth.c",
	"embedded/sta/p2pcli_assoc.c",
	"embedded/include/p2p.h",
	"embedded/include/p2p_cmm.h",
	"embedded/include/p2p_inf.h",
	"embedded/include/p2pcli.h",
	"embedded/include/wfa_p2p.h",
#endif // INCLUDE_P2P //

#ifndef INCLUDE_MT76XX_BT_COEXISTENCE_SUPPORT
	"embedded/mcu/bt_coex.c",
#endif // INCLUDE_MT76XX_BT_COEXISTENCE_SUPPORT //

#ifndef INCLUDE_WIDI
	"embedded/common/l2sd_ta_mod.c",
	"embedded/include/l2sd_ta.h",
#endif // INCLUDE_WIDI //

#ifndef LINUX
    "embedded/include/rt_linux.h",
#endif

#ifndef INCLUDE_SAMSUNG_SUPPORT
	"embedded/common/auto_provision.c",
#endif // INCLUDE_SAMSUNG_SUPPORT //

#ifndef LED_CONTROL_SUPPORT
	"embedded/common/rt_led.c",
	"embedded/include/rt_led.h",
#endif // LED_CONTROL_SUPPORT //

#ifndef INCLUDE_SMART_ANTENNA
	"embedded/common/smartant.c",
	"embedded/include/smartant.h",
#endif // INCLUDE_SMART_ANTENNA //

#ifndef TXBF_SUPPORT
	"embedded/common/cmm_txbf.c",
	"embedded/common/cmm_txbf_cal.c",
	"embedded/include/rt_txbf.h"
#endif // TXBF_SUPPORT //

#ifndef INCLUDE_TRACELOG_IPARP
	"embedded/debug/tracelog_iparp.c",
	"embedded/include/debug/tracelog_iparp_def.h",
	"embedded/include/debug/tracelog_iparp_func.h",
#endif /* INCLUDE_TRACELOG_IPARP */

#ifndef INCLUDE_AP
	"embedded/RT2860AP.dat",
	"embedded/common/igmp_snoop.c",
	"embedded/common/rt2860.bin.dfs",
	"embedded/common/wsc_ufd.c",
	"embedded/include/ap_apcli.h",
	"embedded/include/ap_autoChSel.h",
	"embedded/include/ap_autoChSel_cmm.h",
	"embedded/include/ap_cfg.h",
	"embedded/include/ap_ids.h",
	"embedded/include/ap_mbss.h",
	"embedded/include/ap_uapsd.h",
	"embedded/include/ap_wds.h",
	"embedded/include/igmp_snoop.h",
    #ifdef LINUX
        "embedded/os/linux/ap_ioctl.c",
    #endif // LINUX //
#else
    #ifndef INCLUDE_11R_FT
	    "embedded/ap/ap_ftkd.c",
    	"embedded/ap/ap_ftrc.c",
    #endif // INCLUDE_11R_FT //
    #ifndef INCLUDE_NINTENDO_AP
    	"embedded/ap/ap_nintendo.c",
    #endif // INCLUDE_NINTENDO_AP //
#ifdef INCLUDE_P2P
#ifdef MODE_APSTA
	"embedded/include/ap_wds.h",
#endif
#endif
#endif // INCLUDE_AP //

#ifndef INCLUDE_STA
	"embedded/RT2860STA.dat",
    #ifdef LINUX
        "embedded/os/linux/sta_ioctl.c",
    #endif // LINUX //
#else
    #ifndef INCLUDE_11R_FT
	    "embedded/sta/ft_action.c",
    	"embedded/sta/ft_auth.c",
    #endif // INCLUDE_11R_FT //
	#ifndef INCLUDE_TDLS
		"embedded/sta/tdls.c",
    	"embedded/sta/tdls_ctrl.c",
		"embedded/sta/tdls_link_mng.c",
		"embedded/sta/tdls_tlv.c",
		"embedded/sta/tdls_chswitch_mng.c",
		"embedded/sta/tdls_uapsd.c",
	#endif // INCLUDE_TDLS //
	//#if !defined(INCLUDE_ETH_CONVERT) && !defined(INCLUDE_P2P)
	//	"embedded/include/ipv6.h",
	//#endif // INCLUDE_ETH_CONVERT //
	#ifndef INCLUDE_SAMSUNG_STA_SUPPORT
		"embedded/sta/sta_auto_provision.c",
	#endif // INCLUDE_SAMSUNG_STA_SUPPORT //
		"embedded/os/linux/load",
		"embedded/os/linux/unload",
#endif // INCLUDE_STA //

#ifdef INCLUDE_P2P
    	"embedded/ap/ap_apcli.c",
    	"embedded/ap/ap_apcli_inf.c",
    	"embedded/ap/apcli_assoc.c",
    	"embedded/ap/apcli_auth.c",
    	"embedded/ap/apcli_ctrl.c",
    	"embedded/ap/apcli_sync.c",
#ifdef MODE_APSTA
	"embedded/ap/ap_wds.c",
	"embedded/ap/ap_cfg.c",
#endif
#endif

#ifdef LINUX
#ifdef OS_ABL_SUPPORT
	"embedded/include/iface/iface_util.h",
	"embedded/include/rtmp_type.h",
	"embedded/include/rtmp_os.h",
	"embedded/include/link_list.h",
	"embedded/include/rtmp_cmd.h",
	"embedded/include/rtmp_comm.h",
	"embedded/include/rt_os_util.h",
	"embedded/include/rtmp_osabl.h",

	"embedded/common/rt_os_util.c",
	"embedded/os/linux/rt_linux_symb.c",
	"embedded/os/linux/rt_linux.c",
	"embedded/os/linux/rt_usb_util.c",
	"embedded/os/linux/rt_rbus_pci_util.c",
	"embedded/os/linux/vr_bdlt.c",
	"embedded/os/linux/br_ftph.c",
	"embedded/os/linux/Makefile.6.util",
	"embedded/os/linux/Makefile.4.util",

	"embedded/ap/ap_mbss_inf.c",
	"embedded/os/linux/ap_ioctl.c",
	"embedded/os/linux/sta_ioctl.c",
	"embedded/os/linux/rt_main_dev.c",
	"embedded/ap/ap_wds_inf.c",
	"embedded/ap/ap_apcli_inf.c",
	"embedded/common/mesh_inf.c",
	"embedded/common/p2p_inf.c",
	"embedded/common/rtusb_dev_id.c",
	"embedded/os/linux/usb_main_dev.c",
	"embedded/os/linux/rt_pci_rbus.c",
	"embedded/os/linux/pci_main_dev.c",
	"embedded/os/linux/cfg80211.c",
	"embedded/os/linux/Makefile.6.netif",
	"embedded/os/linux/Makefile.4.netif",
#endif // OS_ABL_SUPPORT //
#ifndef OS_ABL_SUPPORT
	"embedded/cp_module.sh",
	"embedded/cp_util.sh",
	"embedded/load.4",
	"embedded/load.6",
	"embedded/Makefile.OSABL",
#endif
#endif // LINUX //


/* =======================
 *	Sort by mcu
 * ======================= */
#ifndef INCLUDE_M8051_SUPPORT
	"embedded/include/mcu/mcu_51.h",
	"embedded/mcu/mcu_51.c",
#endif

#ifndef INCLUDE_ANDES_SUPPORT
	"embedded/include/mcu/mcu_and.h",
	"embedded/mcu/mcu_and.c",
#endif
};
#if 0
static char *RELEASE_ROOT_FILE[] = {
	"embedded/Makefile",
	"embedded/Kconfig",
#ifdef INCLUDE_AP
	"embedded/doc/History.txt",
#ifdef RTMP_MAC_PCI
	"embedded/conf/RT2860AP.dat",
	"embedded/conf/RT2860APCard.dat",
#endif
#ifdef RTMP_MAC_USB
	"embedded/conf/RT2870AP.dat",
	"embedded/conf/RT2870APCard.dat",
#endif
#endif // INCLUDE_AP //
#ifdef INCLUDE_STA
#ifdef RTMP_MAC_USB
	"embedded/conf/RT2870STA.dat",
	"embedded/conf/RT2870STACard.dat",
	"embedded/doc/README_STA_usb",
#endif // RTMP_MAC_USB //
#ifdef RTMP_MAC_PCI
	"embedded/conf/RT2860STA.dat",
	"embedded/conf/RT2860STACard.dat",
	"embedded/doc/README_STA_pci",
#endif // RTMP_MAC_PCI //
#ifdef INCLUDE_WPS
	"embedded/doc/wps_iwpriv_usage.txt",
#endif // INCLUDE_WPS //
	"embedded/doc/iwpriv_usage.txt",
	"embedded/doc/sta_ate_iwpriv_usage.txt",
#endif // INCLUDE_STA //
};

static char *RELEASE_FILE[] = {
	"embedded/common/cmm_profile.c",
	"embedded/common/cmm_info.c",
};
#endif
int fnStripModule()
{
    char Command[255];
    int index;

    /* create directories */
    sprintf(Command, "rm -Rf %s", BUILD_MODULE_DIRECTORY);
    system(Command);

    sprintf(Command, "mkdir %s", BUILD_MODULE_DIRECTORY);
    system(Command);
#if 0
#ifdef RELEASE_ARCH
    sprintf(Command, "cp -f * %s/", BUILD_MODULE_DIRECTORY); //Only copy files
    system(Command);
#else
    for (index = 0; index < sizeof(RELEASE_ROOT_FILE)/sizeof(char *);index++)
    {
        sprintf(Command, "cp -f %s %s/", RELEASE_ROOT_FILE[index], BUILD_MODULE_DIRECTORY);
		//printf("%s\n", Command);
        system(Command);
    } /* End of for */
#endif
#endif
    for (index = 0; index < sizeof(RELEASE_DIRECTORY)/sizeof(char *);index++)
    {
        sprintf(Command, "mkdir %s/%s", BUILD_MODULE_DIRECTORY, RELEASE_DIRECTORY[index]);
        system(Command);
        sprintf(Command, "cp -f ../%s/* %s/%s", RELEASE_DIRECTORY[index], BUILD_MODULE_DIRECTORY, RELEASE_DIRECTORY[index]);
		// printf("%s\n", Command);
        system(Command);
    } /* End of for */

#if 0
#ifdef INCLUDE_P2P
#ifdef OS_ABL_SUPPORT
    /* copy file */
    for (index = 0; index < sizeof(RELEASE_FILE)/sizeof(char *);index++)
    {
        sprintf(Command, "cp -f %s%s %s/%s", RELEASE_CMD_PATH, RELEASE_FILE[index], BUILD_MODULE_DIRECTORY, RELEASE_FILE[index]);
        system(Command);
    } /* End of for */
#endif /* OS_ABL_SUPPORT */
#endif /* INCLUDE_P2P */
#endif
#if 0
    /* remove all CVS directory */
    sprintf(Command, "find %s -name CVS -prune -exec rm -rf {} \\;", BUILD_MODULE_DIRECTORY);
    system(Command);
#endif

    /* remove file */
    for (index = 0; index < sizeof(EXCLUDE_FILE)/sizeof(char *);index++)
    {
        sprintf(Command, "rm -f %s/%s", BUILD_MODULE_DIRECTORY, EXCLUDE_FILE[index]);
        system(Command);
    } /* End of for */

#ifdef RTMP_MAC_USB
#ifdef INCLUDE_WOW
#ifdef INCLUDE_WOW_GPIO
    system("mv ./build/common/rt2870.bin.gpio ./build/common/rt2870_wow.bin");
#elif defined(INCLUDE_WOW_HOST_STATUS)
    system("mv ./build/common/rt2870.bin.host_status ./build/common/rt2870_wow.bin");
#endif /* INCLUDE_WOW_GPIO */
#endif /* INCLUDE_WOW */
#endif /* RTMP_MAC_USB */

#ifdef OS_ABL_SUPPORT
//	sprintf(Command, "cp -f Makefile.module %s/Makefile", BUILD_MODULE_DIRECTORY);
//	system(Command);

	fnCommStrip(BUILD_MODULE_DIRECTORY, RELEASE_MODULE_PATH);

#else
	fnCommStrip(BUILD_MODULE_DIRECTORY, RELEASE_PATH);
#endif // OS_ABL_SUPPORT //

remove_build:
    sprintf(Command, "rm -Rf %s", BUILD_MODULE_DIRECTORY);
    system(Command);
    sprintf(Command, "rm -Rf %s", BUILD_CMD_DIRECTORY);
    system(Command);
#ifdef INCLUDE_P2P
#ifdef OS_ABL_SUPPORT
    sprintf(Command, "rm -Rf %s", RELEASE_CMD_PATH);
    system(Command);
#endif /* OS_ABL_SUPPORT */
#endif
    return 0;
} /* End of main */

