#include "identify.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "zb_zcl_identify.h"

#ifdef __cplusplus
}
#endif

#include "commons.h"

namespace {
const zb_zcl_identify_callbacks_t KCallbacksAttributeList = { nullptr, nullptr };
}

IdentifyCluster::IdentifyCluster() :
  attributes_list({
    FirstAttribute(),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID(&attributes.identify_time),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_IDENTIFY_COMMISSION_STATE_HA_ID(&attributes.commission_state),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_IDENTIFY_CALLBACKS_HA_ID(&KCallbacksAttributeList),
    LastAttribute()
  })
{}

IdentifyCluster& IdentifyCluster::GetInstance() {
  static IdentifyCluster instance;
  return instance;
}

void IdentifyCluster::Init() {
    attributes.identify_time    = ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE;
    attributes.commission_state = ZB_ZCL_ATTR_IDENTIFY_COMMISSION_STATE_HA_ID_DEF_VALUE;
}
