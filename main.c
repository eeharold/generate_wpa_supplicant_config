#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "main.h"
WiFi_SCAN_RESULT scan_result_of_sort[MAX_SAVE_AP];

void retrieve_ssid(char *input_ssid)
{
    char tmp_result[64]="\0";
    char tmp_SSID[64]="\0";
    int i=0,j=0;

    strcpy(tmp_SSID,input_ssid);
    memset(input_ssid, 0, strlen(input_ssid));

    if(strlen(tmp_SSID)>0) {
        while(tmp_SSID[j] != '\0') {
            tmp_result[i]=tmp_SSID[j];

            if(tmp_SSID[j] == '\\') {
                tmp_result[i]=tmp_SSID[j];
                j++;

                if((tmp_SSID[j] !='\\') && (tmp_SSID[j]!='\"')) {
                    i++;
                }
                tmp_result[i]=tmp_SSID[j];
            }
            j++;
            i++;
        }
    }

    strcpy(input_ssid, tmp_result);
}



int get_security_mode( char *security_string)
{
    int secmode = WIFI_SEC_MODE_NONE;

    if(strstr(security_string, "WPA")!=NULL) {
        if(strstr(security_string, "WPA2")!=NULL) {
            if(strstr(security_string, "CCMP")!=NULL) {
                if(strstr(security_string, "TKIP")!=NULL) {
                    secmode = WIFI_SEC_MODE_WPA2_PSK_CCMP_TKIP;
                } else {
                    secmode = WIFI_SEC_MODE_WPA2_PSK_CCMP;
                }
            } else {
                secmode = WIFI_SEC_MODE_WPA2_PSK_TKIP;
            }
        } else {
            if(strstr(security_string, "CCMP")!=NULL) {
                if(strstr(security_string, "TKIP")!=NULL) {
                    secmode = WIFI_SEC_MODE_WPA_PSK_CCMP_TKIP;
                } else {
                    secmode = WIFI_SEC_MODE_WPA_PSK_CCMP;
                }
            } else {
                secmode = WIFI_SEC_MODE_WPA_PSK_TKIP;
            }
        }
    } else if (strstr(security_string, "WEB")!=NULL) {
        secmode = WIFI_SEC_MODE_WEP;
    }

    return secmode;
}


int start_site_survey(void)
{
    char cmd[64] = "\n";
    FILE *pp = NULL;
    int c,idx = 0;
    char* strPos = NULL;
    char data[8192]="\0";

    sprintf(cmd, "wpa_cli -i%s scan", INTERFACE_NAME);
    system(cmd);
    sleep(3);

    sprintf(cmd,"wpa_cli -i%s scan_results", INTERFACE_NAME);

    if( (pp = popen(cmd, "r")) == NULL ) {
        return -1;
    }

    memset(data,0,sizeof(data));
    while((c = fgetc(pp))!=EOF) {
        printf("%c",c);
        data[idx]=c;
        idx++;
    }
    pclose(pp);

    printf("site survey: data_len(%d bytes)\n", idx);

    strPos = data;
    strPos = strstr(data, "bssid / frequency / signal level / flags / ssid");

    if(NULL==strPos)
        return 0;

    strPos = strchr( strPos, '\n')+1;

    char *p;
    char *line="\n";
    p=strtok(strPos,line);

    int index_sec_pair = 0;
    char tmp_ssid[MAX_SSID_LENGTH*2+1] ="\0";
    char s_security[64]="\0";
    char frequency[8]="\0";

    char rssi_char[8]= "\0";
    int s_rssi=0;

    static char rssi_table[MAX_SAVE_AP][2];
    WiFi_SCAN_RESULT ssid_scan_result[MAX_SAVE_AP];

    memset(rssi_table,0,sizeof(rssi_table));
    memset(ssid_scan_result,0,sizeof(ssid_scan_result));
    memset(scan_result_of_sort,0,sizeof(scan_result_of_sort));

    while(p!= NULL) {
        memset(tmp_ssid,0,sizeof(tmp_ssid));
        memset(s_security,0,sizeof(s_security));

        sscanf(p, "%*s %s %s %s %[^\n]", frequency, rssi_char, s_security, tmp_ssid);

        s_rssi=atoi(rssi_char);
        /* -45 dbm => 100%, -75 dbm => 0% */
        if(s_rssi > -45) s_rssi = -45;
        else if(s_rssi < -75) s_rssi = -75;
        s_rssi = ((s_rssi + 75) * 10) / 3;

        /* modify ' \" ' and ' \\ ' to  " & \ */
        if(strlen(tmp_ssid)<=(64))
            retrieve_ssid(tmp_ssid);
        else
            goto NEXT_LINE;

        if(index_sec_pair<MAX_SAVE_AP) {
            memcpy(ssid_scan_result[index_sec_pair].ssid, tmp_ssid, strlen(tmp_ssid));
            ssid_scan_result[index_sec_pair].security_mode= get_security_mode(s_security);
            ssid_scan_result[index_sec_pair].rssi = s_rssi;
            ssid_scan_result[index_sec_pair].frequency =atoi(frequency);

            rssi_table[index_sec_pair][0]=s_rssi;
            rssi_table[index_sec_pair][1]=index_sec_pair;

            index_sec_pair++;
        } else {
            break;
        }
    NEXT_LINE:
        p=strtok(NULL, line);
    }

    int i=0, j=0;
    char tmp[2]= {0,0};

    for(i=0; i<index_sec_pair; i++) {
        for(j=0; j<index_sec_pair-1; j++) {
            if(rssi_table[j][0]<rssi_table[j+1][0]) {
                tmp[0]=rssi_table[j][0];
                tmp[1]=rssi_table[j][1];
                rssi_table[j][0]=rssi_table[j+1][0];
                rssi_table[j][1]=rssi_table[j+1][1];
                rssi_table[j+1][0]=tmp[0];
                rssi_table[j+1][1]=tmp[1];
            }
        }
    }

    for(i=0; i<index_sec_pair; i++)
        printf("rssi_table[%d][0]=[%d], rssi_table[%d][1]=[%d]\n", i, rssi_table[i][0], i, rssi_table[i][1]);

    char mapping_flag=0;
    char send_index=0;

    for(i=0; i<index_sec_pair; i++) {
        mapping_flag=0;
        if(i==0) {
            memcpy(&scan_result_of_sort[(int)send_index], &ssid_scan_result[(int)rssi_table[i][1]], sizeof(WiFi_SCAN_RESULT));
            send_index++;
        } else {
            for(j=0; j<send_index; j++) {
                //current ssid compare before ssid
                if(strcmp(ssid_scan_result[(int)rssi_table[i][1]].ssid,scan_result_of_sort[j].ssid)==0) {
                    mapping_flag=1;
                    break;
                }
            }
            if(mapping_flag==0) {
                memcpy(&scan_result_of_sort[(int)send_index], &ssid_scan_result[(int)rssi_table[i][1]], sizeof(WiFi_SCAN_RESULT));
                send_index++;
            }
        }
    }


    /*for(idx=0; idx<index_sec_pair; idx++) {
        printf("%d %s %d %d %d\n", idx+1, ssid_scan_result[idx].ssid, ssid_scan_result[idx].rssi, ssid_scan_result[idx].security_mode, ssid_scan_result[idx].frequency);
    }*/

    /*for(idx=0; idx<send_index; idx++) {
        printf("%d %s %d %d %d\n", idx, scan_result_of_sort[idx].ssid, scan_result_of_sort[idx].rssi, scan_result_of_sort[idx].security_mode, scan_result_of_sort[idx].frequency);
    }*/

    return send_index;
}

