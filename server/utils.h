/*
    utils.h
    basic utility functions 

    developer   r_mcinnis@solidice.com
    date        june 02, 2020
*/
#pragma once
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <stdint.h>

#define SOCKET            int
#define INVALID_SOCKET   -1 
#define TIMEVAL           timeval

extern int64_t  __start_tm ;
extern int64_t  __latest_tm ;
extern int32_t  __now ;

int32_t timeGetTime() ;
bool to_numeric( const char *str, uint16_t &n, uint16_t max_n ) ;
bool valid_ipaddr( const char *ipaddr ) ;
bool file_exists (const std::string& name) ;
int16_t udp_recvfrom( SOCKET sock, char *recv_buf, int buf_sz, struct sockaddr &src_addr, long timeout = 100 ) ;

#endif
