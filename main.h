#define INTERFACE_NAME "wlan0"
#define DIR_PATH "/tmp"
#define CONFIG_FILENAME "wpa_supplicant.conf"
#define CTRL_INTERFACE "/var/run/wpa_supplicant"
#define MAX_SCAN_TIMES 1
#define MAX_SSID_LENGTH 32
#define MAX_SAVE_AP	50


enum WIFI_SECURITY_MODE
{
	WIFI_SEC_MODE_NONE = 0,
	WIFI_SEC_MODE_WPA2_PSK_CCMP=1,
	WIFI_SEC_MODE_WPA_PSK_CCMP=2,
	WIFI_SEC_MODE_WPA2_PSK_TKIP=3,
	WIFI_SEC_MODE_WPA_PSK_TKIP=4,
	WIFI_SEC_MODE_WPA2_PSK_CCMP_TKIP=5,
	WIFI_SEC_MODE_WPA_PSK_CCMP_TKIP=6,
	WIFI_SEC_MODE_WEP,	
};


typedef struct _WiFi_SCAN_RESULT
{
	int frequency;
	int rssi;
	enum WIFI_SECURITY_MODE security_mode;
	char ssid[MAX_SSID_LENGTH+1];
} WiFi_SCAN_RESULT;

enum RETURN_STATUS {
    SUCCESSFUL = 0,
    FILE_OPEN_ERROR = 1,
    INPUT_ERROR = 2,
    FIND_ROUTER_FAIL=3,
};
