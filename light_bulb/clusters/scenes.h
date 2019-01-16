#ifndef APP_SCENES
#define APP_SCENES

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

class ScenesCluster {
public:
  ScenesCluster();

  zb_zcl_attr_t attributes_list[7];
private:
  zb_dev_scenes_attr_t attributes = {};
};

#endif // APP_SCENES