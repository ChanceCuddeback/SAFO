/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Modified by: Chancelor Cuddeback 2021
 */


//west flash --bossac="/Users/chance/Library/Arduino15/packages/arduino/tools/bossac/1.9.1-arduino2/bossac"

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>
#include <bluetooth/services/hrs.h>

#include "ble_led.h"

#define BT_UUID_CUSTOM_VAL 0x1280a
#define BT_UUID_CUSTOM \
	BT_UUID_DECLARE_16(BT_UUID_DIS_VAL)

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL,
		      BT_UUID_16_ENCODE(BT_UUID_BAS_VAL),
			  BT_UUID_16_ENCODE(BT_UUID_CUSTOM_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_DIS_VAL))
};

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed (err 0x%02x)\n", err);
	} else {
		printk("Connected\n");
	}
	set_led(false, GREEN);
	set_led(true, BLUE);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason 0x%02x)\n", reason);
	set_led(false, BLUE);
	set_led(true, GREEN);
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

static void bt_ready(void)
{
	int err;

	printk("Bluetooth initialized\n");

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		set_led(true, RED);
		return;
	}
	set_led(true, GREEN);
	printk("Advertising successfully started\n");
}

static void bas_notify(void)
{
	uint8_t battery_level = bt_bas_get_battery_level();

	battery_level--;

	if (!battery_level) {
		battery_level = 100U;
	}

	bt_bas_set_battery_level(battery_level);
}

void main(void)
{
	int err;
	err = config_leds();
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		set_led(true, RED);
		return;
	}

	bt_ready();

	bt_conn_cb_register(&conn_callbacks);

	while (1) {
		k_sleep(K_SECONDS(1));

		/* Battery level simulation */
		bas_notify();
	}
}