#include <stdio.h>

/*Driver sensor de temperatura*/
#include "driver/ds18b20_example.c"


/*Driver de wifi*/
#include "driver/station_example.c"

/*Driver del accespoint*/
#include "driver/accesPoint.c"

/*Sensor de temperatura*/
#define DS18B20_GPIO 15

/*Tiempo que tarda el sensor en leer*/
#define DS18B20_READ_INTERVAL_MS 2000

/*Credenciales de wifi */
#define EXAMPLE_ESP_WIFI_SSID      "Pixel_1498"
#define EXAMPLE_ESP_WIFI_PASS      "clave345"


/*Credenciales del accespoint*/
#define WIFI_AP_SSID       "ESP32_AP"
#define WIFI_AP_PASSWORD   "12345678"


void app_main(void)
{
    //ds18b20_example_run(DS18B20_GPIO,DS18B20_READ_INTERVAL_MS);

   // wifi_init_sta(EXAMPLE_ESP_WIFI_PASS,EXAMPLE_ESP_WIFI_SSID);

    wifi_init_ap(WIFI_AP_SSID, WIFI_AP_PASSWORD);
}