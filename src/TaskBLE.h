#pragma once

#include <zephyr/kernel.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BLE_EVENT_CONNECTED,
    BLE_EVENT_DISCONNECTED,
    BLE_EVENT_ADV_STARTED,
    BLE_EVENT_ADV_STOPPED,
    BLE_EVENT_DATA_RX,
    // případně další eventy
} ble_event_type_t;

typedef struct {
    ble_event_type_t type;
    uint8_t data[32]; // nebo podle potřeby (RX buffer)
    uint16_t len;
} ble_event_t;

// API
void TaskBLE_Start(void);
int TaskBLE_SendEvent(const ble_event_t *evt);
