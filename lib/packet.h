

/*
 * copyright (C) <2015>  <Slank Hiroki Shirokura>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */



#ifndef PACKET_H
#define PACKET_H



#include "pgen.h"
#include "address.h"
#include "pgen-types.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>
#include <iostream>


#define PGEN_MAX_PACKET_LEN 10000
#define PGEN_MAX_EXT_DATA_LEN 98000


class pgen_packet{
	public:
		int 	len;
		int 	ext_data_len;
		u_char 	data[PGEN_MAX_PACKET_LEN];
		u_char  ext_data[PGEN_MAX_EXT_DATA_LEN];
		
		pgen_packet();
		virtual void clear()=0;
		virtual void compile()=0;
		virtual void cast(const void*, const int)=0;
		virtual void send(const char* ifname)=0;
		virtual void summary()=0;
		virtual void info()=0;	
		virtual void help()=0;

		void hex();
		void addData(const void* , int );
		void compile_addData(); 
		void send_handle(pgen_t*);
		
		int  length();
		u_char* byte();

		int  check(const char* pcapfile);
};





class pgen_eth : public pgen_packet {
	protected:
		struct ethernet_header eth; 
	public:
		static const int minLen = sizeof(struct ethernet_header);
		static const int maxLen = PGEN_MAX_PACKET_LEN;
		struct{
			macaddr dst;
			macaddr src;
			bit16 type;
		}ETH;
		
		pgen_eth();
		pgen_eth(const void*, int);
		void clear();
		void compile();
		void cast(const void*, int);
		void send(const char* ifname){send_L2(ifname);}
		void send_L2(const char* ifname);
		void summary();
		void info();
		void help();
};




class pgen_arp : public pgen_eth {
	protected:
		struct arp_packet arp;
	public:
		static const int minLen = pgen_eth::minLen+sizeof(struct arp_packet);
		static const int maxLen = PGEN_MAX_PACKET_LEN;
		struct{
			int operation;
			macaddr	srcEth;
			macaddr	dstEth;
			ipaddr	srcIp;
			ipaddr	dstIp;
		}ARP;

		pgen_arp();
		pgen_arp(const void*, int);
		void clear();
		void compile();
		void cast(const void*, const int);
		void send(const char* ifname){send_L2(ifname);}
		void summary();
		void info();
		void help();
};



class pgen_ip : public pgen_eth {
	protected:
		struct ip_header		ip;
	public:
		static const int minLen = pgen_eth::minLen+sizeof(struct ip_header);
		static const int maxLen = PGEN_MAX_PACKET_LEN;
		struct{
			bit8  tos;
			bit16 tot_len;
			bit16 id;
			bit16 frag_off;
			bit8  ttl; 
			bit8  protocol;
			ipaddr src;
			ipaddr dst;
		}IP;


		pgen_ip();
		pgen_ip(const void*, int);
		void clear();
		void compile();
		void cast(const void*, int);
		void send(const char* ifname){send_L3(ifname);}
		void send_L3(const char* ifname);
		void summary();
		void info();
		void help();
};




class pgen_icmp : public pgen_ip {
	protected:
		struct icmp_header icmp;
		bit8  icmp_data[256];
		bit32 icmp_data_len;
		bit8  icmp_ext_data[256];
		bit32 icmp_ext_data_len;
	public:
		static const int minLen = pgen_ip::minLen+ICMP_HDR_LEN;
		static const int maxLen = PGEN_MAX_PACKET_LEN;
		struct{	
			int type;
			int code;
			
			struct{
				int id;
				int seq;
			}echo;
			struct{
				ipaddr gw_addr;	
			}redirect;
			struct{
				bit8 len;
				bit16 next_mtu;
			}destination_unreach;
			struct{
				bit8 len;
			}time_exceeded;


		}ICMP;
		
		pgen_icmp();
		pgen_icmp(const void*, int);
		void clear();
		void compile();
		void cast(const void*, int);
		void send(const char* ifname){send_L3(ifname);}
		void summary();
		void info();
		void help();

		void icmp_addData(const void*, int);
};






class pgen_tcp : public pgen_ip {
	protected:
		struct tcp_header tcp;
	public:
		static const int minLen = pgen_ip::minLen+sizeof(struct tcp_header);
		static const int maxLen = PGEN_MAX_PACKET_LEN;
		struct{
			int src;
			int dst;
			struct{
				u_char fin:1;
				u_char syn:1;
				u_char rst:1;
				u_char psh:1;
				u_char ack:1;
				u_char urg:1;
			}flags;
			int window;
			int seq;
			int ack;
		}TCP;

