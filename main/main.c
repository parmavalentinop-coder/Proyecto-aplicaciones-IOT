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

/*Tiempo que tarda el sensor en leer*/
#define DS18B20_READ_INTERVAL_MS 2000



/*Credenciales del accespoint*/
#define WIFI_AP_SSID       "ESP32_AP"
#define WIFI_AP_PASSWORD   "12345678"

/*Evento Para esperar las credenciales de wifi*/
#define WIFI_CREDENTIALS_READY BIT0

static EventGroupHandle_t wifi_event_group = NULL;

/* Credenciales recibidas desde el formulario */
char wifi_ssid[32];
char wifi_password[64];

/* Notificar que llegaron las credenciales */
void notify_wifi_credentials(const char *ssid, const char *password)
{
    strncpy(wifi_ssid, ssid, sizeof(wifi_ssid) - 1);
    wifi_ssid[sizeof(wifi_ssid) - 1] = '\0';

    strncpy(wifi_password, password, sizeof(wifi_password) - 1);
    wifi_password[sizeof(wifi_password) - 1] = '\0';

    xEventGroupSetBits(wifi_event_group, WIFI_CREDENTIALS_READY);
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

void app_main(void)
{

    esp_err_t err = wifi_storage_init();

     if (err != ESP_OK) {
        ESP_LOGE("MAIN", "No se pudo inicializar el almacenamiento");
        return;
}
/* Intentar recuperar las credenciales guardadas */
    err = wifi_credentials_load(
    wifi_ssid,
    sizeof(wifi_ssid),
    wifi_password,
    sizeof(wifi_password)
);

    if (err == ESP_OK) {
    ESP_LOGI("MAIN", "Credenciales WiFi encontradas");
    } else {
    ESP_LOGI("MAIN", "No hay credenciales WiFi guardadas");
}

    wifi_event_group = xEventGroupCreate();

    if (wifi_event_group == NULL) {
        ESP_LOGE("WIFI", "Error al crear grupo de eventos");
        return;
    }

    if (err == ESP_OK) {

    ESP_LOGI("MAIN", "Credenciales WiFi encontradas");

    /* Inicializar la infraestructura WiFi */
    wifi_init_ap(WIFI_AP_SSID, WIFI_AP_PASSWORD);

    /* Intentar conectar usando las credenciales guardadas */
    err = wifi_init_sta(wifi_password, wifi_ssid);

    if (err != ESP_OK) {

        ESP_LOGW("MAIN", "No se pudo conectar. Iniciando modo configuracion");

        /* Volver al modo Access Point */
        wifi_start_ap(WIFI_AP_SSID, WIFI_AP_PASSWORD);

        /* Levantar servidor web */
        start_web_server();

        /* Esperar nuevas credenciales */
        wait_for_wifi_credentials();

        /* Intentar conectar con las nuevas credenciales */
        wifi_init_sta(wifi_password, wifi_ssid);
    }

} else {

        ESP_LOGI("MAIN", "No hay credenciales WiFi guardadas");

     wifi_init_ap(WIFI_AP_SSID, WIFI_AP_PASSWORD);


     start_web_server();

     /*Espero las credenciales*/
    wait_for_wifi_credentials();


    wifi_init_sta(wifi_password, wifi_ssid);

   //ds18b20_example_run(DS18B20_GPIO,DS18B20_READ_INTERVAL_MS);

    
    }

}