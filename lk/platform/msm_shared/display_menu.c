/* Copyright (c) 2015-2018, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*	notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*	copyright notice, this list of conditions and the following
*	disclaimer in the documentation and/or other materials provided
*	with the distribution.
*     * Neither the name of The Linux Foundation nor the names of its
*	contributors may be used to endorse or promote products derived
*	from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <debug.h>
#include <reg.h>
#include <stdlib.h>
#include <dev/fbcon.h>
#include <kernel/thread.h>
#include <display_menu.h>
#include <menu_keys_detect.h>
#include <string.h>
#include <platform.h>
#include <smem.h>
#include <target.h>
#include <sys/types.h>
#include <../../../app/aboot/recovery.h>
#include <version.h>

enum unlock_type {
	UNLOCK = 0,
	UNLOCK_CRITICAL,
};

#ifndef TRUE
#define TRUE true
#endif
#ifndef FALSE
#define FALSE false
#endif

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

#define TITLE_MSG "<!>\n\n"

static const char *unlock_menu_common_msg = "By unlocking the bootloader, you will be able to install "\
				"custom operating system on this phone. "\
				"A custom OS is not subject to the same level of testing "\
				"as the original OS, and can cause your phone "\
				"and installed applications to stop working properly.\n\n"\
				"Software integrity cannot be guaranteed with a custom OS, "\
				"so any data stored on the phone while the bootloader "\
				"is unlocked may be at risk.\n\n"\
				"To prevent unauthorized access to your personal data, "\
				"unlocking the bootloader will also delete all personal "\
				"data on your phone.\n\n"\
				"Press the Volume keys to select whether to unlock the bootloader, "\
				"then the Power Button to continue.\n\n";

static const char *lock_menu_common_msg = "If you lock the bootloader, "\
				"you will not be able to install "\
				"custom operating system on this phone.\n\n"\
				"To prevent unauthorized access to your personal data, "\
				"locking the bootloader will also delete all personal "\
				"data on your phone.\n\n"\
				"Press the Volume keys to select whether to "\
				"lock the bootloader, then the power button to continue.\n\n";

#define YELLOW_WARNING_MSG	"Your device has loaded a different operating system\n\n "\
				"Visit this link on another device:\n"

#define ORANGE_WARNING_MSG	"The boot loader is unlocked and software integrity cannot "\
				"be guaranteed. Any data stored on the device may be available to attackers. "\
				"Do not store any sensitive data on the device.\n\n"\
				"Visit this link on another device:\n"

#define RED_WARNING_MSG	"Your device is corrupt. It can't be\ntrusted and will not boot\n\n" \
				"Visit this link on another device:\n"

#define LOGGING_WARNING_MSG	"The dm-verity is not started in enforcing mode and may "\
				"not work properly\n\nTo learn more, visit:\n"

#define EIO_WARNING_MSG		"Your device is corrupt. It can't be\n trusted and may not work properly.\n\n"\
				"Visit this link on another device:\n"

#define PAUSE_BOOT_MSG "PRESS POWER KEY TO PAUSE BOOT\n\n"

#define CONTINUE_BOOT_MSG "PRESS POWER KEY TO CONTINUE\n\n"

#define URL_MSG "g.co/ABH\n\n"

#define DELAY_5SEC 5000
#define DELAY_10SEC 10000
#define DELAY_30SEC 30000

static bool is_thread_start = false;
static struct select_msg_info msg_info;
uint32_t optionindex = -1;

static char *verify_option_menu[] = {
		[POWEROFF] = "Power off\n",
		[RESTART] = "Restart\n",
		[RECOVER] = "Recovery\n",
		[FASTBOOT] = "Fastboot\n",
		[BACK] = "Back to previous page\n"};

static char *fastboot_option_menu[] = {
		[0] = "START\n",
		[1] = "Restart bootloader\n",
		[2] = "Recovery mode\n",
		[3] = "Power off\n",
#if !ABOOT_STANDALONE
		[4] = "Boot to UEFI\n"
#endif
};

static struct unlock_info munlock_info[] = {
		[DISPLAY_MENU_LOCK] = {UNLOCK, FALSE},
		[DISPLAY_MENU_UNLOCK] = {UNLOCK, TRUE},
		[DISPLAY_MENU_LOCK_CRITICAL] = {UNLOCK_CRITICAL, FALSE},
		[DISPLAY_MENU_UNLOCK_CRITICAL] = {UNLOCK_CRITICAL, TRUE},
};

struct unlock_option_msg munlock_option_msg[] = {
		[TRUE] = {"DO NOT UNLOCK THE BOOTLOADER \n", "UNLOCK THE BOOTLOADER \n"},
		[FALSE] = {"DO NOT LOCK THE BOOTLOADER \n", "LOCK THE BOOTLOADER \n"},
};

static int big_factor = 2;
static int common_factor = 1;
bool buttonthreadrunning = false;

char charVal(char c)
{
	c&=0xf;
	if(c<=9) return '0'+c;
	
	return 'A'+(c-10);
}

int str2u(const char *x)
{
	while(*x==' ')x++;

	unsigned base=10;
	int sign=1;
    unsigned n = 0;

    if(strstr(x,"-")==x) { sign=-1; x++;};
    if(strstr(x,"0x")==x) {base=16; x+=2;}

    while(*x) {
    	char d=0;
        switch(*x) {
        	case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            	d = *x - '0';
            	break;
        	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
            	d = (*x - 'a' + 10);
            	break;
       		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
				d = (*x - 'A' + 10);
				break;
        	default:
				return sign*n;
        }
        if(d>=base) return sign*n;
        n*=base;n+=d;
        x++;
    }
	
	return sign*n;
}

void dump_mem_to_buf(char* start, int len, char *buf)
{
	while(len>0) {
		int slen = len > 29 ? 29 : len;
		for(int i=0; i<slen; i++) {
			buf[i*2] = charVal(start[i]>>4);
			buf[i*2+1]= charVal(start[i]);
		}
		buf[slen*2+1]=0;
		start+=slen;
		len-=slen;
	}
}

void hex2ascii(char *hexstr, char *asciistr)
{
	int newval;
	for (int i = 0; i < (int)(strlen(hexstr) - 1); i+=2) {
		int firstvalue = hexstr[i] - '0';
		int secondvalue;
		switch(hexstr[i+1]) {
			case 'A': case 'a':
				secondvalue = 10;
				break;
			case 'B': case 'b':
				secondvalue = 11;
				break;
			case 'C': case 'c':
				secondvalue = 12;
				break;
			case 'D': case 'd':
				secondvalue = 13;
				break;
			case 'E': case 'e':
				secondvalue = 14;
				break;
			case 'F': case 'f':
				secondvalue = 15;
				break;
			default: 
				secondvalue = hexstr[i+1] - '0';
				break;
		}
		newval = ((16 * firstvalue) + secondvalue);
		*asciistr = (char)(newval);
		asciistr++;
	}
}


/*
 * koko : Get device IMEI
 */
