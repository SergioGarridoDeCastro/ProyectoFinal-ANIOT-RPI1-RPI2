# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/Lenovo/Espressif/frameworks/esp-idf-v5.0.2/components/bootloader/subproject"
  "C:/Users/Lenovo/OneDrive/MASTER_IOT/ProyectoAreaTecnologia/ProyectoFinal-ANIOT-RPI1-RPI2/build/bootloader"
  "C:/Users/Lenovo/OneDrive/MASTER_IOT/ProyectoAreaTecnologia/ProyectoFinal-ANIOT-RPI1-RPI2/build/bootloader-prefix"
  "C:/Users/Lenovo/OneDrive/MASTER_IOT/ProyectoAreaTecnologia/ProyectoFinal-ANIOT-RPI1-RPI2/build/bootloader-prefix/tmp"
  "C:/Users/Lenovo/OneDrive/MASTER_IOT/ProyectoAreaTecnologia/ProyectoFinal-ANIOT-RPI1-RPI2/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/Lenovo/OneDrive/MASTER_IOT/ProyectoAreaTecnologia/ProyectoFinal-ANIOT-RPI1-RPI2/build/bootloader-prefix/src"
  "C:/Users/Lenovo/OneDrive/MASTER_IOT/ProyectoAreaTecnologia/ProyectoFinal-ANIOT-RPI1-RPI2/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/Lenovo/OneDrive/MASTER_IOT/ProyectoAreaTecnologia/ProyectoFinal-ANIOT-RPI1-RPI2/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/Lenovo/OneDrive/MASTER_IOT/ProyectoAreaTecnologia/ProyectoFinal-ANIOT-RPI1-RPI2/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
