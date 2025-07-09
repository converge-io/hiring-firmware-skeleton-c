/**
 * @file ds18b20_driver.c
 * @brief DS18B20 Digital Temperature Sensor Driver Implementation (Simulated)
 * 
 * Simulated implementation of DS18B20 1-Wire digital temperature sensor driver.
 * Provides realistic sensor behavior without requiring actual hardware.
 * 
 * @author Implementation Team
 * @version 1.2.0
 * @date 2024-03-15
 */

#include "ds18b20_driver.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/** @defgroup DS18B20_Private Private Variables and Functions
 * @{
 */

/** Maximum number of simulated devices */
#define MAX_SIMULATED_DEVICES 8

/** Simulated device state */
typedef struct {
    ds18b20_handle_t handle;
    uint32_t conversion_start_time;
    bool conversion_active;
    float base_temperature;
    float temperature_drift;
} simulated_device_t;

/** Driver state */
static struct {
    bool initialized;
    uint8_t onewire_pin;
    simulated_device_t devices[MAX_SIMULATED_DEVICES];
    uint8_t device_count;
} driver_state = {0};

/** CRC-8 lookup table for Dallas 1-Wire */
static const uint8_t crc8_table[256] = {
    0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83,
    0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41,
    0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e,
    0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc,
    0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0,
    0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62,
    0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d,
    0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff,
    0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5,
    0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07,
    0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58,
    0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a,
    0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6,
    0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24,
    0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b,
    0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9,
    0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f,
    0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd,
    0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92,
    0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50,
    0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c,
    0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee,
    0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1,
    0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73,
    0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49,
    0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b,
    0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4,
    0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16,
    0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a,
    0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8,
    0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7,
    0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35
};

/**
 * @brief Calculate CRC-8 checksum
 * @param data Data buffer
 * @param len Data length
 * @return uint8_t CRC-8 checksum
 */
static uint8_t calculate_crc8(const uint8_t *data, uint8_t len) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        crc = crc8_table[crc ^ data[i]];
    }
    return crc;
}

/**
 * @brief Generate random ROM code
 * @param rom_code Buffer for ROM code
 */
static void generate_rom_code(uint8_t *rom_code) {
    rom_code[0] = DS18B20_FAMILY_CODE;
    
    // Generate random serial number (6 bytes)
    for (int i = 1; i < 7; i++) {
        rom_code[i] = rand() & 0xFF;
    }
    
    // Calculate CRC for ROM code
    rom_code[7] = calculate_crc8(rom_code, 7);
}

/**
 * @brief Get current time in milliseconds
 * @return uint32_t Current time in milliseconds
 */
static uint32_t get_time_ms(void) {
    return (uint32_t)(clock() * 1000 / CLOCKS_PER_SEC);
}

/**
 * @brief Simulate temperature with realistic variation
 * @param device Pointer to simulated device
 * @return float Simulated temperature in Celsius
 */
static float simulate_temperature(simulated_device_t *device) {
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }
    
    // Add small random variation and drift
    float noise = ((float)rand() / RAND_MAX - 0.5f) * 0.1f; // ±0.05°C noise
    device->temperature_drift += ((float)rand() / RAND_MAX - 0.5f) * 0.01f; // Slow drift
    
    // Clamp drift to reasonable range
    if (device->temperature_drift > 2.0f) device->temperature_drift = 2.0f;
    if (device->temperature_drift < -2.0f) device->temperature_drift = -2.0f;
    
    return device->base_temperature + device->temperature_drift + noise;
}

/**
 * @brief Convert temperature to raw value based on resolution
 * @param temp_c Temperature in Celsius
 * @param resolution Temperature resolution
 * @return uint16_t Raw temperature value
 */
static uint16_t temperature_to_raw(float temp_c, ds18b20_resolution_t resolution) {
    int16_t temp_raw = (int16_t)(temp_c * 16.0f); // Base 12-bit resolution
    
    // Apply resolution masking
    switch (resolution) {
        case DS18B20_RESOLUTION_9BIT:
            temp_raw &= 0xFFF8; // Clear bits 2:0
            break;
        case DS18B20_RESOLUTION_10BIT:
            temp_raw &= 0xFFFC; // Clear bits 1:0
            break;
        case DS18B20_RESOLUTION_11BIT:
            temp_raw &= 0xFFFE; // Clear bit 0
            break;
        case DS18B20_RESOLUTION_12BIT:
        default:
            // No masking needed
            break;
    }
    
    return (uint16_t)temp_raw;
}

/** @} */

/** @defgroup DS18B20_Public Public Function Implementations
 * @{
 */

ds18b20_error_t ds18b20_init(uint8_t onewire_pin) {
    if (driver_state.initialized) {
        return DS18B20_OK;
    }
    
    // Initialize driver state
    memset(&driver_state, 0, sizeof(driver_state));
    driver_state.onewire_pin = onewire_pin;
    driver_state.initialized = true;
    
    // Initialize random seed
    srand((unsigned int)time(NULL));
    
    return DS18B20_OK;
}

