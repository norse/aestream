#ifndef DVSTOUDP_HPP
#define DVSTOUDP_HPP

# include <string>
#include "aedat.hpp"
#include "generator.hpp"

// socket programming
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

template<typename T>
class DVSToUDP{
    public:
        int sockfd = -1;
        std::string serverport;
        std::string IPAdress;
        struct addrinfo *p = NULL;
        uint32_t container_interval;
        uint32_t buffer_size;

        static const uint16_t UDP_max_bytesize = 512;
        uint32_t message[UDP_max_bytesize / 4]; // 4 bytes (32 bits) per event
        uint64_t events_sent = 0;

        DVSToUDP(uint32_t interval, uint32_t bfsize, std::string port, std::string IP); 

        void sendpacket(Generator<T>& input_generator, bool include_timestamp);
        void closesocket();
};

#endif