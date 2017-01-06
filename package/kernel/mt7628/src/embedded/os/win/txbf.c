#ifdef RTMP_SDIO_SUPPORT
#include "sdio_config.h"
#endif

#ifdef RTMP_USB_SUPPORT
#include "config.h"
#endif

//BF
INT8 TxBfProfileTagValid(
             IN PRTMP_ADAPTER          pAd,
         IN UCHAR                          validFlg, 
         IN UCHAR                   profileIdx)
{
         UINT32    value32, readValue32[5];
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("[%s],profileIdx = %d, validFlg = %d\n",  __FUNCTION__, profileIdx, validFlg));
	// Enable BF clock always on
	RTMP_IO_READ32(pAd, CR_BF_CTRL0, &value32);
	value32 |= 1 << 30;
	RTMP_IO_WRITE32(pAd, CR_BF_CTRL0, value32);
	
         // Tag read selection and trigger profile reading
         RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
         value32 &= ~0x0F00;
         value32 |= (1 << 12) | (profileIdx << 8) ;
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);

         RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
         value32 |= (1 << 30);
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);
         
         // Read CR_BF_PFILE_RDATA0~5
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA0, &readValue32[0]);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA1, &readValue32[1]);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA2, &readValue32[2]);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA3, &readValue32[3]);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA4, &readValue32[4]);


         /*
             Update the valid bit
         */
         readValue32[0] &= ~1;
         readValue32[0] |= validFlg;
         
         // write CR_BF_PFILE_RDATA0~5
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA0, readValue32[0]);
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA1, readValue32[1]);
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA2, readValue32[2]);
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA3, readValue32[3]);
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA4, readValue32[4]);

         // Tag read selection and trigger profile writting
         RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
         value32 |= (1 << 12) | (profileIdx << 8) | (1 << 31);
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);

	// Disable BF clock always on
	RTMP_IO_READ32(pAd, CR_BF_CTRL0, &value32);
	value32 &= ~(1 << 30);
	RTMP_IO_WRITE32(pAd, CR_BF_CTRL0, value32);
         
         return TRUE;
}

INT8 TxBfProfileTagRead(
		IN RTMP_ADAPTER  *pAd,
		IN UCHAR             profileIdx,
		OUT PFMU_PROFILE	*prof
		)
{
         UINT32    value32, readValue32[5];

	   // Enable BF clock always on
	 RTMP_IO_READ32(pAd, CR_BF_CTRL0, &value32);
	 value32 |= 1 << 30;
	 RTMP_IO_WRITE32(pAd, CR_BF_CTRL0, value32);
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("[%s], profileIdx = %d\n", __FUNCTION__, profileIdx));
        // prof = &pAd->prof;
         //RtlZeroMemory(prof, sizeof(PFMU_PROFILE));
         // Tag read selection and trigger profile reading         
         RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
         value32 &= ~0x0F00;
         value32 |= (1 << 12) | (profileIdx << 8) ;
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);

         RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
         value32 |= (1 << 30);
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);
         
         // Read CR_BF_PFILE_RDATA0~5
         RTMPusecDelay(100);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA0, &readValue32[0]);
         RTMPusecDelay(100);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA1, &readValue32[1]);
         RTMPusecDelay(100);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA2, &readValue32[2]);
         RTMPusecDelay(100);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA3, &readValue32[3]);
         RTMPusecDelay(100);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA4, &readValue32[4]);

	// Disable BF clock always on
	RTMP_IO_READ32(pAd, CR_BF_CTRL0, &value32);
	value32 &= ~(1 << 30);
	RTMP_IO_WRITE32(pAd, CR_BF_CTRL0, value32);
         /*
             Bit 31:30           LM[1:0]
                  29:28       Ng[1:0]
                  27:26            Nc[1:0]
                  25:24            Nr[1:0]
                  22:20            BW[2:0]
         */
         MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("CR_BF_PFILE_RDATA0 = %x\n",  readValue32[0]));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("CR_BF_PFILE_RDATA1 = %x\n",  readValue32[1]));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("CR_BF_PFILE_RDATA2 = %x\n",  readValue32[2]));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("CR_BF_PFILE_RDATA3 = %x\n",  readValue32[3]));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("CR_BF_PFILE_RDATA4 = %x\n",  readValue32[4]));
         prof->cw   = (UCHAR)((readValue32[0] >> 20) & 0x7);
         prof->nrow = (UCHAR)((readValue32[0] >> 24) & 0x3);
         prof->ncol = (UCHAR)((readValue32[0] >> 26) & 0x3);
         prof->ng   = (UCHAR)((readValue32[0] >> 28) & 0x3);
         prof->LM   = (UCHAR)((readValue32[0] >> 30) & 0x3);

         /*
             Bit 111 : 64       Mac Adress [47:0]
         */
         prof->DMAC[0] = (UCHAR)((readValue32[3] >> 8) & 0x0FF);
         prof->DMAC[1] = (UCHAR)((readValue32[3] >> 0) & 0x0FF);
         prof->DMAC[2] = (UCHAR)((readValue32[2] >> 24) & 0x0FF);
         prof->DMAC[3] = (UCHAR)((readValue32[2] >> 16) & 0x0FF);
         prof->DMAC[4] = (UCHAR)((readValue32[2] >> 8) & 0x0FF);
         prof->DMAC[5] = (UCHAR)((readValue32[2] >> 0)  & 0x0FF);

         /*
             Bit 63 : 56       SNR_STS3[7:0]
                  55 : 48       SNR_STS2[7:0]
                  47 : 40       SNR_STS1[7:0]
                  36 : 32       SNR_STS0[7:0]
         */
         prof->SNR_STS3 = (UCHAR)((readValue32[1] >> 24) & 0x0FF);
         prof->SNR_STS2 = (UCHAR)((readValue32[1] >> 16) & 0x0FF);
         prof->SNR_STS1 = (UCHAR)((readValue32[1] >> 8) & 0x0FF);
         prof->SNR_STS0 = (UCHAR)((readValue32[1] >> 0) & 0x0FF);

         /*
             Bit 135 : 128       Pn_STS1[7:0]
                  127 : 120       Pn_STS0[7:0]
         */
         prof->Pn_STS1 = (UCHAR)((readValue32[4] >> 00) & 0x0FF);
         prof->Pn_STS0 = (UCHAR)((readValue32[3] >> 24) & 0x0FF);

         /* 
             Bit 141 : 139          MCS_ub[2:0]
                    138 : 136         MCS_lb[2:0]
         */
         prof->MCS_ub = (UCHAR)((readValue32[4] >> 11) & 0x07);
         prof->MCS_lb = (UCHAR)((readValue32[4] >> 8) & 0x07);

         /*
             Bit 20 : 4       PFMU_TimeOut[15:0]
         */
         prof->timeout = (USHORT)((readValue32[0]  >> 4) & 0x0FFFF);

         /*
	Bit 119 : 112       FIX_PSI, LD, EO, IO, I/E
         */
	prof->FIX_PSI = (UCHAR)((readValue32[3] >> 23) & 0x1);
         prof->LD = (UCHAR)((readValue32[3] >> 19) & 0x1);
         prof->EO = (UCHAR)((readValue32[3] >> 18) & 0x1);
         prof->IO = (UCHAR)((readValue32[3] >> 17) & 0x1);
         prof->IE= (UCHAR)((readValue32[3] >> 16) & 0x1);

         /*
             Bit 119 : 116          Arrival order[3:0]
         */
         prof->Arrival_order = (UCHAR)((readValue32[3] >> 20) & 0x0F);
         
         /*
             Check profile valid staus
         */
         prof->validFlg = (readValue32[0] >> 0) & 0x1;