ds18b20_error_t ds18b20_scan_devices(ds18b20_handle_t *devices, 
                                     uint8_t max_devices, 
                                     uint8_t *found_count) {
    if (!driver_state.initialized) {
        return DS18B20_ERROR_INIT;
    }
    
    if (devices == NULL || found_count == NULL) {
        return DS18B20_ERROR_INVALID_PARAM;
    }
    
    // Simulate finding 1-3 devices
    uint8_t num_devices = 1 + (rand() % 3);
    if (num_devices > max_devices) {
        num_devices = max_devices;
    }
    if (num_devices > MAX_SIMULATED_DEVICES) {
        num_devices = MAX_SIMULATED_DEVICES;
    }
    
    // Generate simulated devices
    for (uint8_t i = 0; i < num_devices; i++) {
        // Generate ROM code
        generate_rom_code(devices[i].rom_code);
        
        // Set default configuration
        devices[i].resolution = DS18B20_RESOLUTION_12BIT;
        devices[i].power_mode = DS18B20_POWER_EXTERNAL;
        devices[i].th_register = 125; // Default high alarm
        devices[i].tl_register = -55; // Default low alarm
        devices[i].initialized = true;
        
        // Initialize simulated device state
        driver_state.devices[i].handle = devices[i];
        driver_state.devices[i].conversion_active = false;
        driver_state.devices[i].base_temperature = 20.0f + (float)(rand() % 20); // 20-40°C
        driver_state.devices[i].temperature_drift = 0.0f;
    }
    
    driver_state.device_count = num_devices;
    *found_count = num_devices;
    
    return DS18B20_OK;
}

ds18b20_error_t ds18b20_configure(ds18b20_handle_t *device,
                                  ds18b20_resolution_t resolution,
                                  int8_t th_alarm,
                                  int8_t tl_alarm) {
    if (!driver_state.initialized) {
        return DS18B20_ERROR_INIT;
    }
    
    if (device == NULL || !device->initialized) {
        return DS18B20_ERROR_INVALID_PARAM;
    }
    
    // Validate temperature range
    if (th_alarm < -55 || th_alarm > 125 || tl_alarm < -55 || tl_alarm > 125) {
        return DS18B20_ERROR_INVALID_PARAM;
    }
    
    if (tl_alarm >= th_alarm) {
        return DS18B20_ERROR_INVALID_PARAM;
    }
    
    // Update device configuration
    device->resolution = resolution;
    device->th_register = (uint8_t)th_alarm;
    device->tl_register = (uint8_t)tl_alarm;
    
    // Find and update corresponding simulated device
    for (uint8_t i = 0; i < driver_state.device_count; i++) {
        if (memcmp(driver_state.devices[i].handle.rom_code, device->rom_code, 8) == 0) {
            driver_state.devices[i].handle = *device;
            break;
        }
    }
    
    return DS18B20_OK;
}

ds18b20_error_t ds18b20_start_conversion(const ds18b20_handle_t *device) {
    if (!driver_state.initialized) {
        return DS18B20_ERROR_INIT;
    }
    
    if (device == NULL || !device->initialized) {
        return DS18B20_ERROR_INVALID_PARAM;
    }
    
    // Find corresponding simulated device
    for (uint8_t i = 0; i < driver_state.device_count; i++) {
        if (memcmp(driver_state.devices[i].handle.rom_code, device->rom_code, 8) == 0) {
            driver_state.devices[i].conversion_active = true;
            driver_state.devices[i].conversion_start_time = get_time_ms();
            return DS18B20_OK;
        }
    }
    
    return DS18B20_ERROR_NOT_FOUND;
}

ds18b20_error_t ds18b20_is_conversion_complete(const ds18b20_handle_t *device, 
                                               bool *is_complete) {
    if (!driver_state.initialized) {
        return DS18B20_ERROR_INIT;
    }
    
    if (device == NULL || !device->initialized || is_complete == NULL) {
        return DS18B20_ERROR_INVALID_PARAM;
    }
    
    // Find corresponding simulated device
    for (uint8_t i = 0; i < driver_state.device_count; i++) {
        if (memcmp(driver_state.devices[i].handle.rom_code, device->rom_code, 8) == 0) {
            if (!driver_state.devices[i].conversion_active) {
                *is_complete = true;
                return DS18B20_OK;
            }
            
            // Check if conversion time has elapsed
            uint32_t elapsed = get_time_ms() - driver_state.devices[i].conversion_start_time;
            
            // Conversion time depends on resolution
            uint32_t conversion_time;
            switch (device->resolution) {
                case DS18B20_RESOLUTION_9BIT:  conversion_time = 94; break;
                case DS18B20_RESOLUTION_10BIT: conversion_time = 188; break;
                case DS18B20_RESOLUTION_11BIT: conversion_time = 375; break;
                case DS18B20_RESOLUTION_12BIT: conversion_time = 750; break;
                default: conversion_time = 750; break;
            }
            
            *is_complete = (elapsed >= conversion_time);
            if (*is_complete) {
                driver_state.devices[i].conversion_active = false;
            }
            
            return DS18B20_OK;
        }
    }
    
    return DS18B20_ERROR_NOT_FOUND;
}

