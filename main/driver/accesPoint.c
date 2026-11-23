
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"




#define WIFI_AP_CHANNEL    1
#define WIFI_AP_MAX_CONN   4

static const char *TAG = "WIFI_AP";
void wifi_init_ap(const char *ap_ssid, const char *ap_pass)
{
    // Inicializar interfaz de red
    ESP_ERROR_CHECK(esp_netif_init());

    // Crear loop de eventos
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Crear interfaz Access Point
    esp_netif_create_default_wifi_ap();

    // Inicializar driver Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Configurar y activar el AP
    wifi_config_t wifi_config = {0};

    strncpy((char *)wifi_config.ap.ssid,
            ap_ssid, sizeof(wifi_config.ap.ssid) - 1);

    strncpy((char *)wifi_config.ap.password,
            ap_pass, sizeof(wifi_config.ap.password) - 1);

    wifi_config.ap.ssid_len = strlen(ap_ssid);
    wifi_config.ap.channel = WIFI_AP_CHANNEL;
    wifi_config.ap.max_connection = WIFI_AP_MAX_CONN;
    wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Access Point iniciado");
    ESP_LOGI(TAG, "SSID: %s", ap_ssid);
    ESP_LOGI(TAG, "IP: 192.168.4.1");
}

extern void set_intentional_disconnect(bool state);

void wifi_start_ap(const char *ap_ssid, const char *ap_pass)
{
    wifi_config_t wifi_config = {0};

    /* Indicar que el cambio de modo es intencional */
    set_intentional_disconnect(true);

    strncpy((char *)wifi_config.ap.ssid,
            ap_ssid, sizeof(wifi_config.ap.ssid) - 1);

    strncpy((char *)wifi_config.ap.password,
            ap_pass, sizeof(wifi_config.ap.password) - 1);

    wifi_config.ap.ssid_len = strlen(ap_ssid);
    wifi_config.ap.channel = WIFI_AP_CHANNEL;
    wifi_config.ap.max_connection = WIFI_AP_MAX_CONN;
    wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    ESP_ERROR_CHECK(
        esp_wifi_set_config(WIFI_IF_AP, &wifi_config)
    );

    ESP_LOGI(TAG, "Access Point reactivado");
    ESP_LOGI(TAG, "SSID: %s", ap_ssid);
}