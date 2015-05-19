#ifndef __HEADERS_H__
#define __HEADERS_H__

struct ip_header {
    unsigned char      iph_ihl:4, iph_ver:4;
    unsigned char      iph_tos;
    unsigned short int iph_len;
    unsigned short int iph_ident;
    unsigned short int iph_flag_offset;
    unsigned char      iph_ttl;
    unsigned char      iph_protocol;
    unsigned short int iph_chksum;
    unsigned int       iph_sourceip;
    unsigned int       iph_destip;
};

struct arp_header{
    unsigned short arp_hd;
    unsigned short arp_pr;
    unsigned char arp_hdl;
    unsigned char arp_prl;
    unsigned short arp_op;
    unsigned char arp_sha[6];
    unsigned char arp_spa[4];
    unsigned char arp_dha[6];
    unsigned char arp_dpa[4];
};

typedef struct ip_header ip_header;
typedef struct arp_header arp_header;


unsigned short chksum(unsigned short*buff,int len){
    unsigned long sum=0;
    while(len>1){
        sum += *buff++;
        len -= 2;
    }
    if(len){
        sum +=*(unsigned char*)buff;
    }
    sum = (sum>>16)+(sum & 0xffff);
    sum += sum>>16;
    return (unsigned short)(~sum);
}


#endif