char device_imei[64];
void update_device_imei(void)
{
	char imeiBuffer[64];
	dump_mem_to_buf((char*)str2u("0x1EF260"), str2u("0xF"), imeiBuffer);

	hex2ascii(imeiBuffer, device_imei);
	
	return;
}

/*
 * koko : Get device CID
 */
char device_cid[24];
void update_device_cid(void)
{
	char cidBuffer[24];
	dump_mem_to_buf((char*)str2u("0x1EF270"), str2u("0x8"), cidBuffer);

	hex2ascii(cidBuffer, device_cid);

	return;
}

/*
 * koko : Get radio version
 */
char radio_version[25];
char radBuffer[25];
void update_radio_ver(void)
{
	while(radBuffer[0] != '3')
		dump_mem_to_buf((char*)str2u("0x1EF220"), str2u("0xA"), radBuffer);

	hex2ascii(radBuffer, radio_version);

	return;
}

/*
 * koko : Get spl version
 */
char spl_version[24];
void update_spl_ver(void)
{
	char splBuffer[24];
	dump_mem_to_buf((char*)str2u("0x1004"), str2u("0x9"), splBuffer);

	hex2ascii(splBuffer, spl_version);

	return;
}

/*
 * koko : Get device BT Mac-Addr
 */
char device_btmac[24];
void update_device_btmac(void)
{
	char btmacBuffer[24];
	dump_mem_to_buf((char*)str2u("0x82108"), str2u("0x6"), btmacBuffer);

	sprintf( device_btmac, "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c",
			btmacBuffer[10], btmacBuffer[11], btmacBuffer[8], btmacBuffer[9],
			btmacBuffer[6], btmacBuffer[7], btmacBuffer[4], btmacBuffer[5],
			btmacBuffer[2], btmacBuffer[3], btmacBuffer[0], btmacBuffer[1] );
	
	return;
}

