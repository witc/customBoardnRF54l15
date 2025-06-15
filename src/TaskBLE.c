#include "TaskBLE.h"
#include <zephyr/logging/log.h>
#include "main.h"
#include "TaskMain.h"

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

LOG_MODULE_REGISTER(task_ble, LOG_LEVEL_INF);

#define BLE_STACK_SIZE 4096
#define BLE_PRIORITY 0
#define BLE_MSGQ_LEN 16

K_THREAD_STACK_DEFINE(ble_stack_area, BLE_STACK_SIZE);
static struct k_thread ble_thread;
K_MSGQ_DEFINE(ble_msgq, sizeof(ble_event_t), BLE_MSGQ_LEN, 1);

static struct bt_conn *default_conn=NULL;

static void Task_BLE(void *, void *, void *);
static void connected_cb(struct bt_conn *conn, uint8_t err);
static void disconnected_cb(struct bt_conn *conn, uint8_t reason);

#define DEVICE_NAME             CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN         (sizeof(DEVICE_NAME) - 1)

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

BT_CONN_CB_DEFINE(conn_callbacks) = 
{
    .connected = connected_cb,
    .disconnected = disconnected_cb,
};

int TaskBLE_SendEvent(const ble_event_t *evt)
{
    return k_msgq_put(&ble_msgq, evt, K_FOREVER);
}

// BLE callbacky
static void connected_cb(struct bt_conn *conn, uint8_t err)
{
    ble_event_t evt = 
    {
        .type = BLE_EVENT_CONNECTED
    };
    TaskBLE_SendEvent(&evt);

    if (default_conn)
    {
        bt_conn_unref(default_conn);
    }
    default_conn = bt_conn_ref(conn);

    LOG_INF("BLE Connected");
}

static void disconnected_cb(struct bt_conn *conn, uint8_t reason)
{
    ble_event_t evt = 
    {
        .type = BLE_EVENT_DISCONNECTED
    };
    TaskBLE_SendEvent(&evt);

    if (default_conn) 
    {
        bt_conn_unref(default_conn);
        default_conn = NULL;
    }

    LOG_INF("BLE Disconnected (reason: %d)", reason);
}

static int start_advertising(void)
{
    int err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), NULL, 0);
    if (!err) 
    {
        ble_event_t evt = 
        {
            .type = BLE_EVENT_ADV_STARTED
        };
        TaskBLE_SendEvent(&evt);
        LOG_INF("BLE Advertising started");
    }
    return err;
}

void TaskBLE_Start(void)
{
    k_thread_create(&ble_thread, ble_stack_area, K_THREAD_STACK_SIZEOF(ble_stack_area),
                    Task_BLE, NULL, NULL, NULL,
                    BLE_PRIORITY, 0, K_NO_WAIT);

    k_thread_name_set(&ble_thread, "TaskBLE");
}

// Vlastní BLE task
static void Task_BLE(void *a, void *b, void *c)
{
    int err;

    err = bt_enable(NULL);
    if (err) 
    {
        LOG_ERR("BLE stack init failed (%d)", err);
        return;
    }
    
    start_advertising();
    ble_event_t evt;
    while (1)
    {
        if (k_msgq_get(&ble_msgq, &evt, K_FOREVER) == 0) 
        {
       
        }
    }
}
