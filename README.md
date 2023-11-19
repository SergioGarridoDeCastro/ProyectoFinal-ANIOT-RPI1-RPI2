# ProyectoFinal-ANIOT-RPI1-RPI2
El objetivo global del proyecto es definir e implementa un sistema de monitorización de la calidad del aire (centrado en CO2) en las aulas de la Facultad de Informática, junto a un sistema de estimación de aforo en espacios cerrados, utilizando para ello el hardware (y software) utilizados en el laboratorio de las asignaturas ANIOT, RPI1 y RPI2.
# Sprint 1
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

2. Uso de menuconfig para configurar un SSID wpa2 al que conectar el equipo

3. Inclusión del componente Si7021
4. Cliente MQTT publicador con 


## NOTAS