/*
         MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (
                      "============================= TxBf profile Tage Info ========================================\n"
                      "Pofile valid status = %d\n"
                      "Profile index = %d\n\n"
                      "CR_BF_PFILE_RDATA0 = 0x%x,  CR_BF_PFILE_RDATA1=0x%x,  CR_BF_PFILE_RDATA2=0x%x,  CR_BF_PFILE_RDATA3=0x%x\n"
                      "CR_BF_PFILE_RDATA4 = 0x%x\n\n"
                      "ng[1:0]=%d, cw[1:0]=%d, nrow[1:0]=%d, ncol[1:0]=%d, LM[1:0]=%d\n\n"
                      "Destimation MAC Address=%x:%x:%x:%x:%x:%x\n\n"
                      "Pn_STS1=0x%x, Pn_STS0=0x%x\n\n"
                      "MCS_ub[2:0]=0x%x, MCS_lb[2:0]=0x%x\n\n"
                      "STS3_SNR=0x%x, STS2_SNR=0x%x, STS1_SNR=0x%x, STS0_SNR=0x%x\n\n"
                      "timeout[15:0]=0x%x\n\n"
                      "Arrival order[3:0]=0x%x\n\n"
                      "LD=%d, EO=%d, IO=%d, I/E=%d\n"
                      "===================================================================================\n",
                      prof->validFlg,
                      prof->CMDInIdx,
                      readValue32[0], readValue32[1], readValue32[2], readValue32[3], readValue32[4],
                      prof->ng, prof->cw, prof->nrow, prof->ncol, prof->LM,
                      prof->DMAC[0], prof->DMAC[1], prof->DMAC[2], prof->DMAC[3], prof->DMAC[4], prof->DMAC[5],
                      prof->Pn_STS1, prof->Pn_STS0,
                      prof->MCS_ub, prof->MCS_lb,
                      prof->SNR_STS3, prof->SNR_STS2, prof->SNR_STS1, prof->SNR_STS0,
                      prof->timeout,
                      prof->Arrival_order,
                      prof->LD, prof->EO, prof->IO, prof->IE));
*/                      
	  	  // Disable BF clock always on
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("CMDInIdx = %d\n", prof->CMDInIdx));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("Arrival_order = %d\n", prof->Arrival_order));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,(" cw = %d\n", prof->cw));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("DMAC = %02x:%02x:%02x:%02x:%02x:%02x\n", prof->DMAC[0], prof->DMAC[1],prof->DMAC[2], prof->DMAC[3], prof->DMAC[4], prof->DMAC[5]));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("EO = %d\n", prof->EO));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("IE = %d\n",  prof->IE));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("IO = %d\n",  prof->IO));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("LD = %d\n",  prof->LD));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("LM = %d\n",   prof->LM));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("MCS_lb = %d\n", prof->MCS_lb));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("MCS_u = %d\n", prof->MCS_ub));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("ncol = %d\n",  prof->ncol));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("ng = %d\n",  prof->ng));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("nrow = %d\n",  prof->nrow));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("Pn_STS0 = %d\n",  prof->Pn_STS0));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("Pn_STS1 = %d\n",  prof->Pn_STS1));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("validFlg = %d\n",  prof->validFlg));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("timeout = %d\n",  prof->timeout));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("SNR_STS0 = %d\n",  prof->SNR_STS0));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("SNR_STS1 = %d\n",  prof->SNR_STS1));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("SNR_STS2 = %d\n",  prof->SNR_STS2));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("SNR_STS3 = %d\n",  prof->SNR_STS3));
	  
         return TRUE;
}


