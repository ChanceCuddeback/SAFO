/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Modified by: Chancelor Cuddeback 2021
 */


//west flash --bossac="/Users/chance/Library/Arduino15/packages/arduino/tools/bossac/1.9.1-arduino2/bossac"
//tty.usbmodem0006833434091 for DK port on MacOS

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

#include "ble_led.h"


/* Service */
#define BT_UUID_CUSTOM_SERVICE_VAL \
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)
static struct bt_uuid_128 custom_service_uuid = BT_UUID_INIT_128(
	BT_UUID_CUSTOM_SERVICE_VAL);

/* Characteristics */
static struct bt_uuid_128 custom_chrc_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef1));

/* Characteristic Values */
#define CUSTOM_MAX_LEN 20
static uint8_t custom_chrc_value[CUSTOM_MAX_LEN + 1];
/* Track notify status */
static uint8_t custom_chrc_notify = 0U;


/* Characteristic callbacks */
static ssize_t read_custom(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	const char *value = attr->user_data;
	printk("Read custom\n");
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				 CUSTOM_MAX_LEN);
}

static ssize_t write_custom(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
	printk("Write custom\n");
	uint8_t *value = attr->user_data;

	if (offset + len > CUSTOM_MAX_LEN) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);
	value[offset + len] = 0;

	return len;
}

static void custom_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	printk("CCC Changed to %i\n", value);

	custom_chrc_notify = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

/* Define Custom Service */
BT_GATT_SERVICE_DEFINE(custom_svc,
	BT_GATT_PRIMARY_SERVICE(&custom_service_uuid),
	BT_GATT_CHARACTERISTIC(&custom_chrc_uuid.uuid,
					BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY | BT_GATT_CHRC_WRITE,
					BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
					read_custom,
					write_custom,
					custom_chrc_value),
	BT_GATT_CCC(custom_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);


//Advertisement data
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_CUSTOM_SERVICE_VAL),
};

void mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx)
{
	printk("Updated MTU: TX: %d RX: %d bytes\n", tx, rx);
}

static struct bt_gatt_cb gatt_callbacks = {
	.att_mtu_updated = mtu_updated
};

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed (err 0x%02x)\n", err);
	} else {
		printk("Connected\n");
		/* Increase preffered phy */
		int err = bt_conn_le_phy_update(conn, BT_CONN_LE_PHY_PARAM_2M);
		if (err < 0) {
			printk("Phy Update Error: %i", err);
		}
		err = bt_conn_le_data_len_update(conn, BT_LE_DATA_LEN_PARAM_MAX);
		if (err < 0) {
			printk("DL Update Error: %i", err);
		}
		set_led(true, BLUE);
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason 0x%02x)\n", reason);
	set_led(false, BLUE);
}

static bool param_req(struct bt_conn *conn, struct bt_le_conn_param *param)
{
	/* Return true to accept params, after optional modifications */
	printk("Requesting min interval: %i\n", param->interval_min);
	printk("Requesting max interval: %i\n", param->interval_max);
	printk("Requesting latency: %i\n", param->latency);
	printk("Requesting timout: %i\n", param->timeout);
	return true;
}

static void param_update(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout)
{
	printk("Updated min interval: %i\n", interval);
	printk("Updated latency: %i\n", latency);
	printk("Updated timout: %i\n", timeout);
}

static void phy_updated(struct bt_conn *conn, struct bt_conn_le_phy_info *param)
{
	printk("TX Phy Updated: %i\n", param->tx_phy);
	printk("RX Phy Updated: %i\n", param->rx_phy);
}

static void data_len_updated(struct bt_conn *conn, struct bt_conn_le_data_len_info *info)
{
	/*
	uint16_t tx_max_len;
	uint16_t tx_max_time;
	uint16_t rx_max_len;
	uint16_t rx_max_time;
	*/
	printk("Updated tx max len: %i\n", info->tx_max_len);
	printk("Updated tx max time: %i\n", info->tx_max_time);
	printk("Updated rx max len: %i\n", info->rx_max_len);
	printk("Updated rx max time: %i\n", info->rx_max_time);
}

static struct bt_conn_cb conn_callbacks = {
	/* Upon Connection */
	.connected = connected,
	/* Upon disconnection */
	.disconnected = disconnected,
	/* Upon remote device conn param change request */
	.le_param_req = param_req,
	/* Upon change of conn param */
	.le_param_updated = param_update,
	/* Upon phy update */
	.le_phy_updated = phy_updated,
	/* Upon data length update */
	.le_data_len_updated = data_len_updated,
};

static void bt_ready(void)
{
	int err;

	printk("Bluetooth initialized\n");

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Advertising successfully started\n");

	set_led(true, GREEN);
	set_led(false, BLUE);
}

static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Pairing cancelled: %s\n", addr);
}

static struct bt_conn_auth_cb auth_cb_display = {
	.cancel = auth_cancel,
};

static void bt_custom_notify(uint8_t val)
{
	printk("Sending %i\n", val);

	int rc = bt_gatt_notify(NULL, &custom_svc.attrs[1], &val, sizeof(val));

	printk("Return code: %i\n", rc);
}

static void custom_notify(void)
{

	static uint8_t val = 0;

	/* Vendor Simulation */
	val++;
	if (val == 99) {
		val = 0;
	}


	bt_custom_notify(val);
}

void main(void)
{
	int err;

	config_leds();

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	bt_ready();

	bt_gatt_cb_register(&gatt_callbacks);
	bt_conn_cb_register(&conn_callbacks);
	//bt_conn_auth_cb_register(&auth_cb_display);


	/* Implement notification. At the moment there is no suitable way
	 * of starting delayed work so we do it here
	 */
	while (1) {
		k_sleep(K_SECONDS(1));

		if (custom_chrc_notify) {
			/* Vendor Notify */
			custom_notify();
		}
	}
}