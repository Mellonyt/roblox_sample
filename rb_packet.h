/*
*/
#pragma once
#ifndef RB_PACKET_H
#define RB_PACKET_H

#include <stdint.h>
#include <arpa/inet.h>
#include <cstddef>
#include <string.h>

#define  MAX_PACKETSZ                1400 /* udp allows for much higher packet sizes, but this is 'good enough' for our purposes and MTU limitations */
#define  MSG_MAGIC_NUMBER            0x52424C58  /* "RBLX" */

#ifndef SM_NETWORKORDER
#  define  SM_NETWORKORDER           0x4e4e /*  'NN' */
#endif
#ifndef SM_HOSTORDER
#  define  SM_HOSTORDER              0x4848 /*  'HH' */
#endif

#define  pktRB_PACKET_ID             100 
#define  pktRB_PING                  (pktRB_PACKET_ID +  1)
#define  pktRB_PING_ACK              (pktRB_PACKET_ID +  2)
#define  pktRB_IGNORED               (pktRB_PACKET_ID +  3)
#define  pktRB_SHUTDOWN              (pktRB_PACKET_ID +  4)
#define  pktRB_TOPIC_UPDATE          (pktRB_PACKET_ID +  5)

struct RBPacketHeader {
  uint32_t            som ; // always MSG_MAGIC_NUMBER
  uint16_t            id ;    // msg id
  uint16_t            data_sz ; // sz of data following header
  char                data[1] ;

  static uint16_t     headerSize() { return offsetof(RBPacketHeader,data) ; } 
  static uint16_t     totalSize( int16_t sz ) { return headerSize() + sz ; }
  uint16_t            totalSize() { return totalSize(data_sz) ; }
  void                encodeNetwork() {
      if (MSG_MAGIC_NUMBER != som) 
        return ;
      som     = htonl( som ) ;
      id      = htons( id ) ;
      data_sz = htons( data_sz ) ;
  }

  void             decodeHost() {
      if (MSG_MAGIC_NUMBER == som) 
        return ;
      som     = ntohl( som ) ;
      id      = ntohs( id ) ;
      data_sz = ntohs( data_sz ) ;
  }
  void set( uint16_t id_, uint16_t sz_, const char *d ) {
      som = MSG_MAGIC_NUMBER ;
      id  = id_ ;
      data_sz = sz_ ;
      if (d != nullptr) memcpy( data, d, sz_ ) ;
  }
      
  static RBPacketHeader *create( uint16_t id, const char *d ) { 
    return create( id, d, strlen(d)+1 ) ; 
  }
  static RBPacketHeader *create( uint16_t id, const void *d, uint16_t sz ) {
    RBPacketHeader *p = (RBPacketHeader *) new char[totalSize((d == nullptr) ? 0 : sz)] ;
    p->som     = MSG_MAGIC_NUMBER ;
    p->id      = id ;
    p->data_sz = (d == nullptr) ? 0 : sz ;
    if (d != nullptr)  
      memcpy( p->data, d, sz ) ;
    return( p );
  }
  static int16_t     destroy( RBPacketHeader *&pkt ) { 
    if ((pkt == nullptr) || (pkt->id == (uint32_t)-1L))
      return 0 ;

    pkt->id      = (uint16_t)-1L ;
    pkt->data_sz = 0 ; // to be safe

    delete[] (char*)pkt ; 
    pkt = nullptr ;

    return 0 ; 
  } 
} ; // struct RBPacketHeader

#endif