INT8 TxBfProfileTagWrite(
             IN PRTMP_ADAPTER          pAd,
             IN UCHAR             profileIdx,
             IN PFMU_PROFILE      *prof
         )
{
         UINT32    value32, readValue32[5];
	  // Enable BF clock always on
	  RTMP_IO_READ32(pAd, CR_BF_CTRL0, &value32);
	  value32 |= 1 << 30;
	  RTMP_IO_WRITE32(pAd, CR_BF_CTRL0, value32);
//Debug		 
       //  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("[%s], profileIdx = %d\n",  __FUNCTION__, profileIdx));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("CMDInIdx = %d\n", prof->CMDInIdx));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("Arrival_order = %d\n", prof->Arrival_order));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,(" cw = %d\n", prof->cw));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("DMAC = %02x:%02x:%02x:%02x:%02x:%02x\n", prof->DMAC[0], prof->DMAC[1],prof->DMAC[2], prof->DMAC[3], prof->DMAC[4], prof->DMAC[5]));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("EO = %d\n", prof->EO));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("IE = %d\n",  prof->IE));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("IO = %d\n",  prof->IO));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("LD = %d\n",  prof->LD));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("LM = %d\n",   prof->LM));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("MCS_lb = %d\n", prof->MCS_lb));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("MCS_u = %d\n", prof->MCS_ub));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("ncol = %d\n",  prof->ncol));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("ng = %d\n",  prof->ng));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("nrow = %d\n",  prof->nrow));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("Pn_STS0 = %d\n",  prof->Pn_STS0));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("Pn_STS1 = %d\n",  prof->Pn_STS1));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("validFlg = %d\n",  prof->validFlg));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("timeout = %d\n",  prof->timeout));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("SNR_STS0 = %d\n",  prof->SNR_STS0));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("SNR_STS1 = %d\n",  prof->SNR_STS1));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("SNR_STS2 = %d\n",  prof->SNR_STS2));
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("SNR_STS3 = %d\n",  prof->SNR_STS3));

//Debug
	  //prof->CMDInIdx = 0x1 << prof->CMDInIdx;
	  prof->CMDInIdx = 0x7f;

         // Tag read selection and trigger profile reading
         RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
         value32 &= ~0x0F00;
         value32 |= (1 << 12) | (profileIdx << 8) ;
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);

         RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
         value32 |= (1 << 30);
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);
         
         // Read CR_BF_PFILE_RDATA0~5
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA0, &readValue32[0]);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA1, &readValue32[1]);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA2, &readValue32[2]);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA3, &readValue32[3]);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA4, &readValue32[4]);
         

         MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("============================= TxBf profile Tage Write ==============================\n"
                      "Profile index = %d\n\n",
                      profileIdx));

	/*
	    Bit 0		valid flag
	*/
	if (prof->CMDInIdx & 64)
	{
		readValue32[0] &= ~0x1;
		readValue32[0] |= prof->validFlg;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("valid=%d\n\n", prof->validFlg));
	}

         /*
             Bit 31:30           LM[1:0]
                  29:28       Ng[1:0]
                  27:26            Nc[1:0]
                  25:24            Nr[1:0]
                  22:20            BW[2:0]
                  19:04            TIMEOUT[15:0]
         */
         if (prof->CMDInIdx & 1)
         {
                   value32   = (LONG)prof->cw << 20;
                   value32 |= (LONG)prof->nrow << 24;
                   value32 |= (LONG)prof->ncol << 26;
                   value32 |= (LONG)prof->ng << 28;
                   value32 |= (LONG)prof->LM << 30;
                   
                   readValue32[0] &= 0x000FFFFF;
                   readValue32[0] |= value32;

                   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ng[1:0]=%d, cw[1:0]=%d, nrow[1:0]=%d, ncol[1:0]=%d, LM[1:0]=%d\n\n",
                               prof->ng, prof->cw, prof->nrow, prof->ncol, prof->LM));
         }


         /*
             Bit 111 : 64      Mac Adress [47:0]
         */
         if (prof->CMDInIdx & 2)
         {
                   value32  = (LONG)prof->DMAC[0] << 8;
                   value32 |= (LONG)prof->DMAC[1] <<0;
                   readValue32[3] &= 0xFFFF0000;
                   readValue32[3] |= value32;

                   value32 = (LONG)prof->DMAC[2] << 24;
                   value32 |= (LONG)prof->DMAC[3] << 16;
                   value32 |= (LONG)prof->DMAC[4] << 8;
                   value32 |= (LONG)prof->DMAC[5] << 0;
                   readValue32[2] = value32;
                   
                   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Destimation MAC Address=%x:%x:%x:%x:%x:%x\n\n",
                               prof->DMAC[0], prof->DMAC[1], prof->DMAC[2], prof->DMAC[3], prof->DMAC[4], prof->DMAC[5]));
         }
         
         /*
             Bit 135 : 128       Pn_STS1[7:0]
                  127 : 120       Pn_STS0[7:0]
         */
         if (prof->CMDInIdx & 4)
         {
                   value32  = (LONG)prof->Pn_STS1;
                   readValue32[4] &= 0xFFFFFF00;
                   readValue32[4] |= value32;
                   
                   value32 |= (LONG)prof->Pn_STS0 << 24;
                   readValue32[3] &= 0x00FFFFFF;
                   readValue32[3] |= value32;

                   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Pn_STS1=0x%x, Pn_STS0=0x%x\n\n",
                               prof->Pn_STS1, prof->Pn_STS0));
         }


         /*
             Bit 47 : 40       SNR_STS1[7:0]
                  36 : 32       SNR_STS0[7:0]
         */
         if (prof->CMDInIdx & 8)
         {
                   value32  = (LONG)prof->SNR_STS1 << 8;
                   value32 |= (LONG)prof->SNR_STS0;
                   readValue32[1] &= 0xFFFF0000;
                   readValue32[1] |= value32;
                   
                   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("STS1_SNR=0x%x, STS0_SNR=0x%x\n\n", prof->SNR_STS1, prof->SNR_STS0));
         }


         /* 
             Bit 141 : 139          MCS_ub[2:0]
                    138 : 136         MCS_lb[2:0]
         */
         if (prof->CMDInIdx & 16)
         {
                   value32  = (LONG)prof->MCS_ub << 11;
                   value32 |= (LONG)prof->MCS_lb << 8;
                   readValue32[4] &= 0xFFFFC0FF;
                   readValue32[4] |= value32;
                   
                   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MCS_ub=0x%x, MCS_lb=0x%x\n\n", prof->MCS_ub, prof->MCS_lb));
         }


         /*
             Bit 115 : 112              LD, EO, IO, I/E
         */
         if (prof->CMDInIdx & 32)
         {
                   readValue32[3] &= 0xFF00FFFF;
                   readValue32[3] |= prof->LD << 19;
                   readValue32[3] |= prof->EO << 18;
                   readValue32[3] |= prof->IO << 17;
                   readValue32[3] |= prof->IE<< 16;
                            
                   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("LD=%d, EO=%d, IO=%d, I/E=%d\n",
                               prof->LD, prof->EO, prof->IO, prof->IE));
         }

         MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (
                              " CR_BF_PFILE_RDATA0 = 0x%x,  CR_BF_PFILE_RDATA1=0x%x,  CR_BF_PFILE_RDATA2=0x%x,  CR_BF_PFILE_RDATA3=0x%x\n"
                              " CR_BF_PFILE_RDATA4 = 0x%x\n"
                            "===================================================================================\n",
                            readValue32[0], readValue32[1], readValue32[2], readValue32[3], readValue32[4]));
         
         // write CR_BF_PFILE_RDATA0~5
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA0, readValue32[0]);
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA1, readValue32[1]);
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA2, readValue32[2]);
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA3, readValue32[3]);
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA4, readValue32[4]);

         // Tag read selection and trigger profile writting
         RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
         value32 |= (1 << 12) | (profileIdx << 8) | (1 << 31);
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);

	  // Disable BF clock always on
 	  RTMP_IO_READ32(pAd, CR_BF_CTRL0, &value32);
	  value32 &= ~(1 << 30);
	  RTMP_IO_WRITE32(pAd, CR_BF_CTRL0, value32);

         prof->CMDInIdx = 0; // clear tag indicator
         return TRUE;
}



