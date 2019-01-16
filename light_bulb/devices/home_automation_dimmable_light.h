#ifndef DEVICES_HOME_AUTOMATION_DIMMABLE_LIGHT
#define DEVICES_HOME_AUTOMATION_DIMMABLE_LIGHT

#ifdef __cplusplus
extern "C" {
#endif

#include "zboss_api.h"
#include "zb_zcl_common.h"

#ifdef __cplusplus
}
#endif

#include "clusters/basic.h"
#include "clusters/groups.h"
#include "clusters/identify.h"
#include "clusters/level_control.h"
#include "clusters/on_off.h"
#include "clusters/scenes.h"

class HomeAutomationDimmableLight
{
public:
    HomeAutomationDimmableLight();

    BasicCluster basic;
    IdentifyCluster identify;
    GroupsCluster groups;
    ScenesCluster scenes;
    OnOffCluster on_off;
    LevelControlCluster level_control;

    zb_zcl_cluster_desc_t cluster_descriptors[6];
};

#endif // DEVICES_HOME_AUTOMATION_DIMMABLE_LIGHT