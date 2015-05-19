#include<stdio.h>
#include<string.h>
#include<arpa/inet.h>
#include"conf.h"

route_item      route_tab[MAX_ROUTE_INFO_SIZE];

int main(){
	//设置表项
	int i;
	for(i=0;i<MAX_ROUTE_INFO_SIZE;i++){
		route_tab[i].valid = 0;
	}
	route_tab[0].destination	=	inet_addr("192.168.1.0");
	route_tab[0].gateway		=	inet_addr("192.168.1.1");
	route_tab[0].netmask		=	inet_addr("255.255.255.0");
	route_tab[0].valid			=	1;
	strcpy(route_tab[0].interface,"eth0");

	route_tab[1].destination	=	inet_addr("192.168.2.0");
	route_tab[1].gateway		=	inet_addr("192.168.2.1");
	route_tab[1].netmask		=	inet_addr("255.255.255.0");
	route_tab[1].valid			=	1;
	strcpy(route_tab[0].interface,"eth0");

	FILE* pfile;
	pfile = fopen("route_table.binary","wb");
	if(pfile == NULL){
		printf("error opening a file!\n");
		return -1;
	}

	fwrite(route_tab,sizeof(char),sizeof(route_tab),pfile);
	fclose(pfile);

	pfile = fopen("route_table.binary","rb");
	if(pfile == NULL){
		printf("error opening a file!\n");
		return -1;
	}
	int suc;
	suc = fread(route_tab,1,sizeof(route_tab),pfile);
	if(suc!=sizeof(route_tab)){
		printf("error when reading n");
	}
	fclose(pfile);
	
	

	return 0;
}
