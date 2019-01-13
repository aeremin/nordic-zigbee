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

#include <cmath>

#ifdef __cplusplus
extern "C" {
#endif

#include "zboss_api.h"
#include "zb_mem_config_med.h"
#include "zb_ha_dimmable_light.h"
#include "zb_error_handler.h"
#include "zb_nrf52840_internal.h"

#ifdef __cplusplus
}
#endif

#include "boards.h"
#include "app_pwm.h"

#include "nrf_drv_rng.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "on_off.h"
#include "zigbee_color_light.h"

const int kColorLightEndpoint = 11;

#define MAX_CHILDREN                      10                                    /**< The maximum amount of connected devices. Setting this value to 0 disables association to this device.  */
#define IEEE_CHANNEL_MASK                 (1l << ZIGBEE_CHANNEL)                /**< Scan only one, predefined channel to find the coordinator. */
#define HA_DIMMABLE_LIGHT_ENDPOINT        10                                    /**< Device endpoint, used to receive light controlling commands. */
#define ERASE_PERSISTENT_CONFIG           ZB_TRUE                              /**< Do not erase NVRAM to save the network parameters after device reboot or power-off. */
#define BULB_PWM_NAME                     PWM1                                  /**< PWM instance used to drive dimmable light bulb. */
#define BULB_PWM_TIMER                    2                                     /**< Timer number used by PWM. */

/* Basic cluster attributes initial values. */
#define BULB_INIT_BASIC_APP_VERSION       01                                    /**< Version of the application software (1 byte). */
#define BULB_INIT_BASIC_STACK_VERSION     10                                    /**< Version of the implementation of the ZigBee stack (1 byte). */
#define BULB_INIT_BASIC_HW_VERSION        11                                    /**< Version of the hardware of the device (1 byte). */
#define BULB_INIT_BASIC_MANUF_NAME        "Nordic"                              /**< Manufacturer name (32 bytes). */
// #define BULB_INIT_BASIC_MODEL_ID          "Dimable_Light_v0.1"                  /**< Model number assigned by manufacturer (32-bytes long string). */
// #define BULB_INIT_BASIC_DATE_CODE         "20180416"                            /**< First 8 bytes specify the date of manufacturer of the device in ISO 8601 format (YYYYMMDD). Th rest (8 bytes) are manufacturer specific. */
#define BULB_INIT_BASIC_POWER_SOURCE      ZB_ZCL_BASIC_POWER_SOURCE_DC_SOURCE   /**< Type of power sources available for the device. For possible values see section 3.2.2.2.8 of ZCL specification. */
#define BULB_INIT_BASIC_LOCATION_DESC     "Office desk"                         /**< Describes the physical location of the device (16 bytes). May be modified during commisioning process. */
// #define BULB_INIT_BASIC_PH_ENV            ZB_ZCL_BASIC_ENV_UNSPECIFIED          /**< Describes the type of physical environment. For possible values see section 3.2.2.2.10 of ZCL specification. */

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

/* Basic cluster attributes. */
struct bulb_device_basic_attr_t
{
    zb_uint8_t zcl_version;
    zb_uint8_t app_version;
    zb_uint8_t stack_version;
    zb_uint8_t hw_version;
    zb_char_t  mf_name[32];
    zb_char_t  model_id[32];
    zb_char_t  date_code[16];
    zb_uint8_t power_source;
    zb_char_t  location_id[17];
    zb_uint8_t ph_env;
};

/* Identify cluster attributes. */
struct bulb_device_identify_attr_t
{
    zb_uint16_t identify_time;
    zb_uint8_t  commission_state;
};

/* Level Control cluster attributes. */
struct bulb_device_level_control_attr_t
{
    zb_uint8_t  current_level;
    zb_uint16_t remaining_time;
};

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
    bulb_device_basic_attr_t         basic_attr;
    bulb_device_identify_attr_t      identify_attr;
    bulb_device_scenes_attr_t        scenes_attr;
    bulb_device_groups_attr_t        groups_attr;
    bulb_device_level_control_attr_t level_control_attr;
};


