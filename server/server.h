/*
*/
#pragma once
#ifndef SERVER_H
#define SERVER_H

#include <map>
#include <string>
#include <vector>
#include <deque>
#include <thread>
#include <semaphore.h>
#include "topic.h"
#include "topicupdate.h"
#include "subscriber.h"
#include "rbmessage.h"
#include "args.h"
#include "linestats.h"

typedef std::map<std::string,Topic*>                      TopicMap ;
typedef std::map<std::string,Topic*>::value_type          TopicMap_pair ;
typedef std::map<std::string,Topic*>::iterator            TopicMap_iter ;

typedef std::map<uint32_t,TopicUpdate*>              UpdateMap ;
typedef std::map<uint32_t,TopicUpdate*>::value_type  UpdateMap_pair ;
typedef std::map<uint32_t,TopicUpdate*>::iterator    UpdateMap_iter ;

typedef std::map<std::string,Subscriber*>                 SubscriberMap ;
typedef std::map<std::string,Subscriber*>::value_type     SubscriberMap_pair ;
typedef std::map<std::string,Subscriber*>::iterator       SubscriberMap_iter ;

class RBServer {
    protected:
        Args        _args ;
        std::string _err ;
        bool        _done ;
        SOCKET      _sock ;
        struct sockaddr_in _addr ;
        std::vector<RBMessage*> _msgVec ;
        std::deque <RBMessage*> _workQ ;
        sem_t       _work_sem ;
        std::mutex  _work_mtx ;
        std::mutex  _mtx ;
        int16_t     _nMsgBuffers ;
        LineStats   _stats ;
        TopicMap    _topics ;
        std::mutex  _topic_mtx ;
        UpdateMap   _updates ;
        std::mutex  _update_mtx ;
        SubscriberMap _subscriberMap ;
        std::mutex  _subscriber_mtx ;
        uint32_t    _updateId = 0 ;
        std::vector<std::thread> _threadPool ;
        std::thread _outputThread ;

        int16_t     close() ;
        int16_t     open() ;
        int16_t     parse_config() ;
        void        registerId( const std::string &id, const std::string &topic ) ;
        void        update( const std::string &topic, const std::string &txt, int priority ) ;
        void        work_t() ;
        void        write_t() ;

    public:
                    RBServer( Args &args ) ;
                   ~RBServer() ;

        RBMessage  *get_buffer() ;    

        int16_t     initialize() ;
        bool        is_done() { return _done ; }
        int16_t     process( RBMessage *&msg, int16_t amt ) ;
        void        report_stats() ;
        int16_t     start() ;
        int16_t     stop() ;
        void        tick() ;

        // access methods
        std::string err() { return _err ; }
        SOCKET      sock() { return _sock ; }
} ; // class RBServer

#endif
