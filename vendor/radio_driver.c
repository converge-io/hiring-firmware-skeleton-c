/**
 * @file radio_driver.c
 * @brief Simulated implementation of the wireless radio transceiver driver
 * 
 * This is a simulation implementation that mimics the behavior of a real
 * radio device for testing and development purposes.
 */

#include "radio_driver.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

/* Internal state structure */
typedef struct {
    bool initialized;
    radio_config_t config;
    radio_power_state_t power_state;
    radio_stats_t stats;
    radio_network_info_t network_info;
    bool connected_to_network;
    radio_rx_callback_t rx_callback;
    void *rx_user_data;
    radio_event_callback_t event_callback;
    void *event_user_data;
    uint16_t next_tx_id;
    uint32_t last_activity_time;
    uint8_t rx_buffer_count;
    radio_packet_t rx_buffer[32]; // Simple circular buffer
    uint8_t rx_buffer_head;
    uint8_t rx_buffer_tail;
} radio_state_t;

/* Global state */
static radio_state_t g_radio = {0};

/* Helper functions */
static uint32_t get_current_time_ms(void) {
    return (uint32_t)(clock() * 1000 / CLOCKS_PER_SEC);
}

static int8_t simulate_rssi(void) {
    // Simulate RSSI with some randomness around -70 dBm
    int base_rssi = -70;
    int variation = (rand() % 20) - 10; // ±10 dBm variation
    int8_t rssi = base_rssi + variation;
    
    // Clamp to valid range
    if (rssi < RADIO_RSSI_MIN) rssi = RADIO_RSSI_MIN;
    if (rssi > RADIO_RSSI_MAX) rssi = RADIO_RSSI_MAX;
    
    return rssi;
}

static uint8_t simulate_channel_utilization(void) {
    // Simulate channel utilization based on network activity
    return (rand() % 30) + 10; // 10-40% utilization
}

static bool validate_config(const radio_config_t *config) {
    if (!config) return false;
    if (config->channel >= RADIO_MAX_CHANNELS) return false;
    if (config->max_retries > RADIO_MAX_RETRIES) return false;
    if (config->tx_timeout_ms == 0) return false;
    return true;
}

static void simulate_packet_reception(void) {
    // Randomly generate received packets during idle time
    if (g_radio.power_state != RADIO_POWER_RX && 
        g_radio.power_state != RADIO_POWER_IDLE) {
        return;
    }
    
    // Only simulate reception occasionally
    if ((rand() % 100) < 5) { // 5% chance per call
        if (g_radio.rx_buffer_count < 32) {
            radio_packet_t *packet = &g_radio.rx_buffer[g_radio.rx_buffer_head];
            
            // Generate a simulated packet
            for (int i = 0; i < RADIO_ADDRESS_SIZE; i++) {
                packet->destination[i] = g_radio.config.device_address[i];
                packet->source[i] = rand() % 256;
            }
            packet->packet_id = rand() % 65536;
            packet->priority = RADIO_PRIORITY_NORMAL;
            packet->payload_size = (rand() % 100) + 1;
            for (int i = 0; i < packet->payload_size; i++) {
                packet->payload[i] = rand() % 256;
            }
            packet->timestamp = get_current_time_ms();
            packet->require_ack = false;
            packet->retry_count = 0;
            
            g_radio.rx_buffer_head = (g_radio.rx_buffer_head + 1) % 32;
            g_radio.rx_buffer_count++;
            g_radio.stats.packets_received++;
            
            // Call callback if set
            if (g_radio.rx_callback) {
                g_radio.rx_callback(packet, g_radio.rx_user_data);
            }
        }
    }
}

/* API Implementation */

radio_error_t radio_init(const radio_config_t *config) {
    if (!config) {
        return RADIO_ERROR_INVALID_PARAM;
    }
    
    if (!validate_config(config)) {
        return RADIO_ERROR_CONFIG;
    }
    
    // Initialize random seed
    srand(time(NULL));
    
    // Clear state
    memset(&g_radio, 0, sizeof(g_radio));
    
    // Copy configuration
    memcpy(&g_radio.config, config, sizeof(radio_config_t));
    
    // Initialize state
    g_radio.initialized = true;
    g_radio.power_state = RADIO_POWER_IDLE;
    g_radio.connected_to_network = false;
    g_radio.next_tx_id = 1;
    g_radio.last_activity_time = get_current_time_ms();
    
    // Initialize network info
    g_radio.network_info.network_id = config->network_id;
    g_radio.network_info.connected_devices = 0;
    g_radio.network_info.signal_strength = simulate_rssi();
    g_radio.network_info.link_quality = 0;
    g_radio.network_info.uptime_seconds = 0;
    g_radio.network_info.is_gateway = false;
    g_radio.network_info.hop_count = 255; // Not connected
    
    // Initialize statistics
    memset(&g_radio.stats, 0, sizeof(g_radio.stats));
    g_radio.stats.last_rssi = simulate_rssi();
    
    return RADIO_OK;
}

