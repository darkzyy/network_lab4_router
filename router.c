#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>

#include "conf.h"
#include "headers.h"




//tables will be used:
route_item		route_tab[MAX_ROUTE_INFO_SIZE];
arp_table_item	arp_tab[MAX_ARP_SIZE];
device_item		device_tab[MAX_DEVICE_SIZE];

const char* my_ip_addr = "192.168.x.x";
const char* my_mac_addr = "x.x.x...";

int sock_fd;//套接字




int main(){
	if((sock_fd=socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL)))<0){
		printf("error create raw socket\n");
		return -1;
	}

	return 0;
}
