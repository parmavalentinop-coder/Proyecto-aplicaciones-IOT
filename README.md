# Sistema de monitoreo de temperatura con ESP32

Sistema IoT desarrollado con **ESP32-WROOM-32** y **ESP-IDF 5.5.4** para
medir la temperatura mediante un sensor **DS18B20**, configurar la
conexión Wi-Fi desde una página web local y publicar las mediciones
mediante **MQTT sobre TLS**.

## Funcionalidades

-   Lectura de temperatura del sensor DS18B20 mediante el protocolo
    1-Wire.
-   Configuración inicial de la red Wi-Fi desde un punto de acceso
    creado por el ESP32.
-   Página web local para ingresar el SSID y la contraseña del router.
-   Almacenamiento de las credenciales Wi-Fi en NVS.
-   Conexión a la red configurada mediante el modo estación (STA).
-   Reintentos de conexión y retorno al punto de acceso de configuración
    cuando no se logra conectar.
-   Publicación de las mediciones en un broker MQTT mediante una
    conexión TLS.

## Hardware

-   ESP32-WROOM-32.
-   Sensor de temperatura DS18B20.
-   Resistencia pull-up para el bus 1-Wire.

### Conexión del DS18B20

  DS18B20   ESP32
  --------- ---------
  DATA      GPIO 15
  VDD       3.3 V
  GND       GND

El bus 1-Wire requiere una resistencia pull-up entre DATA y 3.3 V. Como
referencia, suele utilizarse una resistencia de 4,7 kΩ. Verificar el
montaje físico antes de poner el sistema en funcionamiento.

## Requisitos de software

-   ESP-IDF **v5.5.4**.
-   Herramientas de compilación y Python instaladas mediante el
    instalador de ESP-IDF.
-   Controlador USB correspondiente al conversor USB-UART de la placa.
-   Un cliente MQTT, por ejemplo MQTTX, para comprobar los mensajes
    publicados.
-   Acceso a una red Wi-Fi de 2,4 GHz.

## Estructura del proyecto

La estructura lógica utilizada es:

``` text
proyecto/
├── main/
│   ├── main.c
│   ├── driver/
│   │   ├── accesPoint.c
│   │   ├── station_example.c
│   │   ├── wifi_storage.c
│   │   ├── wifi_web_server.c
│   │   ├── ds18b20_example.c
│   │   └── mqtt_client.c
│   ├── web/
│   │   ├── index.html
│   │   └── index.css
│   ├── certs/
│   │   └── mosquitto.org.crt
│   └── CMakeLists.txt
├── CMakeLists.txt
└── README.md
```

Los nombres y la organización deben coincidir con los archivos presentes
en la versión del proyecto que se compile.

## Compilación y carga

Abrir una terminal de ESP-IDF configurada para la versión 5.5.4 y
ubicarse en la carpeta raíz del proyecto.

1.  Configurar el objetivo:

    ``` bash
    idf.py set-target esp32
    ```

2.  Compilar:

    ``` bash
    idf.py build
    ```

3.  Grabar el firmware (reemplazar `COMx` por el puerto
    correspondiente):

    ``` bash
    idf.py -p COMx flash
    ```

4.  Abrir el monitor serie:

    ``` bash
    idf.py -p COMx monitor
    ```

También se puede realizar la compilación, carga y apertura del monitor
desde la extensión de ESP-IDF de Visual Studio Code.

## Configuración de Wi-Fi

1.  Encender el ESP32.

2.  Si todavía no hay credenciales guardadas o el equipo vuelve al modo
    de configuración, buscar la red Wi-Fi **ESP32_AP**.

3.  Conectarse al punto de acceso. La contraseña configurada es
    `12345678`.

4.  Abrir un navegador e ingresar a:

    ``` text
    http://192.168.4.1
    ```

5.  Completar el SSID y la contraseña de la red Wi-Fi a la que se desea
    conectar.

6.  Enviar el formulario y esperar a que el ESP32 intente conectarse.

Las credenciales se guardan en la memoria NVS. Si la conexión con el
router falla y se agotan los reintentos implementados, el dispositivo
vuelve al punto de acceso para permitir una nueva configuración.

## Publicación MQTT

La configuración de prueba utiliza:

  Parámetro             Valor
  --------------------- ---------------------------
  Broker                `test.mosquitto.org`
  Puerto TLS            `8883`
  Tópico                `casa/living/temperatura`
  Formato del mensaje   JSON

Ejemplo de mensaje publicado:

``` json
{
  "temperatura": 29.69
}
```

El valor es ilustrativo: la temperatura real depende de la lectura del
sensor.

Para validar la publicación, conectar un cliente como MQTTX al broker de
prueba utilizando TLS y suscribirse al tópico `casa/living/temperatura`.

El certificado de la autoridad certificante utilizado por el cliente
MQTT se encuentra en `main/certs/mosquitto.org.crt` y se incorpora al
firmware durante la compilación.

> **Importante:** `test.mosquitto.org` es un broker público destinado a
> pruebas. Los tópicos pueden ser accesibles por otros usuarios y no
> debe utilizarse para transmitir información privada ni como
> infraestructura de producción.

## Solución de problemas

### El ESP32 no detecta el DS18B20

-   Comprobar que DATA esté conectado al GPIO 15.
-   Verificar alimentación, masa común y orientación de los terminales
    del sensor.
-   Comprobar que exista una resistencia pull-up entre DATA y 3.3 V.
-   Revisar el monitor serie para identificar errores del bus 1-Wire.

### No aparece la red `ESP32_AP`

-   Reiniciar la placa y observar el monitor serie.
-   Verificar que el firmware haya iniciado el modo de configuración
    Wi-Fi.
-   Comprobar que no haya quedado activa una conexión STA con
    credenciales guardadas.

### No se conecta al router

-   Verificar SSID y contraseña.
-   Comprobar que la red sea compatible con la conexión Wi-Fi del ESP32
    (2,4 GHz).
-   Esperar a que se completen los reintentos y comprobar si vuelve a
    aparecer `ESP32_AP`.

### No llegan mensajes MQTT

-   Comprobar que el ESP32 esté conectado al router.
-   Verificar la disponibilidad del broker y el puerto TLS `8883`.
-   Comprobar que el cliente MQTT esté suscripto al tópico exacto.
-   Revisar los mensajes del monitor serie y la configuración del
    certificado.

## Módulos principales

  -----------------------------------------------------------------------
  Archivo                             Responsabilidad
  ----------------------------------- -----------------------------------
  `main.c`                            Inicialización y coordinación
                                      general del sistema.

  `accesPoint.c`                      Creación y administración del punto
                                      de acceso de configuración.

  `station_example.c`                 Conexión Wi-Fi en modo estación y
                                      gestión de reintentos.

  `wifi_storage.c`                    Guardado y recuperación de
                                      credenciales mediante NVS.

  `wifi_web_server.c`                 Servidor HTTP y recepción de
                                      credenciales desde la página web.

  `ds18b20_example.c`                 Inicialización del bus 1-Wire y
                                      lectura del DS18B20.

  `mqtt_client.c`                     Conexión MQTT segura y publicación
                                      de las mediciones.
  -----------------------------------------------------------------------

## Estado y consideraciones

Este README describe la configuración de desarrollo y prueba del
proyecto. Antes de una instalación definitiva, verificar el
funcionamiento físico del sensor y su resistencia pull-up, revisar los
parámetros de conexión y reemplazar el broker público por una
infraestructura MQTT propia si se requiere un uso real.
