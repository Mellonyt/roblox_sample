/*
*/
#pragma once
#ifndef RBUPDATE_H
#define RBUPDATE_H

#include <string>

class RBUpdate {
    public:
    std::string  topic = "n/a";
    std::string  txt = "n/a";
    int  priority = 0 ;

    RBUpdate() {} ;
    void clear() { topic = "" ; txt = "" ; priority = 0 ; }
} ; // class RBUpdate

#endif
