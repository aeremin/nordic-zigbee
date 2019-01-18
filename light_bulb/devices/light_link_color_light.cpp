#include "light_link_color_light.h"

#include "nrf_log.h"

LightLinkColorLight::LightLinkColorLight(const nrf_serial_t* serial, zb_uint8_t endpoint) :
  cluster_descriptors({
    ZB_ZCL_CLUSTER_DESC(
        ZB_ZCL_CLUSTER_ID_BASIC,
        ZB_ZCL_ARRAY_SIZE(basic.attributes_list, zb_zcl_attr_t),
        basic.attributes_list,
        ZB_ZCL_CLUSTER_SERVER_ROLE,
        ZB_ZCL_MANUF_CODE_INVALID),
    ZB_ZCL_CLUSTER_DESC(
        ZB_ZCL_CLUSTER_ID_IDENTIFY,
        ZB_ZCL_ARRAY_SIZE(identify.attributes_list, zb_zcl_attr_t),
        identify.attributes_list,
        ZB_ZCL_CLUSTER_SERVER_ROLE,
        ZB_ZCL_MANUF_CODE_INVALID),
    ZB_ZCL_CLUSTER_DESC(
        ZB_ZCL_CLUSTER_ID_GROUPS,
        ZB_ZCL_ARRAY_SIZE(groups.attributes_list, zb_zcl_attr_t),
        groups.attributes_list,
        ZB_ZCL_CLUSTER_SERVER_ROLE,
        ZB_ZCL_MANUF_CODE_INVALID),
    ZB_ZCL_CLUSTER_DESC(
        ZB_ZCL_CLUSTER_ID_SCENES,
        ZB_ZCL_ARRAY_SIZE(scenes.attributes_list, zb_zcl_attr_t),
        scenes.attributes_list,
        ZB_ZCL_CLUSTER_SERVER_ROLE,
        ZB_ZCL_MANUF_CODE_INVALID),
    ZB_ZCL_CLUSTER_DESC(
        ZB_ZCL_CLUSTER_ID_ON_OFF,
        ZB_ZCL_ARRAY_SIZE(on_off.attributes_list, zb_zcl_attr_t),
        on_off.attributes_list,
        ZB_ZCL_CLUSTER_SERVER_ROLE,
        ZB_ZCL_MANUF_CODE_INVALID),
    ZB_ZCL_CLUSTER_DESC(
        ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL,
        ZB_ZCL_ARRAY_SIZE(level_control.attributes_list, zb_zcl_attr_t),
        level_control.attributes_list,
        ZB_ZCL_CLUSTER_SERVER_ROLE,
        ZB_ZCL_MANUF_CODE_INVALID),
    ZB_ZCL_CLUSTER_DESC(
        ZB_ZCL_CLUSTER_ID_COLOR_CONTROL,
        ZB_ZCL_ARRAY_SIZE(color_control.attributes_list, zb_zcl_attr_t),
        color_control.attributes_list,
        ZB_ZCL_CLUSTER_SERVER_ROLE,
        ZB_ZCL_MANUF_CODE_INVALID)}),
    endpoint(endpoint),
    serial(serial)
{
}

EndpointContext LightLinkColorLight::CreateColorLightEndpoint() {
    EndpointContext context;
    context.simple_descriptor = {
        endpoint,
        ZB_AF_ZLL_PROFILE_ID,
        0x0210,
        0,
        0,
        kLightLinkColorLightClusterCount,
        0, // out cluster count
        {
            ZB_ZCL_CLUSTER_ID_BASIC,
            ZB_ZCL_CLUSTER_ID_IDENTIFY,
            ZB_ZCL_CLUSTER_ID_GROUPS,
            ZB_ZCL_CLUSTER_ID_SCENES,
            ZB_ZCL_CLUSTER_ID_ON_OFF,
            ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL,
            ZB_ZCL_CLUSTER_ID_COLOR_CONTROL,
        }
    };

    context.endpoint_descriptor = {
        endpoint,
        ZB_AF_HA_PROFILE_ID,
        nullptr,
        nullptr,
        0,
        nullptr,
        kLightLinkColorLightClusterCount,
        cluster_descriptors,
        (zb_af_simple_desc_1_1_t*) &context.simple_descriptor,
        0,
        ZB_ZCL_COLOR_CONTROL_REPORT_ATTR_COUNT,
        context.reporting_info,
        ZB_ZCL_COLOR_DIMMABLE_LIGHT_CVC_ATTR_COUNT,
        context.cvc_alarm_info,
    };
    return context;
}