		pgen_tcp();
		pgen_tcp(const void*, int);
		void clear();
		void compile();
		void cast(const void*, const int len);
		void send(const char* ifname){send_L3(ifname);}
		void summary();
		void info();
		void help();
};




class pgen_udp : public pgen_ip {
	protected:
		struct udp_header udp;
	public:
		static const int minLen = pgen_ip::minLen+sizeof(struct udp_header);
		static const int maxLen = PGEN_MAX_PACKET_LEN;
		struct{
			int src;
			int dst;
			bit16 len;
		}UDP;

		pgen_udp();
		pgen_udp(const void*, int);
		void clear();
		void compile();
		void cast(const void*, const int len);
		void send(const char* ifname){send_L3(ifname);}
		void summary();
		void info();
		void help();
};




#define MAX_QUERY  16
#define MAX_ANSWER 16
#define MAX_AUTH   16
#define MAX_ADD    16
class pgen_dns :public pgen_udp {
	protected:
		struct dns_header dns;
		bit8  query_data[256];
		bit32 query_data_len;
		bit8  answer_data[256];
		bit32 answer_data_len;
		bit8  auth_data[256];
		bit32 auth_data_len;
		bit8  addition_data[256];
		bit32 addition_data_len;
	public:
		static const int minLen = pgen_udp::minLen+DNS_HDR_LEN;
		static const int maxLen = PGEN_MAX_PACKET_LEN; 
		struct{
			u_int16_t id;
			struct{
				bit8 qr:1;
				bit8 opcode:4;
				bit8 aa:1;
				bit8 tc:1;
				bit8 rd:1;
				bit8 ra:1;
				bit8 nouse:3;
				bit8 rcode:4;
			}flags;
			u_int16_t qdcnt;
			u_int16_t ancnt;
			u_int16_t nscnt;
			u_int16_t arcnt;
			struct{
				std::string name;
				u_int16_t type;
				u_int16_t cls;
			}query[MAX_QUERY];
			struct{
				bit16  name;
				bit16  type;
				bit16  cls;
				bit32  ttl;
				bit16  len;
				
				//ipaddr addr;
				//std::string url;
				bit8 data[32];
			}answer[MAX_ANSWER];
			struct{
				bit16 name;
				bit16 type;
				bit16 cls;
				bit16 ttl;
				bit16 len;
				
				//ipaddr addr;
				//std::string url;
				bit8  data[32];
			}auth[MAX_AUTH];
			struct{
				bit16 name;
				bit16 type;
				bit16 cls;
				bit16 ttl;
				bit16 len;
				
				//ipaddr addr;
				//std::string url;
				bit8  data[32];
			}addition[MAX_ADD];
		}DNS;

		pgen_dns();
		pgen_dns(const void*, int);
		void clear();
		void compile();
		void cast(const void*, int);
		void send(const char* ifname){send_L3(ifname);}
		void summary();
		void info();
		void debug();
		void help();
		
		void clear_query();
		void clear_answer();
		void clear_auth();
		void clear_addition();
		void compile_query();
		void compile_answer();
		void compile_auth();
		void compile_addition();
		int  cast_query(const char*, int);
		int  cast_answer(const char*, int);
		int  cast_auth(const char* , int);
		int  cast_addition(const char*, int);

};



typedef enum{
	ARDRONE_CMD_PCMD,		// 0
	ARDRONE_CMD_REF,		// 1
	ARDRONE_CMD_CONFIG_IDS, // 2
	ARDRONE_CMD_ANIM, 		// 3
	ARDRONE_CMD_FTRIM,		// 4
	ARDRONE_CMD_CONFIG, 	// 5
	ARDRONE_CMD_LED,		// 6
	ARDRONE_CMD_COMWDG,		// 7
	ARDRONE_CMD_CTRL		// 8
} ar_drone_cmdtype;

