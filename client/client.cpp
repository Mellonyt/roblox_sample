/*
    client.cpp
    RBClient class implementation

    developer   r_mcinnis@solidice.com
    date        june 02, 2020
*/
#include "client.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include "rb_packet.h"

using namespace std; 

int16_t RBClient :: close() {
    if (_sock != INVALID_SOCKET) {
        ::close( _sock ) ;
        _sock = INVALID_SOCKET ;
    }
    return 0 ;
} // RBClient :: close

int16_t RBClient :: open() {
    if (_sock == INVALID_SOCKET) {
        _sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (_sock == INVALID_SOCKET) {
            return -1 ;
        }
    }

    memset((char*)&_addr, 0, sizeof(_addr));
    _addr.sin_family = AF_INET;
    _addr.sin_port = htons( _args.port );
    inet_pton( AF_INET, _args.ipaddr.c_str(), &_addr.sin_addr) ;

    return 0 ;
} // RBClient :: open

void RBClient :: work_proc() {

} // RBClient :: work_proc

RBClient :: RBClient( Args &args ) 
: _args( args ) {
    _done = true ;
    _sock = INVALID_SOCKET ;
    _buffer = nullptr ;
    _buffer_sz = 0 ;
} // RBClient [ctor]

RBClient :: ~RBClient() {
    stop() ;
} // RBClient [dtor]

int16_t RBClient :: send( uint16_t id, uint16_t sz, const char *data ) {
  if (sz > MAX_PACKETSZ) {
      _err = "error( packet size too large )";
      return -2 ;
  }
  if (open() != 0) {
      return 0 ;
  }

  uint16_t  send_sz = RBPacketHeader::totalSize(sz) ;
  if (send_sz > _buffer_sz) {
      delete[] _buffer ;
      _buffer = new char[ (_buffer_sz = send_sz+1) ] ;
  }
  RBPacketHeader *pkt = (RBPacketHeader*)_buffer ;
  pkt->set( id, sz, data ) ;
  pkt->encodeNetwork() ;

  ssize_t rc = sendto( _sock, pkt, send_sz, 0, (struct sockaddr*)&_addr, sizeof(_addr) );
  if (rc == -1) {
      std::stringstream ss ;
      ss << "error( " << errno << ")";
      _err = ss.str() ;
  }
  return rc ;
} // RBClient :: send

int16_t RBClient :: send( uint16_t id, const std::string &s ) {
  return send( id, s.size()+1, s.c_str() ) ;
} // RBClient :: send

int16_t RBClient :: send( const RBUpdate &data ) {
  if (open() != 0) {
      return 0 ;
  }

  std::cout << "sending: [" << data.topic << "] [" << data.txt << "] p: " << data.priority << endl ;

  std::stringstream  ss ;
  char delim = 0x01 ;
  ss << data.topic << delim << data.txt << delim << data.priority ;

  return send( pktRB_TOPIC_UPDATE, ss.str() ) ;
} // RBClient :: send

bool RBClient :: start() {
    _done = false ;
    return true ;
} // RBClient :: start

bool RBClient :: stop() {
    close() ;
    return true ;
} // RBClient :: stop

