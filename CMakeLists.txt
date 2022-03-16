cmake_minimum_required(VERSION 3.12)

# Change your executable name to something creative!
set(NAME RP2040MQTT) # <-- Name your project/executable here!

include(rp2040_hat_freertos_c-patch.cmake)
include(pico_sdk_import.cmake)
include(rp2040_hat_freertos_c_sdk_version.cmake)
#include(rp2040_hat_c_sdk_version.cmake)

# Gooey boilerplate
project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)
set(PICO_CXX_ENABLE_RTTI 1)



# Initialize the SDK
pico_sdk_init()

include("lib/json-maker.cmake")
include("lib/tiny-json.cmake")

# Set the project root directory if it's not already defined, as may happen if
# the tests folder is included directly by a parent project, without including
# the top level CMakeLists.txt.
if(NOT DEFINED WIZNET_DIR)
    set(WIZNET_DIR ${CMAKE_SOURCE_DIR}/lib/ioLibrary_Driver)
    message(STATUS "WIZNET_DIR = ${WIZNET_DIR}")
endif()

if(NOT DEFINED MBEDTLS_LIB_DIR)
    set(MBEDTLS_LIB_DIR ${CMAKE_SOURCE_DIR}/lib/mbedtls)
    message(STATUS "MBEDTLS_LIB_DIR = ${MBEDTLS_LIB_DIR}")
endif()

if(NOT DEFINED PORT_DIR)
    set(PORT_DIR ${CMAKE_SOURCE_DIR}/port)
    message(STATUS "PORT_DIR = ${PORT_DIR}")
endif()

# Turn off mbedtls test mode 
set(ENABLE_PROGRAMS OFF CACHE BOOL "Build mbedtls programs")
set(ENABLE_TESTING OFF CACHE BOOL "Build mbedtls testing")
add_definitions(-DMBEDTLS_CONFIG_FILE="${PORT_DIR}/mbedtls/inc/ssl_config.h")
add_definitions(-DSET_TRUSTED_CERT_IN_SAMPLES)


add_subdirectory(src)
#add_subdirectory(${MBEDTLS_LIB_DIR})
add_subdirectory(${PORT_DIR})



set(TWIN_THING_PICO_CONFIG_PATH ${CMAKE_CURRENT_LIST_DIR}/src/config)
set(COREMQTT_PORT_PATH      ${CMAKE_CURRENT_LIST_DIR}/port/CoreMQTT)
set(COREMQTT_AGENT_PORT_PATH      ${CMAKE_CURRENT_LIST_DIR}/port/CoreMQTT-Agent)


# Include required libraries
include("lib/twinThingRP2040W5x00/twinThingRP2040W5x00.cmake")
include("lib/FreeRTOS-Kernel.cmake")
include("lib/FreeRTOS-coreMQTT.cmake")
include("lib/FreeRTOS-coreMQTT-Agent.cmake")

include("lib/RtosComponents/RTOSComponents.cmake")

include("lib/ioLibrary.cmake")

include("lib/pico-ssd1306.cmake")




#Set up files for the release packages
install(CODE "execute_process(COMMAND $ENV{HOME}/bin/picoDeploy.sh ${CMAKE_CURRENT_BINARY_DIR}/src/${NAME}.elf)")

# Set up files for the release packages
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/src/${NAME}.uf2
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)
