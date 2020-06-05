/*
    topic.h
    Topic class definition

    developer   r_mcinnis@solidice.com
    date        june 03, 2020
*/
#pragma once 
#ifndef TOPIC_H
#define TOPIC_H

#include <vector>
#include <string>
#include "subscriber.h"

class Topic {
    public:
    std::string  id ;
    std::vector<Subscriber*>  observers ; // i'd MUCh rather use proper observables, but this is what i've got

    Topic( const std::string &id_ ) : id( id_ ) {}
} ; // class Topic

#endif
