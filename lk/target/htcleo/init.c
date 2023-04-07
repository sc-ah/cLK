/*
 * author: cedesmith
 * license: GPL
 */

#include <string.h>
#include <debug.h>
#include <dev/keys.h>
#include <dev/gpio_keypad.h>
#include <lib/ptable.h>
#include <dev/flash.h>
#include <smem.h>
#include <platform/iomap.h>
#include <reg.h>
#include <stdio.h>
#include <stdlib.h>
// #include <target/microp.h>
// #include <target/board_htcleo.h>
// #include <platform/gpio.h>
// #include <msm_i2c.h>
// #include <target/clock.h>
// #include <platform/irqs.h>
// #include <pcom.h>

#include "version.h"
#include <pcg_basic.h>

#define LINUX_MACHTYPE  2524
#define HTCLEO_FLASH_OFFSET	0x219
#define OUTPUT_LEN = 10;

struct ptable flash_ptable;

// align data on a 512 boundary so will not be interrupted in nbh
static struct ptentry board_part_list[MAX_PTABLE_PARTS] __attribute__ ((aligned (512))) = {
		{
				.name = "PTABLE-MB", // PTABLE-BLK or PTABLE-MB for length in MB or BLOCKS
		},
		{
				.name = "misc",
				.length = 1 /* In MB */,
		},
		{
				.name = "recovery",
				.length = 64 /* In MB */,
		},
		{
				.name = "uefi",
				.length = 20 /* In MB */,
		},
		{
				.name = "boot",
				.length = 5 /* In MB */,
		},
		{
				.name = "system",
				.length = SYSTEM_PARTITION_SIZE /* In MB */,
		},
		{
				.length = CACHE_PARTITION_SIZE /* In MB */,
				.name = "cache",
		},
		{
				.name = "userdata",
		},
};

// /*******************************************************************************
//  * i2C
//  ******************************************************************************/
// static void msm_set_i2c_mux(int mux_to_i2c) {
// 	if (mux_to_i2c) {
// 		pcom_gpio_tlmm_config(MSM_GPIO_CFG(GPIO_I2C_CLK, 0, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_NO_PULL, MSM_GPIO_CFG_8MA), 0);
// 		pcom_gpio_tlmm_config(MSM_GPIO_CFG(GPIO_I2C_DAT, 0, MSM_GPIO_CFG_OUTPUT, MSM_GPIO_CFG_NO_PULL, MSM_GPIO_CFG_8MA), 0);
// 	} else {
// 		pcom_gpio_tlmm_config(MSM_GPIO_CFG(GPIO_I2C_CLK, 1, MSM_GPIO_CFG_INPUT, MSM_GPIO_CFG_NO_PULL, MSM_GPIO_CFG_2MA), 0);
// 		pcom_gpio_tlmm_config(MSM_GPIO_CFG(GPIO_I2C_DAT, 1, MSM_GPIO_CFG_INPUT, MSM_GPIO_CFG_NO_PULL, MSM_GPIO_CFG_2MA), 0);
// 	}
// }

// static struct msm_i2c_pdata i2c_pdata = {
// 	.i2c_clock = 400000,
// 	.clk_nr	= I2C_CLK,
// 	.irq_nr = INT_PWB_I2C,
// 	.scl_gpio = GPIO_I2C_CLK,
// 	.sda_gpio = GPIO_I2C_DAT,
// 	.set_mux_to_i2c = &msm_set_i2c_mux,
// 	.i2c_base = (void*)MSM_I2C_BASE,
// };

// void msm_i2c_init(void)
// {
// 	msm_i2c_probe(&i2c_pdata);
// }

// /*******************************************************************************
//  * MicroP
//  ******************************************************************************/
// int msm_microp_i2c_status = 0;
// static struct microp_platform_data microp_pdata = {
// 	.chip = MICROP_I2C_ADDR,
// 	.gpio_reset = HTCLEO_GPIO_UP_RESET_N,
// };

// void msm_microp_i2c_init(void)
// {
// 	microp_i2c_probe(&microp_pdata);
// }

