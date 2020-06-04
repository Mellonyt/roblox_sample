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

bool to_numeric( const char *str, uint16_t &n, uint16_t max_n ) ;
bool valid_ipaddr( const char *ipaddr ) ;
bool file_exists (const std::string& name) ;

#endif
