#ifndef APP_ON_OFF
#define APP_ON_OFF
#include <array>

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

class OnOffCluster {
public:
  OnOffCluster();
  static OnOffCluster& GetInstance();

  void Init(zb_uint8_t endpoint);
  void SetOn(bool is_on);

  zb_zcl_attr_t on_off_attr_list[6];
private:
  zb_dev_on_off_attr_t attributes;
};

#endif // APP_ON_OFF