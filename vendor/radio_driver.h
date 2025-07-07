/**
 * @file radio_driver.h
 * @brief Wireless Radio Transceiver Driver API
 * 
 * Vendor-supplied driver for embedded wireless radio module.
 * Provides APIs for radio configuration, power management, packet transmission,
 * and network connectivity management.
 * 
 * @author Radio Vendor Engineering Team
 * @version 2.1.4
 * @date 2024-04-20
 * 
 * @copyright Copyright (c) 2024 Radio Solutions Corp. All rights reserved.
 */

#ifndef RADIO_DRIVER_H
#define RADIO_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup Radio_Constants Radio Constants
 * @{
 */

/** Maximum packet payload size (bytes) */
#define RADIO_MAX_PAYLOAD_SIZE      246

/** Maximum number of retries for packet transmission */
#define RADIO_MAX_RETRIES           5

/** Default transmission timeout (milliseconds) */
#define RADIO_DEFAULT_TX_TIMEOUT_MS 5000

/** Maximum number of channels */
#define RADIO_MAX_CHANNELS          125

/** RSSI measurement range */
#define RADIO_RSSI_MIN              -120
#define RADIO_RSSI_MAX              -30

/** Device address size (bytes) */
#define RADIO_ADDRESS_SIZE          8

/** Network key size (bytes) */
#define RADIO_NETWORK_KEY_SIZE      16

/** @} */

/** @defgroup Radio_Types Radio Type Definitions
 * @{
 */

/**
 * @brief Radio power states
 */
typedef enum {
    RADIO_POWER_OFF = 0,              /**< Radio completely powered down */
    RADIO_POWER_SLEEP = 1,            /**< Low power sleep mode */
    RADIO_POWER_STANDBY = 2,          /**< Standby mode, ready to wake */
    RADIO_POWER_IDLE = 3,             /**< Idle mode, ready for operation */
    RADIO_POWER_RX = 4,               /**< Receiving mode */
    RADIO_POWER_TX = 5                /**< Transmitting mode */
} radio_power_state_t;

/**
 * @brief Radio transmission power levels
 */
typedef enum {
    RADIO_TX_POWER_MIN = 0,           /**< Minimum power (-20 dBm) */
    RADIO_TX_POWER_LOW = 1,           /**< Low power (-10 dBm) */
    RADIO_TX_POWER_MEDIUM = 2,        /**< Medium power (0 dBm) */
    RADIO_TX_POWER_HIGH = 3,          /**< High power (+10 dBm) */
    RADIO_TX_POWER_MAX = 4            /**< Maximum power (+20 dBm) */
} radio_tx_power_t;

/**
 * @brief Radio data rates
 */
typedef enum {
    RADIO_DATA_RATE_1K = 0,           /**< 1 kbps */
    RADIO_DATA_RATE_10K = 1,          /**< 10 kbps */
    RADIO_DATA_RATE_50K = 2,          /**< 50 kbps */
    RADIO_DATA_RATE_100K = 3,         /**< 100 kbps */
    RADIO_DATA_RATE_250K = 4          /**< 250 kbps */
} radio_data_rate_t;

/**
 * @brief Radio modulation schemes
 */
typedef enum {
    RADIO_MODULATION_FSK = 0,         /**< Frequency Shift Keying */
    RADIO_MODULATION_GFSK = 1,        /**< Gaussian FSK */
    RADIO_MODULATION_LORA = 2,        /**< LoRa modulation */
    RADIO_MODULATION_OOK = 3          /**< On-Off Keying */
} radio_modulation_t;

/**
 * @brief Radio error codes
 */
typedef enum {
    RADIO_OK = 0,                     /**< Operation successful */
    RADIO_ERROR_INIT = -1,            /**< Initialization error */
    RADIO_ERROR_CONFIG = -2,          /**< Configuration error */
    RADIO_ERROR_TIMEOUT = -3,         /**< Operation timeout */
    RADIO_ERROR_NO_ACK = -4,          /**< No acknowledgment received */
    RADIO_ERROR_CRC = -5,             /**< CRC error */
    RADIO_ERROR_INVALID_PARAM = -6,   /**< Invalid parameter */
    RADIO_ERROR_BUFFER_FULL = -7,     /**< Buffer full */
    RADIO_ERROR_BUFFER_EMPTY = -8,    /**< Buffer empty */
    RADIO_ERROR_CHANNEL_BUSY = -9,    /**< Channel busy */
    RADIO_ERROR_POWER_FAILURE = -10,  /**< Power supply failure */
    RADIO_ERROR_HARDWARE = -11,       /**< Hardware failure */
    RADIO_ERROR_NOT_CONNECTED = -12,  /**< Not connected to network */
    RADIO_ERROR_ENCRYPTION = -13,     /**< Encryption/decryption error */
    RADIO_ERROR_PACKET_TOO_LARGE = -14, /**< Packet exceeds size limit */
    RADIO_ERROR_NETWORK_FULL = -15,   /**< Network capacity exceeded */
    RADIO_ERROR_RATE_LIMITED = -16    /**< Rate limit exceeded */
} radio_error_t;

/**
 * @brief Radio packet priority levels
 */
typedef enum {
    RADIO_PRIORITY_LOW = 0,           /**< Low priority, best effort */
    RADIO_PRIORITY_NORMAL = 1,        /**< Normal priority */
    RADIO_PRIORITY_HIGH = 2,          /**< High priority */
    RADIO_PRIORITY_CRITICAL = 3       /**< Critical priority */
} radio_packet_priority_t;

/**
 * @brief Radio network security modes
 */
typedef enum {
    RADIO_SECURITY_NONE = 0,          /**< No encryption */
    RADIO_SECURITY_WEP = 1,           /**< WEP encryption */
    RADIO_SECURITY_WPA = 2,           /**< WPA encryption */
    RADIO_SECURITY_AES128 = 3,        /**< AES-128 encryption */
    RADIO_SECURITY_AES256 = 4         /**< AES-256 encryption */
} radio_security_mode_t;

/**
 * @brief Radio configuration structure
 */
typedef struct {
    uint32_t frequency_hz;            /**< Operating frequency in Hz */
    uint8_t channel;                  /**< Channel number (0-124) */
    radio_tx_power_t tx_power;        /**< Transmission power level */
    radio_data_rate_t data_rate;      /**< Data transmission rate */
    radio_modulation_t modulation;    /**< Modulation scheme */
    radio_security_mode_t security;   /**< Security/encryption mode */
    uint8_t network_key[RADIO_NETWORK_KEY_SIZE]; /**< Network encryption key */
    uint8_t device_address[RADIO_ADDRESS_SIZE];  /**< Device address */
    uint16_t network_id;              /**< Network identifier */
    bool auto_ack;                    /**< Automatic acknowledgment */
    bool auto_retry;                  /**< Automatic retry on failure */
    uint8_t max_retries;              /**< Maximum retry attempts */
    uint32_t tx_timeout_ms;           /**< Transmission timeout */
} radio_config_t;

/**
 * @brief Radio packet structure
 */
typedef struct {
    uint8_t destination[RADIO_ADDRESS_SIZE]; /**< Destination address */
    uint8_t source[RADIO_ADDRESS_SIZE];      /**< Source address */
    uint16_t packet_id;               /**< Unique packet identifier */
    radio_packet_priority_t priority; /**< Packet priority */
    uint8_t payload_size;             /**< Payload size in bytes */
    uint8_t payload[RADIO_MAX_PAYLOAD_SIZE]; /**< Packet payload */
    uint32_t timestamp;               /**< Transmission timestamp */
    bool require_ack;                 /**< Require acknowledgment */
    uint8_t retry_count;              /**< Current retry count */
} radio_packet_t;

/**
 * @brief Radio statistics structure
 */
typedef struct {
    uint32_t packets_sent;            /**< Total packets sent */
    uint32_t packets_received;        /**< Total packets received */
    uint32_t packets_lost;            /**< Total packets lost */
    uint32_t retries_attempted;       /**< Total retry attempts */
    uint32_t crc_errors;              /**< CRC error count */
    uint32_t timeouts;                /**< Timeout count */
    int8_t last_rssi;                 /**< Last RSSI measurement */
    uint8_t channel_utilization;      /**< Channel utilization (0-100%) */
    uint32_t total_airtime_ms;        /**< Total transmission time */
    uint32_t power_consumption_mw;    /**< Power consumption estimate */
} radio_stats_t;

/**
 * @brief Radio network information
 */
typedef struct {
    uint16_t network_id;              /**< Network identifier */
    uint8_t connected_devices;        /**< Number of connected devices */
    int8_t signal_strength;           /**< Signal strength (RSSI) */
    uint8_t link_quality;             /**< Link quality (0-100%) */
    uint32_t uptime_seconds;          /**< Network uptime */
    bool is_gateway;                  /**< Is this device a gateway */
    uint8_t hop_count;                /**< Hops to gateway */
} radio_network_info_t;

/**
 * @brief Radio event callback function type
 */
typedef void (*radio_event_callback_t)(radio_error_t event, void *user_data);

/**
 * @brief Radio packet received callback function type
 */
typedef void (*radio_rx_callback_t)(const radio_packet_t *packet, void *user_data);

/** @} */

/** @defgroup Radio_Functions Radio API Functions
 * @{
 */

/**
 * @brief Initialize radio driver
 * 
 * Initializes the radio hardware and driver subsystem.
 * Must be called before any other radio functions.
 * 
 * @param[in] config Pointer to radio configuration structure
 * @return radio_error_t Error code
 */
radio_error_t radio_init(const radio_config_t *config);

/**
 * @brief Configure radio parameters
 * 
 * Updates radio configuration with new parameters.
 * Radio must be in idle state for configuration changes.
 * 
 * @param[in] config Pointer to new configuration
 * @return radio_error_t Error code
 */
radio_error_t radio_configure(const radio_config_t *config);

/**
 * @brief Set radio power state
 * 
 * Controls the radio power management state.
 * 
 * @param[in] power_state Desired power state
 * @return radio_error_t Error code
 */
radio_error_t radio_set_power_state(radio_power_state_t power_state);

/**
 * @brief Get current radio power state
 * 
 * @param[out] power_state Pointer to store current power state
 * @return radio_error_t Error code
 */
radio_error_t radio_get_power_state(radio_power_state_t *power_state);

/**
 * @brief Send data packet
 * 
 * Transmits a data packet with optional acknowledgment and retry.
 * 
 * @param[in] packet Pointer to packet structure
 * @return radio_error_t Error code
 */
radio_error_t radio_send_packet(const radio_packet_t *packet);

/**
 * @brief Send data packet (non-blocking)
 * 
 * Queues a packet for transmission without blocking.
 * Use radio_get_tx_status() to check completion.
 * 
 * @param[in] packet Pointer to packet structure
 * @param[out] tx_id Pointer to store transaction ID
 * @return radio_error_t Error code
 */
radio_error_t radio_send_packet_async(const radio_packet_t *packet, uint16_t *tx_id);

/**
 * @brief Get transmission status
 * 
 * Checks the status of an asynchronous transmission.
 * 
 * @param[in] tx_id Transaction ID from send_packet_async
 * @param[out] status Pointer to store transmission status
 * @return radio_error_t Error code
 */
radio_error_t radio_get_tx_status(uint16_t tx_id, radio_error_t *status);

/**
 * @brief Receive data packet
 * 
 * Receives a data packet from the radio buffer.
 * 
 * @param[out] packet Pointer to store received packet
 * @param[in] timeout_ms Timeout in milliseconds (0 for non-blocking)
 * @return radio_error_t Error code
 */
radio_error_t radio_receive_packet(radio_packet_t *packet, uint32_t timeout_ms);

/**
 * @brief Set packet received callback
 * 
 * Registers a callback function for packet reception events.
 * 
 * @param[in] callback Callback function pointer
 * @param[in] user_data User data pointer passed to callback
 * @return radio_error_t Error code
 */
radio_error_t radio_set_rx_callback(radio_rx_callback_t callback, void *user_data);

/**
 * @brief Set event callback
 * 
 * Registers a callback function for radio events (errors, state changes).
 * 
 * @param[in] callback Callback function pointer
 * @param[in] user_data User data pointer passed to callback
 * @return radio_error_t Error code
 */
radio_error_t radio_set_event_callback(radio_event_callback_t callback, void *user_data);

/**
 * @brief Scan for available networks
 * 
 * Scans for available networks and returns network information.
 * 
 * @param[out] networks Array to store network information
 * @param[in] max_networks Maximum number of networks to find
 * @param[out] found_count Pointer to store actual number found
 * @param[in] scan_time_ms Scan duration in milliseconds
 * @return radio_error_t Error code
 */
radio_error_t radio_scan_networks(radio_network_info_t *networks,
                                  uint8_t max_networks,
                                  uint8_t *found_count,
                                  uint32_t scan_time_ms);

/**
 * @brief Join a network
 * 
 * Attempts to join a specific network using provided credentials.
 * 
 * @param[in] network_id Network identifier to join
 * @param[in] network_key Network encryption key
 * @param[in] timeout_ms Join timeout in milliseconds
 * @return radio_error_t Error code
 */
radio_error_t radio_join_network(uint16_t network_id,
                                 const uint8_t *network_key,
                                 uint32_t timeout_ms);

/**
 * @brief Leave current network
 * 
 * Disconnects from the current network.
 * 
 * @return radio_error_t Error code
 */
radio_error_t radio_leave_network(void);

/**
 * @brief Get network information
 * 
 * Retrieves information about the current network connection.
 * 
 * @param[out] network_info Pointer to store network information
 * @return radio_error_t Error code
 */
radio_error_t radio_get_network_info(radio_network_info_t *network_info);

/**
 * @brief Measure signal strength (RSSI)
 * 
 * Measures the received signal strength indicator.
 * 
 * @param[out] rssi Pointer to store RSSI value (dBm)
 * @return radio_error_t Error code
 */
radio_error_t radio_measure_rssi(int8_t *rssi);

/**
 * @brief Get channel utilization
 * 
 * Measures the current channel utilization percentage.
 * 
 * @param[out] utilization Pointer to store utilization (0-100%)
 * @return radio_error_t Error code
 */
radio_error_t radio_get_channel_utilization(uint8_t *utilization);

/**
 * @brief Get radio statistics
 * 
 * Retrieves comprehensive radio usage statistics.
 * 
 * @param[out] stats Pointer to store statistics
 * @return radio_error_t Error code
 */
radio_error_t radio_get_statistics(radio_stats_t *stats);

/**
 * @brief Reset radio statistics
 * 
 * Resets all radio statistics counters to zero.
 * 
 * @return radio_error_t Error code
 */
radio_error_t radio_reset_statistics(void);

/**
 * @brief Perform radio self-test
 * 
 * Executes built-in self-test procedures.
 * 
 * @param[out] test_results Pointer to store test results bitmask
 * @return radio_error_t Error code
 */
radio_error_t radio_self_test(uint32_t *test_results);

/**
 * @brief Get radio firmware version
 * 
 * Retrieves the radio module firmware version.
 * 
 * @param[out] version_string Buffer to store version string
 * @param[in] buffer_size Size of version string buffer
 * @return radio_error_t Error code
 */
radio_error_t radio_get_firmware_version(char *version_string, size_t buffer_size);

/**
 * @brief Get error string description
 * 
 * Returns a human-readable string describing the error code.
 * 
 * @param[in] error Error code
 * @return const char* Error description string
 */
const char* radio_get_error_string(radio_error_t error);

/**
 * @brief Calculate packet airtime
 * 
 * Calculates the transmission time for a packet with given parameters.
 * 
 * @param[in] payload_size Payload size in bytes
 * @param[in] data_rate Data transmission rate
 * @param[in] modulation Modulation scheme
 * @return uint32_t Airtime in microseconds
 */
uint32_t radio_calculate_airtime(uint8_t payload_size,
                                 radio_data_rate_t data_rate,
                                 radio_modulation_t modulation);

/**
 * @brief Estimate power consumption
 * 
 * Estimates power consumption for a given operation.
 * 
 * @param[in] power_state Power state
 * @param[in] duration_ms Operation duration in milliseconds
 * @return uint32_t Estimated power consumption in microampere-hours
 */
uint32_t radio_estimate_power_consumption(radio_power_state_t power_state,
                                          uint32_t duration_ms);

/**
 * @brief Deinitialize radio driver
 * 
 * Cleanup function to release resources and power down radio.
 * 
 * @return radio_error_t Error code
 */
radio_error_t radio_deinit(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* RADIO_DRIVER_H */