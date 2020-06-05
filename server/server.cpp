/*
*/
#include "server.h"
#include <unistd.h>
#include <iomanip>

int16_t RBServer :: close() {
    if (_sock != INVALID_SOCKET) {
        ::close( _sock ) ;
        _sock = INVALID_SOCKET ;
    }
    return 0 ;
} // RBServer :: close

int16_t RBServer :: open() {
    if (_sock == INVALID_SOCKET) {
        _sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (_sock == INVALID_SOCKET) {
            return -1 ;
        }
    }

    memset((char*)&_addr, 0, sizeof(_addr));
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY ;
    _addr.sin_port = htons( _args.port );
    bind( _sock, (sockaddr*)&_addr, sizeof(_addr) ) ;

    return 0 ;
} // RBServer :: open

int16_t RBServer :: parse_config() {
    std::ifstream   inf ;
    inf.open( _args.infilename ) ;
    while (inf.good()) {
        std::string id, line, topic ;
        inf >> id ;
        getline( inf, line ) ;
        size_t p1 = line.find( '\"' );
        if (p1 == std::string::npos) {
            continue ;
        }
        size_t p2 = line.find( '\"', p1+1 ) ;
        topic = line.substr( p1+1, (p2-p1-1) ) ;

        registerId( topic, id ) ;
    }
    inf.close() ;
    return 0 ;
} // RBServer :: parse_config

void RBServer :: registerId( const std::string &topic, const std::string &id ) {
    Subscriber *sub = nullptr ;
    {
        std::unique_lock<std::mutex> guard(_subscriber_mtx);
        auto it = _subscriberMap.find( id ) ;
        if (it == _subscriberMap.end()) {
            // create new
            sub = new Subscriber( id ) ;
            _subscriberMap.insert( SubscriberMap_pair( id, sub )) ;
        } else {
            sub = (*it).second ;
        }
    }
    {
        std::unique_lock<std::mutex> guard(_topic_mtx);
        auto it = _topics.find( topic ) ;
        Topic *t = nullptr ;
        if (it == _topics.end()) {
            // create new
            t = new Topic( topic ) ;
            _topics.insert(TopicMap_pair( topic, t ));
        } else {
            t = (*it).second ;
        }
        // append id
        t->observers.push_back( sub ) ; // there's a much better way to do this
    }    
} // RBServer :: registerId

void RBServer :: update( const std::string &topic, const std::string &txt, int priority ) {
    TopicUpdate  *upd = new TopicUpdate( (++_updateId), topic, txt, priority ) ;
    {
        std::unique_lock<std::mutex> guard(_update_mtx);
        _updates.insert( UpdateMap_pair( upd->id, upd ) );
    }
    // find topic, push update
    auto it = _topics.find( topic ) ;
    if (it != _topics.end()) {
        for (auto x : (*it).second->observers) {
            x->push_update( upd ) ;            
        }
    } else {
        if (_args.verbose >= 2) {
            std::cout << "error:  topic not found [" << topic << "].  unable to process.  dropped.\n" ;
        }
    }
} // RBServer :: update

// threaded work method
void RBServer :: work_t() {
    if (_args.verbose >= 2) {
        std::cout << "worker_thread." << std::this_thread::get_id() << " started \n" ;
    }

    struct timespec ts;
    int rc ;
    ts.tv_sec = 1 ;
    ts.tv_nsec = 0 ;
//    ts.tv_nsec = 500'000'000 ;

    while (!_done) {
        // block on message deque semaphore
        clock_gettime(CLOCK_REALTIME, &ts) ;
        ts.tv_sec += 1 ;
        if ((rc = sem_timedwait(&_work_sem, &ts)) != 0) {
            int err = errno ;
            if (_done) break ;
            if (err == ETIMEDOUT) {
                // do_idle() ;
                continue ;
            } else {
                std::cout << "sem.error: " << err << "\n" ;
                break ;
            }
        }
        if (_done) break ;

        RBMessage *msg = nullptr ;
        {
            std::unique_lock<std::mutex> guard(_work_mtx);
            if (_workQ.size() == 0) {
                continue ;
            }
            msg = (*_workQ.begin()) ;
            _workQ.pop_front() ;
        }

        // process msg; fmt:  <topic><delim><txt><delim><priority>
        char delim = 0x01 ;
        std::string topic, txt ;
        int priority ;
        char *p1 = strchr( msg->pkt()->data, delim ) ;
        *p1 = '\0' ;
        topic = msg->pkt()->data ;
        char *p2 = strchr( &p1[1], delim ) ;
        *p2 = '\0' ;
        txt = &p1[1] ;
        priority = atoi( &p2[1] ) ;

        update( topic, txt, priority ) ;

        // increment pkt count
        _stats.nPackets++ ;
        _stats.nBytesRecv += msg->recv_sz() ;

        // put the msg pkt back
        std::unique_lock<std::mutex> guard(_mtx);
        _msgVec.push_back( msg ) ;
        msg = nullptr ;
    }
    if (_args.verbose >= 2) {
        std::cout << "worker_thread." << std::this_thread::get_id() << " exiting \n" ;
    }
} // RBServer :: work_t

