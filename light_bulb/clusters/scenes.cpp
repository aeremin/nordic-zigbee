#include "scenes.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "zb_zcl_scenes.h"

#ifdef __cplusplus
}
#endif

#include "commons.h"

ScenesCluster::ScenesCluster() :
  attributes_list({
    FirstAttribute(),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_SCENES_SCENE_COUNT_ID(&attributes.scene_count),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_SCENES_CURRENT_SCENE_ID(&attributes.current_scene),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_SCENES_CURRENT_GROUP_ID( &attributes.current_group),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_SCENES_SCENE_VALID_ID( &attributes.scene_valid),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_SCENES_NAME_SUPPORT_ID(&attributes.name_support),
    LastAttribute()
  })
{}
