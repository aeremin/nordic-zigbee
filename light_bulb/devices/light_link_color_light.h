#ifndef DEVICES_LIGHT_LINK_COLOR_LIGHT
#define DEVICES_LIGHT_LINK_COLOR_LIGHT

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#include "zboss_api.h"
#include "zb_zcl_common.h"
#include "nrf_serial.h"

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
#include "color_helpers.h"

class LightLinkColorLight
{
public:
    LightLinkColorLight(const nrf_serial_t* serial);

    void Init(const std::string& name, zb_uint8_t endpoint);

    void SetBrightness(uint8_t brightness);
    void SetHue(uint8_t hue);
    void SetSaturation(uint8_t saturation);

    void SetOn(bool is_on);

    // TODO: Remove it
    uint16_t GetRemainingTime() const;

    void RecalculateRgbFromXy();

    zb_zcl_cluster_desc_t cluster_descriptors[7];
private:
    BasicCluster basic;
    IdentifyCluster identify;
    GroupsCluster groups;
    ScenesCluster scenes;
    OnOffCluster on_off;
    LevelControlCluster level_control;
    ColorControlCluster color_control;

    RgbColor color;
    const nrf_serial_t* serial;

    void RecalculateRgbFromHsb();
    void SendColorUpdate();
};

#endif // DEVICES_LIGHT_LINK_COLOR_LIGHT