static bulb_device_ctx_t m_dev_ctx;

ZB_ZCL_DECLARE_IDENTIFY_ATTRIB_LIST_HA(identify_attr_list,
                                       &m_dev_ctx.identify_attr.identify_time,
                                       &m_dev_ctx.identify_attr.commission_state);


ZB_ZCL_DECLARE_GROUPS_ATTRIB_LIST(groups_attr_list, &m_dev_ctx.groups_attr.name_support);

ZB_ZCL_DECLARE_SCENES_ATTRIB_LIST(scenes_attr_list,
                                  &m_dev_ctx.scenes_attr.scene_count,
                                  &m_dev_ctx.scenes_attr.current_scene,
                                  &m_dev_ctx.scenes_attr.current_group,
                                  &m_dev_ctx.scenes_attr.scene_valid,
                                  &m_dev_ctx.scenes_attr.name_support);

ZB_ZCL_DECLARE_BASIC_ATTRIB_LIST_HA_ADDS_FULL(basic_attr_list,
                                              &m_dev_ctx.basic_attr.zcl_version,
                                              &m_dev_ctx.basic_attr.app_version,
                                              &m_dev_ctx.basic_attr.stack_version,
                                              &m_dev_ctx.basic_attr.hw_version,
                                              m_dev_ctx.basic_attr.mf_name,
                                              m_dev_ctx.basic_attr.model_id,
                                              m_dev_ctx.basic_attr.date_code,
                                              &m_dev_ctx.basic_attr.power_source,
                                              m_dev_ctx.basic_attr.location_id,
                                              &m_dev_ctx.basic_attr.ph_env);

ZB_ZCL_DECLARE_LEVEL_CONTROL_ATTRIB_LIST(level_control_attr_list,
                                         &m_dev_ctx.level_control_attr.current_level,
                                         &m_dev_ctx.level_control_attr.remaining_time);

ZB_HA_DECLARE_DIMMABLE_LIGHT_CLUSTER_LIST(dimmable_light_clusters,
                                          basic_attr_list,
                                          identify_attr_list,
                                          groups_attr_list,
                                          scenes_attr_list,
                                          OnOffCluster::GetInstance().on_off_attr_list,
                                          level_control_attr_list);

ZB_HA_DECLARE_LIGHT_EP(dimmable_light_ep,
                       HA_DIMMABLE_LIGHT_ENDPOINT,
                       dimmable_light_clusters);

/////////////////////////////////////// COLOR LIGHT ///////////////////////////////////////////////////
struct led_params_t
{
    uint8_t  r_value;               /**< Red color value. */
    uint8_t  g_value;               /**< Green color value. */
    uint8_t  b_value;               /**< Blue color value. */
};

