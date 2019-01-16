/**
 * Copyright (c) 2018, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup zigbee_examples_light_bulb main.c
 * @{
 * @ingroup zigbee_examples
 * @brief Dimmable light sample (HA profile)
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "zboss_api.h"
#include "zb_mem_config_med.h"
#include "zb_ha_dimmable_light.h"
#include "zb_error_handler.h"
#include "zb_nrf52840_internal.h"
#include "nrf_serial.h"

#ifdef __cplusplus
}
#endif

#include "boards.h"
#include "app_pwm.h"

#include "nrf_drv_rng.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "clusters/basic.h"
#include "clusters/groups.h"
#include "clusters/identify.h"
#include "clusters/level_control.h"
#include "clusters/on_off.h"
#include "clusters/scenes.h"
#include "color_helpers.h"
#include "zigbee_color_light.h"

const int kDimmableLightEndoint = 10;
const int kColorLightEndpoint = 11;

const int kMaxZigbeeChildren = 10;
const int kIeeeChannelMask = 1l << 20;

const zb_bool_t kErasePersistentConfigOnRestart = ZB_TRUE;


#define BULB_PWM_NAME                     PWM1                                  /**< PWM instance used to drive dimmable light bulb. */
#define BULB_PWM_TIMER                    2                                     /**< Timer number used by PWM. */

#ifdef  BOARD_PCA10059                                                          /**< If it is Dongle */
#define ZIGBEE_NETWORK_STATE_LED          BSP_BOARD_LED_0                       /**< LED indicating that light switch successfully joind ZigBee network. */
#else
#define ZIGBEE_NETWORK_STATE_LED          BSP_BOARD_LED_2                       /**< LED indicating that light switch successfully joind ZigBee network. */
#endif
#define BULB_LED                          BSP_BOARD_LED_3                       /**< LED immitaing dimmable light bulb. */

