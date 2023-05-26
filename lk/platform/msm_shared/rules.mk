LOCAL_DIR := $(GET_LOCAL_DIR)

INCLUDES += \
			-I$(LOCAL_DIR)/include

DEFINES += $(TARGET_XRES)
DEFINES += $(TARGET_YRES)

OBJS += \
	$(LOCAL_DIR)/timer.o \
	$(LOCAL_DIR)/proc_comm.o \
	$(LOCAL_DIR)/debug.o \
	$(LOCAL_DIR)/smem.o \
	$(LOCAL_DIR)/smem_ptable.o \
	$(LOCAL_DIR)/jtag_hook.o \
	$(LOCAL_DIR)/jtag.o \
	$(LOCAL_DIR)/mmc.o\
	 $(LOCAL_DIR)/display_menu.o\
	 $(LOCAL_DIR)/pcg_basic.o\
	 $(LOCAL_DIR)/msm_i2c.o\
	 
    ifneq ($(PLATFORM),msm7200a)
        OBJS += \
        	$(LOCAL_DIR)/lcdc.o \
        	$(LOCAL_DIR)/mddi.o \
        	$(LOCAL_DIR)/menu_keys_detect.o \
        	$(LOCAL_DIR)/hsusb.o
    endif
    
    ifeq ($(PLATFORM),msm7200a)
        OBJS += \
        	$(LOCAL_DIR)/mddi_semc.o \
        	$(LOCAL_DIR)/hsusb_semc.o
    endif

ifeq ($(PLATFORM),msm8x60)
	OBJS += $(LOCAL_DIR)/mipi_dsi.o \
	        $(LOCAL_DIR)/i2c_qup.o
endif

ifeq ($(PLATFORM),msm8x60)
        OBJS += $(LOCAL_DIR)/uart_dm.o
else
        OBJS += $(LOCAL_DIR)/uart.o
endif