/* Structure to store zigbee endpoint related variables */
struct bulb_device_ep_ctx_t
{
    zb_bulb_dev_ctx_t         * const p_device_ctx;                 /**< Pointer to structure containing cluster attributes. */
    led_params_t                led_params;                         /**< Table to store RGB color values to control thigny LED. */
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

/**@brief Function to convert hue_stauration to RGB color space.
 *
 * @param[IN]  hue          Hue value of color.
 * @param[IN]  saturation   Saturation value of color.
 * @param[IN]  brightness   Brightness value of color.
 * @param[OUT] p_led_params Pointer to structure containing parameters to write to LED characteristic
 */
static void convert_hsb_to_rgb(uint8_t hue, uint8_t saturation, uint8_t brightness, led_params_t * p_led_params)
{
    /* Check if p_leds_params is not NULL pointer */
    if (p_led_params == NULL)
    {
        NRF_LOG_INFO("Incorrect pointer to led params");
        return;
    }
    /* C, X, m are auxiliary variables */
    float C     = 0.0;
    float X     = 0.0;
    float m     = 0.0;
    /* Convertion HSB --> RGB */
    C = (brightness / 255.0f) * (saturation / 254.0f);
    X = (hue / 254.0f) * 6.0f;
    /* Casting in var X is necessary due to implementation of floating-point modulo_2 */
    /*lint -e653 */
    X = (X - (2 * (((uint8_t) X) / 2)));
    /*lint -restore */
    X -= 1.0f;
    X = C * (1.0f - ((X > 0.0f) ? (X) : (-1.0f * X)));
    m = (brightness / 255.0f) - C;

    /* Hue value is stored in range (0 - 255) instead of (0 - 360) degree */
    if (hue <= 42) /* hue < 60 degree */
    {
        p_led_params->r_value = (uint8_t)((C + m) * 255.0f);
        p_led_params->g_value = (uint8_t)((X + m) * 255.0f);
        p_led_params->b_value = (uint8_t)((0.0f + m) * 255.0f);
    }
    else if (hue <= 84)  /* hue < 120 degree */
    {
        p_led_params->r_value = (uint8_t)((X + m) * 255.0f);
        p_led_params->g_value = (uint8_t)((C + m) * 255.0f);
        p_led_params->b_value = (uint8_t)((0.0f + m) * 255.0f);
    }
    else if (hue <= 127) /* hue < 180 degree */
    {
        p_led_params->r_value = (uint8_t)((0.0f + m) * 255.0f);
        p_led_params->g_value = (uint8_t)((C + m) * 255.0f);
        p_led_params->b_value = (uint8_t)((X + m) * 255.0f);
    }
    else if (hue < 170)  /* hue < 240 degree */
    {
        p_led_params->r_value = (uint8_t)((0.0f + m) * 255.0f);
        p_led_params->g_value = (uint8_t)((X + m) * 255.0f);
        p_led_params->b_value = (uint8_t)((C + m) * 255.0f);
    }
    else if (hue <= 212) /* hue < 300 degree */
    {
        p_led_params->r_value = (uint8_t)((X + m) * 255.0f);
        p_led_params->g_value = (uint8_t)((0.0f + m) * 255.0f);
        p_led_params->b_value = (uint8_t)((C + m) * 255.0f);
    }
    else                /* hue < 360 degree */
    {
        p_led_params->r_value = (uint8_t)((C + m) * 255.0f);
        p_led_params->g_value = (uint8_t)((0.0f + m) * 255.0f);
        p_led_params->b_value = (uint8_t)((X + m) * 255.0f);
    }
}

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
    convert_hsb_to_rgb(p_ep_dev_ctx->p_device_ctx->color_control_attr.set_color_info.current_hue,
                       p_ep_dev_ctx->p_device_ctx->color_control_attr.set_color_info.current_saturation,
                       p_ep_dev_ctx->p_device_ctx->level_control_attr.current_level,
                       &p_ep_dev_ctx->led_params);
}

void ble_thingy_master_update_led(led_params_t* p_led_params) {
    NRF_LOG_INFO("Setting color to %d %d %d", p_led_params->r_value, p_led_params->g_value, p_led_params->b_value);
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
    ble_thingy_master_update_led(&p_ep_dev_ctx->led_params);

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
    ble_thingy_master_update_led(&p_ep_dev_ctx->led_params);

    NRF_LOG_INFO("Set color saturation value: %i on endpoint: %hu", new_saturation, p_ep_dev_ctx->ep_id);
}

