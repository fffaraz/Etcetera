#ifndef DNSPACKET_H
#define DNSPACKET_H

#include <cstdlib>
#include <cstdint>

struct dns_flags
{
    // fields in first byte
    uint8_t rd :1;     // recursion desired
    uint8_t tc :1;     // truncated message
    uint8_t aa :1;     // authoritive answer
    uint8_t opcode :4; // purpose of message
    uint8_t qr :1;     // query/response flag

    // fields in second byte
    uint8_t rcode :4;  // response code
    uint8_t cd :1;     // checking disabled
    uint8_t ad :1;     // authenticated data
    uint8_t z :1;      // its z! reserved
    uint8_t ra :1;     // recursion available
};

//DNS header structure
struct dns_header
{
     uint16_t   query_id; // identification number
     dns_flags  flags;
     uint16_t   qdcount; // number of question entries
     uint16_t   ancount; // number of answer entries
     uint16_t   nscount; // number of authority entries
     uint16_t   arcount; // number of resource entries
};

//Constant sized fields of query structure
struct dns_question
{
    uint16_t    qtype;
    uint16_t    qclass;
};

//Constant sized fields of the resource record structure
struct dns_record
{
    uint16_t    type;
    uint16_t    rclass;
    uint32_t    ttl;
    uint16_t    data_len;
};

struct dns_packet
{
    dns_header header;

};

class DNSPacket
{
public:
    DNSPacket();
};

#endif // DNSPACKET_H
