#pragma once

#include <map>
#include <tuple>
#include <deque>

#include "esp_zigbee_core.h"
#include "zboss_api.h"
#include "ha/esp_zigbee_ha_standard.h"
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "zigbee_helpers.h"

namespace esphome {
namespace zigbee {

static const char *const TAG = "zigbee";

using device_params_t = struct DeviceParamsS {
  esp_zb_ieee_addr_t ieee_addr;
  uint8_t endpoint;
  uint16_t short_addr;
};

using zdo_info_user_ctx_t = struct ZdoInfoCtxS {
  uint8_t endpoint;
  uint16_t short_addr;
};

/* Zigbee configuration */
#define INSTALLCODE_POLICY_ENABLE false /* enable the install code policy for security */
#define ED_AGING_TIMEOUT ESP_ZB_ED_AGING_TIMEOUT_64MIN
#define ED_KEEP_ALIVE 3000 /* 3000 millisecond */

#define MAX_CHILDREN 10                     /* the max amount of connected devices */
#define INSTALLCODE_POLICY_ENABLE false     /* enable the install code policy for security */
#define HA_COLOR_DIMMABLE_LIGHT_ENDPOINT 10 /* esp light switch device endpoint */

#define ESP_ZB_PRIMARY_CHANNEL_MASK \
  ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK /* Zigbee primary channel mask use in the example */

#define ESP_ZB_ZED_CONFIG() \
  { \
    .esp_zb_role = ESP_ZB_DEVICE_TYPE_ED, .install_code_policy = INSTALLCODE_POLICY_ENABLE, \
    .nwk_cfg = { \
        .zed_cfg = \
            { \
                .ed_timeout = ED_AGING_TIMEOUT, \
                .keep_alive = ED_KEEP_ALIVE, \
            }, \
    }, \
  }

#define ESP_ZB_ZR_CONFIG() \
  { \
    .esp_zb_role = ESP_ZB_DEVICE_TYPE_ROUTER, .install_code_policy = INSTALLCODE_POLICY_ENABLE, \
    .nwk_cfg = { \
        .zczr_cfg = \
            { \
                .max_children = MAX_CHILDREN, \
            }, \
    }, \
  }

#define ESP_ZB_DEFAULT_RADIO_CONFIG() \
  { .radio_mode = ZB_RADIO_MODE_NATIVE, }

#define ESP_ZB_DEFAULT_HOST_CONFIG() \
  { .host_connection_mode = ZB_HOST_CONNECTION_MODE_NONE, }

template<class T> T get_value_by_type(uint8_t attr_type, void *data);

class ZigBeeComponent : public Component {
 public:
  void setup() override;
  void dump_config() override;
  esp_err_t create_endpoint(uint8_t endpoint_id, esp_zb_ha_standard_devices_t device_id);
  void set_ident_time(uint8_t ident_time);
  void set_basic_cluster(std::string model, std::string manufacturer, std::string date, uint8_t power,
                         uint8_t app_version, uint8_t stack_version, uint8_t hw_version, std::string area,
                         uint8_t physical_env);
  void add_cluster(uint8_t endpoint_id, uint16_t cluster_id, uint8_t role);
  void create_default_cluster(uint8_t endpoint_id, esp_zb_ha_standard_devices_t device_id);

  template<typename T>
  void add_attr(uint8_t endpoint_id, uint16_t cluster_id, uint8_t role, uint16_t attr_id, uint8_t attr_type,
                uint8_t attr_access, T value_p);

  void set_attr(uint8_t endpoint_id, uint16_t cluster_id, uint8_t role, uint16_t attr_id, void *value_p);
  void set_report(uint8_t endpoint_id, uint16_t cluster_id, uint8_t role, uint16_t attr_id);

  void reset() {
    esp_zb_lock_acquire(portMAX_DELAY);
    esp_zb_factory_reset();
    esp_zb_lock_release();
  }
  void report();

  void add_on_join_callback(std::function<void()> &&callback) { this->on_join_callback_.add(std::move(callback)); }
  void add_on_value_callback(
      std::function<void(esp_zb_device_cb_common_info_t info, esp_zb_zcl_attribute_t attribute)> callback) {
    on_value_callback_.add(std::move(callback));
  }

  CallbackManager<void()> on_join_callback_{};
  CallbackManager<void(esp_zb_device_cb_common_info_t info, esp_zb_zcl_attribute_t attribute)> on_value_callback_{};
  std::deque<esp_zb_zcl_reporting_info_t> reporting_list;

 protected:
  void esp_zb_task_();
  esp_zb_attribute_list_t *create_ident_cluster_();
  esp_zb_attribute_list_t *create_basic_cluster_();
  bool connected_ = false;
  std::map<uint8_t, esp_zb_ha_standard_devices_t> endpoint_list_;
  std::map<uint8_t, esp_zb_cluster_list_t *> cluster_list_;
  std::map<std::tuple<uint8_t, uint16_t, uint8_t>, esp_zb_attribute_list_t *> attribute_list_;
  esp_zb_nwk_device_type_t device_role_ = ESP_ZB_DEVICE_TYPE_ED;
  esp_zb_ep_list_t *esp_zb_ep_list_ = esp_zb_ep_list_create();
  struct {
    std::string model;
    std::string manufacturer;
    std::string date;
    uint8_t power;
    uint8_t app_version;
    uint8_t stack_version;
    uint8_t hw_version;
    std::string area;
    uint8_t physical_env;
  } basic_cluster_data_;
  uint8_t ident_time_;
};

extern "C" void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct);

template<typename T>
void ZigBeeComponent::add_attr(uint8_t endpoint_id, uint16_t cluster_id, uint8_t role, uint16_t attr_id,
                               uint8_t attr_type, uint8_t attr_access, T value_p) {
  esp_zb_attribute_list_t *attr_list = this->attribute_list_[{endpoint_id, cluster_id, role}];
  if (esphome_zb_cluster_add_or_update_attr(cluster_id, attr_list, attr_id, attr_type, attr_access, &value_p) !=
      ESP_OK) {
    ESP_LOGE(TAG, "Could not add attribute 0x%04X to cluster 0x%04X in endpoint %u", attr_id, cluster_id, endpoint_id);
  }
}

}  // namespace zigbee
}  // namespace esphome