/* Declare endpoint for Dimmable Light device with scenes. */
#define ZB_HA_DECLARE_LIGHT_EP(ep_name, ep_id, cluster_list)                         \
  ZB_ZCL_DECLARE_HA_DIMMABLE_LIGHT_SIMPLE_DESC(ep_name, ep_id,                       \
    ZB_HA_DIMMABLE_LIGHT_IN_CLUSTER_NUM, ZB_HA_DIMMABLE_LIGHT_OUT_CLUSTER_NUM);      \
  ZBOSS_DEVICE_DECLARE_REPORTING_CTX(reporting_info## device_ctx_name,               \
                                     ZB_HA_DIMMABLE_LIGHT_REPORT_ATTR_COUNT);        \
  ZBOSS_DEVICE_DECLARE_LEVEL_CONTROL_CTX(cvc_alarm_info## device_ctx_name,           \
                                         ZB_HA_DIMMABLE_LIGHT_CVC_ATTR_COUNT);       \
  ZB_AF_DECLARE_ENDPOINT_DESC(ep_name, ep_id, ZB_AF_HA_PROFILE_ID,                   \
                              0,     \
                              NULL,                 \
                              ZB_ZCL_ARRAY_SIZE(cluster_list, zb_zcl_cluster_desc_t),\
                              cluster_list,                                          \
                              (zb_af_simple_desc_1_1_t*)&simple_desc_##ep_name,      \
                              ZB_HA_DIMMABLE_LIGHT_REPORT_ATTR_COUNT,                \
                              reporting_info## device_ctx_name,                      \
                              ZB_HA_DIMMABLE_LIGHT_CVC_ATTR_COUNT,                   \
                              cvc_alarm_info## device_ctx_name)

#if !defined ZB_ROUTER_ROLE
#error Define ZB_ROUTER_ROLE to compile light bulb (Router) source code.
#endif

APP_PWM_INSTANCE(BULB_PWM_NAME, BULB_PWM_TIMER);

/* Scenes cluster attributes. */
struct bulb_device_scenes_attr_t
{
    zb_uint8_t  scene_count;
    zb_uint8_t  current_scene;
    zb_uint8_t  scene_valid;
    zb_uint8_t  name_support;
    zb_uint16_t current_group;
};

/* Groups cluster attributes. */
struct bulb_device_groups_attr_t
{
    zb_uint8_t name_support;
};

/* Main application customizable context. Stores all settings and static values. */
struct bulb_device_ctx_t
{
    BasicCluster basic;
    IdentifyCluster identify;
    GroupsCluster groups;
    ScenesCluster scenes;
    OnOffCluster on_off;
    LevelControlCluster level_control;
};


static bulb_device_ctx_t m_dev_ctx;

ZB_HA_DECLARE_DIMMABLE_LIGHT_CLUSTER_LIST(dimmable_light_clusters,
                                          m_dev_ctx.basic.attributes_list,
                                          m_dev_ctx.identify.attributes_list,
                                          m_dev_ctx.groups.attributes_list,
                                          m_dev_ctx.scenes.attributes_list,
                                          m_dev_ctx.on_off.attributes_list,
                                          m_dev_ctx.level_control.attributes_list);

ZB_HA_DECLARE_LIGHT_EP(dimmable_light_ep,
                       kDimmableLightEndoint,
                       dimmable_light_clusters);

/////////////////////////////////////// COLOR LIGHT ///////////////////////////////////////////////////

/* Structure to store zigbee endpoint related variables */
struct bulb_device_ep_ctx_t
{
    zb_bulb_dev_ctx_t         * const p_device_ctx;                 /**< Pointer to structure containing cluster attributes. */
    RgbColor                    rgb_color;
    const uint8_t               ep_id;                              /**< Endpoint ID. */
    zb_bool_t                   value_changing_flag;                /**< Variable used as flag while detecting changing value in Level Control attribute. */
    uint8_t                     prev_lvl_ctrl_value;                /**< Variable used to store previous attribute value while detecting changing value in Level Control attribute. */
};

/* Declare context variable and cluster attribute list for first endpoint */
static zb_bulb_dev_ctx_t zb_dev_ctx_first;
ZB_DECLARE_COLOR_LIGHT_BULB_CLUSTER_ATTR_LIST(zb_dev_ctx_first, color_light_bulb_clusters_first);
ZB_ZCL_DECLARE_COLOR_LIGHT_EP(color_light_bulb_ep_first, kColorLightEndpoint, color_light_bulb_clusters_first);
ZBOSS_DECLARE_DEVICE_CTX_2_EP(double_light_ctx, dimmable_light_ep, color_light_bulb_ep_first);

static bulb_device_ep_ctx_t zb_ep_dev_ctx = {
    &zb_dev_ctx_first,
    {0, 0, 0},
    kColorLightEndpoint,
    ZB_FALSE,
    0
};

///////////////////// UART ////////////////////////
static void sleep_handler(void)
{
    __WFE();
    __SEV();
    __WFE();
}

static const nrf_drv_uart_config_t m_uarte0_drv_config = [](){
    nrf_drv_uart_config_t res;
    res.pselrxd = ARDUINO_SDA_PIN;
    res.pseltxd = ARDUINO_SCL_PIN;
    res.pselrts = RTS_PIN_NUMBER;
    res.pselcts = CTS_PIN_NUMBER;
    res.hwfc = NRF_UART_HWFC_DISABLED;
    res.parity = NRF_UART_PARITY_EXCLUDED;
    res.baudrate = NRF_UART_BAUDRATE_115200;
    res.interrupt_priority = UART_DEFAULT_CONFIG_IRQ_PRIORITY;
    return res;
}();

#define SERIAL_FIFO_TX_SIZE 32
#define SERIAL_FIFO_RX_SIZE 32

NRF_SERIAL_QUEUES_DEF(serial0_queues, SERIAL_FIFO_TX_SIZE, SERIAL_FIFO_RX_SIZE);

#define SERIAL_BUFF_TX_SIZE 1
#define SERIAL_BUFF_RX_SIZE 1

NRF_SERIAL_BUFFERS_DEF(serial0_buffs, SERIAL_BUFF_TX_SIZE, SERIAL_BUFF_RX_SIZE);

NRF_SERIAL_CONFIG_DEF(serial0_config, NRF_SERIAL_MODE_DMA,
                      &serial0_queues, &serial0_buffs, NULL, sleep_handler);

NRF_SERIAL_UART_DEF(serial0_uarte, 1);

/**@brief Function for updating RGB color value.
 *
 * @param[IN] p_ep_dev_ctx pointer to endpoint device ctx.
 */
static void zb_update_color_values(bulb_device_ep_ctx_t * p_ep_dev_ctx)
{
    if ((p_ep_dev_ctx == NULL) || (p_ep_dev_ctx->p_device_ctx == NULL))
    {
        NRF_LOG_INFO("Can not update color values - bulb_device_ep_ctx uninitialised");
        return;
    }
    p_ep_dev_ctx->rgb_color = ConvertHsbToRgb(p_ep_dev_ctx->p_device_ctx->color_control_attr.set_color_info.current_hue,
                                              p_ep_dev_ctx->p_device_ctx->color_control_attr.set_color_info.current_saturation,
                                              p_ep_dev_ctx->p_device_ctx->level_control_attr.current_level);
}

void ble_thingy_master_update_led(RgbColor* p_rgb_color) {
    NRF_LOG_INFO("Setting color to %d %d %d", p_rgb_color->r_value, p_rgb_color->g_value, p_rgb_color->b_value);
    char buffer[20]; // Maximal string is 'RGB 255 255 255\n\r\0' which is 18 symbols
    int n = sprintf(buffer, "RGB %d %d %d\n\r", p_rgb_color->r_value, p_rgb_color->g_value, p_rgb_color->b_value);
    nrf_serial_write(&serial0_uarte, buffer, n, nullptr, NRF_SERIAL_MAX_TIMEOUT);
    nrf_serial_flush(&serial0_uarte, NRF_SERIAL_MAX_TIMEOUT);
}

/**@brief Function for changing the hue of the light bulb.
 *
 * @param[IN] p_ep_dev_ctx  Pointer to endpoint device ctx.
 * @param[IN] new_hue       New value for hue.
 */
static void color_control_set_value_hue(bulb_device_ep_ctx_t * p_ep_dev_ctx, zb_uint8_t new_hue)
{
    p_ep_dev_ctx->p_device_ctx->color_control_attr.set_color_info.current_hue = new_hue;

    zb_update_color_values(p_ep_dev_ctx);
    ble_thingy_master_update_led(&p_ep_dev_ctx->rgb_color);

    NRF_LOG_INFO("Set color hue value: %i on endpoint: %hu", new_hue, p_ep_dev_ctx->ep_id);
}

/**@brief Function for changing the saturation of the light bulb.
 *
 * @param[IN] p_ep_dev_ctx   pointer to endpoint device ctx.
 * @param[IN] new_saturation new value for saturation.
 */
static void color_control_set_value_saturation(bulb_device_ep_ctx_t * p_ep_dev_ctx, zb_uint8_t new_saturation)
{
    p_ep_dev_ctx->p_device_ctx->color_control_attr.set_color_info.current_saturation = new_saturation;

    zb_update_color_values(p_ep_dev_ctx);
    ble_thingy_master_update_led(&p_ep_dev_ctx->rgb_color);

    NRF_LOG_INFO("Set color saturation value: %i on endpoint: %hu", new_saturation, p_ep_dev_ctx->ep_id);
}

void UpdateStateFromXy(zb_uint8_t) {
    auto* p_ep_dev_ctx = &zb_ep_dev_ctx;
    p_ep_dev_ctx->rgb_color = ConvertXyToRgb(p_ep_dev_ctx->p_device_ctx->color_control_attr.set_color_info.current_X,
                                             p_ep_dev_ctx->p_device_ctx->color_control_attr.set_color_info.current_Y);
    ble_thingy_master_update_led(&p_ep_dev_ctx->rgb_color);
}

void color_control_set_value_x(bulb_device_ep_ctx_t * p_ep_dev_ctx)
{
    // For whatever reason, ZBoss updates set_color_info.current_X and set_color_info.current_Y
    // after this callback is called (not before!). So we need to schedule an alarm.
    ZB_SCHEDULE_ALARM(UpdateStateFromXy, /* unused */ 0, /* ASAP */ 1);
}

void color_control_set_value_y(bulb_device_ep_ctx_t * p_ep_dev_ctx)
{
    // For whatever reason, ZBoss updates set_color_info.current_X and set_color_info.current_Y
    // after this callback is called (not before!). So we need to schedule an alarm.
    ZB_SCHEDULE_ALARM(UpdateStateFromXy, /* unused */ 0, /* ASAP */ 1);
}

/**@brief Function for setting the light bulb brightness.
 *
 * @param[IN] p_ep_dev_ctx Pointer to endpoint device ctx.
 * @param[IN] new_level    Light bulb brightness value.
 */
static void level_control_set_value(bulb_device_ep_ctx_t * p_ep_dev_ctx, zb_uint16_t new_level)
{
    p_ep_dev_ctx->p_device_ctx->level_control_attr.current_level = new_level;

    zb_update_color_values(p_ep_dev_ctx);
    ble_thingy_master_update_led(&p_ep_dev_ctx->rgb_color);

    NRF_LOG_INFO("Set level value: %i on endpoint: %hu", new_level, p_ep_dev_ctx->ep_id);

    /* According to the table 7.3 of Home Automation Profile Specification v 1.2 rev 29, chapter 7.1.3. */
    p_ep_dev_ctx->p_device_ctx->on_off_attr.on_off = (new_level ? ZB_TRUE : ZB_FALSE);
}


/**@brief Function for turning ON/OFF the light bulb.
 *
 * @param[IN] p_ep_dev_ctx Pointer to endpoint device ctx.
 * @param[IN] on           Boolean light bulb state.
 */
static void on_off_set_value(bulb_device_ep_ctx_t * p_ep_dev_ctx, zb_bool_t on)
{
    p_ep_dev_ctx->p_device_ctx->on_off_attr.on_off = on;

    NRF_LOG_INFO("Set ON/OFF value: %i on endpoint: %hu", on, p_ep_dev_ctx->ep_id);

    if (on)
    {
        level_control_set_value(p_ep_dev_ctx, p_ep_dev_ctx->p_device_ctx->level_control_attr.current_level);
    }
    else
    {
        p_ep_dev_ctx->rgb_color.r_value = 0;
        p_ep_dev_ctx->rgb_color.g_value = 0;
        p_ep_dev_ctx->rgb_color.b_value = 0;
        ble_thingy_master_update_led(&p_ep_dev_ctx->rgb_color);
    }
}

/**@brief Function for initializing clusters attributes.
 *
 * @param[IN]   p_device_ctx   Pointer to structure with device_ctx.
 * @param[IN]   ep_id          Endpoint ID.
 */
static void bulb_clusters_attr_init(zb_bulb_dev_ctx_t * p_device_ctx, zb_uint8_t ep_id)
{
    /* Basic cluster attributes data */
    p_device_ctx->basic_attr.zcl_version   = ZB_ZCL_VERSION;
    p_device_ctx->basic_attr.app_version   = BULB_INIT_BASIC_APP_VERSION;
    p_device_ctx->basic_attr.stack_version = BULB_INIT_BASIC_STACK_VERSION;
    p_device_ctx->basic_attr.hw_version    = BULB_INIT_BASIC_HW_VERSION;

    /* Use ZB_ZCL_SET_STRING_VAL to set strings, because the first byte should
     * contain string length without trailing zero.
     *
     * For example "test" string wil be encoded as:
     *   [(0x4), 't', 'e', 's', 't']
     */
    ZB_ZCL_SET_STRING_VAL(p_device_ctx->basic_attr.mf_name,
                          BULB_INIT_BASIC_MANUF_NAME,
                          ZB_ZCL_STRING_CONST_SIZE(BULB_INIT_BASIC_MANUF_NAME));

    ZB_ZCL_SET_STRING_VAL(p_device_ctx->basic_attr.model_id,
                          BULB_INIT_BASIC_MODEL_ID,
                          ZB_ZCL_STRING_CONST_SIZE(BULB_INIT_BASIC_MODEL_ID));

    ZB_ZCL_SET_STRING_VAL(p_device_ctx->basic_attr.date_code,
                          BULB_INIT_BASIC_DATE_CODE,
                          ZB_ZCL_STRING_CONST_SIZE(BULB_INIT_BASIC_DATE_CODE));

    p_device_ctx->basic_attr.power_source = BULB_INIT_BASIC_POWER_SOURCE;

    ZB_ZCL_SET_STRING_VAL(p_device_ctx->basic_attr.location_id,
                          BULB_INIT_BASIC_LOCATION_DESC,
                          ZB_ZCL_STRING_CONST_SIZE(BULB_INIT_BASIC_LOCATION_DESC));


    p_device_ctx->basic_attr.ph_env = BULB_INIT_BASIC_PH_ENV;

    /* Identify cluster attributes data */
    p_device_ctx->identify_attr.identify_time       = ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE;
    p_device_ctx->identify_attr.commission_state    = ZB_ZCL_ATTR_IDENTIFY_COMMISSION_STATE_HA_ID_DEF_VALUE;

    /* On/Off cluster attributes data */
    p_device_ctx->on_off_attr.on_off                = (zb_bool_t)ZB_ZCL_ON_OFF_IS_ON;
    p_device_ctx->on_off_attr.global_scene_ctrl     = ZB_TRUE;
    p_device_ctx->on_off_attr.on_time               = 0;
    p_device_ctx->on_off_attr.off_wait_time         = 0;

    /* Level control cluster attributes data */
    p_device_ctx->level_control_attr.current_level  = ZB_ZCL_LEVEL_CONTROL_LEVEL_MAX_VALUE; // Set current level value to maximum
    p_device_ctx->level_control_attr.remaining_time = ZB_ZCL_LEVEL_CONTROL_REMAINING_TIME_DEFAULT_VALUE;
    ZB_ZCL_LEVEL_CONTROL_SET_ON_OFF_VALUE(ep_id, p_device_ctx->on_off_attr.on_off);
    ZB_ZCL_LEVEL_CONTROL_SET_LEVEL_VALUE(ep_id, p_device_ctx->level_control_attr.current_level);

    /* Color control cluster attributes data */
    p_device_ctx->color_control_attr.set_color_info.current_hue         = ZB_ZCL_COLOR_CONTROL_HUE_RED;
    p_device_ctx->color_control_attr.set_color_info.current_saturation  = ZB_ZCL_COLOR_CONTROL_CURRENT_SATURATION_MAX_VALUE;
    /* Set to use hue & saturation */
    p_device_ctx->color_control_attr.set_color_info.color_mode          = ZB_ZCL_COLOR_CONTROL_COLOR_MODE_HUE_SATURATION;
    p_device_ctx->color_control_attr.set_color_info.color_temperature   = ZB_ZCL_COLOR_CONTROL_COLOR_TEMPERATURE_DEF_VALUE;
    p_device_ctx->color_control_attr.set_color_info.remaining_time      = ZB_ZCL_COLOR_CONTROL_REMAINING_TIME_MIN_VALUE;
    p_device_ctx->color_control_attr.set_color_info.color_capabilities  = ZB_ZCL_COLOR_CONTROL_CAPABILITIES_HUE_SATURATION;
    /* According to ZCL spec 5.2.2.2.1.12 0x00 shall be set when CurrentHue and CurrentSaturation are used. */
    p_device_ctx->color_control_attr.set_color_info.enhanced_color_mode = 0x00;
    /* According to 5.2.2.2.1.10 execute commands when device is off. */
    p_device_ctx->color_control_attr.set_color_info.color_capabilities  = ZB_ZCL_COLOR_CONTROL_OPTIONS_EXECUTE_IF_OFF;
    /* According to ZCL spec 5.2.2.2.2 0xFF shall be set when specific value is unknown. */
    p_device_ctx->color_control_attr.set_defined_primaries_info.number_primaries = 0xff;
}

/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

/**@brief Function for setting the light bulb brightness.
  *
  * @param[in]   new_level   Light bulb brightness value.
 */
static void level_control_set_value(uint8_t new_level)
{
    NRF_LOG_INFO("Set level value: %i", new_level);

    m_dev_ctx.level_control.SetLevel(new_level);

    /* Scale level value: APP_PWM uses 0-100 scale, but ZigBee level control cluster uses values from 0 up to 255. */
    new_level = new_level * 100 / 256;

    /* Set the duty cycle - keep trying until PWM is ready. */
    while (app_pwm_channel_duty_set(&BULB_PWM_NAME, 0, new_level) == NRF_ERROR_BUSY)
    {
    }

    /* According to the table 7.3 of Home Automation Profile Specification v 1.2 rev 29, chapter 7.1.3. */
    m_dev_ctx.on_off.SetOn(new_level == 0);
}

/**@brief Function for turning ON/OFF the light bulb.
 *
 * @param[in]   on   Boolean light bulb state.
 */
static void on_off_set_value(zb_bool_t on)
{
    m_dev_ctx.on_off.SetOn(on == ZB_TRUE);

    NRF_LOG_INFO("Set ON/OFF value: %i", on);

    if (on)
    {
        level_control_set_value(m_dev_ctx.level_control.GetLevel());
    }
    else
    {
        while (app_pwm_channel_duty_set(&BULB_PWM_NAME, 0, 0) == NRF_ERROR_BUSY)
        {
        }
    }
}

/**@brief Function for initializing LEDs and a single PWM channel.
 */
static void leds_init(void)
{
    ret_code_t       err_code;
    app_pwm_config_t pwm_cfg = APP_PWM_DEFAULT_CONFIG_1CH(5000L, bsp_board_led_idx_to_pin(BULB_LED));

    /* Initialize all LEDs. */
    bsp_board_init(BSP_INIT_LEDS);

    /* Initialize PWM running on timer 1 in order to control dimmable light bulb. */
    err_code = app_pwm_init(&BULB_PWM_NAME, &pwm_cfg, NULL);
    APP_ERROR_CHECK(err_code);

    app_pwm_enable(&BULB_PWM_NAME);

    while (app_pwm_channel_duty_set(&BULB_PWM_NAME, 0, 99) == NRF_ERROR_BUSY)
    {
    }
}

/**@brief Function for initializing all clusters attributes.
 */
static void bulb_clusters_attr_init(void)
{
    m_dev_ctx.identify.Init();
    m_dev_ctx.basic.Init("DimmableLight");
    m_dev_ctx.on_off.Init(kDimmableLightEndoint);
    m_dev_ctx.level_control.Init(kDimmableLightEndoint);

    bulb_clusters_attr_init(zb_ep_dev_ctx.p_device_ctx, kColorLightEndpoint);
}

/**@brief Function which tries to sleep down the MCU
 *
 * Function which sleeps the MCU on the non-sleepy End Devices to optimize the power saving.
 * The weak definition inside the OSIF layer provides some minimal working template
 */
zb_void_t zb_osif_go_idle(zb_void_t)
{
    /* A check that we're not a Sleep End Device -- they sleep elsewhere */
    if (ZB_PIBCACHE_RX_ON_WHEN_IDLE())
    {
        //TODO: implement your own logic if needed
        if (NRF_LOG_PROCESS() == false)
        {
            zb_osif_wait_for_event();
        }
    }
}

/**@brief Callback function for handling ZCL commands.
 *
 * @param[in]   param   Reference to ZigBee stack buffer used to pass received data.
 */
static zb_void_t zcl_device_cb(zb_uint8_t param)
{
    zb_buf_t                       * p_buffer = ZB_BUF_FROM_REF(param);
    zb_zcl_device_callback_param_t * p_device_cb_param = ZB_GET_BUF_PARAM(p_buffer, zb_zcl_device_callback_param_t);

    if (p_device_cb_param->endpoint == kColorLightEndpoint) {
        auto* p_device_ep_ctx = &zb_ep_dev_ctx;

        /* Set default response value. */
        p_device_cb_param->status = RET_OK;

        switch (p_device_cb_param->device_cb_id)
        {
            case ZB_ZCL_LEVEL_CONTROL_SET_VALUE_CB_ID:
                /* Set new value in cluster and then use nrf_app_timer to delay thingy led update if value is changing quickly */
                NRF_LOG_INFO("Level control setting to %d", p_device_cb_param->cb_param.level_control_set_value_param.new_value);
                p_device_ep_ctx->p_device_ctx->level_control_attr.current_level = p_device_cb_param->cb_param.level_control_set_value_param.new_value;
                break;

            case ZB_ZCL_ON_OFF_WITH_EFFECT_VALUE_CB_ID:
                NRF_LOG_INFO("Turning off. Additional data: effect_id: %d, effect_variant: %d",
                    p_device_cb_param->cb_param.on_off_set_effect_value_param.effect_id,
                    p_device_cb_param->cb_param.on_off_set_effect_value_param.effect_variant);
                on_off_set_value(p_device_ep_ctx, ZB_FALSE);
                break;

            case ZB_ZCL_SET_ATTR_VALUE_CB_ID:
                {
                    const zb_uint8_t cluster_id = p_device_cb_param->cb_param.set_attr_value_param.cluster_id;
                    const zb_uint8_t attr_id    = p_device_cb_param->cb_param.set_attr_value_param.attr_id;
                    if (cluster_id == ZB_ZCL_CLUSTER_ID_ON_OFF)
                    {
                        uint8_t value = p_device_cb_param->cb_param.set_attr_value_param.values.data8;

                        NRF_LOG_INFO("on/off attribute setting to %hd", value);
                        if (attr_id == ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID)
                        {
                            on_off_set_value(p_device_ep_ctx, (zb_bool_t)value);
                        }
                    }
                    else if (cluster_id == ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL)
                    {
                        uint16_t value = p_device_cb_param->cb_param.set_attr_value_param.values.data16;

                        NRF_LOG_INFO("level control attribute setting to %hd", value);
                        if (attr_id == ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID)
                        {
                            level_control_set_value(p_device_ep_ctx, value);
                        }
                    }
                    else if (cluster_id == ZB_ZCL_CLUSTER_ID_COLOR_CONTROL || cluster_id == ZB_ZCL_CLUSTER_ID_BASIC) // Why ZB_ZCL_CLUSTER_ID_BASIC?!
                    {
                        // TODO: Ideally, we should do smooth animation instead of waiting
                        // and then switching to final state.
                        if (p_device_ep_ctx->p_device_ctx->color_control_attr.set_color_info.remaining_time <= 1)
                        {
                            const auto& values = p_device_cb_param->cb_param.set_attr_value_param.values;
                            switch (attr_id)
                            {
                                case ZB_ZCL_ATTR_COLOR_CONTROL_CURRENT_HUE_ID:
                                    color_control_set_value_hue(p_device_ep_ctx, values.data16);
                                    break;

                                case ZB_ZCL_ATTR_COLOR_CONTROL_CURRENT_SATURATION_ID:
                                    color_control_set_value_saturation(p_device_ep_ctx, values.data8);
                                    break;

                                case ZB_ZCL_ATTR_COLOR_CONTROL_CURRENT_X_ID:
                                    color_control_set_value_x(p_device_ep_ctx);
                                    break;

                                case ZB_ZCL_ATTR_COLOR_CONTROL_CURRENT_Y_ID:
                                    color_control_set_value_y(p_device_ep_ctx);
                                    break;

                                default:
                                    NRF_LOG_WARNING("Got request to change unexpected attribute");
                                    break;
                            }
                        }
                    }
                    else {
                        NRF_LOG_WARNING("Unhandled cluster_id: %d", cluster_id);
                    }
                    break;
                }

            default:
                p_device_cb_param->status = RET_ERROR;
                NRF_LOG_INFO("Default case, returned error");
                break;
        }

        if (p_device_cb_param->status != 0) {
            NRF_LOG_WARNING("zcl_device_cb status: %hd", p_device_cb_param->status);
        }

        return;
    }


    /* Set default response value. */
    p_device_cb_param->status = RET_OK;

    switch (p_device_cb_param->device_cb_id)
    {
        case ZB_ZCL_ON_OFF_WITH_EFFECT_VALUE_CB_ID:
            NRF_LOG_INFO("Turning off. Additional data: effect_id: %d, effect_variant: %d",
                p_device_cb_param->cb_param.on_off_set_effect_value_param.effect_id,
                p_device_cb_param->cb_param.on_off_set_effect_value_param.effect_variant);
            on_off_set_value(ZB_FALSE);
            break;

        case ZB_ZCL_LEVEL_CONTROL_SET_VALUE_CB_ID:
            NRF_LOG_INFO("Level control setting to %d", p_device_cb_param->cb_param.level_control_set_value_param.new_value);
            level_control_set_value(p_device_cb_param->cb_param.level_control_set_value_param.new_value);
            break;

        case ZB_ZCL_SET_ATTR_VALUE_CB_ID: {
                const zb_uint8_t cluster_id = p_device_cb_param->cb_param.set_attr_value_param.cluster_id;
                const zb_uint8_t attr_id    = p_device_cb_param->cb_param.set_attr_value_param.attr_id;

                if (cluster_id == ZB_ZCL_CLUSTER_ID_ON_OFF)
                {
                    uint8_t value = p_device_cb_param->cb_param.set_attr_value_param.values.data8;

                    NRF_LOG_INFO("on/off attribute setting to %hd", value);
                    if (attr_id == ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID)
                    {
                        on_off_set_value((zb_bool_t) value);
                    }
                }
                else if (cluster_id == ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL)
                {
                    uint16_t value = p_device_cb_param->cb_param.set_attr_value_param.values.data16;

                    NRF_LOG_INFO("level control attribute setting to %hd", value);
                    if (attr_id == ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID)
                    {
                        level_control_set_value(value);
                    }
                }
                else
                {
                    /* Other clusters can be processed here */
                    NRF_LOG_INFO("Unhandled cluster attribute id: %d", cluster_id);
                }
                break;
            }

        default:
            p_device_cb_param->status = RET_ERROR;
            break;
    }

    NRF_LOG_INFO("zcl_device_cb status: %hd", p_device_cb_param->status);
}

/**@brief ZigBee stack event handler.
 *
 * @param[in]   param   Reference to ZigBee stack buffer used to pass arguments (signal).
 */
void zboss_signal_handler(zb_uint8_t param)
{
    zb_zdo_app_signal_type_t sig    = zb_get_app_signal(param, NULL);
    zb_ret_t                 status = ZB_GET_APP_SIGNAL_STATUS(param);
    zb_bool_t                comm_status;

    switch (sig)
    {
        case ZB_BDB_SIGNAL_DEVICE_FIRST_START:
        case ZB_BDB_SIGNAL_DEVICE_REBOOT:
            if (status == RET_OK)
            {
                NRF_LOG_INFO("Joined network successfully");
                bsp_board_led_on(ZIGBEE_NETWORK_STATE_LED);
            }
            else
            {
                NRF_LOG_ERROR("Failed to join network. Status: %d", status);
                bsp_board_led_off(ZIGBEE_NETWORK_STATE_LED);
                comm_status = bdb_start_top_level_commissioning(ZB_BDB_NETWORK_STEERING);
                ZB_COMM_STATUS_CHECK(comm_status);
            }
            break;

        case ZB_ZDO_SIGNAL_PRODUCTION_CONFIG_READY:
            if (status != RET_OK)
            {
                NRF_LOG_WARNING("Production config is not present or invalid");
            }
            break;

        default:
            /* Unhandled signal. For more information see: zb_zdo_app_signal_type_e and zb_ret_e */
            NRF_LOG_INFO("Unhandled signal %d. Status: %d", sig, status);
            break;
    }

    if (param)
    {
        ZB_FREE_BUF_BY_REF(param);
    }
}

void InitUart() {
    // Used by nrf_serial calls when timeout < NRF_SERIAL_MAX_TIMEOUT
    APP_ERROR_CHECK(app_timer_init());

    APP_ERROR_CHECK(nrf_serial_init(&serial0_uarte, &m_uarte0_drv_config, &serial0_config));
}

/**@brief Function for application main entry.
 */
int main(void)
{
    /* Initialize loging system and GPIOs. */
    log_init();
    leds_init();

    NRF_LOG_INFO("Started light_bulb, v001");
    InitUart();

    /* Set ZigBee stack logging level and traffic dump subsystem. */
    ZB_SET_TRACE_LEVEL(ZIGBEE_TRACE_LEVEL);
    ZB_SET_TRACE_MASK(ZIGBEE_TRACE_MASK);
    ZB_SET_TRAF_DUMP_OFF();

    /* Initialize ZigBee stack. */
    ZB_INIT("led_bulb");

    /* Set device address to the value read from FICR registers. */
    zb_ieee_addr_t ieee_addr;
    zb_osif_get_ieee_eui64(ieee_addr);
    nrf_drv_rng_block_rand(ieee_addr, 3);
    NRF_LOG_INFO("Last address bits: %d %d %d", ieee_addr[0], ieee_addr[1], ieee_addr[2]);
    zb_set_long_address(ieee_addr);

    /* Set static long IEEE address. */
    zb_set_network_router_role(kIeeeChannelMask);
    zb_set_max_children(kMaxZigbeeChildren);
    zb_set_nvram_erase_at_start(kErasePersistentConfigOnRestart);
    zb_set_keepalive_timeout(ZB_MILLISECONDS_TO_BEACON_INTERVAL(3000));

    /* Register callback for handling ZCL commands. */
    ZB_ZCL_REGISTER_DEVICE_CB(zcl_device_cb);

    /* Register dimmer switch device context (endpoints). */
    ZB_AF_REGISTER_DEVICE_CTX(&double_light_ctx);

    bulb_clusters_attr_init();
    level_control_set_value(m_dev_ctx.level_control.GetLevel());

    /** Start Zigbee Stack. */
    ZB_ERROR_CHECK(zboss_start());

    while (true)
    {
        zboss_main_loop_iteration();
        NRF_LOG_PROCESS();
    }
}


/**
 * @}
 */
