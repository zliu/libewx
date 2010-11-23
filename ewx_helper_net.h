#ifndef __EWX_HELPER_NET_H__
#define __EWX_HELPER_NET_H__


typedef struct ewx_l2_header_s {
    uint8_t     dst[6];
    uint8_t     src[6];
    uint16_t    type;
    uint8_t     payload[0];
} ewx_l2_hdr_t;

#define EWX_L2_TYPE_IP      0x0800
#define EWX_L2_TYPE_ARP     0x0806
#define EWX_L2_TYPE_RARP    0x8035
#define EWX_L2_TYPE_VLAN    0x8100
#define EWX_L2_TYPE_IPv6    0x86DD

typedef struct ewx_vlan_s {
    uint16_t    priority    : 3;
    uint16_t    cfi         : 1;
    uint16_t    vid         : 12;
    uint16_t    type;
    uint8_t     payload[0];
} ewx_vlan_hdr_t;

typedef struct ewx_arp_header_s {
    uint32_t    hw_type     : 16;
    uint32_t    proto_type  : 16;
    uint32_t    hw_size     : 8;
    uint32_t    proto_size  : 8;
    uint32_t    opcode      : 16;
    uint8_t     sender_mac[6];
    uint8_t     sender_ip[4];
    uint8_t     target_mac[6];
    uint8_t     target_ip[4];
    uint8_t     payload[0];
} ewx_arp_hdr_t;

typedef struct ewx_ip_header_s {
    uint32_t    version     : 4;
    uint32_t    IHL         : 4;
    uint32_t    TOS         : 8;
    uint32_t    len         : 16;
    uint32_t    id          : 16;
    uint32_t    res         : 1;
    uint32_t    dont_frag   : 1;
    uint32_t    more_frag   : 1;
    uint32_t    frag_offset : 13;
    uint32_t    TTL         : 8;
    uint32_t    protocol    : 8;
    uint32_t    hdr_chksm   : 16;
    uint32_t    src;
    uint32_t    dst;
    uint8_t     payload[0];
} ewx_ip_hdr_t;

typedef struct ewx_udp_header_s {
    uint16_t    src;
    uint16_t    dst;
    uint16_t    len;
    uint16_t    chksm;
    uint8_t     payload[0];
} ewx_udp_hdr_t;

typedef struct ewx_tcp_header_s {
    uint16_t    src;
    uint16_t    dst;
    uint32_t    seq_num;
    uint32_t    ack_num;
    uint8_t     offset      : 4;
    uint8_t     res         : 4;
    uint8_t     TCP_flags;
    uint16_t    win_size;
    uint16_t    chksm;
    uint16_t    urg_ptr;
    uint8_t     payload[0];
} ewx_tcp_hdr_t;

#endif
