# generate_wpa_supplicant_config
A sample which generates various wpa_supplicant configuration files.

In the embedded system, most WI-FI modules support "wpa_supplicant", so we can use "WPA" command to control it.
But, if your SSID or password includes a special character, you can't run by the shell command.
Therefore, I use these program to generate different encrypted type of  "wpa_supplicant" configure file.

The program's flow as follow.

1)  Input SSID and password.

2)  Use "wpa_cli" command to get the result of the site survey, and classify them by encrypting way.

3)  When the result of the input data is matched, it will come out "wpa_supplicant" configure file.

And then, you could use "wpa_cli" command to connect the network.
