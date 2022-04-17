file(GLOB HAL_TESTS_SRC
    "tests/*.cpp"
    )

#-------------------
# HAL library
#-------------------
file(GLOB HAL_SRC
    "${PROJECT_SOURCE_DIR}/${HAL_DIR}/Src/*.c"
    "${PROJECT_SOURCE_DIR}/${HAL_DIR}/Src/*.s"
    "uair_board.c"
    "*.cpp"
    "models/*.c"
    "models/*.cpp"
    "models/network/*.cpp"
    ${HAL_TESTS_SRC}
    )

add_library(hal STATIC
    ${HAL_SRC}
    )

target_include_directories(hal
    PUBLIC
    ${PROJECT_SOURCE_DIR}/${HAL_DIR}/Inc
    ${PROJECT_SOURCE_DIR}/${MCU_DIR}/Include
    ${PROJECT_SOURCE_DIR}/${CMSIS_DIR}/Core/Include
    ${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/SubGHz_Phy
    ${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/LoRaWAN/Crypto
    ${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/LoRaWAN/Utilities
    ${PROJECT_SOURCE_DIR}/lib/Utilities
    ${PROJECT_SOURCE_DIR}/mais/uair/Software/lib
    ${PROJECT_SOURCE_DIR}/${LORAWAN_DIR}/SubGHz_Phy/stm32_radio_driver
    PRIVATE
    ${PROJECT_SOURCE_DIR}/${MCU_DIR}
    )

target_compile_definitions(hal
    PUBLIC
    ${MCU}
    -DUNITTESTS=${UNITTESTS}
    )

#-------------------
# int library
#-------------------
file(GLOB INTR_SRC
    "interrupt_handlers.c"
    )
add_library(intrhandler STATIC
    ${INTR_SRC}
    )
target_include_directories(intrhandler
    PUBLIC
    ${PROJECT_SOURCE_DIR}/${HAL_DIR}/Inc
    ${PROJECT_SOURCE_DIR}/${MCU_DIR}/Include
    ${PROJECT_SOURCE_DIR}/${CMSIS_DIR}/Core/Include
    PRIVATE
    ${PROJECT_SOURCE_DIR}/target
    )
target_compile_definitions(intrhandler
    PUBLIC
    ${MCU}
    )
