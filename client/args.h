/*
    args.h
    Args class definition

    developer   r_mcinnis@solidice.com
    date        june 02, 2020
*/
#pragma once
#ifndef ARGS_H
#define ARGS_H

#include <stdint.h>
#include <string>

class Args {
public:
    uint16_t        nDelay = 50 ; // default: 50ms delay between sends 
    uint16_t        nRepeats = 1 ; // default: no repeats
    bool            continous = false ;
    std::string     infilename = "" ;
    uint16_t        port = 15005 ;
    std::string     ipaddr = "127.0.0.1" ;

    Args() ;

    int16_t         parse( int argc, const char *argv[], std::string &msg ) ;
} ; // class Args

#endif

