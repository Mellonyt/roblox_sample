/*
    topicupdate.h
    TopicUpdate class definition

    developer   r_mcinnis@solidice.com
    date        june 03, 2020
*/
#pragma once
#ifndef TOPICUPDATE_H
#define TOPICUPDATE_H

#include <string>
#include <stdint.h>

class TopicUpdate {
    public:
        uint32_t id ;
        std::string  topic ;
        std::string  txt ;
        int priority ;
        int nPendingNotifications ;
        
        TopicUpdate( uint32_t id_, const std::string &topic_, const std::string &txt_, int priority_ ) 
        : topic( topic_ ), txt( txt_ ) { id = id_ ; priority = priority_ ; nPendingNotifications = 0 ; }
        TopicUpdate( const TopicUpdate &c ) { *this = c ; }

        bool operator< ( const TopicUpdate &c ) { return (priority == c.priority) ? (topic < c.topic) : (priority < c.priority) ; }
        TopicUpdate &operator= ( const TopicUpdate &c ) {
            if (this != &c) {
                topic = c.topic ;
                txt = c.txt ;
                priority = c.priority ;
                id = c.id ;
                nPendingNotifications = c.nPendingNotifications ;
            }
            return *this ;
        }
} ; // class TopicUpdate

#endif
