#ifndef APP_COMMONS
#define APP_COMMONS

#ifdef __cplusplus
extern "C" {
#endif

#include "zboss_api.h"
#include "zb_types.h"
#include "zb_zcl_common.h"

#ifdef __cplusplus
}
#endif

zb_zcl_attr_t FirstAttribute();
zb_zcl_attr_t LastAttribute();

#endif // APP_COMMONS