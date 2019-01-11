#include "on_off.h"
#include "zb_zcl_on_off.h"

/* On/Off cluster attributes additions data */
bulb_device_on_off_attr_t on_off_attributes;
zb_uint16_t cluster_revision_on_off_attr_list = ZB_ZCL_CLUSTER_REVISION_DEFAULT;
zb_zcl_attr_t on_off_attr_list[6] = {
    {
        ZB_ZCL_ATTR_GLOBAL_CLUSTER_REVISION_ID,
        ZB_ZCL_ATTR_TYPE_U16,
        ZB_ZCL_ATTR_ACCESS_READ_ONLY,
        (zb_voidp_t) &(cluster_revision_on_off_attr_list)
    },
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID(&on_off_attributes.on_off),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ON_OFF_GLOBAL_SCENE_CONTROL(&on_off_attributes.global_scene_ctrl),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ON_OFF_ON_TIME( &on_off_attributes.on_time),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ON_OFF_OFF_WAIT_TIME(&on_off_attributes.off_wait_time),
    {
        ZB_ZCL_NULL_ID,
        0,
        0,
        nullptr
    }
};