/**
 * @file ds18b20_driver.h
 * @brief DS18B20 Digital Temperature Sensor Driver API
 * 
 * Vendor-supplied driver for DS18B20 1-Wire digital temperature sensor.
 * Provides APIs for sensor initialization, temperature reading, and configuration.
 * 
 * @author Vendor Engineering Team
 * @version 1.2.0
 * @date 2024-03-15
 * 
 * @copyright Copyright (c) 2024 Sensor Vendor Inc. All rights reserved.
 */

#ifndef DS18B20_DRIVER_H
#define DS18B20_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup DS18B20_Constants DS18B20 Constants
 * @{
 */

/** DS18B20 Family Code */
#define DS18B20_FAMILY_CODE         0x28

/** DS18B20 ROM Command Codes */
#define DS18B20_CMD_SEARCH_ROM      0xF0
#define DS18B20_CMD_READ_ROM        0x33
#define DS18B20_CMD_MATCH_ROM       0x55
#define DS18B20_CMD_SKIP_ROM        0xCC
#define DS18B20_CMD_ALARM_SEARCH    0xEC

/** DS18B20 Function Command Codes */
#define DS18B20_CMD_CONVERT_T       0x44
#define DS18B20_CMD_WRITE_SCRATCHPAD 0x4E
#define DS18B20_CMD_READ_SCRATCHPAD  0xBE
#define DS18B20_CMD_COPY_SCRATCHPAD  0x48
#define DS18B20_CMD_RECALL_E2       0xB8
#define DS18B20_CMD_READ_POWER_SUPPLY 0xB4

/** Temperature conversion time (milliseconds) */
#define DS18B20_CONVERSION_TIME_MS  750

/** @} */

/** @defgroup DS18B20_Types DS18B20 Type Definitions
 * @{
 */

/**
 * @brief DS18B20 resolution configuration
 */
typedef enum {
    DS18B20_RESOLUTION_9BIT  = 0x1F,  /**< 9-bit resolution (0.5°C) */
    DS18B20_RESOLUTION_10BIT = 0x3F,  /**< 10-bit resolution (0.25°C) */
    DS18B20_RESOLUTION_11BIT = 0x5F,  /**< 11-bit resolution (0.125°C) */
    DS18B20_RESOLUTION_12BIT = 0x7F   /**< 12-bit resolution (0.0625°C) */
} ds18b20_resolution_t;

/**
 * @brief DS18B20 power mode
 */
typedef enum {
    DS18B20_POWER_PARASITIC = 0,      /**< Parasitic power mode */
    DS18B20_POWER_EXTERNAL  = 1       /**< External power mode */
} ds18b20_power_mode_t;

/**
 * @brief DS18B20 error codes
 */
typedef enum {
    DS18B20_OK = 0,                   /**< Operation successful */
    DS18B20_ERROR_INIT = -1,          /**< Initialization error */
    DS18B20_ERROR_NOT_FOUND = -2,     /**< Sensor not found */
    DS18B20_ERROR_CRC = -3,           /**< CRC error */
    DS18B20_ERROR_TIMEOUT = -4,       /**< Operation timeout */
    DS18B20_ERROR_INVALID_PARAM = -5, /**< Invalid parameter */
    DS18B20_ERROR_CONVERSION = -6,    /**< Temperature conversion error */
    DS18B20_ERROR_COMM = -7           /**< Communication error */
} ds18b20_error_t;

/**
 * @brief DS18B20 device handle
 */
typedef struct {
    uint8_t rom_code[8];              /**< 64-bit ROM code */
    ds18b20_resolution_t resolution;  /**< Temperature resolution */
    ds18b20_power_mode_t power_mode;  /**< Power mode */
    uint8_t th_register;              /**< Temperature high alarm threshold */
    uint8_t tl_register;              /**< Temperature low alarm threshold */
    bool initialized;                 /**< Initialization status */
} ds18b20_handle_t;

/**
 * @brief Temperature data structure
 */
typedef struct {
    float temperature_c;              /**< Temperature in Celsius */
    float temperature_f;              /**< Temperature in Fahrenheit */
    uint16_t raw_value;               /**< Raw temperature value */
    bool valid;                       /**< Data validity flag */
} ds18b20_temperature_t;

/** @} */

/** @defgroup DS18B20_Functions DS18B20 API Functions
 * @{
 */

/**
 * @brief Initialize DS18B20 sensor driver
 * 
 * Initializes the 1-Wire bus and prepares the driver for communication.
 * Must be called before any other DS18B20 functions.
 * 
 * @param[in] onewire_pin GPIO pin number for 1-Wire bus
 * @return ds18b20_error_t Error code
 */
ds18b20_error_t ds18b20_init(uint8_t onewire_pin);

/**
 * @brief Scan for DS18B20 devices on the 1-Wire bus
 * 
 * Searches for all DS18B20 devices connected to the bus and populates
 * the provided array with device handles.
 * 
 * @param[out] devices Array to store found device handles
 * @param[in] max_devices Maximum number of devices to find
 * @param[out] found_count Pointer to store actual number of devices found
 * @return ds18b20_error_t Error code
 */
ds18b20_error_t ds18b20_scan_devices(ds18b20_handle_t *devices, 
                                     uint8_t max_devices, 
                                     uint8_t *found_count);

/**
 * @brief Configure DS18B20 sensor settings
 * 
 * Sets the temperature resolution and alarm thresholds for the specified device.
 * 
 * @param[in,out] device Pointer to device handle
 * @param[in] resolution Temperature resolution setting
 * @param[in] th_alarm High temperature alarm threshold (-55 to 125°C)
 * @param[in] tl_alarm Low temperature alarm threshold (-55 to 125°C)
 * @return ds18b20_error_t Error code
 */
ds18b20_error_t ds18b20_configure(ds18b20_handle_t *device,
                                  ds18b20_resolution_t resolution,
                                  int8_t th_alarm,
                                  int8_t tl_alarm);

/**
 * @brief Start temperature conversion
 * 
 * Initiates temperature conversion on the specified device.
 * Use ds18b20_is_conversion_complete() to check completion status.
 * 
 * @param[in] device Pointer to device handle
 * @return ds18b20_error_t Error code
 */
ds18b20_error_t ds18b20_start_conversion(const ds18b20_handle_t *device);

/**
 * @brief Check if temperature conversion is complete
 * 
 * @param[in] device Pointer to device handle
 * @param[out] is_complete Pointer to store completion status
 * @return ds18b20_error_t Error code
 */
ds18b20_error_t ds18b20_is_conversion_complete(const ds18b20_handle_t *device, 
                                               bool *is_complete);

/**
 * @brief Read temperature from DS18B20 sensor
 * 
 * Reads the temperature data from the sensor's scratchpad memory.
 * Performs CRC validation on the received data.
 * 
 * @param[in] device Pointer to device handle
 * @param[out] temperature Pointer to store temperature data
 * @return ds18b20_error_t Error code
 */
ds18b20_error_t ds18b20_read_temperature(const ds18b20_handle_t *device,
                                         ds18b20_temperature_t *temperature);

/**
 * @brief Read temperature with automatic conversion
 * 
 * Convenience function that starts conversion, waits for completion,
 * and reads the temperature in a single call.
 * 
 * @param[in] device Pointer to device handle
 * @param[out] temperature Pointer to store temperature data
 * @return ds18b20_error_t Error code
 */
ds18b20_error_t ds18b20_read_temperature_blocking(const ds18b20_handle_t *device,
                                                  ds18b20_temperature_t *temperature);

/**
 * @brief Get power supply mode of the sensor
 * 
 * Determines if the sensor is operating in parasitic or external power mode.
 * 
 * @param[in] device Pointer to device handle
 * @param[out] power_mode Pointer to store power mode
 * @return ds18b20_error_t Error code
 */
ds18b20_error_t ds18b20_get_power_mode(const ds18b20_handle_t *device,
                                       ds18b20_power_mode_t *power_mode);

/**
 * @brief Convert raw temperature value to Celsius
 * 
 * Utility function to convert raw temperature data to Celsius.
 * 
 * @param[in] raw_value Raw temperature value from sensor
 * @param[in] resolution Temperature resolution used
 * @return float Temperature in Celsius
 */
float ds18b20_raw_to_celsius(uint16_t raw_value, ds18b20_resolution_t resolution);

/**
 * @brief Convert Celsius to Fahrenheit
 * 
 * Utility function for temperature unit conversion.
 * 
 * @param[in] celsius Temperature in Celsius
 * @return float Temperature in Fahrenheit
 */
float ds18b20_celsius_to_fahrenheit(float celsius);

/**
 * @brief Get error string description
 * 
 * Returns a human-readable string describing the error code.
 * 
 * @param[in] error Error code
 * @return const char* Error description string
 */
const char* ds18b20_get_error_string(ds18b20_error_t error);

/**
 * @brief Deinitialize DS18B20 driver
 * 
 * Cleanup function to release resources used by the driver.
 * Should be called when the driver is no longer needed.
 * 
 * @return ds18b20_error_t Error code
 */
ds18b20_error_t ds18b20_deinit(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* DS18B20_DRIVER_H */
