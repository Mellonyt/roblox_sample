/*
    utils.cpp
    basic utility function implementations

    developer   r_mcinnis@solidice.com
    date        june 02, 2020
*/
#include "utils.h"
#include <arpa/inet.h>
#include <sys/stat.h>
#include <chrono>
#include <string.h>
#include <sys/time.h>

int64_t  __start_tm = 0 ;
int64_t  __latest_tm = 0 ;
int32_t  __now = 0 ;

int32_t timeGetTime() {
    __latest_tm = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1) ;
    if (__start_tm == 0) __start_tm = __latest_tm ;
    return (__now = (__latest_tm - __start_tm)) ;
} // :: timeGetTime

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

int16_t udp_recvfrom( SOCKET sock, char *recv_buf, int buf_sz, struct sockaddr &src_addr, long timeout ) 
{
  fd_set      readfds ;
  socklen_t   n = 0 ;
  TIMEVAL     timeval ;
  timeval.tv_sec  = (timeout / 1000) ;
  timeval.tv_usec = (timeout % 1000) * 1000 ;
  memset( &readfds  , 0x00, sizeof(fd_set) ) ;
  FD_SET( sock, &readfds);

  n = ::select( sock+1, &readfds, nullptr, nullptr, &timeval ) ;
  if (n <= 0)
    return n ; // timeout == 0; error == -1

  n = sizeof(src_addr) ;
  return recvfrom( sock, (char*)recv_buf, buf_sz, 0, &src_addr, &n ) ;
} // :: udp_recvfrom

