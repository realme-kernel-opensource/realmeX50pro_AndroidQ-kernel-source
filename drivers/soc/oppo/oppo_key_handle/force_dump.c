/*
 * Copyright (C), 2019, OPPO Mobile Comm Corp., Ltd.
 * VENDOR_EDIT
 * File: - oppo_key_handle/force_dump.c
 * Description: Util about dump in the period of debugging.
 * Version: 1.0
 * Date: 2019/10/01
 * Author: xiong.xin@BSP.Kernel.Stability
 *
 *----------------------Revision History: ---------------------------
 *   <author>        <date>         <version>         <desc>
 *   xiong.xin       2019/10/01        1.0              created
 *   Bin.Xu          2019/12/07        1.1              updated
 *-------------------------------------------------------------------
 */
#include <linux/oppo_key_handle.h>

#define OPPO_DUMP_MASK 0x00db
#define OPPO_DUMP_TEXT "OPPO_FORCE_DUMP"

__attribute__((weak)) void oppo_switch_fulldump(bool open)
{
	return;
}

void __attribute__((unused)) dump_process(struct input_dev *dev, int key, int val)
{
	if (OPPO_KEY_TASKS[OPPO_KEY_DUMP].open == 0) {
		pr_info("%s open \n", OPPO_DUMP_TEXT);
		oppo_switch_fulldump(true);
		OPPO_KEY_TASKS[OPPO_KEY_DUMP].open = 1;
	}
}

void __attribute__((unused)) dump_post_process(struct input_dev *dev, int key, int val)
{
	if (OPPO_KEY_TASKS[OPPO_KEY_DUMP].open) {
	// TO DO if needed
	}
}

void __attribute__((unused)) dump_close_process(void) {
	if (OPPO_KEY_TASKS[OPPO_KEY_DUMP].open == 0) {
		oppo_switch_fulldump(false);
	}
}

void __attribute__((unused)) OPPO_DUMP(const char f[24])
{
	if (OPPO_KEY_TASKS[OPPO_KEY_DUMP].open) {
		pr_err("%s: enter dump\n", f);
		BUG_ON(1);
	}
}

static int __init oppo_dump_init(void)
{
	register_oppo_key_task(OPPO_KEY_DUMP,
							false, 
							OPPO_DUMP_MASK,
							OPPO_DUMP_TEXT,
							dump_process,
							dump_post_process,
							dump_close_process);
	return 0;
}

device_initcall(oppo_dump_init);