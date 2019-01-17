#include "level_control.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "zb_zcl_level_control.h"

#ifdef __cplusplus
}
#endif

#include "commons.h"

LevelControlCluster::LevelControlCluster() :
  attributes_list({
    FirstAttribute(),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID(&attributes.current_level),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_LEVEL_CONTROL_REMAINING_TIME_ID(&attributes.remaining_time),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_LEVEL_CONTROL_MOVE_STATUS_ID(&move_status),
    LastAttribute()
  })
{}

void LevelControlCluster::SetLevel(uint8_t level) {
  attributes.current_level = level;
}

uint8_t LevelControlCluster::GetLevel() const {
  return attributes.current_level;
}


void LevelControlCluster::Init(zb_uint8_t endpoint) {
  attributes.current_level  = ZB_ZCL_LEVEL_CONTROL_LEVEL_MAX_VALUE;
  attributes.remaining_time = ZB_ZCL_LEVEL_CONTROL_REMAINING_TIME_DEFAULT_VALUE;
  ZB_ZCL_LEVEL_CONTROL_SET_LEVEL_VALUE(endpoint, attributes.current_level);
}