class pgen_ardrone : public pgen_udp {
	protected:
		char  pcmd_data[256];
		bit32 pcmd_data_len;
		char  ref_data[256];
		bit32 ref_data_len;
		char  configids_data[256];
		bit32 configids_data_len;
		char  anim_data[256];
		bit32 anim_data_len;
		char  ftrim_data[256];
		bit32 ftrim_data_len;
		char  config_data[256];
		bit32 config_data_len;
		char  led_data[256];
		bit32 led_data_len;
		char  comwdg_data[256];
		bit32 comwdg_data_len;
		char  ctrl_data[256];
		bit32 ctrl_data_len;
	public:
		static const int minLength = pgen_udp::minLen+39; // minimum ardrone packet
		static const int macLength = PGEN_MAX_PACKET_LEN;
		struct{
			struct{
				long seq;
				long flag;
				long roll;
				long pitch;
				long gaz;
				struct{
					long x;
					long y;
					long z;
				}yaw;
			}pcmd;
			struct{
				long seq;
				long command;
			}ref;
			struct{
				long seq;
				char session[256];
				char user[256];
				char app[256];
			}configids;
			struct{}anim;
			struct{}ftrim;
			struct{
				long seq;
				char name[256];
				char parameter[256];
			}config;
			struct{}led;
			struct{}comwdg;
			struct{
				long seq;
				long ctrlmode;
				long fw_update_filesize;
			}ctrl;
			
			int cmd_count;
			int cmd_type[10];
		}ARDRONE;

		pgen_ardrone();
		pgen_ardrone(const void*, int);
		void clear();
		void compile();
		void cast(const void*, const int);
		void send(const char* ifname){send_L3(ifname);}
		void summary();
		void info();
		void help();
	
		int compile_pcmd();
		int compile_ref();
		int compile_configids();
		int compile_anim();
		int compile_ftrim();
		int compile_config();
		int compile_led();
		int compile_comwdg();
		int compile_ctrl();
		int cast_pcmd(const char*);
		int cast_ref(const  char*);
		int cast_configids(const char*);
		int cast_anim(const char*);
		int cast_ftrim(const char*);
		int cast_config(const char*);
		int cast_led(const char*);
		int cast_comwdg(const char*);
		int cast_ctrl(const char*);
		void clear_pcmd();
		void clear_ref();
		void clear_configids();
		void clear_anim();
		void clear_ftrim();
		void clear_config();
		void clear_led();
		void clear_comwdg();
		void clear_ctrl();

		void DSUMMARY();
};


#define DHCP_MAX_OPT 64
class pgen_dhcp : public pgen_udp {
	protected:
		struct dhcp_header dhcp;
		bit8  dhcp_option[256];
		bit32 dhcp_option_len;
	public:
		static const int minLen = pgen_udp::minLen+DNS_HDR_LEN;
		static const int maxLen = PGEN_MAX_PACKET_LEN; 
		struct{
			bit8   op;
			bit8   htype;
			bit8   hlen;
			bit8    hops;
			bit32   xid;
			bit16   secs;
			bit16   flags;
			ipaddr  ciaddr;
			ipaddr  yiaddr;
			ipaddr  siaddr;
			ipaddr  giaddr;
			macaddr chaddr;
			bit8    sname[64];
			bit8    file[128];
			
			bit32   option_len;
			struct dhcp_option option[DHCP_MAX_OPT];
		}DHCP;
		
		pgen_dhcp();
		pgen_dhcp(const void*, int);
		void clear();
		void compile();
		void cast(const void*, int);
		void send(const char* ifname){send_L3(ifname);}
		void summary();
		void info();
		void help();

		void dhcp_set_option(int,int,int,void*);
		void dhcp_get_option(const void*, struct dhcp_option*);
};


class pgen_http : public pgen_tcp {
	protected:
		
	public:
		static const int minLen = pgen_tcp::minLen;
		static const int macLen = PGEN_MAX_PACKET_LEN;
	struct{
		int a;
		struct{
			int method;
			char header[2][256]; 
		}request;
	}HTTP;

	pgen_http();
	pgen_http(const void*, int);
	void clear();
	void compile();
	void cast(const void*, int);
	void send(const char* ifname){send_L3(ifname);}
	void summary();
	void info();
	void help();
	
};





class pgen_unknown{
	private:
		bit8 data[PGEN_MAX_PACKET_LEN];
	public:
		int len;
		bool isETH;
		bool isARP;
		bool isIP;
		bool isICMP;
		bool isTCP;
		bool isUDP;
		struct{
			macaddr src;
			macaddr dst;
		}ETH;
		struct{
			ipaddr src;
			ipaddr dst;
		}IP;
		struct{
			bit16 src;
			bit16 dst;
		}TCP;
		struct{
			bit16 src;
			bit16 dst;
		}UDP;

		pgen_unknown();
		pgen_unknown(const void*, int);
		void clear();
		void send_handle(pgen_t*);
		int  cast(const void*, int);
		void summary();

		void hex();
		bool ipaddris(ipaddr addr);
		bool macaddris(macaddr addr);
		bool portis(unsigned short port);
};





#endif /* PACKET_H */
