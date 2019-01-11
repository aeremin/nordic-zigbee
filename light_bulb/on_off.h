#ifndef APP_ON_OFF
#define APP_ON_OFF
#include "zboss_api.h"
#include "zb_types.h"
#include "zb_zcl_common.h"

/* ON/Off cluster attributes. */
struct bulb_device_on_off_attr_t
{
    zb_bool_t   on_off;
    zb_bool_t   global_scene_ctrl;
    zb_uint16_t on_time;
    zb_uint16_t off_wait_time;
};

extern bulb_device_on_off_attr_t on_off_attributes;
extern zb_zcl_attr_t on_off_attr_list[6];

#endif // APP_ON_OFF