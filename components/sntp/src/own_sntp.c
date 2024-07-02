#include <time.h>
#include <sys/time.h>
#include "esp_system.h"
#include "own_sntp.h"
#include "esp_sntp.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_netif_sntp.h"
#include "lwip/ip_addr.h"
#include "protocol_examples_common.h"
#include "esp_sleep.h"
static const char *TAG = "SNTP";

void set_timezone();
static void connect_to_server();

void set_timezone()
{
    time_t now = time(NULL);
    struct tm *local = localtime(&now);

    // Asumiendo que el horario de verano en España comienza el último domingo de marzo
    // y termina el último domingo de octubre.
    int month = local->tm_mon + 1; // tm_mon es 0-11
    int day = local->tm_mday;
    int wday = local->tm_wday;

    // Calcular el último domingo de marzo
    int last_sunday_march = 31 - (wday - day % 7);
    // Calcular el último domingo de octubre
    int last_sunday_october = 31 - (wday - day % 7);

    // Establecer la zona horaria a CET o CEST
    if ((month > 3 && month < 10) || (month == 3 && day >= last_sunday_march) || (month == 10 && day < last_sunday_october))
    {
        // Horario de verano (CEST)
        setenv("TZ", "CEST-2", 1);
    }
    else
    {
        // Horario estándar (CET)
        setenv("TZ", "CET-1", 1);
    }
    tzset(); // Aplicar los cambios de la zona horaria
}

void obtain_time()
{

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900))
    {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        connect_to_server();
        // update 'now' variable with current time
        time(&now);
    }
    // Set timezone to Eastern Standard Time and print local time
    set_timezone();

    // Obtener y mostrar la hora actual
    time_t aux = time(NULL);
    struct tm *local = localtime(&aux);
    ESP_LOGI(TAG, "La hora actual en España es: %02d:%02d\n", local->tm_hour, local->tm_min);

    if (timeinfo.tm_year < (2016 - 1900))
    {
        const int deep_sleep_sec = 10;
        ESP_LOGI(TAG, "Entering deep sleep for %d seconds", deep_sleep_sec);
        esp_deep_sleep(1000000LL * deep_sleep_sec);
    }
}

static void connect_to_server(void)
{

#if LWIP_DHCP_GET_NTP_SRV
    /**
     * NTP server address could be acquired via DHCP,
     * see following menuconfig options:
     * 'LWIP_DHCP_GET_NTP_SRV' - enable STNP over DHCP
     * 'LWIP_SNTP_DEBUG' - enable debugging messages
     *
     * NOTE: This call should be made BEFORE esp acquires IP address from DHCP,
     * otherwise NTP option would be rejected by default.
     */
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(CONFIG_SNTP_TIME_SERVER);
    config.start = false;                     // start SNTP service explicitly (after connecting)
    config.server_from_dhcp = true;           // accept NTP offers from DHCP server, if any (need to enable *before* connecting)
    config.renew_servers_after_new_IP = true; // let esp-netif update configured SNTP server(s) after receiving DHCP lease
    config.index_of_first_server = 1;         // updates from server num 1, leaving server 0 (from DHCP) intact
    // configure the event on which we renew servers
#ifdef CONFIG_EXAMPLE_CONNECT_WIFI
    config.ip_event_to_renew = IP_EVENT_STA_GOT_IP;
#else
    config.ip_event_to_renew = IP_EVENT_ETH_GOT_IP;
#endif
    config.sync_cb = sntp_set_time_sync_notification_cb; // only if we need the notification function
    esp_netif_sntp_init(&config);

#endif /* LWIP_DHCP_GET_NTP_SRV */

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

#if LWIP_DHCP_GET_NTP_SRV
    ESP_LOGI(TAG, "Starting SNTP");
    esp_netif_sntp_start();
#if LWIP_IPV6 && SNTP_MAX_SERVERS > 2
    /* This demonstrates using IPv6 address as an additional SNTP server
     * (statically assigned IPv6 address is also possible)
     */
    ip_addr_t ip6;
    if (ipaddr_aton("2a01:3f7::1", &ip6))
    { // ipv6 ntp source "ntp.netnod.se"
        esp_sntp_setserver(2, &ip6);
    }
#endif /* LWIP_IPV6 */

#else
    ESP_LOGI(TAG, "Initializing and starting SNTP");
#if CONFIG_LWIP_SNTP_MAX_SERVERS > 1
    /* This demonstrates configuring more than one server
     */
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG_MULTIPLE(2,
                                                                      ESP_SNTP_SERVER_LIST(CONFIG_SNTP_TIME_SERVER, "pool.ntp.org"));
#else
    /*
     * This is the basic default config with one server and starting the service
     */
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(CONFIG_SNTP_TIME_SERVER);
#endif
    config.sync_cb = sntp_set_time_sync_notification_cb; // Note: This is only needed if we want
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    config.smooth_sync = true;
#endif

    esp_netif_sntp_init(&config);
#endif

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 15;
    while (esp_netif_sntp_sync_wait(2000 / portTICK_PERIOD_MS) == ESP_ERR_TIMEOUT && ++retry < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
    }

    time(&now);
    localtime_r(&now, &timeinfo);
    ESP_ERROR_CHECK(example_disconnect());

    esp_netif_sntp_deinit();
}
