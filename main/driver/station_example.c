#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_err.h"


/* Grupo de eventos de la conexion STA */
static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define WIFI_MAXIMUM_RETRY 3

static const char *TAG_wifi = "WIFI_STA";

static int s_retry_num = 0;


/* Manejador de eventos WiFi */
static void event_handler(void *arg,
                          esp_event_base_t event_base,
                          int32_t event_id,
                          void *event_data)
{
    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_DISCONNECTED) {

        if (s_retry_num < WIFI_MAXIMUM_RETRY) {

            esp_wifi_connect();
            s_retry_num++;

            ESP_LOGI(TAG_wifi, "Reintentando conexion WiFi...");

        } else {

            ESP_LOGE(TAG_wifi, "No se pudo conectar al router");

            xEventGroupSetBits(
                s_wifi_event_group,
                WIFI_FAIL_BIT
            );
        }
    }

    else if (event_base == IP_EVENT &&
             event_id == IP_EVENT_STA_GOT_IP) {

        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *) event_data;

        ESP_LOGI(TAG_wifi, "Direccion IP: " IPSTR,
                 IP2STR(&event->ip_info.ip));

        s_retry_num = 0;

        xEventGroupSetBits(
            s_wifi_event_group,
            WIFI_CONNECTED_BIT
        );
    }
}


/* Inicializacion de la estacion WiFi */
esp_err_t wifi_init_sta(const char *wifi_pass, const char *wifi_ssid)
{
    ESP_LOGI(TAG_wifi, "Configurando conexion WiFi STA...");

    /* Crear grupo de eventos */
    s_wifi_event_group = xEventGroupCreate();

    if (s_wifi_event_group == NULL) {
        ESP_LOGE(TAG_wifi, "Error al crear grupo de eventos");
        return ESP_FAIL;
    }

    /*
     * El AP ya inicializo:
     * - esp_netif
     * - event loop
     * - driver WiFi
     *
     * 
     */

    /* Crear interfaz de estacion */
    esp_netif_create_default_wifi_sta();

    /* Registrar eventos WiFi */
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &event_handler,
            NULL,
            &instance_any_id
        )
    );

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &event_handler,
            NULL,
            &instance_got_ip
        )
    );

    /* Configurar credenciales */
    wifi_config_t wifi_config = {0};

    strncpy(
        (char *)wifi_config.sta.ssid,
        wifi_ssid,
        sizeof(wifi_config.sta.ssid) - 1
    );

    strncpy(
        (char *)wifi_config.sta.password,
        wifi_pass,
        sizeof(wifi_config.sta.password) - 1
    );

    /* Cambiar de Access Point a Station */
    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_STA)
    );

    /* Aplicar configuracion */
    ESP_ERROR_CHECK(
        esp_wifi_set_config(WIFI_IF_STA, &wifi_config)
    );

    ESP_LOGI(TAG_wifi, "Intentando conectar a: %s", wifi_ssid);

    /* El driver ya estaba iniciado por el AP */
    ESP_ERROR_CHECK(
        esp_wifi_connect()
    );

    /* Esperar conexion o fallo */
    EventBits_t bits = xEventGroupWaitBits(
    s_wifi_event_group,
    WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
    pdFALSE,
    pdFALSE,
    pdMS_TO_TICKS(15000)
);

    if (bits & WIFI_CONNECTED_BIT) {

    ESP_LOGI(TAG_wifi, "Conexion WiFi exitosa");
    return ESP_OK;

} else if (bits & WIFI_FAIL_BIT) {

    ESP_LOGE(TAG_wifi, "Fallo la conexion WiFi");

} else {

    ESP_LOGE(TAG_wifi, "Tiempo de conexion agotado");
}

return ESP_FAIL;
}