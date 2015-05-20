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

	FILE* pfile;
	pfile = fopen("arp_table.binary","wb");
	if(pfile == NULL){
		printf("error opening a file!\n");
		return -1;
	}

	fwrite(arp_tab,sizeof(char),sizeof(arp_tab),pfile);
	fclose(pfile);

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
