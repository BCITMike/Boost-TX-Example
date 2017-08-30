#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>

#include <iostream>
#include <iterator>
#include <string>
#include <deque>
#include <stdexcept>
#include <cstdint>
#include <utility>

#include <boost/asio.hpp>
#include <boost/tokenizer.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/copy.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>

#include <iomanip>

enum { max_length = 9028 };

void getMACAddress(std::string _iface,unsigned char MAC[6]) {
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        struct ifreq ifr;
        ifr.ifr_addr.sa_family = AF_INET;
        strncpy(ifr.ifr_name , _iface.c_str() , IFNAMSIZ-1);
        ioctl(fd, SIOCGIFHWADDR, &ifr);
        for(unsigned int i=0;i<6;i++)
            MAC[i] = ifr.ifr_hwaddr.sa_data[i];
        ioctl(fd, SIOCGIFMTU, &ifr);
        close(fd); // printf("MTU: %d\n",ifr.ifr_mtu); // printf("MAC:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",MAC[0],MAC[1],MAC[2],MAC[3],MAC[4],MAC[5]);
    }

const char *if_name = "ens39";

int main()
{
    std::cout << std::endl;

    std::string ifname(if_name);

    unsigned char mac_str[6];
    char mac_src[18];
    getMACAddress(if_name, mac_str);

    snprintf(mac_src, sizeof(mac_src), "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",mac_str[0],mac_str[1],mac_str[2],mac_str[3],mac_str[4],mac_str[5]);
    mac_src[17] = '\0';
    std::cout << "Sending Interface: " << if_name;
    std::cout << "\nSending MAC:   " << mac_src << '\n';

    typedef boost::asio::generic::raw_protocol raw_protocol_t;
    typedef boost::asio::generic::basic_endpoint<raw_protocol_t> raw_endpoint_t;

    sockaddr_ll sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sll_family = PF_PACKET;
    sockaddr.sll_protocol = htons(ETH_P_ALL);
    sockaddr.sll_ifindex = if_nametoindex(ifname.c_str());
    sockaddr.sll_hatype = 1;

    boost::asio::io_service io_service;
    raw_protocol_t::socket socket(io_service, raw_protocol_t(PF_PACKET, SOCK_RAW));
    socket.bind(raw_endpoint_t(&sockaddr, sizeof(sockaddr)));
        
    boost::asio::streambuf buffer;
    std::ostream stream( &buffer );    
    unsigned char const deadbeef[] = { 0xde, 0xad, 0xbe, 0xef };
    stream << "Hello, World!!!"
           << reinterpret_cast< char const * >( deadbeef );   
    socket.send(buffer.data());       

    std::cout << "-------------------------------------------------------------------------------------------\n";
    return 0;
}

