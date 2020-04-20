/*
 * Copyright (C), 2019, OPPO Mobile Comm Corp., Ltd.
 * VENDOR_EDIT
 * File: - oppo_key_handle/oppo_key_handle.c
 * Description: Util about dump in the period of debugging.
 * Version: 1.0
 * Date: 2019/12/23
 * Author: Bin.Xu@BSP.Kernel.Stability
 *
 *----------------------Revision History: ---------------------------
 *   <author>        <date>         <version>         <desc>
 *    Bin.Xu       2019/11/01        1.0              created
 *-------------------------------------------------------------------
 */

#include <linux/bug.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <uapi/linux/input-event-codes.h>

#include <linux/oppo_key_handle.h>

typedef enum {
    SECURE_BOOT_OFF,
    SECURE_BOOT_ON,
    SECURE_BOOT_ON_STAGE_1,
    SECURE_BOOT_ON_STAGE_2,
    SECURE_BOOT_UNKNOWN,
} secure_type_t;

extern secure_type_t get_secureType(void);

static unsigned int vol_key_password = 0;
static unsigned long start_timer_last = 0;
static secure_type_t secure_type = SECURE_BOOT_ON_STAGE_2;

struct oppo_key_task_struct OPPO_KEY_TASKS[OPPO_KEY_NUM];

__attribute__((weak)) secure_type_t get_secureType(void)
{
 	return SECURE_BOOT_ON_STAGE_2;
}

static oppo_key_handle_t password_match()
{
	int i;
 	for(i = 0; i < OPPO_KEY_NUM; i++) {
  		if (vol_key_password == OPPO_KEY_TASKS[i].passwd) {
  			return i;
  		}
  	}
 	return OPPO_KEY_NUM;
}

static void doors_reset()
{
	int i;
	bool reset = false;
	for(i = 0; i < OPPO_KEY_NUM; i++) {
		if(OPPO_KEY_TASKS[i].open) {
			if (!reset) reset = true;
			OPPO_KEY_TASKS[i].open = 0;
			pr_info("OPPO_KEY_HANDLE, DOOR %s close.\n", OPPO_KEY_TASKS[i].name);
			(*OPPO_KEY_TASKS[i].close_process)();
		}
	}
	if (reset) vol_key_password = 0;
}

static __attribute__((unused)) int atoib10(const char *str)
{
	int num = 0;
	if (strlen(str) < 1)
	{
		return -1;
	}

	while (*str <= '9' && *str >= '0')
	{
		num = num * 10 + *str - '0';
		str++;
	}
	return num;
}

void __attribute__((unused)) oppo_key_post_process(struct input_dev *dev, int key, int val) {
	int i;
	for(i = 0; i < OPPO_KEY_NUM; i++)
		if (OPPO_KEY_TASKS[i].open) (*OPPO_KEY_TASKS[i].post_process)(dev, key, val);
}

void __attribute__((unused)) oppo_key_process(struct input_dev *dev, int key, int val)
{
	unsigned long start_timer_current;
	oppo_key_handle_t type;

	if (val) {
		if (key == KEY_POWER) {
			doors_reset();
			vol_key_password = 0;
			start_timer_last = 0;
			return;
		}

		if (key == KEY_VOLUMEUP)
			vol_key_password = (vol_key_password << 1)|0x01;

		if(key == KEY_VOLUMEDOWN)
			vol_key_password = (vol_key_password << 1)&~0x01;

		doors_reset();

		start_timer_current = jiffies;

		if(start_timer_last != 0){

			if (time_after(start_timer_current,start_timer_last + msecs_to_jiffies(2000))) {
				vol_key_password = 0;
			}
			type = password_match();
			if (type < OPPO_KEY_NUM) (*OPPO_KEY_TASKS[type].process)(dev, key, val);
		}
		start_timer_last = start_timer_current;
	}
}

void __attribute__((unused)) oppo_key_event(struct input_dev *dev, int type, int key, int val)
{
	if (type != EV_KEY) return;

	if (key != KEY_POWER && key != KEY_VOLUMEUP && key != KEY_VOLUMEDOWN) return;

	oppo_key_process(dev, key, val);

	oppo_key_post_process(dev, key, val);
}

void __attribute__((unused)) get_passwd_base(const char *val)
{
	u16 PASSWORD_BASE;
	int i;
	for(i = 0; i < OPPO_KEY_NUM; i++)
		OPPO_KEY_TASKS[i].open = 0;
	PASSWORD_BASE = (u16)atoib10(val) ^ 0x2019;
	for(i = 0; i < OPPO_KEY_NUM; i++)
		OPPO_KEY_TASKS[i].passwd = PASSWORD_BASE ^ i;
	pr_info("get_passwd_base already.\n");
}

void __attribute__((unused))
register_oppo_key_task(oppo_key_handle_t type,
						bool open, 
						u16 mask, 
						char* name,
						void *process,
						void *post_process,
						void *close_process)
{
	OPPO_KEY_TASKS[type].open          = open;
	OPPO_KEY_TASKS[type].name          = name;
	OPPO_KEY_TASKS[type].passwd        = OPPO_KEY_TASKS[type].passwd ^ type ^ mask;
	OPPO_KEY_TASKS[type].process       = process;
	OPPO_KEY_TASKS[type].post_process  = post_process;
	OPPO_KEY_TASKS[type].close_process = close_process;
}

static int __init oppo_key_handle_init(void)
{
	int i;
	secure_type = get_secureType();
	pr_info("%s- secure type %d", __func__, secure_type);
	if (secure_type != SECURE_BOOT_ON_STAGE_2) {
		for(i = 0; i < OPPO_KEY_NUM; i++)
			OPPO_KEY_TASKS[i].passwd = DEFAULT_BASE ^ i;
	}
	pr_info("oppo_key_handle_init.\n");
	return 0;
}

device_initcall(oppo_key_handle_init);