#include "on_off.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "zb_zcl_on_off.h"
#include "zb_zcl_level_control.h"

#ifdef __cplusplus
}
#endif

#include "commons.h"

OnOffCluster::OnOffCluster() :
  attributes_list({
    FirstAttribute(),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID(&attributes.on_off),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ON_OFF_GLOBAL_SCENE_CONTROL(&attributes.global_scene_ctrl),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ON_OFF_ON_TIME( &attributes.on_time),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ON_OFF_OFF_WAIT_TIME(&attributes.off_wait_time),
    LastAttribute()
  })
{}

OnOffCluster& OnOffCluster::GetInstance() {
  static OnOffCluster instance;
  return instance;
}

void OnOffCluster::SetOn(bool is_on) {
  attributes.on_off = is_on ? ZB_TRUE : ZB_FALSE;
}

void OnOffCluster::Init(zb_uint8_t endpoint) {
  SetOn(true);
  ZB_ZCL_LEVEL_CONTROL_SET_ON_OFF_VALUE(endpoint, attributes.on_off);
}
