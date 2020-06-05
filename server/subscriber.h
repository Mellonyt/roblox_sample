/*
*/
#pragma once
#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <vector>
#include <mutex>
#include <algorithm>
#include <iostream>
#include <fstream>
#include "topicupdate.h"

class Subscriber {
    protected:
        std::string  _id ;
        std::vector<TopicUpdate*>  _updates ;
        std::mutex   _mtx ;
        std::string  _outfname ;

    public:
        Subscriber( const std::string id_ ) : _id(id_) {
            _outfname = _id + ".log" ; // this could be configured to write into a specific folder.  as is, it'll drop local to the program
        }

        void  push_update( TopicUpdate *upd ) {
            std::unique_lock<std::mutex> guard(_mtx);
            _updates.push_back( upd ) ;
            upd->nPendingNotifications++ ;
        }
        void release_updates() {
            if (_updates.size() == 0) {
                return ;
            }
            std::unique_lock<std::mutex> guard(_mtx);
            std::sort(_updates.begin(), _updates.end(), [](TopicUpdate* a, TopicUpdate* b){ return (*a < *b) ; } );

            // letting the os file cacheing handle the writes efficiently
            // if it were a problem, i'd buffer the writes here into large bulk writes
            //
            std::ofstream outf ;
            outf.open( _outfname, std::ofstream::out | std::ofstream::app ) ;
            outf << "--\n" ;
            for (auto it = _updates.begin(); it != _updates.end(); it++) {
                // i see no description of the output; assuming all data
                outf << (*it)->priority << " | " << (*it)->topic << " | " << (*it)->txt << "\n" ;
                (*it)->nPendingNotifications-- ;
            }
            outf.flush() ;
            outf.close() ;

            _updates.clear() ;
        }
} ; // class Subscriber 

#endif

