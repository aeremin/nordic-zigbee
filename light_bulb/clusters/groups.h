#ifndef APP_GROUPS
#define APP_GROUPS

#ifdef __cplusplus
extern "C" {
#endif

#include "zboss_api.h"
#include "zb_types.h"
#include "zb_zcl_common.h"
#include "zigbee_types.h"

#ifdef __cplusplus
}
#endif

class GroupsCluster {
public:
  GroupsCluster();

  zb_zcl_attr_t attributes_list[3];
private:
  zb_dev_groups_attr_t attributes = {};
};

#endif // APP_GROUPS