INT8 TxBfProfileDataRead(
             IN PRTMP_ADAPTER          pAd,
         IN UCHAR                   profileIdx,
         IN UCHAR			SubcarrierIdx,
         OUT PFMU_DATA 		*pPfmuD)
{
         UINT32    value32, readValue32[5];

	UINT8   psi21, psi31, psi41, psi32, psi42, psi43;
	UINT16 phi11, phi21, phi31, phi22, phi32, phi33;
	UINT8   dSNR0, dSNR1, dSNR2, dSNR3; 
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("[%s], profileIdx = %d, subcarrierIdx = %d\n",  __FUNCTION__, profileIdx, SubcarrierIdx));	

	// Enable BF clock always on
	RTMP_IO_READ32(pAd, CR_BF_CTRL0, &value32);
	value32 |= 1 << 30;
	RTMP_IO_WRITE32(pAd, CR_BF_CTRL0, value32);
      // Data read selection and trigger profile reading
         RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
         value32 &= 0x00000000;
         value32 |= (profileIdx << 8) | SubcarrierIdx;
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);

         RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
         value32 |= (1 << 30);
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);
         
         // Read CR_BF_PFILE_RDATA0~5
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA0, &readValue32[0]);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA1, &readValue32[1]);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA2, &readValue32[2]);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA3, &readValue32[3]);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA4, &readValue32[4]);

	// Disable BF clock always on
	RTMP_IO_READ32(pAd, CR_BF_CTRL0, &value32);
	value32 &= ~(1 << 30);
	RTMP_IO_WRITE32(pAd, CR_BF_CTRL0, value32);
	pPfmuD->psi21 = (readValue32[0] >> 9) & 0x007F;
	pPfmuD->phi11 = (readValue32[0] >> 0)  & 0x01FF;
	pPfmuD->psi31 = (readValue32[0] >> 25) & 0x007F;
	pPfmuD->phi21 = (readValue32[0] >> 16)  & 0x01FF;
	pPfmuD->psi41 = (readValue32[1] >> 9) & 0x007F;
	pPfmuD->phi31 = (readValue32[1] >> 0)  & 0x01FF;
	pPfmuD->psi32 = (readValue32[1] >> 25) & 0x007F;
	pPfmuD->phi22 = (readValue32[1] >> 16)  & 0x01FF;
	pPfmuD->psi42 = (readValue32[2] >> 9) & 0x007F;
	pPfmuD->phi32 = (readValue32[2] >> 0)  & 0x01FF;
	pPfmuD->psi43 = (readValue32[2] >> 25) & 0x007F;
	pPfmuD->phi33 = (readValue32[2] >> 16)  & 0x01FF;

	pPfmuD->Snr0 = (readValue32[3] >> 0)  & 0x0F;
	pPfmuD->Snr1 = (readValue32[3] >> 4)  & 0x0F;
	pPfmuD->Snr2 = (readValue32[3] >> 8)  & 0x0F;
	pPfmuD->Snr3 = (readValue32[3] >> 12)  & 0x0F;
	pPfmuD->SubcarrierIdx = SubcarrierIdx;
         MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (
                      "============================= TxBf profile Data Info ==============================\n"
                      "Profile index = %d,    subcarrierIdx = %d\n\n"
                      "CR_BF_PFILE_RDATA0 = 0x%x\n\n"
                      "psi21 = 0x%x\n\n"
                      "phi11 = 0x%x\n\n"
                      "==========================================================================\n",
                      profileIdx, SubcarrierIdx,
                      readValue32[0], pPfmuD->psi21, pPfmuD->phi11));
	
         return TRUE;
}