void LightLinkColorLight::Init(const std::string& name) {
    basic.Init(name);
    identify.Init();
    on_off.Init(endpoint);
    level_control.Init(endpoint);
    color_control.Init();
}

void LightLinkColorLight::RecalculateRgbFromLast() {
    const auto mode = color_control.attributes.set_color_info.color_mode;
    if (mode == ZB_ZCL_COLOR_CONTROL_COLOR_MODE_HUE_SATURATION) {
        RecalculateRgbFromHsb();
    } else if (mode == ZB_ZCL_COLOR_CONTROL_COLOR_MODE_CURRENT_X_Y) {
        RecalculateRgbFromXy();
    } else {
        NRF_LOG_WARNING("Color light: unexpected color mode: %d", mode);
    }
}

void LightLinkColorLight::RecalculateRgbFromHsb() {
    color = ConvertHsbToRgb(color_control.attributes.set_color_info.current_hue,
                            color_control.attributes.set_color_info.current_saturation,
                            level_control.GetLevel());
    SendColorUpdate();
}

void LightLinkColorLight::RecalculateRgbFromXy() {
    color = ConvertXyToRgb(color_control.attributes.set_color_info.current_X,
                           color_control.attributes.set_color_info.current_Y,
                           level_control.GetLevel());
    SendColorUpdate();
}

void LightLinkColorLight::SetHue(uint8_t hue) {
    NRF_LOG_INFO("Color light: set color hue value: %i", hue);
    color_control.attributes.set_color_info.current_hue = hue;
    RecalculateRgbFromHsb();
}


void LightLinkColorLight::SetSaturation(uint8_t saturation) {
    NRF_LOG_INFO("Color light: set color saturation value: %i", saturation);
    color_control.attributes.set_color_info.current_saturation = saturation;
    RecalculateRgbFromHsb();
}


void LightLinkColorLight::SetBrightness(uint8_t brightness) {
    NRF_LOG_INFO("Color light: set level value: %i", brightness);
    level_control.SetLevel(brightness);
    // According to the table 7.3 of Home Automation Profile Specification v 1.2 rev 29, chapter 7.1.3.
    on_off.SetOn(brightness != 0);
    RecalculateRgbFromLast();
}

void LightLinkColorLight::SetOn(bool is_on) {
    NRF_LOG_INFO("Color light: set ON/OFF value: %i", is_on);
    on_off.SetOn(is_on);
    if (is_on)
    {
        RecalculateRgbFromLast();
    }
    else
    {
        color = {0, 0, 0};
        SendColorUpdate();
    }
}

void LightLinkColorLight::SendColorUpdate() {
    NRF_LOG_INFO("Color light: Setting color to %d %d %d", color.r_value, color.g_value, color.b_value);
    char buffer[20]; // Maximal string is 'RGB 255 255 255\n\r\0' which is 18 symbols
    int n = sprintf(buffer, "RGB %d %d %d\n\r", color.r_value,color.g_value, color.b_value);
    nrf_serial_write(serial, buffer, n, nullptr, NRF_SERIAL_MAX_TIMEOUT);
    nrf_serial_flush(serial, NRF_SERIAL_MAX_TIMEOUT);
}

uint16_t LightLinkColorLight::GetRemainingTime() const {
    return color_control.attributes.set_color_info.remaining_time;
}

void LightLinkColorLight::ActuateColorUpdate() {
    if (dirty) {
        RecalculateRgbFromLast();
        dirty = false;
    }
}