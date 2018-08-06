

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
	char security_mode;//reference WIFI_SECURITY_MODE
	char ssid[MAX_SSID_LENGTH+1];
} WiFi_SCAN_RESULT;
