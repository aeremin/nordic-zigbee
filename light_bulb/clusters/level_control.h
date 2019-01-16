#ifndef APP_LEVEL_CONTROL
#define APP_LEVEL_CONTROL

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

class LevelControlCluster {
public:
  LevelControlCluster();

  void Init(zb_uint8_t endpoint);
  void SetLevel(uint8_t level);
  uint16_t GetLevel() const;

  zb_zcl_attr_t attributes_list[5];
private:
  zb_dev_level_control_attr_t attributes;
  // No idea why it's needed. Was present in ZB_ZCL_DECLARE_LEVEL_CONTROL_ATTRIB_LIST macro
  zb_zcl_level_control_move_status_t move_status;
};

#endif // APP_LEVEL_CONTROL