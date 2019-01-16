#ifndef APP_BASIC
#define APP_BASIC

#include <string>

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

class BasicCluster {
public:
  BasicCluster();
  static BasicCluster& GetInstance();

  void Init(const std::string& model_id, zb_zcl_basic_power_source_e power_source=ZB_ZCL_BASIC_POWER_SOURCE_UNKNOWN);

  zb_zcl_attr_t attributes_list[13];
private:
  zb_dev_basic_attr_t attributes;
};

#endif // APP_BASIC