radio_error_t radio_configure(const radio_config_t *config) {
    if (!g_radio.initialized) {
        return RADIO_ERROR_INIT;
    }
    
    if (!config) {
        return RADIO_ERROR_INVALID_PARAM;
    }
    
    if (!validate_config(config)) {
        return RADIO_ERROR_CONFIG;
    }
    
    if (g_radio.power_state != RADIO_POWER_IDLE) {
        return RADIO_ERROR_CONFIG;
    }
    
    // Copy new configuration
    memcpy(&g_radio.config, config, sizeof(radio_config_t));
    
    return RADIO_OK;
}

radio_error_t radio_set_power_state(radio_power_state_t power_state) {
    if (!g_radio.initialized) {
        return RADIO_ERROR_INIT;
    }
    
    if (power_state < RADIO_POWER_OFF || power_state > RADIO_POWER_TX) {
        return RADIO_ERROR_INVALID_PARAM;
    }
    
    // Simulate power state transitions
    switch (power_state) {
        case RADIO_POWER_OFF:
            g_radio.connected_to_network = false;
            break;
        case RADIO_POWER_RX:
            if (g_radio.power_state == RADIO_POWER_OFF) {
                return RADIO_ERROR_CONFIG;
            }
            break;
        case RADIO_POWER_TX:
            if (g_radio.power_state == RADIO_POWER_OFF) {
                return RADIO_ERROR_CONFIG;
            }
            break;
        default:
            break;
    }
    
    g_radio.power_state = power_state;
    g_radio.last_activity_time = get_current_time_ms();
    
    return RADIO_OK;
}

radio_error_t radio_get_power_state(radio_power_state_t *power_state) {
    if (!g_radio.initialized) {
        return RADIO_ERROR_INIT;
    }
    
    if (!power_state) {
        return RADIO_ERROR_INVALID_PARAM;
    }
    
    *power_state = g_radio.power_state;
    return RADIO_OK;
}

radio_error_t radio_send_packet(const radio_packet_t *packet) {
    if (!g_radio.initialized) {
        return RADIO_ERROR_INIT;
    }
    
    if (!packet) {
        return RADIO_ERROR_INVALID_PARAM;
    }
    
    if (packet->payload_size > RADIO_MAX_PAYLOAD_SIZE) {
        return RADIO_ERROR_PACKET_TOO_LARGE;
    }
    
    if (g_radio.power_state == RADIO_POWER_OFF) {
        return RADIO_ERROR_POWER_FAILURE;
    }
    
    // Simulate transmission
    g_radio.power_state = RADIO_POWER_TX;
    g_radio.last_activity_time = get_current_time_ms();
    
    // Simulate transmission delay
    uint32_t airtime = radio_calculate_airtime(packet->payload_size, 
                                               g_radio.config.data_rate,
                                               g_radio.config.modulation);
    
    // Add to statistics
    g_radio.stats.packets_sent++;
    g_radio.stats.total_airtime_ms += airtime / 1000;
    
    // Simulate success/failure
    if ((rand() % 100) < 5) { // 5% failure rate
        g_radio.stats.packets_lost++;
        g_radio.power_state = RADIO_POWER_IDLE;
        return RADIO_ERROR_NO_ACK;
    }
    
    g_radio.power_state = RADIO_POWER_IDLE;
    return RADIO_OK;
}

radio_error_t radio_send_packet_async(const radio_packet_t *packet, uint16_t *tx_id) {
    if (!g_radio.initialized) {
        return RADIO_ERROR_INIT;
    }
    
    if (!packet || !tx_id) {
        return RADIO_ERROR_INVALID_PARAM;
    }
    
    if (packet->payload_size > RADIO_MAX_PAYLOAD_SIZE) {
        return RADIO_ERROR_PACKET_TOO_LARGE;
    }
    
    if (g_radio.power_state == RADIO_POWER_OFF) {
        return RADIO_ERROR_POWER_FAILURE;
    }
    
    *tx_id = g_radio.next_tx_id++;
    
    // For simulation, we'll assume transmission completes immediately
    g_radio.stats.packets_sent++;
    
    return RADIO_OK;
}

