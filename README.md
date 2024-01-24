# ProyectoFinal-ANIOT-RPI1-RPI2
El objetivo global del proyecto es definir e implementa un sistema de monitorización de la calidad del aire (centrado en CO2) en las aulas de la Facultad de Informática, junto a un sistema de estimación de aforo en espacios cerrados, utilizando para ello el hardware (y software) utilizados en el laboratorio de las asignaturas ANIOT, RPI1 y RPI2.
# Sprint 1
## ANIOT
1. 
## RPI-I
1. Uso de menuconfig para configurar un SSID wpa2 al que conectar el equipo
2. Obtener una conexión a internet a través de un punto de acceso wifi wpa2, tomando la SSID configurada con menuconfig
3. Contemplar en el diseño basado en eventos incluyendo los eventos de la wifi, como por ejemplo la desconexión del punto de acceso
- Manejo de Eventos WiFi
se implemento un manejador de eventos WiFi para diferentes situaciones, como el inicio de la conexión (WIFI_EVENT_STA_START), la desconexión (WIFI_EVENT_STA_DISCONNECTED), y la conexión exitosa (WIFI_EVENT_STA_CONNECTED). Estos eventos proporcionan información clave sobre el estado de la conexión.

- Reconexión Automática
En caso de que la conexión WiFi se pierda, el código intenta automáticamente reconectarse al punto de acceso (AP). Se implementa un mecanismo de reintento controlado por el contador s_retry_num, y después de un número máximo de intentos, el sistema se reinicia. Además, la variable s_retry_num lleva la cuenta de los intentos de reconexión y un EventGroupHandle_t para manejar los eventos de conexión y fallo de WiFi.

- Limpieza de Recursos
Se implemento la función cleanup() que se llama cuando se pierde la conexioón WiFi, esta función se encarga de desconectar la WiFi y desregistrar los manejadores de eventos, Esto asegura que los recursos se liberen adecuadamente.

## RPI-II
1. Establecida la jerarquía MQTT.
La jerarquía establecida tendrá la siguiente forma: facultad/piso/aula/numero/tipo_sensor/. A continuación dos ejemplos de como se representaria un topic segun la jeraquía:

- facultad_informatica/piso_1/aula_7/1/sgp30
- facultad_informatica/piso_1/aula_7/15/si7021

Se establece lo siguiente:
- En el topic facultad solo habra un único valor: facultad_informatica.
- En el topic piso habra 5 pisos, numerandose de piso_1  a piso_5.
- En el topic aula habra 8 aulas, numerandose de aula_1 hasta aula_8
- En el topic numero, hemos considerado que se referia a los puestos de laboratorio por ejemplo, asi que habra 15 puestos (numeros), numerandose de 1 a 15
- En el topic tipo_sensor solo hay dos posibles valores, que se corresponden con los sensores que se van a utilizar en el proyecto, que son: SGP30 y Si7021.

Se consideran tambien como topic para la configuracion de la frecuencia de muestreo: v1/gateway/configure/frequency
Y como topic para provisionar al nodo con los topics en los que se publicaran: v1/gateway/control/node_provisioning

2. Esquema publicación/suscripción a partir de datos sensorizados.
El nodo al conectarse por primera se suscribe al topic v1/gateway/control/node_provisioning del dashboard de Thingsboard para obtener los topics en los que debe publicar. Tambien se suscribe al topic v1/gateway/configure/frequency para configurar la frecuencia de muestreo.
3. Uso de usuario/contraseña en MQTT (configurable vía menuconfig).
4. Notificación de activación/caídas de nodos (LWT, timeout configurable) y otros eventos de interés.
Se plantea notificar cuando un nodo recibe los siguientes eventos:
    MQTT_EVENT_CONNECTED
    MQTT_EVENT_DISCONNECTED
Todos los parametros relativos a LWT y el timeout (keepalive) se configuran mediante menuconfig.
5. Comunicación bidireccional para control remoto de frecuencia de muestreo de cada sensor.
6. Parámetros de QoS (MQTT) configurables vía menuconfig y aplicados donde corresponda.

# Sprint 2
## ANIOT
1. Actualizacion del reloj de la placa por medio de SNTP
2. La estrategia de provisioning escogida es la proporcionada por Thingsboard. A traves de MQTT se envia un fichero JSON con un token unico para todos los nodos para que Thingsboard reconozca al nodo dentro de un grupo de nodos. A esto Thingsboard contestara con un JSON donde se incluye un token exlusivo para el nodo que se utilizara para autentificarse en distintas conexiones a Thingsboard como puede ser en el caso de OTA
## RPI-I
1. Provisionamiento del nodo por WiFi utilizando la aplicación móvil de Espressif. Al nodo se le propocionan el SSID y el password de la red a la que ha de conectarse.
2. Utilización de los modos de bajo consumo de la WiFi para ahorrar energia cuando no se envie nada desde el nodo.
## RPI-II
1. 

# Sprint 3
## ANIOT
1. 
## RPI-I
1. Estimacion del aforo mediante la utilizacion de la sala contando los dispositivos BLE usando GAP para escanear dichos dispositivos. A partir del RSSI de cada dispositivo se estima la distancia a la que está este. Se ha considerado utilizar una trilateracion 2D ( técnica geométrica que determina la posición de un objeto conociendo su distancia a tres puntos de referencia.) pero se ha decartado debido a que solo se conocen la situacion concreta de un ESP. Es por ello que se plantea utilizar una distancia arbitraria que sirva como umbral de referencia para determinar que dispositivo esta dentro y cual no. Se plantea utilizar un filtro de kalman para reducir el ruido ambiental y las interferencias que podrían afectar las mediciones de RSSI. La funcion de calculo de la distancia se ha tomado del siguiente documento: https://repositorio.unican.es/xmlui/bitstream/handle/10902/19626/428600.pdf?sequence=1 pagina 11
## RPI-II
1. 
## NOTAS
1. En el componente MQTT se decide que los datos que recoge el sensor SGP30 se representen como CJSON y los datos que recoge el sensor Si7021 se representen como CBOR antes de publicarlos en el broker. La razon de la eleccion de CBOR en los datos del sensor Si7021 es que se considera CBOR es un formato de representación más compacto y que al recoger el sensor datos de temperatura y humedad tiene más sentido enviarlos más compactos de cara a ahorrar ancho de banda.