/*
 * koko : Get device WiFi Mac-Addr
 */
char device_wifimac[24];
void update_device_wifimac(void)
{	/*
	 * koko: If we install LK right after
	 *		 having flashed a WM ROM the
	 *		 wifi mac addr is read from '0xFC028', 
	 *		 BUT after flashing anything else
	 *		 we get only FFs. (?)
	 */
	char wifimacBuffer[24];
	dump_mem_to_buf((char*)str2u("0xFC028"), str2u("0x6"), wifimacBuffer);

	sprintf( device_wifimac, "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c",
			wifimacBuffer[0], wifimacBuffer[1], wifimacBuffer[2], wifimacBuffer[3],
			wifimacBuffer[4], wifimacBuffer[5], wifimacBuffer[6], wifimacBuffer[7],
			wifimacBuffer[8], wifimacBuffer[9], wifimacBuffer[10], wifimacBuffer[11] );
	
	return;
}

static int update_ver_str(void *arg)
{
	update_spl_ver();
	update_radio_ver();
	update_device_imei();
	update_device_cid();
	update_device_wifimac();
	update_device_btmac();

	thread_exit(0);
	return 0;
}
static void wait_for_exit(void)
{
	struct select_msg_info *select_msg;
	select_msg = &msg_info;

	mutex_acquire(&select_msg->msg_lock);
	while(!select_msg->info.rel_exit) {
		mutex_release(&select_msg->msg_lock);
		thread_sleep(10);
		mutex_acquire(&select_msg->msg_lock);
	}
	mutex_release(&select_msg->msg_lock);

	is_thread_start = false;
	fbcon_clear();
	display_image_on_screen();
}

void wait_for_users_action(void)
{
	/* Waiting for exit menu keys detection if there is no any usr action
	 * otherwise it will do the action base on the keys detection thread
	 */
	wait_for_exit();
}

void exit_menu_keys_detection(void)
{
	struct select_msg_info *select_msg;
	select_msg = &msg_info;

	mutex_acquire(&select_msg->msg_lock);
	select_msg->info.is_exit = true;
	mutex_release(&select_msg->msg_lock);

	wait_for_exit();
}

static void set_message_factor(void)
{
	uint32_t tmp_factor = 0;
	uint32_t max_x_count = 0;
	uint32_t max_x = 0;

	if (fbcon_get_width() < fbcon_get_height())
		max_x_count = CHAR_NUM_PERROW_POR;
	else
		max_x_count = CHAR_NUM_PERROW_HOR;

	max_x = fbcon_get_max_x();
	tmp_factor = (max_x+max_x_count/2)/max_x_count;

	if(tmp_factor <= 1) {
		big_factor = 2;
		common_factor = 1;
	} else {
		big_factor = tmp_factor*2;
		common_factor = tmp_factor;
	}
}

/*  If y_start == 0, it will draw message from the current Y axis location,
    otherwise it will start from the the value of y_start. */
static void display_fbcon_menu_message(char *str, unsigned type,
	unsigned scale_factor, int y_start)
{
	while(*str != 0) {
		fbcon_putc_factor(*str++, type, scale_factor, y_start);
		y_start = 0;
	}
}

