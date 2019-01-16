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

#ifdef __cplusplus
extern "C" {
#endif

#define ZB_HA_DEVICE_VER_COLOR_CONTROL          0                                   /**< Color light device version. */
#define ZB_HA_COLOR_CONTROL_IN_CLUSTER_NUM      8                                   /**< Color light input clusters number. */
#define ZB_HA_COLOR_CONTROL_OUT_CLUSTER_NUM     0                                   /**< Color light output clusters number. */

#define ZB_ZCL_COLOR_DIMMABLE_LIGHT_CVC_ATTR_COUNT (ZB_HA_DIMMABLE_LIGHT_CVC_ATTR_COUNT + 3)

/**@brief Redefinition of __CAT__ as variadic macro
 *
 * @param[IN]  a   Mandatory argument to concatenate
 * @param[IN]  b   Mandatory argument to concatenate
 * @param[IN]  ... Optional argument to concatenate
 */
#define __CAT_VA__(a, b, ...)                               a## __VA_ARGS__## b

/**@brief Redefinition of ZB_AF_SIMPLE_DESC_TYPE as variadic macro
 *
 * @param[IN]  ...      Optional argument to concatenate to type name
 */
#define ZB_AF_SIMPLE_DESC_TYPE_VA(in_num, out_num, ...)     __CAT_VA__(zb_af_simple_desc_, _t, __VA_ARGS__)

/**@brief Redefinition of ZB_DECLARE_SIMPLE_DESC as variadic macro
 *
 * @param[IN]  in_clusters_count   Number of input clusters
 * @param[IN]  out_clusters_count  Number of output clusters
 * @param[IN]  ...                 Optional argument to concatenate to type name
 */
#define ZB_DECLARE_SIMPLE_DESC_VA(in_clusters_count, out_clusters_count, ...)                                   \
    typedef ZB_PACKED_PRE struct zb_af_simple_desc_## __VA_ARGS__## _s                                          \
    {                                                                                                           \
        zb_uint8_t    endpoint;                                         /* Endpoint */                          \
        zb_uint16_t   app_profile_id;                                   /* Application profile identifier */    \
        zb_uint16_t   app_device_id;                                    /* Application device identifier */     \
        zb_bitfield_t app_device_version:4;                             /* Application device version */        \
        zb_bitfield_t reserved:4;                                       /* Reserved */                          \
        zb_uint8_t    app_input_cluster_count;                          /* Application input cluster count */   \
        zb_uint8_t    app_output_cluster_count;                         /* Application output cluster count */  \
        /* Application input and output cluster list */                                                         \
        zb_uint16_t   app_cluster_list[in_clusters_count + out_clusters_count];                                 \
    } ZB_PACKED_STRUCT zb_af_simple_desc_## __VA_ARGS__## _t

/**@brief Declare color light simple descriptor.
 *
 * @param[IN] ep_name                endpoint variable name.
 * @param[IN] ep_id [IN]             endpoint ID.
 * @param[IN] in_clust_num           number of supported input clusters.
 * @param[IN] out_clust_num          number of supported output clusters.
 *
 * @note in_clust_num, out_clust_num should be defined by numeric constants, not variables or any
 * definitions, because these values are used to form simple descriptor type name.
 */

// 0xc05e,
// 0x0210,

#define ZB_ZCL_DECLARE_COLOR_LIGHT_SIMPLE_DESC(ep_name, ep_id, in_clust_num, out_clust_num)             \
            ZB_DECLARE_SIMPLE_DESC_VA(in_clust_num, out_clust_num, ep_name);                            \
            ZB_AF_SIMPLE_DESC_TYPE_VA(in_clust_num, out_clust_num, ep_name)  simple_desc_## ep_name =   \
            {                                                                                           \
                ep_id,                                                                                  \
                ZB_AF_ZLL_PROFILE_ID,                                                                    \
                0x0210,                                                                                 \
                ZB_HA_DEVICE_VER_COLOR_CONTROL,                                                         \
                0,                                                                                      \
                in_clust_num,                                                                           \
                out_clust_num,                                                                          \
                {                                                                                       \
                  ZB_ZCL_CLUSTER_ID_BASIC,                                                              \
                  ZB_ZCL_CLUSTER_ID_IDENTIFY,                                                           \
                  ZB_ZCL_CLUSTER_ID_GROUPS,                                                             \
                  ZB_ZCL_CLUSTER_ID_SCENES,                                                             \
                  ZB_ZCL_CLUSTER_ID_ON_OFF,                                                             \
                  ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL,                                                      \
                  ZB_ZCL_CLUSTER_ID_COLOR_CONTROL,                                                      \
                }                                                                                       \
            }

/**@brief Declare Color Light endpoint.
 *
 * @param[IN] ep_name                endpoint variable name.
 * @param[IN] ep_id [IN]             endpoint ID.
 * @param[IN] cluster_list [IN]      list of endpoint clusters
 */
#define ZB_ZCL_DECLARE_COLOR_LIGHT_EP(ep_name, ep_id, cluster_list)                 \
    ZB_ZCL_DECLARE_COLOR_LIGHT_SIMPLE_DESC(                                         \
        ep_name,                                                                    \
        ep_id,                                                                      \
        ZB_HA_COLOR_CONTROL_IN_CLUSTER_NUM,                                         \
        ZB_HA_COLOR_CONTROL_OUT_CLUSTER_NUM);                                       \
    ZBOSS_DEVICE_DECLARE_REPORTING_CTX(reporting_info## ep_name,                    \
        (ZB_ZCL_COLOR_CONTROL_REPORT_ATTR_COUNT));                                  \
    ZBOSS_DEVICE_DECLARE_LEVEL_CONTROL_CTX(cvc_alarm_info## ep_name,                \
        ZB_ZCL_COLOR_DIMMABLE_LIGHT_CVC_ATTR_COUNT);                                \
    ZB_AF_DECLARE_ENDPOINT_DESC(                                                    \
        ep_name,                                                                    \
        ep_id,                                                                      \
        ZB_AF_HA_PROFILE_ID,                                                        \
        0,                                                                          \
        NULL                                       ,                                \
        ZB_ZCL_ARRAY_SIZE(                                                          \
            cluster_list,                                                           \
            zb_zcl_cluster_desc_t),                                                 \
        cluster_list,                                                               \
        (zb_af_simple_desc_1_1_t*)&simple_desc_## ep_name,                          \
        ZB_ZCL_COLOR_CONTROL_REPORT_ATTR_COUNT,                                     \
        reporting_info## ep_name,                                                   \
        ZB_ZCL_COLOR_DIMMABLE_LIGHT_CVC_ATTR_COUNT,                                 \
        cvc_alarm_info## ep_name)

#ifdef __cplusplus
}
#endif
#endif // ZIGBEE_COLOR_LIGHT_H__


/**
 * @}
 */
