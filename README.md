# ProyectoFinal-ANIOT-RPI1-RPI2
El objetivo global del proyecto es definir e implementa un sistema de monitorización de la calidad del aire (centrado en CO2) en las aulas de la Facultad de Informática, junto a un sistema de estimación de aforo en espacios cerrados, utilizando para ello el hardware (y software) utilizados en el laboratorio de las asignaturas ANIOT, RPI1 y RPI2.
# Sprint 1
## ANIOT
1. 
## RPI-I
1. 
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


## NOTAS
