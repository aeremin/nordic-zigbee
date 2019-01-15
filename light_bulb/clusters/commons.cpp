#include "commons.h"

namespace {
zb_uint16_t kDefaultClusterRevision = ZB_ZCL_CLUSTER_REVISION_DEFAULT;
}

zb_zcl_attr_t FirstAttribute()
{
  return
  {
    ZB_ZCL_ATTR_GLOBAL_CLUSTER_REVISION_ID,
    ZB_ZCL_ATTR_TYPE_U16,
    ZB_ZCL_ATTR_ACCESS_READ_ONLY,
    (zb_voidp_t)&kDefaultClusterRevision
  };
}

zb_zcl_attr_t LastAttribute()
{
  return
  {
      ZB_ZCL_NULL_ID,
      0,
      0,
      nullptr
  };
}
