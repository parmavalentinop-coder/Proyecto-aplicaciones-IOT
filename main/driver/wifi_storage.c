#include <string.h>
#include "nvs.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"


static const char *TAG_STORAGE = "WIFI_STORAGE";


esp_err_t wifi_storage_init(void)
{
    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {

        ESP_LOGW(TAG_STORAGE, "Reinicializando particion NVS");

        ESP_ERROR_CHECK(nvs_flash_erase());

        err = nvs_flash_init();
    }

    if (err != ESP_OK) {
        ESP_LOGE(TAG_STORAGE, "Error al inicializar NVS: %s",
                 esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG_STORAGE, "NVS inicializada correctamente");

    return ESP_OK;
}

/*
 * Guarda las credenciales WiFi en NVS.
 */
esp_err_t wifi_credentials_save(const char *ssid, const char *password)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Abrir espacio de almacenamiento
    err = nvs_open("wifi_cfg", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_STORAGE, "Error al abrir NVS: %s", esp_err_to_name(err));
        return err;
    }

    // Guardar SSID
    err = nvs_set_str(nvs_handle, "ssid", ssid);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_STORAGE, "Error al guardar SSID");
        nvs_close(nvs_handle);
        return err;
    }

    // Guardar contraseña
    err = nvs_set_str(nvs_handle, "password", password);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_STORAGE, "Error al guardar password");
        nvs_close(nvs_handle);
        return err;
    }

    // Confirmar los cambios
    err = nvs_commit(nvs_handle);

    nvs_close(nvs_handle);

    if (err == ESP_OK) {
        ESP_LOGI(TAG_STORAGE, "Credenciales WiFi guardadas correctamente");
    } else {
        ESP_LOGE(TAG_STORAGE, "Error al confirmar los cambios en NVS");
    }

    return err;
}

/*
 * Recupera las credenciales WiFi almacenadas en NVS.
 */
esp_err_t wifi_credentials_load(
    char *ssid,
    size_t ssid_size,
    char *password,
    size_t password_size)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Abrir espacio de almacenamiento
    err = nvs_open("wifi_cfg", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG_STORAGE, "No se pudieron abrir las credenciales: %s",
                 esp_err_to_name(err));
        return err;
    }

    // Recuperar SSID
    err = nvs_get_str(nvs_handle, "ssid", ssid, &ssid_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_STORAGE, "Error al recuperar SSID");
        nvs_close(nvs_handle);
        return err;
    }

    // Recuperar contraseña
    err = nvs_get_str(nvs_handle, "password", password, &password_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_STORAGE, "Error al recuperar password");
        nvs_close(nvs_handle);
        return err;
    }

    nvs_close(nvs_handle);

    ESP_LOGI(TAG_STORAGE, "Credenciales WiFi recuperadas correctamente");

    return ESP_OK;
}