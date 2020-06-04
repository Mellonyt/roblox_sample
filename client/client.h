/*
    client.h
    RBClient class definition

    developer   r_mcinnis@solidice.com
    date        june 02, 2020
*/
#pragma once
#ifndef CLIENT_H
#define CLIENT_H

#include "args.h"
#include "rbupdate.h"
#include <netinet/in.h>

#define SOCKET            int
#define INVALID_SOCKET   -1 

class RBClient {
    protected :
        Args        &_args ;
        SOCKET      _sock ;
        bool        _done ;
        std::string _err ;
        struct sockaddr_in _addr ;
        char       *_buffer ;
        uint16_t    _buffer_sz ;

        int16_t     close() ;
        int16_t     open() ;
        void        work_proc() ;

    public :
        RBClient( Args &args ) ;
        ~RBClient() ;

        std::string &err() { return _err ; }
        bool        is_done() { return _done ; }
        int16_t     send( uint16_t id, uint16_t sz, const char *data = nullptr ) ;
        int16_t     send( uint16_t id, const std::string &s ) ;
        int16_t     send( const RBUpdate &data ) ;
        bool        start() ;
        bool        stop() ;
} ; // class RBClient


#endif