#include <stdio.h>
#include <stdbool.h>
#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG_MQTT = "MQTT_CLIENT";

static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqtt_connected = false;

// Certificado del broker Mosquitto
extern const uint8_t mosquitto_crt_start[] asm("_binary_mosquitto_org_crt_start");
extern const uint8_t mosquitto_crt_end[] asm("_binary_mosquitto_org_crt_end");

// Manejador de eventos MQTT
static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {

        case MQTT_EVENT_CONNECTED:
            mqtt_connected = true;
            ESP_LOGI(TAG_MQTT, "Conectado al broker MQTT");

            esp_mqtt_client_subscribe(
                event->client,
                "esp32/domotica/prueba",
                0
            );

            ESP_LOGI(TAG_MQTT, "Suscripcion realizada");
            break;

        case MQTT_EVENT_DISCONNECTED:
            mqtt_connected = false;
            ESP_LOGW(TAG_MQTT, "Desconectado del broker MQTT");
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG_MQTT, "Mensaje publicado. ID: %d", event->msg_id);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG_MQTT, "Error en la conexion MQTT");
            break;

        default:
            break;
    }
}

// Inicializa y conecta el cliente MQTT
void mqtt_client_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtts://test.mosquitto.org:8883",
        .broker.verification.certificate = (const char *)mosquitto_crt_start,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

    if (mqtt_client == NULL) {
        ESP_LOGE(TAG_MQTT, "No se pudo inicializar el cliente MQTT");
        return;
    }

    esp_mqtt_client_register_event(
        mqtt_client,
        ESP_EVENT_ANY_ID,
        mqtt_event_handler,
        NULL
    );

    esp_mqtt_client_start(mqtt_client);

    ESP_LOGI(TAG_MQTT, "Cliente MQTT iniciado");
}

// Publica la temperatura del DS18B20
void mqtt_client_publish_temperature(float temperature)
{
    if (mqtt_client == NULL || !mqtt_connected) {
        ESP_LOGW(TAG_MQTT, "No se puede publicar: MQTT desconectado");
        return;
    }

    char message[32];

    snprintf(message, sizeof(message),
             "{\"temperatura\":%.2f}", temperature);

    int msg_id = esp_mqtt_client_publish(
        mqtt_client,
        "esp32/temperatura",
        message,
        0,
        1,
        0
    );

    if (msg_id >= 0) {
        ESP_LOGI(TAG_MQTT, "Temperatura publicada: %s", message);
    } else {
        ESP_LOGE(TAG_MQTT, "Error al publicar temperatura");
    }
}