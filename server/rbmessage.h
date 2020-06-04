/*
    rbmessage.h
    RBMessage class definition

    developer   r_mcinnis@solidice.com
    date        june 03, 2020
*/
#pragma once
#ifndef RBMESSAGE_H
#define RBMESSAGE_H

#include "rb_packet.h"

class RBMessage {
    protected:
        RBPacketHeader *_pkt ;
        char           *_buffer ;
        uint16_t        _buffer_sz ;
        sockaddr        _addr ;

    public:
                        RBMessage() ;
                       ~RBMessage() ;

        void            allocate( uint16_t sz ) ;
        void            deallocate() ;
        RBPacketHeader *pkt() { return _pkt ; }
        uint16_t        sz() { return _buffer_sz ; }
        sockaddr       &addr() { return _addr ; }
} ; // class RBMessage

#endif