radio_error_t radio_get_tx_status(uint16_t tx_id, radio_error_t *status) {
    if (!g_radio.initialized) {
        return RADIO_ERROR_INIT;
    }
    
    if (!status) {
        return RADIO_ERROR_INVALID_PARAM;
    }
    
    // For simulation, assume all transmissions succeed
    *status = RADIO_OK;
    return RADIO_OK;
}

radio_error_t radio_receive_packet(radio_packet_t *packet, uint32_t timeout_ms) {
    if (!g_radio.initialized) {
        return RADIO_ERROR_INIT;
    }
    
    if (!packet) {
        return RADIO_ERROR_INVALID_PARAM;
    }
    
    if (g_radio.power_state == RADIO_POWER_OFF) {
        return RADIO_ERROR_POWER_FAILURE;
    }
    
    // Simulate packet reception
    simulate_packet_reception();
    
    // Check if we have packets in buffer
    if (g_radio.rx_buffer_count > 0) {
        memcpy(packet, &g_radio.rx_buffer[g_radio.rx_buffer_tail], sizeof(radio_packet_t));
        g_radio.rx_buffer_tail = (g_radio.rx_buffer_tail + 1) % 32;
        g_radio.rx_buffer_count--;
        return RADIO_OK;
    }
    
    // No packets available
    if (timeout_ms == 0) {
        return RADIO_ERROR_BUFFER_EMPTY;
    }
    
    // For simulation, we'll wait a bit and try again
    if (timeout_ms > 100) {
        simulate_packet_reception();
        if (g_radio.rx_buffer_count > 0) {
            memcpy(packet, &g_radio.rx_buffer[g_radio.rx_buffer_tail], sizeof(radio_packet_t));
            g_radio.rx_buffer_tail = (g_radio.rx_buffer_tail + 1) % 32;
            g_radio.rx_buffer_count--;
            return RADIO_OK;
        }
    }
    
    return RADIO_ERROR_TIMEOUT;
}

radio_error_t radio_set_rx_callback(radio_rx_callback_t callback, void *user_data) {
    if (!g_radio.initialized) {
        return RADIO_ERROR_INIT;
    }
    
    g_radio.rx_callback = callback;
    g_radio.rx_user_data = user_data;
    
    return RADIO_OK;
}

radio_error_t radio_set_event_callback(radio_event_callback_t callback, void *user_data) {
    if (!g_radio.initialized) {
        return RADIO_ERROR_INIT;
    }
    
    g_radio.event_callback = callback;
    g_radio.event_user_data = user_data;
    
    return RADIO_OK;
}

radio_error_t radio_scan_networks(radio_network_info_t *networks,
                                  uint8_t max_networks,
                                  uint8_t *found_count,
                                  uint32_t scan_time_ms) {
    if (!g_radio.initialized) {
        return RADIO_ERROR_INIT;
    }
    
    if (!networks || !found_count) {
        return RADIO_ERROR_INVALID_PARAM;
    }
    
    if (g_radio.power_state == RADIO_POWER_OFF) {
        return RADIO_ERROR_POWER_FAILURE;
    }
    
    // Simulate network scanning
    uint8_t num_networks = (rand() % 5) + 1; // 1-5 networks
    if (num_networks > max_networks) {
        num_networks = max_networks;
    }
    
    for (uint8_t i = 0; i < num_networks; i++) {
        networks[i].network_id = 1000 + i;
        networks[i].connected_devices = (rand() % 10) + 1;
        networks[i].signal_strength = simulate_rssi();
        networks[i].link_quality = (rand() % 50) + 50; // 50-100%
        networks[i].uptime_seconds = rand() % 86400;
        networks[i].is_gateway = (i == 0); // First network is gateway
        networks[i].hop_count = (rand() % 5) + 1;
    }
    
    *found_count = num_networks;
    return RADIO_OK;
}