void RBServer :: write_t() {
    if (_args.verbose >= 2) {
        std::cout << "output_thread." << std::this_thread::get_id() << " started \n" ;
    }
    while (!_done) {        
        usleep(100'000) ;
        if (!_done) {
            for (auto s : _subscriberMap) {
                s.second->release_updates() ;
            }
            // remove updates no longer needed from pending queue
            std::unique_lock<std::mutex> guard(_update_mtx);
            std::vector<uint32_t> dead_ids ;
            // delete the elements
            for (auto x : _updates) {
                if (x.second->nPendingNotifications < 1) {
                    delete x.second ;
                    dead_ids.push_back( x.first ) ;
                }
            }
            // delete the keys
            for (auto i : dead_ids) {
                _updates.erase( i ) ;
            }
        }
    }
    if (_args.verbose >= 2) {
        std::cout << "output_thread." << std::this_thread::get_id() << " exiting \n" ;
    }
} // RBServer :: write_t

RBServer :: RBServer( Args &args ) 
: _args( args ) {
    _done = false ;
    _nMsgBuffers = 0 ;
    _sock = INVALID_SOCKET ;
} // RBServer [ctor] 

RBServer :: ~RBServer() {
    stop() ;
} // RBServer [dtor]

RBMessage *RBServer :: get_buffer() {
    std::unique_lock<std::mutex> guard(_mtx);
    RBMessage *msg = nullptr ;
    if (_msgVec.size() == 0) {
        msg = new RBMessage() ;
        msg->allocate( MAX_PACKETSZ ) ;
        _nMsgBuffers++ ;
    } else {
        msg = _msgVec.back() ;
        _msgVec.pop_back() ;
    }
    return msg ;
} // RBServer :: read

int16_t RBServer :: initialize() {
    _done = false ;

    parse_config() ;

    sem_init(&_work_sem, 0, 0) ;

    return 0 ;
} // RBServer :: initialize

int16_t RBServer :: process( RBMessage *&msg, int16_t amt ) {
    // push msg onto semaphore queue
    // threads will pop the msg and process, returning msg when done
    msg->pkt()->decodeHost() ;
    if (msg->pkt()->totalSize() != amt) {
        std::cout << "bad packet recvd.  amt: " << amt << "  pkt.sz: " << msg->pkt()->totalSize() << "\n" ;
        return -1 ;
    }
    msg->recv_sz() = amt ;

    _workQ.push_back( msg ) ;
    sem_post( &_work_sem ) ;

    return 0 ;
} // RBServer :: process

void RBServer :: report_stats() {
    if (_args.verbose < 1) {
        return ;
    }

    std::cout << std::fixed << std::setprecision(1) 
         << _stats.packetsPerSec << " pkts/sec   " 
         << _stats.bytesRecvPerSec << " rB/sec" 
         << std::endl ;
} // RBServer :: report_stats

int16_t RBServer :: start() {
    _done = false ;
    _stats.reset() ;

    open() ;

    // allocate N threads ; N == (#cpus - 2); leave 2 cpus for system and main thread
    // no need to assign affinity; let the os manage it
    int num_cpus = std::thread::hardware_concurrency();
    if (_args.verbose >= 1) {
        std::cout << "number of cpus: " << num_cpus << "\n" ;
    }
    for (int i = 0; i < num_cpus-1; i++) {
        _threadPool.push_back( std::thread( [this](){ this->work_t() ; } ) ) ;
    }

    _outputThread = std::thread( [this](){ this->write_t() ; }) ;

    return 0 ;
} // RBServer :: start 

int16_t RBServer :: stop() {
    _done = true ;
    // wait for each thread to end
    std::for_each( _threadPool.begin(), _threadPool.end(), 
                   [this]( std::thread &t ){ t.join() ; } 
                 );
    _outputThread.join() ;

    close() ;

    return 0 ;
} // RBServer :: stop

void RBServer :: tick() {
    _stats.tick() ;

//    cout << timeGetTime() << "\n";
} // RBServer :: tick

