#include <stdio.h>

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

    wifi_event_group = xEventGroupCreate();

    if (wifi_event_group == NULL) {
        ESP_LOGE("WIFI", "Error al crear grupo de eventos");
        return;
    }

     wifi_init_ap(WIFI_AP_SSID, WIFI_AP_PASSWORD);


     start_web_server();

     /*Espero las credenciales*/
    wait_for_wifi_credentials();


    // wifi_init_sta( SSID ,  PASSWORD );

   //ds18b20_example_run(DS18B20_GPIO,DS18B20_READ_INTERVAL_MS);

    


}