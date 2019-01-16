#include "color_control.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "zb_zcl_color_control.h"

#ifdef __cplusplus
}
#endif

#include "commons.h"

ColorControlCluster::ColorControlCluster() :
  attributes_list({
    FirstAttribute(),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_CURRENT_HUE_ID(&attributes.set_color_info.current_hue),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_CURRENT_SATURATION_ID(&attributes.set_color_info.current_saturation),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_REMAINING_TIME_ID(&attributes.set_color_info.remaining_time),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_CURRENT_X_ID(&attributes.set_color_info.current_X),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_CURRENT_Y_ID(&attributes.set_color_info.current_Y),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_COLOR_TEMPERATURE_ID(&attributes.set_color_info.color_temperature),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_COLOR_MODE_ID(&attributes.set_color_info.color_mode),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_OPTIONS_ID(&attributes.set_color_info.options),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_NUMBER_OF_PRIMARIES_ID(&attributes.set_defined_primaries_info.number_primaries),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_PRIMARY_1_X_ID(&attributes.set_defined_primaries_info.primary_1_X),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_PRIMARY_1_Y_ID(&attributes.set_defined_primaries_info.primary_1_Y),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_PRIMARY_1_INTENSITY_ID(&attributes.set_defined_primaries_info.primary_1_intensity),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_PRIMARY_2_X_ID(&attributes.set_defined_primaries_info.primary_2_X),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_PRIMARY_2_Y_ID(&attributes.set_defined_primaries_info.primary_2_Y),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_PRIMARY_2_INTENSITY_ID(&attributes.set_defined_primaries_info.primary_2_intensity),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_PRIMARY_3_X_ID(&attributes.set_defined_primaries_info.primary_3_X),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_PRIMARY_3_Y_ID(&attributes.set_defined_primaries_info.primary_3_Y),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_PRIMARY_3_INTENSITY_ID(&attributes.set_defined_primaries_info.primary_3_intensity),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_PRIMARY_4_X_ID(&attributes.set_additional_defined_primaries_info.primary_4_X),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_PRIMARY_4_Y_ID(&attributes.set_additional_defined_primaries_info.primary_4_Y),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_PRIMARY_4_INTENSITY_ID(&attributes.set_additional_defined_primaries_info.primary_4_intensity),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_PRIMARY_5_X_ID(&attributes.set_additional_defined_primaries_info.primary_5_X),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_PRIMARY_5_Y_ID(&attributes.set_additional_defined_primaries_info.primary_5_Y),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_PRIMARY_5_INTENSITY_ID(&attributes.set_additional_defined_primaries_info.primary_5_intensity),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_PRIMARY_6_X_ID(&attributes.set_additional_defined_primaries_info.primary_6_X),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_PRIMARY_6_Y_ID(&attributes.set_additional_defined_primaries_info.primary_6_Y),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_PRIMARY_6_INTENSITY_ID(&attributes.set_additional_defined_primaries_info.primary_6_intensity),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_ENHANCED_CURRENT_HUE_ID(&attributes.set_color_info.enhanced_current_hue),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_ENHANCED_COLOR_MODE_ID(&attributes.set_color_info.enhanced_color_mode),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_COLOR_LOOP_ACTIVE_ID(&attributes.set_color_info.color_loop_active),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_COLOR_LOOP_DIRECTION_ID(&attributes.set_color_info.color_loop_direction),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_COLOR_LOOP_TIME_ID(&attributes.set_color_info.color_loop_time),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_COLOR_LOOP_START_ENHANCED_HUE_ID(&attributes.set_color_info.color_loop_start_enhanced_hue),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_COLOR_LOOP_STORED_ENHANCED_HUE_ID(&attributes.set_color_info.color_loop_stored_enhanced_hue),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_COLOR_CAPABILITIES_ID(&attributes.set_color_info.color_capabilities),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MIN_MIREDS_ID(&attributes.set_color_info.color_temp_physical_min_mireds),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MAX_MIREDS_ID(&attributes.set_color_info.color_temp_physical_max_mireds),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_COUPLE_COLOR_TEMP_TO_LEVEL_MIN_MIREDS_ID(&attributes.set_color_info.couple_color_temp_to_level_min_mireds),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_COLOR_CONTROL_START_UP_COLOR_TEMPERATURE_MIREDS_ID(&attributes.set_color_info.start_up_color_temp_mireds),
    LastAttribute()
  })
{}


 void ColorControlCluster::Init() {
    /* Color control cluster attributes data */
    attributes.set_color_info.current_hue = ZB_ZCL_COLOR_CONTROL_HUE_RED;
    attributes.set_color_info.current_saturation = ZB_ZCL_COLOR_CONTROL_CURRENT_SATURATION_MAX_VALUE;
    /* Set to use hue & saturation */
    attributes.set_color_info.color_mode = ZB_ZCL_COLOR_CONTROL_COLOR_MODE_HUE_SATURATION;
    attributes.set_color_info.color_temperature = ZB_ZCL_COLOR_CONTROL_COLOR_TEMPERATURE_DEF_VALUE;
    attributes.set_color_info.remaining_time = ZB_ZCL_COLOR_CONTROL_REMAINING_TIME_MIN_VALUE;
    attributes.set_color_info.color_capabilities = ZB_ZCL_COLOR_CONTROL_CAPABILITIES_HUE_SATURATION;
    /* According to ZCL spec 5.2.2.2.1.12 0x00 shall be set when CurrentHue and CurrentSaturation are used. */
    attributes.set_color_info.enhanced_color_mode = 0x00;
    /* According to 5.2.2.2.1.10 execute commands when device is off. */
    attributes.set_color_info.color_capabilities = ZB_ZCL_COLOR_CONTROL_OPTIONS_EXECUTE_IF_OFF;
    /* According to ZCL spec 5.2.2.2.2 0xFF shall be set when specific value is unknown. */
    attributes.set_defined_primaries_info.number_primaries = 0xff;
 }