led_params_t ConvertXyToRgb(zb_uint16_t input_x, zb_uint16_t input_y) {
    float x = float(input_x) / 0xffff;
    float y = float(input_y) / 0xffff;
    float z = 1.0f - x - y;
    float Y = 1.0f; // Brightness. TODO: Use value from level control cluster?
    float X = (Y / y) * x;
    float Z = (Y / y) * z;
    float r = X * 3.2406f - Y * 1.5372f - Z * 0.4986f;
    float g = -X * 0.9689f + Y * 1.8758f + Z * 0.0415f;
    float b = X * 0.0557f - Y * 0.2040f + Z * 1.0570f;

    if (r > b && r > g && r > 1.0f) {
        // red is too big
        g = g / r;
        b = b / r;
        r = 1.0f;
    }
    else if (g > b && g > r && g > 1.0f) {
        // green is too big
        r = r / g;
        b = b / g;
        g = 1.0f;
    }
    else if (b > r && b > g && b > 1.0f) {
        // blue is too big
        r = r / b;
        g = g / b;
        b = 1.0f;
    }

    r = r <= 0.0031308f ? 12.92f * r : (1.0f + 0.055f) * pow(r, (1.0f / 2.4f)) - 0.055f;
    g = g <= 0.0031308f ? 12.92f * g : (1.0f + 0.055f) * pow(g, (1.0f / 2.4f)) - 0.055f;
    b = b <= 0.0031308f ? 12.92f * b : (1.0f + 0.055f) * pow(b, (1.0f / 2.4f)) - 0.055f;
    return { uint8_t(r * 256), uint8_t(g * 256), uint8_t(b * 256) };
}

