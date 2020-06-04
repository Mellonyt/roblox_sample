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
    std::string     infilename = "" ;
    uint16_t        port = 15005 ;
    uint16_t        verbose = 0 ;

    Args() ;

    int16_t         parse( int argc, const char *argv[], std::string &msg ) ;
} ; // class Args

#endif

