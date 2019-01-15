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
    {
        ZB_ZCL_ATTR_GLOBAL_CLUSTER_REVISION_ID,
        ZB_ZCL_ATTR_TYPE_U16,
        ZB_ZCL_ATTR_ACCESS_READ_ONLY,
        (zb_voidp_t) &kDefaultClusterRevision
    },
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID(&attributes.current_level),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_LEVEL_CONTROL_REMAINING_TIME_ID(&attributes.remaining_time),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_LEVEL_CONTROL_MOVE_STATUS_ID(&move_status),
    {
        ZB_ZCL_NULL_ID,
        0,
        0,
        nullptr
    }
  })
{}

LevelControlCluster& LevelControlCluster::GetInstance() {
  static LevelControlCluster instance;
  return instance;
}

void LevelControlCluster::SetLevel(uint8_t level) {
  attributes.current_level = level;
}

uint16_t LevelControlCluster::GetLevel() const {
  return attributes.current_level;
}


void LevelControlCluster::Init(zb_uint8_t endpoint) {
  attributes.current_level  = ZB_ZCL_LEVEL_CONTROL_LEVEL_MAX_VALUE;
  attributes.remaining_time = ZB_ZCL_LEVEL_CONTROL_REMAINING_TIME_DEFAULT_VALUE;
  ZB_ZCL_LEVEL_CONTROL_SET_LEVEL_VALUE(endpoint, attributes.current_level);
}
