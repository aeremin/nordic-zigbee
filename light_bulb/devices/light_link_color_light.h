#ifndef DEVICES_LIGHT_LINK_COLOR_LIGHT
#define DEVICES_LIGHT_LINK_COLOR_LIGHT

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#include "zboss_api.h"
#include "zb_zcl_common.h"

#ifdef __cplusplus
}
#endif

#include "clusters/basic.h"
#include "clusters/color_control.h"
#include "clusters/groups.h"
#include "clusters/identify.h"
#include "clusters/level_control.h"
#include "clusters/on_off.h"
#include "clusters/scenes.h"

class LightLinkColorLight
{
public:
    LightLinkColorLight();

    void Init(const std::string& name, zb_uint8_t endpoint);

    BasicCluster basic;
    IdentifyCluster identify;
    GroupsCluster groups;
    ScenesCluster scenes;
    OnOffCluster on_off;
    LevelControlCluster level_control;
    ColorControlCluster color_control;

    zb_zcl_cluster_desc_t cluster_descriptors[7];
};

#endif // DEVICES_LIGHT_LINK_COLOR_LIGHT