int create_configfile(char *SSID, char *PASSWORD)
{
    char cmd[256]="\0";
    FILE *fptr=NULL;
    int index=0;
    int match_flag=0;
    char scan_times_count=0;

    sprintf(cmd, "rm -rf %s/%s", DIR_PATH, CONFIG_FILENAME);
    system(cmd);

    sprintf(cmd, "%s/%s", DIR_PATH, CONFIG_FILENAME);
    fptr=fopen(cmd, "w+");

    if(fptr == NULL)
        return FILE_OPEN_ERROR;

    fprintf(fptr, "ctrl_interface=%s\n", CTRL_INTERFACE);
    fprintf(fptr, "update_config=1\n");



MATCH_START:
    index=0;
    match_flag=0;
    while(scan_result_of_sort[index].frequency!=0) {
        if(strcmp(scan_result_of_sort[index].ssid, SSID)==0) {
            match_flag=1;
            break;
        }
        index++;
    }


    if(scan_times_count>=MAX_SCAN_TIMES && match_flag==0) {
        printf("The result of site survey does not match input SSID\n");
        return FIND_ROUTER_FAIL;
    } else if(match_flag==1) {
        //printf("scan_result_of_sort[%d].ssid=[%s]\n",index, scan_result_of_sort[index].ssid);
        //printf("scan_result_of_sort[%d].security_mode=[%d]\n",index, scan_result_of_sort[index].security_mode);

	if( (scan_result_of_sort[index].security_mode!=WIFI_SEC_MODE_NONE) && PASSWORD==NULL)
	{
		fclose(fptr);
        	fptr=NULL;
		return INPUT_ERROR;
	}


        fprintf(fptr, "network={\n");
        fprintf(fptr, "ssid=\"%s\"\n", scan_result_of_sort[index].ssid);
        fprintf(fptr, "key_mgmt=WPA-PSK\n");
        if( (scan_result_of_sort[index].security_mode>0) && (scan_result_of_sort[index].security_mode<7)) {
            if( (scan_result_of_sort[index].security_mode%2)==1) {
                fprintf(fptr, "proto=WPA2\n");
            } else {
                fprintf(fptr, "proto=WPA\n");
            }

            if(scan_result_of_sort[index].security_mode>4) {
                fprintf(fptr, "pairwise=CCMP TKIP\n");
                fprintf(fptr, "group=TKIP\n");
            } else if(scan_result_of_sort[index].security_mode>2) {
                fprintf(fptr, "pairwise=TKIP\n");
            } else {
                fprintf(fptr, "pairwise=CCMP\n");
            }

            fprintf(fptr, "psk=\"%s\"\n", PASSWORD);

        } else if(scan_result_of_sort[index].security_mode==7) {
            fprintf(fptr, "key_mgmt=NONE\n");
            fprintf(fptr, "wep_key0=%s\n", PASSWORD);
            fprintf(fptr, "wep_tx_keyidx=0\n");
        } else {
            fprintf(fptr, "key_mgmt=NONE\n");
        }

        fprintf(fptr, "disabled=1\n");
        fprintf(fptr, "}\n");
        fclose(fptr);
        fptr=NULL;
    } else if(match_flag==0) {
        scan_times_count++;
        printf("scan_result_count=[%d]\n", start_site_survey());
        goto MATCH_START;
    }

    return SUCCESSFUL;
}

static void usage(void)
{
    printf("Please input SSID and PASSWORD\n");
}

int main(int argc, char *argv[])
{
    if(argc<2 || argc >3) {
        usage();
        return INPUT_ERROR;
    }


    if(argc ==2)
        printf("ret=[%d]\n", create_configfile(argv[1], NULL));
    else
        printf("ret=[%d]\n", create_configfile(argv[1], argv[2]));

    return SUCCESSFUL;
}