static char *str_align_right(char *str, int factor)
{
	uint32_t max_x = 0;
	int diff = 0;
	int i = 0;
	char *str_target = NULL;

	max_x = fbcon_get_max_x();
	if (!str_target && max_x) {
		str_target = malloc(max_x);
	}

	if (str_target) {
		memset(str_target, 0, max_x);
		if ( max_x/factor > strlen(str)) {
			if (factor == 1)
				diff = max_x/factor - strlen(str) - 1;
			else
				diff = max_x/factor - strlen(str);
			for (i = 0; i < diff; i++) {
				strlcat(str_target, " ", max_x);
			}
		}
		strlcat(str_target, str, max_x);
	}
	return str_target;
}

/**
  Reset device unlock status
  @param[in] Type    The type of the unlock.
                     [DISPLAY_MENU_UNLOCK]: unlock the device
                     [DISPLAY_MENU_UNLOCK_CRITICAL]: critical unlock the device
                     [DISPLAY_MENU_LOCK]: lock the device
                     [DISPLAY_MENU_LOCK_CRITICAL]: critical lock the device
 **/
void reset_device_unlock_status (int type)
{
	struct recovery_message msg;

	if (type == DISPLAY_MENU_LOCK ||
		type == DISPLAY_MENU_UNLOCK ||
		type == DISPLAY_MENU_LOCK_CRITICAL ||
		type == DISPLAY_MENU_UNLOCK_CRITICAL) {
		set_device_unlock_value (munlock_info[type].unlock_type,
				munlock_info[type].unlock_value);
		memset(&msg, 0, sizeof(msg));
		snprintf(msg.recovery, sizeof(msg.recovery), "recovery\n--wipe_data");
		write_misc(0, &msg, sizeof(msg));
	}
}

/* msg_lock need to be holded when call this function. */
static void display_unlock_menu_renew(struct select_msg_info *unlock_msg_info,
                                      int type, bool status)
{
	fbcon_clear();
	memset(&unlock_msg_info->info, 0, sizeof(struct menu_info));

	display_fbcon_menu_message("<!>\n\n",
		FBCON_UNLOCK_TITLE_MSG, big_factor, 0);

	if (status) {
		display_fbcon_menu_message((char*)unlock_menu_common_msg,
			FBCON_COMMON_MSG, common_factor, 0);
	} else {
		display_fbcon_menu_message((char*)lock_menu_common_msg,
			FBCON_COMMON_MSG, common_factor, 0);
	}

	fbcon_draw_line(FBCON_COMMON_MSG);
	unlock_msg_info->info.option_start[0] = fbcon_get_current_line();
	display_fbcon_menu_message((char *)munlock_option_msg[status].ignore_msg,
                               FBCON_COMMON_MSG, common_factor, 0);
	unlock_msg_info->info.option_bg[0] = fbcon_get_current_bg();
	unlock_msg_info->info.option_end[0] = fbcon_get_current_line();
	fbcon_draw_line(FBCON_COMMON_MSG);
	unlock_msg_info->info.option_start[1] = fbcon_get_current_line();
	display_fbcon_menu_message((char *)munlock_option_msg[status].comfirm_msg,
                               FBCON_COMMON_MSG, common_factor, 0);
	unlock_msg_info->info.option_bg[1] = fbcon_get_current_bg();
	unlock_msg_info->info.option_end[1] = fbcon_get_current_line();
	fbcon_draw_line(FBCON_COMMON_MSG);

	if (type == UNLOCK) {
		unlock_msg_info->info.msg_type = DISPLAY_MENU_UNLOCK;
		if (!status)
			unlock_msg_info->info.msg_type = DISPLAY_MENU_LOCK;
	} else if (type == UNLOCK_CRITICAL) {
		unlock_msg_info->info.msg_type = DISPLAY_MENU_UNLOCK_CRITICAL;
		if (!status)
			unlock_msg_info->info.msg_type = DISPLAY_MENU_LOCK_CRITICAL;
	}

	unlock_msg_info->info.option_num = 2;

	/* Initialize the option index */
	unlock_msg_info->info.option_index= 2;
}

