/*
    utils.cpp
    basic utility function implementations

    developer   r_mcinnis@solidice.com
    date        june 02, 2020
*/
#include "utils.h"
#include <arpa/inet.h>
#include <sys/stat.h>

bool to_numeric( const char *str, uint16_t &n, uint16_t max_n ) {
    if (str == nullptr) {
        return false ;
    }
    uint32_t tmp = 0 ;
    n = 0 ;
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) {
            return false ;
        }
        tmp = tmp*10 + (str[i] - '0') ;
    }
    if (tmp > max_n) {
        return false ;
    }
    n = tmp ;
    return true ;
} // :: to_numeric

bool valid_ipaddr( const char *ipaddr ) {
    unsigned char buf[ sizeof(struct in6_addr) ];
    return (inet_pton( AF_INET, ipaddr, buf ) == 1) ;
} // :: valid_ipaddr

bool file_exists (const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
} // file_exists