void UpdateStateFromXy(zb_uint8_t) {
    auto* p_ep_dev_ctx = &zb_ep_dev_ctx;
    p_ep_dev_ctx->led_params = ConvertXyToRgb(p_ep_dev_ctx->p_device_ctx->color_control_attr.set_color_info.current_X,
                                              p_ep_dev_ctx->p_device_ctx->color_control_attr.set_color_info.current_Y);
    ble_thingy_master_update_led(&p_ep_dev_ctx->led_params);
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
    ble_thingy_master_update_led(&p_ep_dev_ctx->led_params);

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
    /*
    if (on)
    {
        level_control_set_value(p_ep_dev_ctx, p_ep_dev_ctx->p_device_ctx->level_control_attr.current_level);
    }
    else
    {
        p_ep_dev_ctx->led_params.r_value = 0;
        p_ep_dev_ctx->led_params.g_value = 0;
        p_ep_dev_ctx->led_params.b_value = 0;
        ble_thingy_master_update_led(&p_ep_dev_ctx->led_params);
    }
    */
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
static void level_control_set_value(zb_uint16_t new_level)
{
    m_dev_ctx.level_control_attr.current_level = new_level;

    NRF_LOG_INFO("Set level value: %i", new_level);

    /* Scale level value: APP_PWM uses 0-100 scale, but ZigBee level control cluster uses values from 0 up to 255. */
    new_level = new_level * 100 / 256;

    /* Set the duty cycle - keep trying until PWM is ready. */
    while (app_pwm_channel_duty_set(&BULB_PWM_NAME, 0, new_level) == NRF_ERROR_BUSY)
    {
    }

    /* According to the table 7.3 of Home Automation Profile Specification v 1.2 rev 29, chapter 7.1.3. */
    OnOffCluster::GetInstance().SetOn(new_level == 0);
}

/**@brief Function for turning ON/OFF the light bulb.
 *
 * @param[in]   on   Boolean light bulb state.
 */
static void on_off_set_value(zb_bool_t on)
{
    OnOffCluster::GetInstance().SetOn(on == ZB_TRUE);

    NRF_LOG_INFO("Set ON/OFF value: %i", on);

    if (on)
    {
        level_control_set_value(m_dev_ctx.level_control_attr.current_level);
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
    /* Basic cluster attributes data */
    m_dev_ctx.basic_attr.zcl_version   = ZB_ZCL_VERSION;
    m_dev_ctx.basic_attr.app_version   = BULB_INIT_BASIC_APP_VERSION;
    m_dev_ctx.basic_attr.stack_version = BULB_INIT_BASIC_STACK_VERSION;
    m_dev_ctx.basic_attr.hw_version    = BULB_INIT_BASIC_HW_VERSION;

    /* Use ZB_ZCL_SET_STRING_VAL to set strings, because the first byte should
     * contain string length without trailing zero.
     *
     * For example "test" string wil be encoded as:
     *   [(0x4), 't', 'e', 's', 't']
     */
    ZB_ZCL_SET_STRING_VAL(m_dev_ctx.basic_attr.mf_name,
                          BULB_INIT_BASIC_MANUF_NAME,
                          ZB_ZCL_STRING_CONST_SIZE(BULB_INIT_BASIC_MANUF_NAME));

    ZB_ZCL_SET_STRING_VAL(m_dev_ctx.basic_attr.model_id,
                          BULB_INIT_BASIC_MODEL_ID,
                          ZB_ZCL_STRING_CONST_SIZE(BULB_INIT_BASIC_MODEL_ID));

    ZB_ZCL_SET_STRING_VAL(m_dev_ctx.basic_attr.date_code,
                          BULB_INIT_BASIC_DATE_CODE,
                          ZB_ZCL_STRING_CONST_SIZE(BULB_INIT_BASIC_DATE_CODE));

    m_dev_ctx.basic_attr.power_source = BULB_INIT_BASIC_POWER_SOURCE;

    ZB_ZCL_SET_STRING_VAL(m_dev_ctx.basic_attr.location_id,
                          BULB_INIT_BASIC_LOCATION_DESC,
                          ZB_ZCL_STRING_CONST_SIZE(BULB_INIT_BASIC_LOCATION_DESC));


    m_dev_ctx.basic_attr.ph_env = BULB_INIT_BASIC_PH_ENV;

    /* Identify cluster attributes data */
    m_dev_ctx.identify_attr.identify_time    = ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE;
    m_dev_ctx.identify_attr.commission_state = ZB_ZCL_ATTR_IDENTIFY_COMMISSION_STATE_HA_ID_DEF_VALUE;

    /* On/Off cluster attributes data */
    OnOffCluster::GetInstance().Init(HA_DIMMABLE_LIGHT_ENDPOINT);

    m_dev_ctx.level_control_attr.current_level  = ZB_ZCL_LEVEL_CONTROL_LEVEL_MAX_VALUE;
    m_dev_ctx.level_control_attr.remaining_time = ZB_ZCL_LEVEL_CONTROL_REMAINING_TIME_DEFAULT_VALUE;
    ZB_ZCL_LEVEL_CONTROL_SET_LEVEL_VALUE(HA_DIMMABLE_LIGHT_ENDPOINT, m_dev_ctx.level_control_attr.current_level);

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
        NRF_LOG_INFO("Current hue %d, enhanced current hue: %d ",
            p_device_ep_ctx->p_device_ctx->color_control_attr.set_color_info.current_hue,
            p_device_ep_ctx->p_device_ctx->color_control_attr.set_color_info.enhanced_current_hue);

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
                    NRF_LOG_INFO("Set Attribute callback. Cluster: %d, attribute: %d. Value = %d",
                        cluster_id, attr_id, p_device_cb_param->cb_param.set_attr_value_param.values.data16);

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

/**@brief Function for application main entry.
 */
int main(void)
{
    /* Initialize loging system and GPIOs. */
    log_init();
    leds_init();

    NRF_LOG_INFO("Started light_bulb, v001");

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
    zb_set_network_router_role(IEEE_CHANNEL_MASK);
    zb_set_max_children(MAX_CHILDREN);
    zb_set_nvram_erase_at_start(ERASE_PERSISTENT_CONFIG);
    zb_set_keepalive_timeout(ZB_MILLISECONDS_TO_BEACON_INTERVAL(3000));

    /* Initialize application context structure. */
    UNUSED_RETURN_VALUE(ZB_MEMSET(&m_dev_ctx, 0, sizeof(m_dev_ctx)));

    /* Register callback for handling ZCL commands. */
    ZB_ZCL_REGISTER_DEVICE_CB(zcl_device_cb);

    /* Register dimmer switch device context (endpoints). */
    ZB_AF_REGISTER_DEVICE_CTX(&double_light_ctx);

    bulb_clusters_attr_init();
    level_control_set_value(m_dev_ctx.level_control_attr.current_level);

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
