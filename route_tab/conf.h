#ifndef __CONF_H__
#define __CONF_H__

#define MAX_ROUTE_INFO_SIZE 50
#define MAX_ARP_SIZE 100
#define MAX_DEVICE_SIZE 10
/*
struct route_item{ 
	//the information of the static routing table  
	char destination[16]; 
	char gateway[16]; 
	char netmask[16]; 
	char interface[16]; 
};*/  
struct route_item{ 
	//the information of the static routing table  
	int valid;
	unsigned int destination; 
	unsigned int gateway; 
	unsigned int netmask; 
	char interface[16]; 
};

struct arp_table_item{  
	//the informaiton of the " my arp cache"
	int valid;
	unsigned int ip_addr; 
	unsigned char mac_addr[6]; 
};

struct device_item{  
	//interface and mac
	int valid;
	char interface[14]; 
	unsigned char mac_addr[6]; 
	unsigned int ip_addr;
};

typedef struct route_item route_item;
typedef struct arp_table_item arp_table_item;
typedef struct device_item device_item;

#endif
