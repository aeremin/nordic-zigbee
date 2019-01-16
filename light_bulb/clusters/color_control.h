#ifndef APP_COLOR_CONTROL
#define APP_COLOR_CONTROL

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

class ColorControlCluster {
public:
  ColorControlCluster();
  void Init();

  zb_zcl_attr_t attributes_list[41];
  // TODO: Make private?
  zb_dev_color_control_attr_t attributes = {};
};

#endif // APP_COLOR_CONTROL