INT8 TxBfProfileDataWrite(
             IN PRTMP_ADAPTER      pAd,
         IN UCHAR                   profileIdx,
         IN UCHAR			SubcarrierIdx,
         IN PFMU_DATA 		*pPfmuD)
{
         UINT32    value32, readValue32[5];
         //psi21 &= 0x7f;
	  //phill&= 0x1ff;
	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("[%s],profileIdx = %d, subcarrierIdx = 0x%x, psi21 = 0x%x, phi11 = 0x%x \n", __FUNCTION__, profileIdx, SubcarrierIdx, pPfmuD->psi21, pPfmuD->phi11));
  	  // Enable BF clock always on
	  RTMP_IO_READ32(pAd, CR_BF_CTRL0, &value32);
	  value32 |= 1 << 30;
	  RTMP_IO_WRITE32(pAd, CR_BF_CTRL0, value32);                   
	// Data read selection and trigger profile reading
         RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
         value32 &= 0x00000000;
         value32 |= (profileIdx << 8) | SubcarrierIdx;
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);

         RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
         value32 |= (1 << 30);
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);
         
         // Read CR_BF_PFILE_RDATA0~5
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA0, &readValue32[0]);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA1, &readValue32[1]);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA2, &readValue32[2]);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA3, &readValue32[3]);
         RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA4, &readValue32[4]);

         MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Profile index = %d,    subcarrierIdx = %d\n\n",
                      profileIdx, SubcarrierIdx));

         readValue32[0] &= 0xFFFF03FF;
         readValue32[0] |= pPfmuD->psi21 << 9;          
         MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("psi21 = 0x%x, ", pPfmuD->psi21));

         readValue32[0] &= 0xFFFFFF00;
         readValue32[0] |= pPfmuD->phi11;
         MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("phi11 = 0x%x\n", pPfmuD->phi11));
         
         //Write CR_BF_PFILE_RDATA0~5
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA0, readValue32[0]);
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA1, readValue32[1]);
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA2, readValue32[2]);
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA3, readValue32[3]);
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA4, readValue32[4]);

         // Data read selection and trigger profile writting
         RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
	  value32 &= 0x00000000;
         value32 |= (1 << 31) | (profileIdx << 8) | SubcarrierIdx;
         RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);

  	  // Disable BF clock always on
 	  RTMP_IO_READ32(pAd, CR_BF_CTRL0, &value32);
	  value32 &= ~(1 << 30);
	  RTMP_IO_WRITE32(pAd, CR_BF_CTRL0, value32);
         
         return TRUE;
}

static SC_TABLE_ENTRY impSCTable[3] = { {224, 255, 1, 31}, {198, 254, 2, 58}, {134, 254, 2, 122} };
static SC_TABLE_ENTRY expSCTable[3] = { {224, 255, 1, 31}, {198, 254, 2, 58}, {134, 254, 2, 122} };

INT8 TxBfProfileDataWriteAll(
         IN RTMP_ADAPTER *pAd, 
         IN UCHAR TxBfType,
         IN UCHAR profileIdx,
         IN UCHAR BW, 
         IN PFMU_DATA *pPfmuD)
{
         INT32  scIndex = 0;
         UCHAR     psi21_I, phill_I;
         UINT32         value32, readValue32[5];
         SC_TABLE_ENTRY *pTab;

	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("[%s],profileIdx = %d, TxBfType = %d, BW = %d\n", __FUNCTION__, profileIdx, TxBfType, BW));
         //psi21_I     = 0x00;
         //phill_I       = 0xFF;
      	  
         if (TxBfType == 0) // ITxBf
                   pTab = &impSCTable[BW];
         else
                   pTab = &expSCTable[BW];
	// Enable BF clock always on
	  RTMP_IO_READ32(pAd, CR_BF_CTRL0, &value32);
	  value32 |= 1 << 30;
	  RTMP_IO_WRITE32(pAd, CR_BF_CTRL0, value32);

         /* Negative subcarriers */
         for (scIndex = pTab->lwb1; scIndex <= pTab->upb1; scIndex++)
         {
                   // Tag read selection and trigger profile reading
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
		value32 &= 0x00000000;
		value32 |= (profileIdx << 8) | scIndex;
                   RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);

                   RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
                   value32 |= (1 << 30);
                   RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);
         
                   // Read CR_BF_PFILE_RDATA0~5
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA0, &readValue32[0]);
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA1, &readValue32[1]);
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA2, &readValue32[2]);
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA3, &readValue32[3]);
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA4, &readValue32[4]);

                   readValue32[0] = (pPfmuD->psi21 << 9) | pPfmuD->phi11;
         
                   // Write CR_BF_PFILE_RDATA0~5
                   RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA0, readValue32[0]);
                   RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA1, readValue32[1]);
                   RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA2, readValue32[2]);
                   RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA3, readValue32[3]);
                   RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA4, readValue32[4]);

                   // Tag read selection and trigger profile writting
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
		value32 &= 0x00000000;
                   value32 |= (1 << 31) | (profileIdx << 8) | scIndex;
                   RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);
         }

         /* Positive subcarriers */
         for (scIndex = pTab->lwb2; scIndex <= pTab->upb2; scIndex++)
         {
                   // Tag read selection and trigger profile reading
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
		value32 &= 0x00000000;
		value32 |= (profileIdx << 8) | scIndex;
                   RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);

                   RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
                   value32 |= (1 << 30);
                   RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);
         
                   // Read CR_BF_PFILE_RDATA0~5
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA0, &readValue32[0]);
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA1, &readValue32[1]);
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA2, &readValue32[2]);
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA3, &readValue32[3]);
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA4, &readValue32[4]);

                   readValue32[0] = (pPfmuD->psi21 << 9) | pPfmuD->phi11;
         
                   // Write CR_BF_PFILE_RDATA0~5
                   RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA0, readValue32[0]);
                   RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA1, readValue32[1]);
                   RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA2, readValue32[2]);
                   RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA3, readValue32[3]);
                   RTMP_IO_WRITE32(pAd, CR_BF_PFILE_WDATA4, readValue32[4]);

                   // Tag read selection and trigger profile writting
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
		value32 &= 0x00000000;
                   value32 |= (1 << 31) | (profileIdx << 8) | scIndex;
                   RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);
         }

    	  // Disable BF clock always on
 	  RTMP_IO_READ32(pAd, CR_BF_CTRL0, &value32);
	  value32 &= ~(1 << 30);
	  RTMP_IO_WRITE32(pAd, CR_BF_CTRL0, value32);

         return TRUE;
}


