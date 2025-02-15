/*
 * HoloCube Network Management
 * 
 * This header file defines the networking functionality for the HoloCube:
 * - WiFi connection management (both station and AP modes)
 * - Network configuration constants
 * - Platform-specific networking includes
 */

#ifndef NETWORK_H
#define NETWORK_H

// Timezone offset in milliseconds (8 hours = 8*60*60*1000)
#define TIMEZERO_OFFSIZE (28800000)

// WiFi connection status codes
#define CONN_SUCC 0          // Connection successful
#define CONN_ERROR 1         // Connection failed
#define CONN_ERR_TIMEOUT 15  // WiFi connection timeout in seconds

// Access Point (AP) mode flags
#define AP_DISABLE 0         // AP mode disabled
#define AP_ENABLE 1          // AP mode enabled

// Set your server's logical name here e.g. if 'myserver' then address is http://myserver.local/
#define SERVER_NAME "fileserver"

#ifdef ESP8266
#include <ESP8266WiFi.h>      // Built-in
#include <ESP8266WiFiMulti.h> // Built-in
#include <ESP8266WebServer.h> // Built-in
#include <ESP8266mDNS.h>
#else
#include <WiFi.h>      // Built-in
#include <WiFiMulti.h> // 当我们需要使用ESP8266开发板存储多个WiFi网络连接信息时，可以使用ESP8266WiFiMulti库来实现。
#include <WebServer.h> // https://github.com/Pedroalbuquerque/ESP32WebServer download and place in your Libraries folder
#include <ESPmDNS.h>
#include <HTTPClient.h>
#endif

// #ifdef __cplusplus
// extern "C"
// {
// #endif

// #include "WiFiGeneric.h"

// #ifdef __cplusplus
// } /* extern "C" */
// #endif

// Network configuration
extern IPAddress local_ip;   // Device's fixed IP address
extern IPAddress gateway;    // Network gateway (usually router address)
extern IPAddress subnet;     // Network subnet mask
extern IPAddress dns;        // DNS server address (usually router address)

extern const char *AP_SSID;  // Access Point SSID when in AP mode

/**
 * Timer callback for network reset functionality
 * @param xTimer Timer handle that triggered the callback
 */
void restCallback(TimerHandle_t xTimer);

/**
 * Network Management Class
 * 
 * Handles all network-related operations including:
 * - WiFi network scanning
 * - Connection management
 * - Access Point mode configuration
 */
class Network
{
private:
    unsigned long m_preDisWifiConnInfoMillis; // Timestamp of last connection status display

public:
    Network();  // Constructor

    /**
     * Scan for available WiFi networks
     */
    void search_wifi(void);

    /**
     * Attempt to connect to a WiFi network
     * @param ssid Network SSID to connect to
     * @param password Network password
     * @return true if connection successful, false otherwise
     */
    boolean start_conn_wifi(const char *ssid, const char *password);

    /**
     * End current WiFi connection
     * @return true if disconnection successful
     */
    boolean end_conn_wifi(void);

    /**
     * Disable WiFi functionality
     * @return true if WiFi successfully disabled
     */
    boolean close_wifi(void);

    /**
     * Enable Access Point mode
     * @param ap_ssid SSID for the access point (defaults to AP_SSID)
     * @param ap_password Optional password for the access point
     * @return true if AP mode successfully enabled
     */
    boolean open_ap(const char *ap_ssid = AP_SSID, const char *ap_password = NULL);
};

#endif