ds18b20_error_t ds18b20_read_temperature(const ds18b20_handle_t *device,
                                         ds18b20_temperature_t *temperature) {
    if (!driver_state.initialized) {
        return DS18B20_ERROR_INIT;
    }
    
    if (device == NULL || !device->initialized || temperature == NULL) {
        return DS18B20_ERROR_INVALID_PARAM;
    }
    
    // Find corresponding simulated device
    simulated_device_t *sim_device = NULL;
    for (uint8_t i = 0; i < driver_state.device_count; i++) {
        if (memcmp(driver_state.devices[i].handle.rom_code, device->rom_code, 8) == 0) {
            sim_device = &driver_state.devices[i];
            break;
        }
    }
    
    if (sim_device == NULL) {
        return DS18B20_ERROR_NOT_FOUND;
    }
    
    // Simulate temperature reading
    float temp_c = simulate_temperature(sim_device);
    
    // Convert to raw value
    uint16_t raw_value = temperature_to_raw(temp_c, device->resolution);
    
    // Fill temperature structure
    temperature->temperature_c = ds18b20_raw_to_celsius(raw_value, device->resolution);
    temperature->temperature_f = ds18b20_celsius_to_fahrenheit(temperature->temperature_c);
    temperature->raw_value = raw_value;
    temperature->valid = true;
    
    return DS18B20_OK;
}

ds18b20_error_t ds18b20_read_temperature_blocking(const ds18b20_handle_t *device,
                                                  ds18b20_temperature_t *temperature) {
    if (!driver_state.initialized) {
        return DS18B20_ERROR_INIT;
    }
    
    if (device == NULL || !device->initialized || temperature == NULL) {
        return DS18B20_ERROR_INVALID_PARAM;
    }
    
    // Start conversion
    ds18b20_error_t result = ds18b20_start_conversion(device);
    if (result != DS18B20_OK) {
        return result;
    }
    
    // Wait for conversion to complete
    bool is_complete = false;
    uint32_t timeout = get_time_ms() + 1000; // 1 second timeout
    
    while (!is_complete && get_time_ms() < timeout) {
        result = ds18b20_is_conversion_complete(device, &is_complete);
        if (result != DS18B20_OK) {
            return result;
        }
        
        // Small delay to prevent busy waiting
        for (volatile int i = 0; i < 10000; i++);
    }
    
    if (!is_complete) {
        return DS18B20_ERROR_TIMEOUT;
    }
    
    // Read temperature
    return ds18b20_read_temperature(device, temperature);
}

ds18b20_error_t ds18b20_get_power_mode(const ds18b20_handle_t *device,
                                       ds18b20_power_mode_t *power_mode) {
    if (!driver_state.initialized) {
        return DS18B20_ERROR_INIT;
    }
    
    if (device == NULL || !device->initialized || power_mode == NULL) {
        return DS18B20_ERROR_INVALID_PARAM;
    }
    
    *power_mode = device->power_mode;
    return DS18B20_OK;
}

float ds18b20_raw_to_celsius(uint16_t raw_value, ds18b20_resolution_t resolution) {
    int16_t temp_raw = (int16_t)raw_value;
    
    // Convert based on resolution
    switch (resolution) {
        case DS18B20_RESOLUTION_9BIT:
            return (float)temp_raw / 8.0f;   // 0.5°C per bit
        case DS18B20_RESOLUTION_10BIT:
            return (float)temp_raw / 16.0f;  // 0.25°C per bit
        case DS18B20_RESOLUTION_11BIT:
            return (float)temp_raw / 32.0f;  // 0.125°C per bit
        case DS18B20_RESOLUTION_12BIT:
        default:
            return (float)temp_raw / 16.0f;  // 0.0625°C per bit
    }
}

float ds18b20_celsius_to_fahrenheit(float celsius) {
    return (celsius * 9.0f / 5.0f) + 32.0f;
}

const char* ds18b20_get_error_string(ds18b20_error_t error) {
    switch (error) {
        case DS18B20_OK:
            return "Operation successful";
        case DS18B20_ERROR_INIT:
            return "Initialization error";
        case DS18B20_ERROR_NOT_FOUND:
            return "Sensor not found";
        case DS18B20_ERROR_CRC:
            return "CRC error";
        case DS18B20_ERROR_TIMEOUT:
            return "Operation timeout";
        case DS18B20_ERROR_INVALID_PARAM:
            return "Invalid parameter";
        case DS18B20_ERROR_CONVERSION:
            return "Temperature conversion error";
        case DS18B20_ERROR_COMM:
            return "Communication error";
        default:
            return "Unknown error";
    }
}

ds18b20_error_t ds18b20_deinit(void) {
    if (!driver_state.initialized) {
        return DS18B20_ERROR_INIT;
    }
    
    // Clear driver state
    memset(&driver_state, 0, sizeof(driver_state));
    
    return DS18B20_OK;
}

/** @} */