INT8 TxBfProfileDataReadAll(
         IN RTMP_ADAPTER *pAd, 
         IN UCHAR TxBfType,
         IN UCHAR profileIdx,
         IN UCHAR BW,
         OUT PFMU_DATA *pPfmuD,
         OUT UINT32 *size)
{
          UCHAR    psi21_I, phill_I;
         UINT32         value32, readValue32[5];
         SC_TABLE_ENTRY *pTab;
         INT32           scIndex = 0, idx = 0;         

	  MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("[%s],profileIdx = %d, TxBfType = %d, BW = %d\n", __FUNCTION__, profileIdx, TxBfType, BW));

         if (TxBfType == 0) // ITxBf
                   pTab = &impSCTable[BW];
         else
                   pTab = &expSCTable[BW];

	// Enable BF clock always on
	RTMP_IO_READ32(pAd, CR_BF_CTRL0, &value32);
	value32 |= 1 << 30;
	RTMP_IO_WRITE32(pAd, CR_BF_CTRL0, value32);
         /* Negative subcarriers */
         idx = 0;
         for (scIndex = pTab->lwb1; scIndex <= pTab->upb1; scIndex++)
         {
         	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("[%s],scIndex = %d\n", __FUNCTION__, scIndex));
                   // Tag read selection and trigger profile reading
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
		value32 &= 0x00000000;
		value32 |= (profileIdx << 8)  | scIndex;
                   RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);

                   RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
                   value32 |= (1 << 30);
                   RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);
         
                   // Read CR_BF_PFILE_RDATA0~5
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA0, &readValue32[0]);
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA1, &readValue32[1]);
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA2, &readValue32[2]);
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA3, &readValue32[3]);
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA4, &readValue32[4]);
				   
			pPfmuD[idx].psi21 = (readValue32[0] >> 9) & 0x007F;
			pPfmuD[idx].phi11 = (readValue32[0] >> 0) & 0x01FF;
			pPfmuD[idx].psi31 =  (readValue32[0] >> 25)& 0x007F;
			pPfmuD[idx].phi21 = (readValue32[0] >> 16) & 0x01FF;
			pPfmuD[idx].psi41 = (readValue32[1] >> 9) & 0x007F;
			pPfmuD[idx].phi31 = (readValue32[1] >> 0) & 0x01FF;
			pPfmuD[idx].psi32 = (readValue32[1] >> 25) & 0x007F;
			pPfmuD[idx].phi22 = (readValue32[1] >> 16) & 0x01FF;
			pPfmuD[idx].psi42 = (readValue32[2] >> 9) & 0x007F;
			pPfmuD[idx].phi32 = (readValue32[2] >> 0) & 0x01FF;
			pPfmuD[idx].psi43 = (readValue32[2] >> 25) & 0x007F;
			pPfmuD[idx].phi33 = (readValue32[2] >> 16) & 0x01FF;

			pPfmuD[idx].Snr0= (readValue32[3] >> 0)  & 0x0F;
			pPfmuD[idx].Snr1= (readValue32[3] >> 4)  & 0x0F;
			pPfmuD[idx].Snr2 = (readValue32[3] >> 8)  & 0x0F;
			pPfmuD[idx].Snr3 = (readValue32[3] >> 12)  & 0x0F;
			pPfmuD[idx].SubcarrierIdx = scIndex;
			idx++;
         }

         MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

         /* Positive subcarriers */
       //  idx = 0;
         for (scIndex = pTab->lwb2; scIndex <= pTab->upb2; scIndex++)
         {
         	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("[%s],scIndex = %d\n", __FUNCTION__, scIndex));
                   // Tag read selection and trigger profile reading
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
		value32 &= 0x00000000;
		value32 |= (profileIdx << 8) | scIndex;
                   RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);

                   RTMP_IO_READ32(pAd, CR_BF_PFILE_ACCESS, &value32);
                   value32 |= (1 << 30);
                   RTMP_IO_WRITE32(pAd, CR_BF_PFILE_ACCESS, value32);
         
                   // Read CR_BF_PFILE_RDATA0~5
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA0, &readValue32[0]);
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA1, &readValue32[1]);
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA2, &readValue32[2]);
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA3, &readValue32[3]);
                   RTMP_IO_READ32(pAd, CR_BF_PFILE_RDATA4, &readValue32[4]);

		pPfmuD[idx].psi21 = (readValue32[0] >> 9) & 0x007F;
		pPfmuD[idx].phi11 = (readValue32[0] >> 0)  & 0x01FF;
		pPfmuD[idx].psi31 = (readValue32[0] >> 25) & 0x007F;
		pPfmuD[idx].phi21 = (readValue32[0] >> 16)  & 0x01FF;
		pPfmuD[idx].psi41 = (readValue32[1] >> 9) & 0x007F;
		pPfmuD[idx].phi31 = (readValue32[1] >> 0)  & 0x01FF;
		pPfmuD[idx].psi32 = (readValue32[1] >> 25) & 0x007F;
		pPfmuD[idx].phi22 = (readValue32[1] >> 16)  & 0x01FF;
		pPfmuD[idx].psi42 = (readValue32[2] >> 9) & 0x007F;
		pPfmuD[idx].phi32 = (readValue32[2] >> 0)  & 0x01FF;
		pPfmuD[idx].psi43 = (readValue32[2] >> 25) & 0x007F;
		pPfmuD[idx].phi33 = (readValue32[2] >> 16)  & 0x01FF;
			 
		pPfmuD[idx].Snr0 = (readValue32[3] >> 0)  & 0x0F;
		pPfmuD[idx].Snr1 = (readValue32[3] >> 4)  & 0x0F;
		pPfmuD[idx].Snr2 = (readValue32[3] >> 8)  & 0x0F;
		pPfmuD[idx].Snr3 = (readValue32[3] >> 12)  & 0x0F;
		pPfmuD[idx].SubcarrierIdx = scIndex;

		idx++;			   
         }
		 MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("total return %d", idx));
		*size = sizeof(PFMU_DATA) * idx;
	// Disable BF clock always on
	RTMP_IO_READ32(pAd, CR_BF_CTRL0, &value32);
	value32 &= ~(1 << 30);
	RTMP_IO_WRITE32(pAd, CR_BF_CTRL0, value32);

         return TRUE;
}



