/*
*/
#include "rbmessage.h"
#include <algorithm>

RBMessage :: RBMessage() {
    _pkt = nullptr ;
    _buffer = nullptr ;
    _buffer_sz = 0 ;
} // RBMessage [ctor]

RBMessage :: ~RBMessage() {
    deallocate() ;
} // RBMessage [dtor]

void RBMessage :: allocate( uint16_t sz ) {
    if (sz < _buffer_sz) {
        return ;
    }
    delete[] _buffer ;
    _buffer = new char[ (_buffer_sz = std::max( sz, (uint16_t)MAX_PACKETSZ )) ] ;
    _pkt = (RBPacketHeader*)_buffer ;
} // RBMessage :: allocate

void RBMessage :: deallocate() {
    if (_pkt != nullptr) {
        delete[] _buffer ;
        _pkt = nullptr ;
        _buffer = nullptr ;
        _buffer_sz = 0 ;
    }
} // RBMessage :: deallocate

