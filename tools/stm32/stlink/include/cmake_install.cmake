# Install script for directory: /home/mint/Contiki-LoRa/tools/stm32/stlink-src/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES "/home/mint/Contiki-LoRa/tools/stm32/stlink-src/include/stlink.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/stlink" TYPE FILE FILES
    "/home/mint/Contiki-LoRa/tools/stm32/stlink-src/include/stlink/logging.h"
    "/home/mint/Contiki-LoRa/tools/stm32/stlink-src/include/stlink/usb.h"
    "/home/mint/Contiki-LoRa/tools/stm32/stlink-src/include/stlink/chipid.h"
    "/home/mint/Contiki-LoRa/tools/stm32/stlink-src/include/stlink/mmap.h"
    "/home/mint/Contiki-LoRa/tools/stm32/stlink-src/include/stlink/flash_loader.h"
    "/home/mint/Contiki-LoRa/tools/stm32/stlink-src/include/stlink/reg.h"
    "/home/mint/Contiki-LoRa/tools/stm32/stlink-src/include/stlink/sg.h"
    "/home/mint/Contiki-LoRa/tools/stm32/stlink-src/include/stlink/commands.h"
    "/home/mint/Contiki-LoRa/tools/stm32/stlink-src/include/stlink/backend.h"
    "/home/mint/Contiki-LoRa/tools/stm32/stlink-src/build/Release/include/stlink/version.h"
    )
endif()