static unsigned num_parts = sizeof(board_part_list)/sizeof(struct ptentry);
//#define part_empty(p) (p->name[0]==0 && p->start==0 && p->length==0 && p->flags==0 && p->type==0 && p->perm==0)
#define IS_PART_EMPTY(p) (p->name[0]==0)

extern unsigned load_address;
extern unsigned boot_into_recovery;

void keypad_init(void);
void display_init(void);
void display_lk_version();
void htcleo_ptable_dump(struct ptable *ptable);
void cmd_dmesg(const char *arg, void *data, unsigned sz);
void reboot(unsigned reboot_reason);
void target_display_init();
unsigned get_boot_reason(void);
void cmd_oem_register();

void target_init(void)
{
	struct flash_info *flash_info;
	unsigned start_block;
	unsigned blocks_per_plen = 1; //blocks per partition length
	unsigned nand_num_blocks;

	keys_init();
	keypad_init();
	// msm_i2c_init();
	// msm_microp_i2c_init();

	uint16_t keys[] = {
			KEY_VOLUMEUP,
			KEY_VOLUMEDOWN,
			KEY_SOFT1,
			KEY_SEND,
			KEY_CLEAR,
			KEY_BACK,
			KEY_HOME
	};

	for(unsigned i = 0; i < sizeof(keys)/sizeof(uint16_t); i++)
		if (keys_get_state(keys[i]) != 0) {
			display_init();
			//display_lk_version();
			break;
		}
	dprintf(INFO, "htcleo_init\n");

	if(get_boot_reason() == 2) { // booting for offmode charging, start recovery so kernel will charge phone
		boot_into_recovery = 1;
		//dprintf(INFO, "reboot needed... \n");
		//reboot(0);
	}
	dprintf(ALWAYS, "load address %x\n", load_address);

	dprintf(INFO, "flash init\n");
	flash_init();
	flash_info = flash_get_info();
	ASSERT(flash_info);
	ASSERT(flash_info->num_blocks);
	nand_num_blocks = flash_info->num_blocks;

	ptable_init(&flash_ptable);

	if( strcmp(board_part_list[0].name,"PTABLE-BLK")==0 )
		blocks_per_plen =1 ;
	else if( strcmp(board_part_list[0].name,"PTABLE-MB")==0 )
		blocks_per_plen = (1024*1024)/flash_info->block_size;
	else
		panic("Invalid partition table\n");

	start_block = HTCLEO_FLASH_OFFSET;
	for (unsigned i = 1; i < num_parts; i++) {
		struct ptentry *ptn = &board_part_list[i];
		if( IS_PART_EMPTY(ptn) )
			break;
		int len = ((ptn->length) * blocks_per_plen);

		if( ptn->start == 0 )
			ptn->start = start_block;
		else if( ptn->start < start_block)
			panic("Partition %s start %x < %x\n", ptn->name, ptn->start, start_block);

		if(ptn->length == 0) {
			unsigned length_for_prt = 0;
			if( i<num_parts && !IS_PART_EMPTY((&board_part_list[i+1]))
					&& board_part_list[i+1].start!=0)
			{
				length_for_prt =  board_part_list[i+1].start - ptn->start;
			}
			else
			{
				for (unsigned j = i+1; j < num_parts; j++) {
						struct ptentry *temp_ptn = &board_part_list[j];
						if( IS_PART_EMPTY(temp_ptn) ) break;
						if( temp_ptn->length==0 ) panic("partition %s and %s have variable length\n", ptn->name, temp_ptn->name);
						length_for_prt += ((temp_ptn->length) * blocks_per_plen);
				}
			}
			len = (nand_num_blocks - 1 - 186 - 4) - (ptn->start + length_for_prt);
			ASSERT(len >= 0);
		}

		start_block = ptn->start + len;
		ptable_add(&flash_ptable, ptn->name, ptn->start, len, ptn->flags,
				TYPE_APPS_PARTITION, PERM_WRITEABLE);
	}

	htcleo_ptable_dump(&flash_ptable);
	flash_set_ptable(&flash_ptable);
}
void display_lk_version()
{
    _dputs("cedesmith's LK (CLK) v" CLK_VERSION "\n");
}

