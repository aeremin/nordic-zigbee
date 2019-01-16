#ifndef APP_IDENTIFY
#define APP_IDENTIFY

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

class IdentifyCluster {
public:
  IdentifyCluster();

  void Init();

  zb_zcl_attr_t attributes_list[5];
private:
  zb_dev_identify_attr_t attributes;
};

#endif // APP_IDENTIFY