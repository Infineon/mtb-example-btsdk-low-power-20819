/*******************************************************************************
* Copyright 2020-2022, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

/*******************************************************************************
 * File name: low_power_20819.c
 *
 * Description: Main file which is the entry point to the application.
 *              The application_start function initializes UART for trace
 *              messages, initializes the BT stack and configures sleep
 *              parameters. The low_power_sleep_handler function handles the
 *              sleep related callbacks from the PMU.
 *              Refer to the CE document CE225540_CYW20819_Low_Power.pdf
 *              provided along with the code example for more details.
 *
 * Features demonstrated:
 *              ePDS and HID-Off functionality with BLE advertisements (ADV) and
 *              connection.
 *
 *
 *  Controls:
 *  - Start ADV with button press when not connected.
 *  - Disconnect and enter HID-Off with button press when connected.
 *  - Wake from HID-Off on button press.
 *
 *  Note: Do not connect HCI UART when running this application as it will
 *  prevent the device from entering sleep as we have configured sleep mode as
 *  WICED_SLEEP_MODE_NO_TRANSPORT (see header file wiced_sleep.h)
 ******************************************************************************/

#include "wiced.h"
#include "wiced_bt_dev.h"
#include "wiced_hal_gpio.h"
#include "wiced_platform.h"
#include "wiced_bt_trace.h"
#include "wiced_bt_stack.h"
#include "app_bt_cfg.h"
#include "wiced_sleep.h"
#include "wiced_rtc.h"
#include "wiced_hal_mia.h"
#include "GeneratedSource/cycfg_pins.h"
#include "low_power_20819.h"

/*******************************************************************************
 *                    Macros
 ******************************************************************************/
/* Pass 10000000 to HID-Off API to sleep for 10 seconds */
#define HIDOFF_SLEEP_TIME 10000000

/*******************************************************************************
 * Variable Definitions
 ******************************************************************************/

/* sleep configuration */
wiced_sleep_config_t    low_power_sleep_config;

/*******************************************************************************
 *                  Function Prototypes
 ******************************************************************************/
static uint32_t               low_power_sleep_handler(wiced_sleep_poll_type_t type);
static void                   low_power_post_sleep_cb(
                              wiced_bool_t restore_configuration);

extern wiced_bt_dev_status_t  low_power_20819_bt_management_callback(
                              wiced_bt_management_evt_t event,
                              wiced_bt_management_evt_data_t *p_event_data);

/******************************************************************************/

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

/*******************************************************************************
 * Function Name: application_start()
 *******************************************************************************
* Summary: Initialize transport configuration, register BLE
*          management event callback and configure sleep.
*
* Parameters:
*   None
*
* Return:
*  None
*
*******************************************************************************/
void application_start(void)
{
    /* Following line of code is required to enable Hibernate */
    g_foundation_config_PMUflags |= PMU_CONFIG_FLAGS_ENABLE_SDS;

    wiced_sleep_wake_reason_t wake_reason = 0;

    /* Set Debug UART as WICED_ROUTE_DEBUG_TO_PUART to see debug traces on
     * Peripheral UART (PUART) */
    wiced_set_debug_uart(WICED_ROUTE_DEBUG_TO_PUART);

    WICED_BT_TRACE ("\r\n==========================================================\r\n\n"
                    "                  Low Power 20819\r\n\r\n"
                    "==========================================================\r\n"
                    "  This application implements low power modes (ePDS and\r\n"
                    "  HID-Off) in CYW20819\r\n"
                    "==========================================================\r\n\n");

    wake_reason = wiced_sleep_hid_off_wake_reason();

    if(WICED_SLEEP_WAKE_REASON_POR == wake_reason)
    {
        WICED_BT_TRACE("Reset due to POR\n");
    }
    else if(WICED_SLEEP_WAKE_REASON_HIDOFF_TIMEOUT == wake_reason)
    {
        WICED_BT_TRACE("Reset due to timed wake from HID-Off\n");
    }
    else
    {
        if(wiced_hal_gpio_get_pin_interrupt_status(
           WICED_GET_PIN_FOR_BUTTON(WICED_PLATFORM_BUTTON_1)))
        {
            WICED_BT_TRACE("Reset due to GPIO wake from HID-Off\n");
        }
        else
        {
            WICED_BT_TRACE("Reset due to other reasons\n");
        }
    }

    /* Initialize Bluetooth Controller and Host Stack */
    if(WICED_BT_SUCCESS != wiced_bt_stack_init(
                           low_power_20819_bt_management_callback,
                           &wiced_bt_cfg_settings,
                           wiced_bt_cfg_buf_pools))
    {
        WICED_BT_TRACE("Bluetooth LE Stack initialization failed\r\n");
    }

    /* configure to sleep if sensor is idle */
    low_power_sleep_config.sleep_mode               = WICED_SLEEP_MODE_NO_TRANSPORT;
    low_power_sleep_config.device_wake_mode         = WICED_SLEEP_WAKE_ACTIVE_LOW;
    low_power_sleep_config.device_wake_source       = WICED_SLEEP_WAKE_SOURCE_GPIO;
    low_power_sleep_config.device_wake_gpio_num     = WICED_GET_PIN_FOR_BUTTON(WICED_PLATFORM_BUTTON_1);
    low_power_sleep_config.host_wake_mode           = WICED_SLEEP_WAKE_ACTIVE_HIGH;
    low_power_sleep_config.sleep_permit_handler     = low_power_sleep_handler;
    low_power_sleep_config.post_sleep_cback_handler = low_power_post_sleep_cb;

    if(WICED_BT_SUCCESS != wiced_sleep_configure(&low_power_sleep_config))
    {
        WICED_BT_TRACE("Sleep Configure failed\r\n");
    }
}