#define FNV_PRIME_32 16777619
#define FNV_OFFSET_32 2166136261U

uint32_t fnv1a_32(const char* str, size_t len) {
    uint32_t hash = FNV_OFFSET_32;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint32_t)str[i];
        hash *= FNV_PRIME_32;
    }
    return hash;
}

char* generate_serial_from_cid(const char* input) {
    char* result = malloc(12 * sizeof(char));
    pcg32_random_t rng;
    uint32_t seed = fnv1a_32(input, strlen(input));
    pcg32_srandom_r(&rng, seed, 0);
    const char* chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int j = 0;
    for (int i = 0; i < 11; i++) {
        char c;
        if (j < strlen(input) && (input[j] == '-' || input[j] == '_')) {
            j++;
        }
        if (j < strlen(input)) {
            c = input[j];
            j++;
        } else {
            c = chars[pcg32_boundedrand_r(&rng, 36)];
        }
        result[i] = c;
    }
    result[11] = '\0';
    if (strstr(result, "_")) {
        for (int i = 0; i < 11; i++) {
            if (result[i] == '_') {
                result[i] = chars[pcg32_boundedrand_r(&rng, 36)];
                break;
            }
        }
    }
    return result;
}

struct fbcon_config* fbcon_display(void);
void htcleo_fastboot_init()
{
	// off charge and recovery boot failed, reboot to normal mode
	if(get_boot_reason() == 2)
		reboot(0);

	// display not initialized
	if(fbcon_display() == NULL) {
		display_init();
		//display_lk_version();
		htcleo_ptable_dump(&flash_ptable);
	}

	cmd_oem_register();
}
void target_early_init(void)
{
	//cedesmith: write reset vector while we can as MPU kicks in after flash_init();
	writel(0xe3a00546, 0); //mov r0, #0x11800000
	writel(0xe590f004, 4); //ldr	r15, [r0, #4]
}
unsigned board_machtype(void)
{
    return LINUX_MACHTYPE;
}

void reboot_device(unsigned reboot_reason)
{
	writel(reboot_reason, 0x2FFB0000);
	writel(reboot_reason^0x004b4c63, 0x2FFB0004); //XOR with cLK signature
    reboot(reboot_reason);
}

unsigned boot_reason = 0xFFFFFFFF;
unsigned android_reboot_reason = 0;
unsigned check_reboot_mode(void);
unsigned get_boot_reason(void)
{
	if(boot_reason==0xFFFFFFFF) {
		boot_reason = readl(MSM_SHARED_BASE+0xef244);
		dprintf(INFO, "boot reason %x\n", boot_reason);
		if(boot_reason!=2) {
			if(readl(0x2FFB0000)==(readl(0x2FFB0004)^0x004b4c63)) {
				android_reboot_reason = readl(0x2FFB0000);
				dprintf(INFO, "android reboot reason %x\n", android_reboot_reason);
				writel(0, 0x2FFB0000);
			}
		}
	}
	return boot_reason;
}
unsigned check_reboot_mode(void)
{
	get_boot_reason();
	return android_reboot_reason;
}

unsigned target_pause_for_battery_charge(void)
{
    if (get_boot_reason() == 2) return 1;
    return 0;
}

int target_is_emmc_boot(void)
{
	return 0;
}

void htcleo_ptable_dump(struct ptable *ptable)
{
	struct ptentry *ptn;
	int i;

	for (i = 0; i < ptable->count; ++i) {
		ptn = &ptable->parts[i];
		dprintf(INFO, "ptn %d name='%s' start=%08x len=%08x end=%08x \n",i, ptn->name, ptn->start, ptn->length,  ptn->start+ptn->length);
	}
}

//cedesmith: current version of qsd8k platform missing display_shutdown so add it
void lcdc_shutdown(void);
void display_shutdown(void)
{
    lcdc_shutdown();
}