radio_error_t radio_join_network(uint16_t network_id,
                                 const uint8_t *network_key,
                                 uint32_t timeout_ms) {
    if (!g_radio.initialized) {
        return RADIO_ERROR_INIT;
    }
    
    if (!network_key) {
        return RADIO_ERROR_INVALID_PARAM;
    }
    
    if (g_radio.power_state == RADIO_POWER_OFF) {
        return RADIO_ERROR_POWER_FAILURE;
    }
    
    // Simulate network join
    if ((rand() % 100) < 10) { // 10% failure rate
        return RADIO_ERROR_TIMEOUT;
    }
    
    // Update network info
    g_radio.network_info.network_id = network_id;
    g_radio.network_info.connected_devices = (rand() % 10) + 1;
    g_radio.network_info.signal_strength = simulate_rssi();
    g_radio.network_info.link_quality = (rand() % 30) + 70; // 70-100%
    g_radio.network_info.uptime_seconds = 0;
    g_radio.network_info.is_gateway = false;
    g_radio.network_info.hop_count = (rand() % 5) + 1;
    
    g_radio.connected_to_network = true;
    
    return RADIO_OK;
}

radio_error_t radio_leave_network(void) {
    if (!g_radio.initialized) {
        return RADIO_ERROR_INIT;
    }
    
    g_radio.connected_to_network = false;
    g_radio.network_info.hop_count = 255; // Not connected
    g_radio.network_info.link_quality = 0;
    
    return RADIO_OK;
}

radio_error_t radio_get_network_info(radio_network_info_t *network_info) {
    if (!g_radio.initialized) {
        return RADIO_ERROR_INIT;
    }
    
    if (!network_info) {
        return RADIO_ERROR_INVALID_PARAM;
    }
    
    if (!g_radio.connected_to_network) {
        return RADIO_ERROR_NOT_CONNECTED;
    }
    
    // Update dynamic fields
    g_radio.network_info.signal_strength = simulate_rssi();
    g_radio.network_info.uptime_seconds = (get_current_time_ms() - g_radio.last_activity_time) / 1000;
    
    memcpy(network_info, &g_radio.network_info, sizeof(radio_network_info_t));
    
    return RADIO_OK;
}

radio_error_t radio_measure_rssi(int8_t *rssi) {
    if (!g_radio.initialized) {
        return RADIO_ERROR_INIT;
    }
    
    if (!rssi) {
        return RADIO_ERROR_INVALID_PARAM;
    }
    
    if (g_radio.power_state == RADIO_POWER_OFF) {
        return RADIO_ERROR_POWER_FAILURE;
    }
    
    *rssi = simulate_rssi();
    g_radio.stats.last_rssi = *rssi;
    
    return RADIO_OK;
}

radio_error_t radio_get_channel_utilization(uint8_t *utilization) {
    if (!g_radio.initialized) {
        return RADIO_ERROR_INIT;
    }
    
    if (!utilization) {
        return RADIO_ERROR_INVALID_PARAM;
    }
    
    if (g_radio.power_state == RADIO_POWER_OFF) {
        return RADIO_ERROR_POWER_FAILURE;
    }
    
    *utilization = simulate_channel_utilization();
    g_radio.stats.channel_utilization = *utilization;
    
    return RADIO_OK;
}

radio_error_t radio_get_statistics(radio_stats_t *stats) {
    if (!g_radio.initialized) {
        return RADIO_ERROR_INIT;
    }
    
    if (!stats) {
        return RADIO_ERROR_INVALID_PARAM;
    }
    
    // Update dynamic statistics
    g_radio.stats.last_rssi = simulate_rssi();
    g_radio.stats.channel_utilization = simulate_channel_utilization();
    
    // Estimate power consumption based on activity
    uint32_t elapsed_ms = get_current_time_ms() - g_radio.last_activity_time;
    g_radio.stats.power_consumption_mw = radio_estimate_power_consumption(g_radio.power_state, elapsed_ms) / 1000;
    
    memcpy(stats, &g_radio.stats, sizeof(radio_stats_t));
    
    return RADIO_OK;
}

radio_error_t radio_reset_statistics(void) {
    if (!g_radio.initialized) {
        return RADIO_ERROR_INIT;
    }
    
    memset(&g_radio.stats, 0, sizeof(g_radio.stats));
    g_radio.stats.last_rssi = simulate_rssi();
    
    return RADIO_OK;
}

radio_error_t radio_self_test(uint32_t *test_results) {
    if (!g_radio.initialized) {
        return RADIO_ERROR_INIT;
    }
    
    if (!test_results) {
        return RADIO_ERROR_INVALID_PARAM;
    }
    
    // Simulate self-test results (all tests pass)
    *test_results = 0xFFFFFFFF; // All bits set = all tests pass
    
    return RADIO_OK;
}

