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

#define ZB_ZCL_COLOR_DIMMABLE_LIGHT_CVC_ATTR_COUNT (ZB_HA_DIMMABLE_LIGHT_CVC_ATTR_COUNT + 3)

template<int ClustersCount>
ZB_PACKED_PRE struct zb_af_simple_descriptor
{
    zb_uint8_t    endpoint;                                         /* Endpoint */
    zb_uint16_t   app_profile_id;                                   /* Application profile identifier */
    zb_uint16_t   app_device_id;                                    /* Application device identifier */
    zb_bitfield_t app_device_version:4;                             /* Application device version */
    zb_bitfield_t reserved:4;                                       /* Reserved */
    zb_uint8_t    app_input_cluster_count;                          /* Application input cluster count */
    zb_uint8_t    app_output_cluster_count;                         /* Application output cluster count */
    /* Application input and output cluster list */
    zb_uint16_t   app_cluster_list[ClustersCount];
} ZB_PACKED_STRUCT;

constexpr const int kLightLinkColorLightClusterCount = 7;

// EndpointContext::endpoint_descriptor contains pointers to another fields of EndpointContext,
// so it must not outlive EndpointContext structure.
struct EndpointContext {
    zb_af_endpoint_desc_t endpoint_descriptor;
    zb_zcl_cvc_alarm_variables_t cvc_alarm_info[ZB_ZCL_COLOR_DIMMABLE_LIGHT_CVC_ATTR_COUNT];
    zb_zcl_reporting_info_t reporting_info[ZB_ZCL_COLOR_CONTROL_REPORT_ATTR_COUNT];
    zb_af_simple_descriptor<kLightLinkColorLightClusterCount> simple_descriptor;
};

class LightLinkColorLight
{
public:
    LightLinkColorLight(const nrf_serial_t* serial, zb_uint8_t endpoint);

    EndpointContext CreateColorLightEndpoint();

    void Init(const std::string& name);

    void SetBrightness(uint8_t brightness);
    void SetHue(uint8_t hue);
    void SetSaturation(uint8_t saturation);

    void SetOn(bool is_on);

    // TODO: Remove it
    uint16_t GetRemainingTime() const;

    void RecalculateRgbFromXy();

private:
    BasicCluster basic;
    IdentifyCluster identify;
    GroupsCluster groups;
    ScenesCluster scenes;
    OnOffCluster on_off;
    LevelControlCluster level_control;
    ColorControlCluster color_control;

    zb_zcl_cluster_desc_t cluster_descriptors[kLightLinkColorLightClusterCount];

    RgbColor color;
    const zb_uint8_t endpoint;
    const nrf_serial_t* serial;

    void RecalculateRgbFromHsb();
    void SendColorUpdate();
};

#endif // DEVICES_LIGHT_LINK_COLOR_LIGHT