/* msg_lock need to be holded when call this function. */
void display_bootverify_option_menu_renew(struct select_msg_info *msg_info)
{
	int i = 0;
	int len = 0;

	fbcon_clear();
	memset(&msg_info->info, 0, sizeof(struct menu_info));

	len = ARRAY_SIZE(verify_option_menu);
	display_fbcon_menu_message("Options menu:\n\n",
		FBCON_COMMON_MSG, big_factor, 0);
	display_fbcon_menu_message("Press volume key to select, and "\
		"press power key to select\n\n", FBCON_COMMON_MSG, common_factor, 0);

	for (i = 0; i < len; i++) {
		fbcon_draw_line(FBCON_COMMON_MSG);
		msg_info->info.option_start[i] = fbcon_get_current_line();
		display_fbcon_menu_message(verify_option_menu[i],
			FBCON_COMMON_MSG, common_factor, 0);
		msg_info->info.option_bg[i]= fbcon_get_current_bg();
		msg_info->info.option_end[i]= fbcon_get_current_line();
	}

	fbcon_draw_line(FBCON_COMMON_MSG);
	msg_info->info.msg_type = DISPLAY_MENU_MORE_OPTION;
	msg_info->info.option_num = len;

	/* Initialize the option index */
	msg_info->info.option_index= len;
}

/* msg_lock need to be holded when call this function. */
void display_fastboot_menu_renew(struct select_msg_info *fastboot_msg_info)
{
	int len;
	int msg_type = FBCON_COMMON_MSG;
	char msg_buf[64];
	char msg[128];

	/* The fastboot menu is switched base on the option index
	 * So it's need to store the index for the menu switching
	 */
	uint32_t option_index = fastboot_msg_info->info.option_index;

	fbcon_clear();
	memset(&fastboot_msg_info->info, 0, sizeof(struct menu_info));

	len = ARRAY_SIZE(fastboot_option_menu);
	switch(option_index) {
		case 0:
			msg_type = FBCON_GREEN_MSG;
			break;
		case 1:
		case 2:
			msg_type = FBCON_RED_MSG;
			break;
		case 3:
		case 4:
			msg_type = FBCON_COMMON_MSG;
			break;
	}
	fbcon_draw_line(msg_type);

	display_fbcon_menu_message(fastboot_option_menu[option_index],
		msg_type, big_factor, 0);
	fbcon_draw_line(msg_type);
	display_fbcon_menu_message("\n\nUse keys to navigate\n\n", FBCON_COMMON_MSG, common_factor, 0);

	display_fbcon_menu_message("FASTBOOT MODE\n", FBCON_RED_MSG, common_factor, 0);

	snprintf(msg, sizeof(msg), "PRODUCT_NAME - %s\n", "Htc Leo");
	display_fbcon_menu_message(msg, FBCON_COMMON_MSG, common_factor, 0);

	// memset(msg_buf, 0, sizeof(msg_buf));
	// smem_get_hw_platform_name((unsigned char *) msg_buf, sizeof(msg_buf));
	// snprintf(msg, sizeof(msg), "VARIANT - %s %s\n",
	// 	msg_buf, target_is_emmc_boot()? "eMMC":"UFS");
	// display_fbcon_menu_message(msg, FBCON_COMMON_MSG, common_factor, 0);
	
	//replace with clk version here

	memset(msg_buf, 0, sizeof(msg_buf));
	//get_bootloader_version((unsigned char *) msg_buf);
	snprintf(msg, sizeof(msg), "BOOTLOADER VERSION - %s\n",
		"CLK-"CLK_VERSION);
	display_fbcon_menu_message(msg, FBCON_COMMON_MSG, common_factor, 0);


	memset(msg_buf, 0, sizeof(msg_buf));
	//get_bootloader_version((unsigned char *) msg_buf);
	snprintf(msg, sizeof(msg), "SPL VERSION - %s\n",
		spl_version);
	display_fbcon_menu_message(msg, FBCON_COMMON_MSG, common_factor, 0);
	

	 memset(msg_buf, 0, sizeof(msg_buf));
	// get_baseband_version((unsigned char *) msg_buf);
	 snprintf(msg, sizeof(msg), "BASEBAND VERSION - %s\n",
	 	radio_version);
	 display_fbcon_menu_message(msg, FBCON_COMMON_MSG, common_factor, 0);

	memset(msg_buf, 0, sizeof(msg_buf));
	//target_serialno((unsigned char *) msg_buf);
	snprintf(msg, sizeof(msg), "SERIAL NUMBER - %s\n", "#123456A");
	display_fbcon_menu_message(msg, FBCON_COMMON_MSG, common_factor, 0);

	snprintf(msg, sizeof(msg), "SECURE BOOT - %s\n","disabled");
	display_fbcon_menu_message(msg, FBCON_COMMON_MSG, common_factor, 0);

	snprintf(msg, sizeof(msg), "DEVICE STATE - %s\n","unlocked");
	display_fbcon_menu_message(msg, FBCON_RED_MSG, common_factor, 0);

	fastboot_msg_info->info.msg_type = DISPLAY_MENU_FASTBOOT;
	fastboot_msg_info->info.option_num = len;
	fastboot_msg_info->info.option_index = option_index;
}

