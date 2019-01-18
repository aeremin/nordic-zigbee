/**
 * Copyright (c) 2018, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup zigbee_examples_thingy_master_zed_color_light_bulb zigbee_color_light.h
 * @{
 * @ingroup zigbee_examples
 * @brief Dynamic multiprotocol example application to demonstrate control on BLE device (peripheral role) using zigbee device.
 */

#ifndef ZIGBEE_COLOR_LIGHT_H__
#define ZIGBEE_COLOR_LIGHT_H__

#include <stdint.h>
#include "zigbee_types.h"


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


#define ZB_HA_DEVICE_VER_COLOR_CONTROL          0                                   /**< Color light device version. */
#define ZB_HA_COLOR_CONTROL_IN_CLUSTER_NUM      7                                   /**< Color light input clusters number. */
#define ZB_HA_COLOR_CONTROL_OUT_CLUSTER_NUM     0                                   /**< Color light output clusters number. */
#define ZB_ZCL_COLOR_DIMMABLE_LIGHT_CVC_ATTR_COUNT (ZB_HA_DIMMABLE_LIGHT_CVC_ATTR_COUNT + 3)


struct EndpointContext {
    zb_af_endpoint_desc_t endpoint_descriptor;
    zb_zcl_cvc_alarm_variables_t cvc_alarm_info[ZB_ZCL_COLOR_DIMMABLE_LIGHT_CVC_ATTR_COUNT];
    zb_zcl_reporting_info_t reporting_info[ZB_ZCL_COLOR_CONTROL_REPORT_ATTR_COUNT];
    zb_af_simple_descriptor<ZB_HA_COLOR_CONTROL_IN_CLUSTER_NUM + ZB_HA_COLOR_CONTROL_OUT_CLUSTER_NUM> simple_descriptor;
};

// EndpointContext::endpoint_descriptor contains pointers to another fields of EndpointContext,
// so it must not outlive EndpointContext structure.
EndpointContext CreateColorLightEndpoint(zb_uint8_t endpoint, /*array of size 7 */ zb_zcl_cluster_desc_t* cluster_descriptors) {
    EndpointContext context;
    context.simple_descriptor = {
        endpoint,
        ZB_AF_ZLL_PROFILE_ID,
        0x0210,
        ZB_HA_DEVICE_VER_COLOR_CONTROL,
        0,
        ZB_HA_COLOR_CONTROL_IN_CLUSTER_NUM,
        ZB_HA_COLOR_CONTROL_OUT_CLUSTER_NUM,
        {
            ZB_ZCL_CLUSTER_ID_BASIC,
            ZB_ZCL_CLUSTER_ID_IDENTIFY,
            ZB_ZCL_CLUSTER_ID_GROUPS,
            ZB_ZCL_CLUSTER_ID_SCENES,
            ZB_ZCL_CLUSTER_ID_ON_OFF,
            ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL,
            ZB_ZCL_CLUSTER_ID_COLOR_CONTROL,
        }
    };

    context.endpoint_descriptor = {
        endpoint,
        ZB_AF_HA_PROFILE_ID,
        nullptr,
        nullptr,
        0,
        nullptr,
        7,
        cluster_descriptors,
        (zb_af_simple_desc_1_1_t*) &context.simple_descriptor,
        0,
        ZB_ZCL_COLOR_CONTROL_REPORT_ATTR_COUNT,
        context.reporting_info,
        ZB_ZCL_COLOR_DIMMABLE_LIGHT_CVC_ATTR_COUNT,
        context.cvc_alarm_info,
    };
    return context;
}

#endif // ZIGBEE_COLOR_LIGHT_H__