INT8 TxBfProfileTag_MCS(
         IN RTMP_ADAPTER *pAd, 
         IN UCHAR MCS_ub,
         IN UCHAR MCS_lb)
{
         PFMU_PROFILE *prof;

         prof = &pAd->prof;

         prof->CMDInIdx |= 16;
         prof->MCS_ub = MCS_ub;
         prof->MCS_lb = MCS_lb;

         return TRUE;
}


INT8 TxBfProfileTag_Matrix(
         IN RTMP_ADAPTER *pAd, 
         IN UCHAR ng,
         IN UCHAR cw,
         IN UCHAR nrow,
         IN UCHAR ncol,
         IN UCHAR LM)
{
         PFMU_PROFILE *prof;

         prof = &pAd->prof;

             /* ng[1:0], cw[1:0], nrow[1:0], ncol[1:0] */
         prof->CMDInIdx |= 1;
             
             prof->ng    = ng;
             prof->cw    = cw;
             prof->nrow = nrow;
             prof->ncol  = ncol;
             prof->LM    = LM;

         MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ng[1:0]=%x\n" "cw[1:0]=%x\n" "nrow[1:0]=%x\n" "nol[1:0]=%x\n" "LM[1:0]=%x\n",
                      ng, cw, nrow, ncol, LM));

         return TRUE;
}


INT8 TxBfProfileTag_SNR(
         IN RTMP_ADAPTER *pAd, 
         IN UCHAR StsSnr0,
         IN UCHAR StsSnr1)
{
         PFMU_PROFILE *prof;

         prof = &pAd->prof;

         /* STS1_SNR: STS0_SNR */
         prof->CMDInIdx |= 8;

         prof->SNR_STS0 = StsSnr0;
         prof->SNR_STS1 = StsSnr1;

         MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("STS1_SNR=0x%x\n" "STS1_SNR=0x%x\n",
                      StsSnr0, StsSnr1));

         return TRUE;
}

INT8 TxBfProfileTag_TxScale(
         IN RTMP_ADAPTER *pAd, 
         IN UCHAR Tx1_scale_2ss,
         IN UCHAR Tx0_scale_2ss,
         IN UCHAR Tx1_scale_1ss,
         IN UCHAR Tx0_scale_1ss)
{
         PFMU_PROFILE *prof;

         prof = &pAd->prof;

         /* Tx1_scale_2ss: Tx0_scale_2ss: Tx1_scale_1ss: Tx0_scale_1ss*/
         prof->CMDInIdx |= 4;

         prof->Pn_STS1 = Tx1_scale_1ss;
         prof->Pn_STS0 = Tx0_scale_1ss;

         MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Pn_STS1=0x%x\n" "Pn_STS0=0x%x\n",
                      Tx1_scale_1ss, Tx0_scale_1ss));

         return TRUE;
}



INT8 TxBfProfileTag_MAC(
         IN RTMP_ADAPTER *pAd, 
         IN UCHAR *pMac)
{
         PFMU_PROFILE *prof;

         prof = &pAd->prof;

         /* Mac address acceptable format 01:02:03:04:05:06 length 17 */
         prof->CMDInIdx |= 2;

         prof->DMAC[0] = pMac[0];
         prof->DMAC[1] = pMac[1];
         prof->DMAC[2] = pMac[2];
         prof->DMAC[3] = pMac[3];
         prof->DMAC[4] = pMac[4];
         prof->DMAC[5] = pMac[5];

         MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Set_TxBfProfileTage_MAC\n" "DS MAC=0x%x:0x%x:0x%x:0x%x:0x%x:0x%x \n",
                     pMac[0],pMac[1],pMac[2],pMac[3],pMac[4],pMac[5]));
    return TRUE;
}

