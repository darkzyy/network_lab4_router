#include<stdio.h>
#include<string.h>
#include<arpa/inet.h>
#include"conf.h"

arp_table_item arp_tab[MAX_ARP_SIZE];

int main(){
	//设置表项
	int i;
	for(i=0;i<MAX_ARP_SIZE;i++){
		arp_tab[i].valid = 0;
	}
	arp_tab[0].valid		=	1;//PC0_
	arp_tab[0].ip_addr		=	inet_addr("192.168.1.2");
	arp_tab[0].mac_addr[0]	=	0x00;
	arp_tab[0].mac_addr[1]	=	0x0c;
	arp_tab[0].mac_addr[2]	=	0x29;
	arp_tab[0].mac_addr[3]	=	0x87;
	arp_tab[0].mac_addr[4]	=	0xa4;
	arp_tab[0].mac_addr[5]	=	0xce;
	arp_tab[1].valid		=	1;//PC0_
	arp_tab[1].ip_addr		=	inet_addr("192.168.2.2");
	arp_tab[1].mac_addr[0]	=	0x00;
	arp_tab[1].mac_addr[1]	=	0x0c;
	arp_tab[1].mac_addr[2]	=	0x29;
	arp_tab[1].mac_addr[3]	=	0x30;
	arp_tab[1].mac_addr[4]	=	0xb4;
	arp_tab[1].mac_addr[5]	=	0x8c;

	FILE* pfile;
	pfile = fopen("arp_table.binary","wb");
	if(pfile == NULL){
		printf("error opening a file!\n");
		return -1;
	}

	fwrite(arp_tab,sizeof(char),sizeof(arp_tab),pfile);
	fclose(pfile);

	/*****************test bellow*****************/
	pfile = fopen("arp_table.binary","rb");
	if(pfile == NULL){
		printf("error opening a file!\n");
		return -1;
	}
	int suc;
	suc = fread(arp_tab,1,sizeof(arp_tab),pfile);
	if(suc!=sizeof(arp_tab)){
		printf("error when reading n");
	}
	fclose(pfile);
	
	

	return 0;
}
