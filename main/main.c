#include <stdio.h>
#include <string.h>

#include "driver/wifi_storage.c"

/*Driver sensor de temperatura*/
#include "driver/ds18b20_example.c"


/*Driver de wifi*/
#include "driver/station_example.c"

/*Driver del accespoint*/
#include "driver/accesPoint.c"

/*Web server*/
#include "driver/wifi_web_server.c"

/*Freertos*/
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

/*Sensor de temperatura*/
#define DS18B20_GPIO 15

/*mqtt*/
#include "driver/mqtt_client.c"


/*Tiempo que tarda el sensor en leer*/
#define DS18B20_READ_INTERVAL_MS 2000



/*Credenciales del accespoint*/
#define WIFI_AP_SSID       "ESP32_AP"
#define WIFI_AP_PASSWORD   "12345678"

/*Evento Para esperar las credenciales de wifi*/
#define WIFI_CREDENTIALS_READY BIT0

#define WIFI_DISCONNECTED_BIT BIT1

static EventGroupHandle_t wifi_event_group = NULL;

/* Credenciales recibidas desde el formulario */
char wifi_ssid[32];
char wifi_password[64];

/*tarea para leer sensor y le paso el pin y tiempo como parametros*/
static void ds18b20_task(void *pvParameters)
{
    ds18b20_example_run(15, 2000);
}

/* Notificar que llegaron las credenciales */
void notify_wifi_credentials(const char *ssid, const char *password)
{
    strncpy(wifi_ssid, ssid, sizeof(wifi_ssid) - 1);
    wifi_ssid[sizeof(wifi_ssid) - 1] = '\0';

    strncpy(wifi_password, password, sizeof(wifi_password) - 1);
    wifi_password[sizeof(wifi_password) - 1] = '\0';

    xEventGroupSetBits(wifi_event_group, WIFI_CREDENTIALS_READY);
}

void notify_wifi_disconnected(void)
{
    xEventGroupSetBits(
        wifi_event_group,
        WIFI_DISCONNECTED_BIT
    );
}

/*esperar credenciales*/

void wait_for_wifi_credentials(void)
{
    ESP_LOGI("WIFI", "Esperando credenciales WiFi...");
    
    /*Una vez tengo las credenciales doy el ready*/
    xEventGroupWaitBits(
        wifi_event_group,
        WIFI_CREDENTIALS_READY,
        pdFALSE,
        pdTRUE,
        portMAX_DELAY
    );

    ESP_LOGI("WIFI", "Credenciales recibidas");
}

void start_wifi_configuration(void)
{
    ESP_LOGW("MAIN", "Iniciando modo configuracion");

    /* Volver al modo Access Point */
    wifi_start_ap(WIFI_AP_SSID, WIFI_AP_PASSWORD);

    /* Levantar servidor web */
    start_web_server();

    /* Esperar nuevas credenciales */
    wait_for_wifi_credentials();

    /* Limpiar evento de credenciales recibidas */
    xEventGroupClearBits(
        wifi_event_group,
        WIFI_CREDENTIALS_READY
    );

    /* Intentar conectar con las nuevas credenciales */
    if (wifi_init_sta(wifi_password, wifi_ssid) == ESP_OK) {
    ESP_LOGI(TAG, "WiFi conectado correctamente");

    mqtt_client_start();
    }
    
}


void app_main(void)
{
    esp_err_t err = wifi_storage_init();

    if (err != ESP_OK) {
        ESP_LOGE("MAIN", "No se pudo inicializar el almacenamiento");
        return;
    }

    /* Recuperar credenciales guardadas */
    err = wifi_credentials_load(
        wifi_ssid,
        sizeof(wifi_ssid),
        wifi_password,
        sizeof(wifi_password)
    );

    /* Crear grupo de eventos */
    wifi_event_group = xEventGroupCreate();

    if (wifi_event_group == NULL) {
        ESP_LOGE("MAIN", "Error al crear grupo de eventos");
        return;
    }
    
    
    /*Leer sensor*/
        xTaskCreate(
        ds18b20_task,
        "ds18b20_task",
        4096,
        NULL,
        5,
        NULL
        );

    /* Inicializar infraestructura WiFi */
    wifi_init_ap(WIFI_AP_SSID, WIFI_AP_PASSWORD);

    if (err == ESP_OK) {
        ESP_LOGI("MAIN", "Credenciales WiFi encontradas");

        err = wifi_init_sta(wifi_password, wifi_ssid);
        
        if (err == ESP_OK) {
        ESP_LOGI("MAIN", "WiFi conectado correctamente");
        mqtt_client_start();
        }
    } else {
        ESP_LOGI("MAIN", "No hay credenciales WiFi guardadas");
        err = ESP_FAIL;
    }

    /* Si no pudo conectarse, iniciar configuracion */
    if (err != ESP_OK) {
        start_wifi_configuration();
    }

    /* Mantener el programa escuchando eventos */
    while (1) {

        EventBits_t bits = xEventGroupWaitBits(
            wifi_event_group,
            WIFI_DISCONNECTED_BIT,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY
        );

        if (bits & WIFI_DISCONNECTED_BIT) {

            ESP_LOGW("MAIN", "Se perdio la conexion WiFi");

            /* Limpiar evento de desconexion */
            xEventGroupClearBits(
                wifi_event_group,
                WIFI_DISCONNECTED_BIT
            );

            start_wifi_configuration();
        }
    }

    
}