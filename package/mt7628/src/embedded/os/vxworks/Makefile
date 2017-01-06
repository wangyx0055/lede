# Makefile - build modules
LIB_BASE_NAME = wlanMT7628

CFLAGS        = $(CFLAGS_COM) -I. -I$(WIND_BASE)/target/h/ -I$(WIND_BASE)/target/h/wrn/coreip -I$(WIND_BASE)/target/h \
			 	-I$(WIND_BASE)/target/config/comps/src -I$(WIND_BASE)/target/src/drv \
			 	-I$(WIND_BASE)/target/config/comps/src/net $(WLAN_EXTRA)
			 	
all:make_obj make_lib

sinclude $(TOP_DIR)/build/rules.mk