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
 * File Name : low_power_20819.h
 *
 *  @brief
 * This file consists of the utility functions that will help debugging and
 * developing the applications easier with much more meaningful information.
 *
 ******************************************************************************/
 #ifndef __LOW_POWER_20819_H__
 #define __LOW_POWER_20819_H__

/* *****************************************************************************
 *                              INCLUDES
 * ****************************************************************************/
#include "wiced_bt_gatt.h"
#include "wiced_platform.h"
#include "GeneratedSource/cycfg_pins.h"

/* *****************************************************************************
 *                              Macros
 * ****************************************************************************/

/* enum for system state */

enum application_state
{
    SLEEP_WITHOUT_BLE,
    SLEEP_WITH_ADV,
    SLEEP_WITH_CONNECTION,
    HIDOFF
};

#define PMU_CONFIG_FLAGS_ENABLE_SDS           0x00002000

/* *****************************************************************************
 *                              VARIABLES
 * ****************************************************************************/
 /* BT configuration from wiced_bt_cfg.c file */
extern const wiced_bt_cfg_settings_t wiced_bt_cfg_settings;

/* BT buffer pools from wiced_bt_cfg.c file */
extern const wiced_bt_cfg_buf_pool_t wiced_bt_cfg_buf_pools[WICED_BT_CFG_NUM_BUF_POOLS];

extern uint8_t low_power_20819_current_state;

extern UINT32 g_foundation_config_PMUflags;

#endif      /* __LOW_POWER_20819_H__ */
