#include "groups.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "zb_zcl_groups.h"

#ifdef __cplusplus
}
#endif

#include "commons.h"

GroupsCluster::GroupsCluster() :
  attributes_list({
    FirstAttribute(),
    ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_GROUPS_NAME_SUPPORT_ID(&attributes.name_support),
    LastAttribute()
  })
{}
