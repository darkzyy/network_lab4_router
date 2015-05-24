#include<stdio.h>
#include<string.h>
#include<arpa/inet.h>
#include"conf.h"

device_item     device_tab[MAX_DEVICE_SIZE];

int main(){
    device_tab[0].valid         = 1;
    strcpy(device_tab[0].interface,"eth1");
    device_tab[0].mac_addr[0]   = 0x00;
    device_tab[0].mac_addr[1]   = 0x0c;
    device_tab[0].mac_addr[2]   = 0x29;
    device_tab[0].mac_addr[3]   = 0x8d;
    device_tab[0].mac_addr[4]   = 0x82;
    device_tab[0].mac_addr[5]   = 0x8b;
    device_tab[0].ip_addr       = inet_addr("192.168.1.1");
    device_tab[1].valid         = 1;
    strcpy(device_tab[1].interface,"eth2");
    device_tab[1].mac_addr[0]   = 0x00;
    device_tab[1].mac_addr[1]   = 0x0c;
    device_tab[1].mac_addr[2]   = 0x29;
    device_tab[1].mac_addr[3]   = 0x8d;
    device_tab[1].mac_addr[4]   = 0x82;
    device_tab[1].mac_addr[5]   = 0x95;
    device_tab[1].ip_addr       = inet_addr("192.168.2.1");

    int i;
    for(i=2;i<MAX_DEVICE_SIZE;++i){
        device_tab[i].valid = 0;
    }
    FILE* pfile;
    pfile = fopen("dev_tab.binary","wb");
    if(pfile == NULL){
        printf("error opening a file!\n");
        return -1;
    }

    fwrite(device_tab,sizeof(char),sizeof(device_tab),pfile);
    fclose(pfile);

	return 0;
}
