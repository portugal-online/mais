cmake_minimum_required(VERSION 3.16.1)

#-------------------
# LoRaWAN library
#-------------------
file(GLOB LORAWAN_SRC
    "${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/LoRaWAN/Crypto/*.c"
    "${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/LoRaWAN/LmHandler/*.c"
    "${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/LoRaWAN/LmHandler/packages/*.c"
    "${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/LoRaWAN/Mac/*.c"
    "${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/LoRaWAN/Mac/region/*.c"
    "${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/LoRaWAN/Patterns/Basic/*.c"
    "${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/LoRaWAN/Utilities/*.c"
    "${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/SubGHz_Phy/stm32_radio_driver/*.c"
    )
add_library(lorawan STATIC
    ${LORAWAN_SRC}
    )

target_include_directories(lorawan
    PUBLIC
    ${PROJECT_SOURCE_DIR}/${HAL_DIR}/Inc
    ${PROJECT_SOURCE_DIR}/${MCU_DIR}/Include
    ${PROJECT_SOURCE_DIR}/${CMSIS_DIR}/Core/Include
    ${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/LoRaWAN/Crypto
    ${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/LoRaWAN/LmHandler
    ${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/LoRaWAN/LmHandler/packages
    ${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/LoRaWAN/Mac
    ${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/LoRaWAN/Mac/region
    ${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/LoRaWAN/Patterns/Basic
    ${PROJECT_SOURCE_DIR}/lib/Utilities
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/UAIR_BSP
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/UAIR_TRACER
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/UAIR_TRACER/adv_tracer
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/UAIR_TRACER/tiny_printf
    ${PROJECT_SOURCE_DIR}/lib/Utilities/baremetal
    ${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/LoRaWAN/Utilities
    ${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/SubGHz_Phy
    ${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/SubGHz_Phy/stm32_radio_driver
    PRIVATE
    ${PROJECT_SOURCE_DIR}/target
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/app
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/app/uair_weather_data/conf
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/app/uair_weather_data/lib_dependency
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/UAIR_BSP
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/UAIR_HAL
    ${PROJECT_SOURCE_DIR}/lib/SHTC3
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/HS300X
    )
target_compile_definitions(lorawan
    PUBLIC
    ${MCU}
    BOARD=UAIR
    )
#-------------------
# Main elf
#-------------------

file(GLOB UAIR_LIB_SRC
        "${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/UAIR_BSP/*.c"
        "${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/UAIR_HAL/*.c"
    "${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/UAIR_TRACER/adv_tracer/*.c"
        "${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/UAIR_TRACER/tiny_printf/*.c"
	"${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/UAIR_TRACER/*.c"
        "${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/HS300X/*.c"
        "${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/VM3011/*.c"
        "${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/ZMOD4510/*.c"
        )

file(GLOB UAIR_LIB_TESTS_SRC
	"${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/UAIR_BSP/*_t.cc"
    "${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/UAIR_HAL/*_t.cc"
    "${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/UAIR_TRACER/adv_tracer/*.c"
    "${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/UAIR_TRACER/tiny_printf/*.c"
    "${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/HS300X/*_t.cc"
    "${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/VM3011/*_t.cc"
    "${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/ZMOD4510/*_t.cc"
)

file(GLOB MAIN_SRC
    "${MCU_DIR}/*.c"
    "${PROJECT_SOURCE_DIR}/lib/Utilities/*.c"
    "${PROJECT_SOURCE_DIR}/lib/Utilities/baremetal/*.c"
    "${PROJECT_SOURCE_DIR}/lib/SHTC3/*.c"
)

add_executable(${PROJECT_NAME}.elf
    ${MAIN_SRC}
    ${UAIR_LIB_SRC}
    ${PROJECT_SOURCE_DIR}/${STARTUP_FILE}
    ${PROJECT_SOURCE_DIR}/${SYSTEM_FILE}
    )

set_target_properties(${PROJECT_NAME}.elf PROPERTIES CXX_STANDARD 11)
set_target_properties(${PROJECT_NAME}.elf PROPERTIES CXX_EXTENSIONS OFF)
target_compile_features(${PROJECT_NAME}.elf PRIVATE c_std_11 cxx_std_17)
target_compile_options(${PROJECT_NAME}.elf
    PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>" # disable RTTI
        "$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>" # disable exceptions
    )

target_sources(${PROJECT_NAME}.elf
    PRIVATE
        lora_app.c
        lora_info.c
        main.c
        weather.c
        uartrx.c
        $<$<BOOL:${UNITTESTS}>:${UAIR_LIB_TESTS_SRC}>
        sys_app.c
)

target_include_directories(${PROJECT_NAME}.elf
    PUBLIC
    ${PROJECT_SOURCE_DIR}/target
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/app
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/target
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/app/uair_weather_data
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/app/uair_weather_data/conf
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/app/uair_weather_data/lib_dependency
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/UAIR_BSP
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/UAIR_HAL
    ${PROJECT_SOURCE_DIR}/lib/SPIFFS
    ${PROJECT_SOURCE_DIR}/lib/Utilities
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/UAIR_TRACER
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/UAIR_TRACER/adv_tracer
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/UAIR_TRACER/tiny_printf
    ${PROJECT_SOURCE_DIR}/lib/Utilities/baremetal
    ${PROJECT_SOURCE_DIR}/lib/SHTC3
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/VM3011
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/HS300X
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/lib/ZMOD4510
    ${PROJECT_SOURCE_DIR}/${MCU_DIR}
    )

    target_link_libraries(${PROJECT_NAME}.elf
        PUBLIC
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/gas-algorithm-libraries/oaq_2nd_gen/${HOST_OS}/${MCU}/lib_oaq_2nd_gen.a
        hal
        lorawan
    m
    ${EXTRA_LIBRARIES}
        )

target_compile_definitions(${PROJECT_NAME}.elf
    PUBLIC
    ${MCU}
    SEMIHOSTING=${SEMIHOSTING}
    BOARD=UAIR
    UAIR_UART_RX_DMA=1
    )

