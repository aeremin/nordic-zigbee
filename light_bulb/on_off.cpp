#include "on_off.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "zb_zcl_on_off.h"
#include "zb_zcl_level_control.h"

#ifdef __cplusplus
}
#endif

zb_uint16_t cluster_revision_on_off_attr_list = ZB_ZCL_CLUSTER_REVISION_DEFAULT;

OnOffCluster::OnOffCluster() :
  on_off_attr_list({
    {
        ZB_ZCL_ATTR_GLOBAL_CLUSTER_REVISION_ID,
        ZB_ZCL_ATTR_TYPE_U16,
        ZB_ZCL_ATTR_ACCESS_READ_ONLY,
        (zb_voidp_t) &(cluster_revision_on_off_attr_list)
    },
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID(&on_off),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ON_OFF_GLOBAL_SCENE_CONTROL(&global_scene_ctrl),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ON_OFF_ON_TIME( &on_time),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ON_OFF_OFF_WAIT_TIME(&off_wait_time),
    {
        ZB_ZCL_NULL_ID,
        0,
        0,
        nullptr
    }
  })
{}

OnOffCluster& OnOffCluster::GetInstance() {
  static OnOffCluster instance;
  return instance;
}

void OnOffCluster::SetOn(bool is_on) {
  on_off = is_on ? ZB_TRUE : ZB_FALSE;
}

void OnOffCluster::Init(zb_uint8_t endpoint) {
  SetOn(true);
  ZB_ZCL_LEVEL_CONTROL_SET_ON_OFF_VALUE(endpoint, on_off);
}
