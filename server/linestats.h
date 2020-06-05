/*
*/
#pragma once
#ifndef LINESTATS_H
#define LINESTATS_H

#include <stdint.h>
#include "utils.h"

class LineStats {
    public:
        uint32_t    nPackets = 0 ;
        uint64_t    nBytesRecv = 0 ;
        uint64_t    nBytesSent = 0 ;
        uint32_t    time_0 = 0 ;
        uint32_t    nPackets_0 = 0 ;
        uint64_t    nBytesRecv_0 = 0 ;
        uint64_t    nBytesSent_0 = 0 ;
        float       packetsPerSec = 0 ;
        float       bytesRecvPerSec = 0 ;
        float       bytesSentPerSec = 0 ;

        LineStats() { reset() ; }

        void  reset() {
            nPackets = 0 ;
            nBytesRecv = 0 ;
            nBytesSent = 0 ;
            time_0 = timeGetTime() ;
            nPackets_0 = 0 ;
            nBytesRecv_0 = 0 ;
            nBytesSent_0 = 0 ;
            packetsPerSec = 0 ;
            bytesRecvPerSec = 0 ;
            bytesSentPerSec = 0 ;
        }
        void  tick() {
            uint32_t  dt = __now - time_0 ;
            if (dt == 0)  return ;

            packetsPerSec = (float)(nPackets - nPackets_0) * 1000.0 / (float)dt ;
            bytesRecvPerSec = (float)(nBytesRecv - nBytesRecv_0) * 1000.0 / (float)dt ;
            bytesSentPerSec = (float)(nBytesSent - nBytesSent_0) * 1000.0 / (float)dt ;
            time_0 = __now ;
            nPackets_0 = nPackets ;
            nBytesRecv_0 = nBytesRecv ;
            nBytesSent_0 = nBytesSent ;            
        }
} ; // class LineStats

#endif