radio_error_t radio_get_firmware_version(char *version_string, size_t buffer_size) {
    if (!g_radio.initialized) {
        return RADIO_ERROR_INIT;
    }
    
    if (!version_string || buffer_size < 8) {
        return RADIO_ERROR_INVALID_PARAM;
    }
    
    const char *version = "v2.1.4-sim";
    size_t len = strlen(version);
    if (len >= buffer_size) {
        len = buffer_size - 1;
    }
    
    strncpy(version_string, version, len);
    version_string[len] = '\0';
    
    return RADIO_OK;
}

const char* radio_get_error_string(radio_error_t error) {
    switch (error) {
        case RADIO_OK: return "Success";
        case RADIO_ERROR_INIT: return "Initialization error";
        case RADIO_ERROR_CONFIG: return "Configuration error";
        case RADIO_ERROR_TIMEOUT: return "Operation timeout";
        case RADIO_ERROR_NO_ACK: return "No acknowledgment received";
        case RADIO_ERROR_CRC: return "CRC error";
        case RADIO_ERROR_INVALID_PARAM: return "Invalid parameter";
        case RADIO_ERROR_BUFFER_FULL: return "Buffer full";
        case RADIO_ERROR_BUFFER_EMPTY: return "Buffer empty";
        case RADIO_ERROR_CHANNEL_BUSY: return "Channel busy";
        case RADIO_ERROR_POWER_FAILURE: return "Power supply failure";
        case RADIO_ERROR_HARDWARE: return "Hardware failure";
        case RADIO_ERROR_NOT_CONNECTED: return "Not connected to network";
        case RADIO_ERROR_ENCRYPTION: return "Encryption/decryption error";
        case RADIO_ERROR_PACKET_TOO_LARGE: return "Packet exceeds size limit";
        case RADIO_ERROR_NETWORK_FULL: return "Network capacity exceeded";
        case RADIO_ERROR_RATE_LIMITED: return "Rate limit exceeded";
        default: return "Unknown error";
    }
}

uint32_t radio_calculate_airtime(uint8_t payload_size,
                                 radio_data_rate_t data_rate,
                                 radio_modulation_t modulation) {
    // Simplified airtime calculation
    uint32_t bps;
    switch (data_rate) {
        case RADIO_DATA_RATE_1K: bps = 1000; break;
        case RADIO_DATA_RATE_10K: bps = 10000; break;
        case RADIO_DATA_RATE_50K: bps = 50000; break;
        case RADIO_DATA_RATE_100K: bps = 100000; break;
        case RADIO_DATA_RATE_250K: bps = 250000; break;
        default: bps = 10000; break;
    }
    
    // Add overhead for headers, preamble, etc.
    uint32_t total_bits = (payload_size + 16) * 8; // 16 bytes overhead
    
    // Apply modulation efficiency factor
    switch (modulation) {
        case RADIO_MODULATION_FSK: break; // No change
        case RADIO_MODULATION_GFSK: total_bits = (total_bits * 9) / 10; break; // 10% better
        case RADIO_MODULATION_LORA: total_bits = (total_bits * 3) / 2; break; // 50% worse but more robust
        case RADIO_MODULATION_OOK: total_bits = total_bits * 2; break; // 2x worse
    }
    
    return (total_bits * 1000000) / bps; // Return microseconds
}

uint32_t radio_estimate_power_consumption(radio_power_state_t power_state,
                                          uint32_t duration_ms) {
    uint32_t current_ma;
    
    switch (power_state) {
        case RADIO_POWER_OFF: current_ma = 0; break;
        case RADIO_POWER_SLEEP: current_ma = 1; break;
        case RADIO_POWER_STANDBY: current_ma = 5; break;
        case RADIO_POWER_IDLE: current_ma = 10; break;
        case RADIO_POWER_RX: current_ma = 20; break;
        case RADIO_POWER_TX: current_ma = 50; break;
        default: current_ma = 10; break;
    }
    
    // Convert to microampere-hours
    return (current_ma * 1000 * duration_ms) / 3600000;
}

radio_error_t radio_deinit(void) {
    if (!g_radio.initialized) {
        return RADIO_ERROR_INIT;
    }
    
    // Power down
    g_radio.power_state = RADIO_POWER_OFF;
    g_radio.connected_to_network = false;
    
    // Clear callbacks
    g_radio.rx_callback = NULL;
    g_radio.event_callback = NULL;
    
    // Clear initialization flag
    g_radio.initialized = false;
    
    return RADIO_OK;
}