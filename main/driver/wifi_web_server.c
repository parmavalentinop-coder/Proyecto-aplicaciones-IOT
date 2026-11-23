#include <stdio.h>
#include <stdint.h>

#include "esp_http_server.h"
#include "esp_log.h"


void notify_wifi_credentials(const char *ssid, const char *password);

esp_err_t wifi_credentials_save(const char *ssid, const char *password);

// Archivo HTML embebido
extern const uint8_t index_html_start[]
    asm("_binary_index_html_start");

extern const uint8_t index_html_end[]
    asm("_binary_index_html_end");

static const char *TAG_WEB_SERVER = "WEB_SERVER";

static httpd_handle_t server = NULL;


// Manejador de la pagina principal
static esp_err_t root_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG_WEB_SERVER, "Solicitud GET recibida");

    httpd_resp_set_type(req, "text/html");

    return httpd_resp_send(
        req,
        (const char *)index_html_start,
        index_html_end - index_html_start
    );
}

// Manejador para recibir las credenciales WiFi
static esp_err_t wifi_post_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG_WEB_SERVER, "Solicitud POST recibida en /wifi");

    char content[128];

    // Recibir el contenido del formulario
    int ret = httpd_req_recv(req, content, sizeof(content) - 1);

    if (ret <= 0) {
        ESP_LOGE(TAG_WEB_SERVER, "Error al recibir los datos");
        return ESP_FAIL;
    }

    content[ret] = '\0';

    ESP_LOGI(TAG_WEB_SERVER, "Datos recibidos correctamente");

    // Variables para almacenar las credenciales
    char ssid[32] = {0};
    char password[64] = {0};

    // Extraer SSID y contraseña
    if (httpd_query_key_value(content, "ssid", ssid, sizeof(ssid)) != ESP_OK ||
        httpd_query_key_value(content, "password", password, sizeof(password)) != ESP_OK) {

        ESP_LOGE(TAG_WEB_SERVER, "No se pudieron obtener las credenciales");

        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                            "Credenciales invalidas");

        return ESP_FAIL;
    }

    // Notificar a main.c que llegaron las credenciales
    esp_err_t err = wifi_credentials_save(ssid, password);

   if (err != ESP_OK) {
    ESP_LOGE(TAG, "No se pudieron guardar las credenciales");

    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "Error al guardar las credenciales");
    return ESP_FAIL;
    }

    notify_wifi_credentials(ssid, password);

    ESP_LOGI(TAG_WEB_SERVER, "Credenciales recibidas y notificadas");

    httpd_resp_sendstr(req, "Credenciales recibidas. El ESP32 intentara conectarse.");

    return ESP_OK;
}

// Inicializacion del servidor HTTP
void start_web_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG_WEB_SERVER, "Iniciando servidor HTTP...");

    if (httpd_start(&server, &config) == ESP_OK) {

        httpd_uri_t root = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = root_get_handler,
            .user_ctx  = NULL
        };

        httpd_register_uri_handler(server, &root);

                httpd_uri_t wifi = {
            .uri       = "/wifi",
            .method    = HTTP_POST,
            .handler   = wifi_post_handler,
            .user_ctx  = NULL
        };

        httpd_register_uri_handler(server, &wifi);

        ESP_LOGI(TAG_WEB_SERVER, "Servidor HTTP iniciado");
        ESP_LOGI(TAG_WEB_SERVER, "Accede al formulario desde: http://192.168.4.1");
    }
    else {
        ESP_LOGE(TAG, "Error al iniciar servidor HTTP");
    }
    
}