void msg_lock_init(void)
{
	static bool is_msg_lock_init = false;
	struct select_msg_info *msg_lock_info;
	msg_lock_info = &msg_info;

	if (!is_msg_lock_init) {
		mutex_init(&msg_lock_info->msg_lock);
		is_msg_lock_init = true;
	}
}

extern int ui_key_listener_thread(void *param);

static void display_menu_thread_start(struct select_msg_info *msg_info)
{
	thread_t *thr;
	if (!is_thread_start) {
		thread_create("key_listener", &ui_key_listener_thread, (void*)msg_info, DEFAULT_PRIORITY, 4096);
		if (!thr) {
			dprintf(CRITICAL, "ERROR: create device status detect thread failed!!\n");
			return;
		}
		thread_resume(thr);
		is_thread_start = true;
		dprintfr(INFO, "index2 is at %u \n", optionindex);
	}
}

/* The fuction be called after device in fastboot mode,
 * so it's no need to initialize the msg_lock again
 */
void display_unlock_menu(int type, bool status)
{
	struct select_msg_info *unlock_menu_msg_info;
	unlock_menu_msg_info = &msg_info;


	set_message_factor();

	msg_lock_init();
	mutex_acquire(&unlock_menu_msg_info->msg_lock);

	/* Initialize the last_msg_type */
	unlock_menu_msg_info->last_msg_type =
		unlock_menu_msg_info->info.msg_type;

	display_unlock_menu_renew(unlock_menu_msg_info, type, status);
	mutex_release(&unlock_menu_msg_info->msg_lock);

	dprintf(INFO, "creating %s keys detect thread\n",
		status ? "unlock":"lock");
	display_menu_thread_start(unlock_menu_msg_info);
}

void display_fastboot_menu(uint32_t menuindex)
{
	struct select_msg_info *fastboot_menu_msg_info;
	fastboot_menu_msg_info = &msg_info;
				thread_resume(thread_create("update_nand_info",
										&update_ver_str,
										NULL,
										HIGH_PRIORITY,
										DEFAULT_STACK_SIZE));

	set_message_factor();

	msg_lock_init();
	mutex_acquire(&fastboot_menu_msg_info->msg_lock);

	/* There are 4 pages for fastboot menu:
	 * Page: Start/Fastboot/Recovery/Poweroff
	 * The menu is switched base on the option index
	 * Initialize the option index and last_msg_type
	 */
	fastboot_menu_msg_info->info.option_index = menuindex;
	fastboot_menu_msg_info->last_msg_type =
		fastboot_menu_msg_info->info.msg_type;

	display_fastboot_menu_renew(fastboot_menu_msg_info);
	mutex_release(&fastboot_menu_msg_info->msg_lock);

	dprintf(INFO, "creating fastboot menu keys detect thread\n");
	optionindex = fastboot_menu_msg_info->info.option_index;
	if (!buttonthreadrunning){
	display_menu_thread_start(fastboot_menu_msg_info );
	}
	buttonthreadrunning = true;
}