/*******************************************************************************
 * Function Name: uint32_t low_power_sleep_handler(wiced_sleep_poll_type_t type)
 *******************************************************************************
 * Summary: Callback for sleep permissions.
 *
 * Parameters:
 *   wiced_sleep_poll_type_t type          : Poll type (see #wiced_sleep_poll_type_t)
 *
 * Return:
 *   uint32_t: if type == WICED_SLEEP_POLL_SLEEP_PERMISSION, application should
 *             return WICED_SLEEP_ALLOWED_WITHOUT_SHUTDOWN or
 *             WICED_SLEEP_NOT_ALLOWED. If type == WICED_SLEEP_POLL_TIME_TO_SLEEP,
 *              application should return WICED_SLEEP_MAX_TIME_TO_SLEEP
 *
 ******************************************************************************/
uint32_t low_power_sleep_handler(wiced_sleep_poll_type_t type)
{
    uint32_t ret = WICED_SLEEP_NOT_ALLOWED;

    switch(type)
    {
        case WICED_SLEEP_POLL_SLEEP_PERMISSION:
            if(HIDOFF == low_power_20819_current_state)
            {
                /* This allows the device to enter HID-Off */
                ret = WICED_SLEEP_ALLOWED_WITH_SHUTDOWN;

            }
            else
            {
                /* This allows the device to enter ePDS */
                ret = WICED_SLEEP_ALLOWED_WITHOUT_SHUTDOWN;

            }
            break;
        case WICED_SLEEP_POLL_TIME_TO_SLEEP:
            if(HIDOFF == low_power_20819_current_state)
            {
                ret = HIDOFF_SLEEP_TIME;
            }
            else
            {
                ret = WICED_SLEEP_MAX_TIME_TO_SLEEP;
            }
            break;
    }
    return ret;
}

/*******************************************************************************
 * Function Name: void low_power_post_sleep_cb(wiced_bool_t
 *                                                        restore_configuration)
 *******************************************************************************
 * Summary: Callback for post sleep initialization.
 *
 * Parameters:
 *   wiced_bool_t restore_configuration      : Tells whether the user needs to
 *                                              restore the configurations or not
 *
 * Return:
 *   None
 *
 ******************************************************************************/
void low_power_post_sleep_cb(wiced_bool_t restore_configuration)
{
    /* Not doing anything in this function right now. But it can be used to
       initialize peripherals such as I2C and SPI upon wakeup from ePDS */
    if(restore_configuration)
    {
        /* Add code to re-initialize peripherals */
    }
}
