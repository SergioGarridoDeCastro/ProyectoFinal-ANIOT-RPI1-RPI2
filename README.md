# ProyectoFinal-ANIOT-RPI1-RPI2

El objetivo global del proyecto es definir e implementa un sistema de monitorización de la calidad del aire (centrado en CO2) en las aulas de la Facultad de Informática, junto a un sistema de estimación de aforo en espacios cerrados, utilizando para ello el hardware (y software) utilizados en el laboratorio de las asignaturas ANIOT, RPI1 y RPI2.

# Sprint 1

## ANIOT

    1.Organización de aplicación basada en eventos
    2. FSM
    3. Preveer futuras ampliaciones y modos de error
    4. Monitorización de sensores Si7021, SGP30
    5. Se han incluido como componentes Si7021, SGP30

## RPI-I

    1. Uso de menuconfig para configurar un SSID wpa2 al que conectar el equipo
    2. Obtener una conexión a internet a través de un punto de acceso wifi wpa2, tomando la SSID configurada con menuconfig
    3. Contemplar en el diseño basado en eventos incluyendo los eventos de la wifi, como por ejemplo la desconexión del punto de acceso

    - Reconexión Automática
    En caso de que la conexión WiFi se pierda, el código intenta automáticamente reconectarse al punto de acceso. con la variable de configuración Maximum retry, y después de un número máximo de intentos, el sistema se reinicia. Además, la variable s_retry_num lleva la cuenta de los intentos de reconexión y un EventGroupHandle_t para manejar los eventos de conexión y fallo de WiFi.

# Sprint 2

## ANIOT

    1. Actualizacion del reloj de la placa por medio de SNTP
    2. Se ha creado un componente aparte para la sincronización horario
        2.1 se ha seguido el ejemplo de sntp
    3. Se ha incluido el modo de bajo consumo al inicio del main

## RPI-II

    1. Se ha incluido el componente de coap
        1.1 la conexión
        1.2 el envio de datos
        1.3 la desconexion
    2. Diseño e implementación de objetos CoAP
    3. Representación de la información transmitida (JSON/CBOR).

# Sprint 3

    1. OTA
      1.1 Creación de cuenta en thingsboard
      1.2 Creación de dispositivo
      1.3 creación de reglas para la actualización de la ota
      1.4 creación de partitions.csv para las nuevas imágenes
    2. NVS
      2.1 creación de componente para escribir y leer en memori
      2.2 uso de nvs para el control de versionado

## NOTAS

    1. Se deja en el repositorio todo lo que se ha implementado de thingsboard (reglas, dispositivos)
       así como un primera versión del que fuera la máquina de estados