INT8 TxBfProfileTag_Flg(
         IN RTMP_ADAPTER *pAd, 
         IN UCHAR LD,
         IN UCHAR EO,
         IN UCHAR IO,
         IN UCHAR IE)
{
         PFMU_PROFILE *prof;

         prof = &pAd->prof;

             /* LD, EO, IO, I/E */
         prof->CMDInIdx |= 32;

             prof->LD = LD;
             prof->EO = EO;
             prof->IO = IO;
             prof->IE = IE;

         MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("LD=%d\n" "EO=%d\n" "IO=%d\n" "I/E=%d\n",
                      LD, EO, IO, IE));
         return TRUE;
}

INT8 TxBfBypassMacAddress(
         IN RTMP_ADAPTER *pAd, 
         IN BOOLEAN IsEnable)
{
	UINT32 CRValue = 0;
	RTMP_IO_READ32(pAd, CR_BF_CTRL1, &CRValue);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("%s, CR_BF_CTRL1 = 0x%x\n", __FUNCTION__, CRValue));

	if(IsEnable)
	{	
		CRValue = BITCLEAR(CRValue, BIT22);
	}
	else
	{
		CRValue = BITSET(CRValue, BIT22);
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("%s, CR_BF_CTRL1 = 0x%x\n", __FUNCTION__, CRValue));
	RTMP_IO_WRITE32(pAd, CR_BF_CTRL1, CRValue);	
	return TRUE;
}

void TxBFPowerNormalize(
         IN RTMP_ADAPTER *pAd, 
         IN BOOLEAN IsEnable)
{
	UINT32 CRValue = 0;
	RTMP_IO_READ32(pAd, CR_BF_CSD_1, &CRValue);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("%s, CR_BF_CSD_1 = 0x%x\n", __FUNCTION__, CRValue));
	if(IsEnable)
	{
		CRValue = BITSET(CRValue, BIT20);
	}
	else
	{
		CRValue = BITCLEAR(CRValue, BIT20);
	}	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("%s, CR_BF_CSD_1 = 0x%x\n", __FUNCTION__,  CRValue));
	RTMP_IO_WRITE32(pAd, CR_BF_CSD_1, CRValue);	
}

/* 
==========================================================================
    Description:
        Set ATE Tx Beamforming mode
        
        Return:
                 TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
void MT_SetATETxBfProc(RTMP_ADAPTER *pAd, UINT8 TxBfEnFlg, UINT32 BW)
{
         switch (TxBfEnFlg)
         {
                   case 0:
                            /* no BF */
                            pAd->iTxBf= FALSE;
                            pAd->eTxBf = FALSE;
                            break;
                   case 1:
                            /* ETxBF */
                            pAd->iTxBf= FALSE;
                            pAd->eTxBf = TRUE;
                            break;
                   case 2:
                            /* ITxBF */
                            pAd->iTxBf= TRUE;
                            pAd->eTxBf = FALSE;
                            break;
                   case 3:
                            /* Enable TXBF support */
                            pAd->iTxBf= TRUE;
                            pAd->eTxBf = TRUE;
                            break;

                   default:
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("%s: Invalid parameter %d\n", __FUNCTION__, TxBfEnFlg));
                            break;
         }
      
         if (pAd->eTxBf == TRUE)
         {
                   // Enable sounding trigger in FW
                   CmdETxBfSoundingPeriodicTriggerCtrl(pAd, BSSID_WCID, TRUE, BW);
         }
         else
         {
                   // Disable sounding trigger in FW
                   CmdETxBfSoundingPeriodicTriggerCtrl(pAd, BSSID_WCID, FALSE, BW);
         }
         
}

void IsPfmuChanged(RTMP_ADAPTER *pAd, UINT32 bw, UINT32 *IsChanged)
{
	UINT32 size = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("%s: BW %d\n", __FUNCTION__, bw));
	if(bw == BW_20)
	{
		TxBfProfileDataReadAll(pAd, 1, 9, BW_20, (PFMU_DATA *) &pAd->postPfmuDataBW20, &size);
	
		*IsChanged = RTMPEqualMemory((PVOID)&pAd->postPfmuDataBW20, (PVOID)&pAd->prePfmuDataBW20, sizeof(pAd->prePfmuDataBW20));
	}
	else if(bw == BW_40)
	{
		TxBfProfileDataReadAll(pAd, 1, 9, BW_40, (PFMU_DATA *) &pAd->postPfmuDataBW40, &size);
		*IsChanged = RTMPEqualMemory((PVOID)&pAd->postPfmuDataBW40, (PVOID)&pAd->prePfmuDataBW40, sizeof(pAd->prePfmuDataBW40));
	}
	else if(bw == BW_80)
	{
		//11ac to do
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("%s: To implement %d\n", __FUNCTION__, bw));
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("%s: Invalid BW %d\n", __FUNCTION__, bw));
	}
	*IsChanged = !*IsChanged;
         
}

void IsBFProfileStore(RTMP_ADAPTER *pAd, UINT32 IsStore)
{	
	pAd->IsBfStored = (BOOLEAN)IsStore;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("%s: Set  IsBfStored: %d\n", __FUNCTION__, pAd->IsBfStored));
}

