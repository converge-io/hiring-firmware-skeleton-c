cmake_minimum_required(VERSION 3.20)
project(hiring-firmware-skeleton-c VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler-specific options
if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vendor)

# Source files
set(SOURCES
    src/main.c
    vendor/ds18b20_driver.c
    vendor/radio_driver.c
    vendor/microcontroller.c
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link with math library (required for math.h functions used in DS18B20 driver)
target_link_libraries(${PROJECT_NAME} m)

# Install target
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
