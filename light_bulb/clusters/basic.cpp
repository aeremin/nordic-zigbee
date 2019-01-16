#include "basic.h"

#include <string>

#ifdef __cplusplus
extern "C"
{
#endif

#include "zb_zcl_basic.h"

#ifdef __cplusplus
}
#endif

#include "commons.h"

namespace
{
zb_bool_t kDeviceEnableAttributesList = ZB_TRUE;
}

BasicCluster::BasicCluster() : attributes_list({FirstAttribute(),
                                                ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_BASIC_ZCL_VERSION_ID(&attributes.zcl_version),
                                                ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_BASIC_APPLICATION_VERSION_ID(&attributes.app_version),
                                                ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_BASIC_STACK_VERSION_ID(&attributes.stack_version),
                                                ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_BASIC_HW_VERSION_ID(&attributes.hw_version),
                                                ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID(&attributes.mf_name),
                                                ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID(&attributes.model_id),
                                                ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_BASIC_DATE_CODE_ID(&attributes.date_code),
                                                ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_BASIC_POWER_SOURCE_ID(&attributes.power_source),
                                                ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_BASIC_LOCATION_DESCRIPTION_ID(&attributes.location_id),
                                                ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_BASIC_PHYSICAL_ENVIRONMENT_ID(&attributes.ph_env),
                                                ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_BASIC_DEVICE_ENABLED_ID(kDeviceEnableAttributesList),
                                                LastAttribute()})
{
}

BasicCluster &BasicCluster::GetInstance()
{
  static BasicCluster instance;
  return instance;
}

void BasicCluster::Init(const std::string& model_id, zb_zcl_basic_power_source_e power_source)
{
  /* Basic cluster attributes data */
  attributes.zcl_version = ZB_ZCL_VERSION;
  attributes.app_version = 1;
  attributes.stack_version = 10;
  attributes.hw_version = 1;

  /* Use ZB_ZCL_SET_STRING_VAL to set strings, because the first byte should
     * contain string length without trailing zero.
     *
     * For example "test" string wil be encoded as:
     *   [(0x4), 't', 'e', 's', 't']
     */
  const char kManufacturerName[] = "aeremin";
  ZB_ZCL_SET_STRING_VAL(attributes.mf_name,
                        kManufacturerName,
                        ZB_ZCL_STRING_CONST_SIZE(kManufacturerName));

  ZB_ZCL_SET_STRING_VAL(attributes.model_id,
                        model_id.data(),
                        model_id.length());

  const char kManufactureDate[] = "20190118";
  ZB_ZCL_SET_STRING_VAL(attributes.date_code,
                        kManufactureDate,
                        ZB_ZCL_STRING_CONST_SIZE(kManufactureDate));

  attributes.power_source = power_source;

  const char kLocation[] = "Undefined";
  ZB_ZCL_SET_STRING_VAL(attributes.location_id,
                        kLocation,
                        ZB_ZCL_STRING_CONST_SIZE(kLocation));

  attributes.ph_env = ZB_ZCL_BASIC_ENV_UNSPECIFIED;
}
