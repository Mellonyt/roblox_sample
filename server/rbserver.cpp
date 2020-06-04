/*
    rbserver.cpp
    main application file for roblox server sample

    developer   r_mcinnis@solidice.com
    date        june 02, 2020
*/
#include <iostream>
#include <unistd.h>
#include <chrono>
#include "args.h"

using namespace std;

#define APP_DESC  "roblox sample server"  
#define VERSION   "v0.1"

Args  args ;

void banner() {
    cout << APP_DESC << " " << VERSION << " \n" ;
} // :: banner

void show_args( const char *app_name ) {
    cout << APP_DESC << "  " << VERSION << " \n" ;
    cout << app_name << " [options] <config_filename.txt> \n" ;
    cout << "  -p port  (range: 1024..65535; default: " << args.port << ") \n" ;
    cout << "  -v N     verbose level (default:  0, range 0..5) \n" ;
} // :: show_args

//-----------------------------------------------------------------------------
int64_t  __start_tm = 0 ;
int64_t  __latest_tm = 0 ;
int32_t  __now = 0 ;

int32_t timeGetTime() {
    __latest_tm = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1) ;
    if (__start_tm == 0) __start_tm = __latest_tm ;
    return (__now = (__latest_tm - __start_tm)) ;
}

//-----------------------------------------------------------------------------
#include "rbmessage.h"
#include <sys/time.h>

#define SOCKET            int
#define INVALID_SOCKET   -1 
#define TIMEVAL           timeval

int16_t udp_recvfrom( SOCKET sock, char *recv_buf, int buf_sz, struct sockaddr &src_addr, long timeout = 100 ) 
{
  fd_set      readfds ;
//  fd_set      writefds ;
//  fd_set      exceptfds ;
  socklen_t   n = 0 ;
  TIMEVAL     timeval ;
  timeval.tv_sec  = (timeout / 1000) ;
  timeval.tv_usec = (timeout % 1000) * 1000 ;
  memset( &readfds  , 0x00, sizeof(fd_set) ) ;
  FD_SET( sock, &readfds);
//  memset( &writefds , 0x00, sizeof(fd_set) ) ;
//  memset( &exceptfds, 0x00, sizeof(fd_set) ) ;
//  readfds.fd_count = 1 ;
//  readfds.fd_array[0] = _sock ;

//cout << "select.sock(" << sock << ") timeout: " << timeout << "  buf_sz: " << buf_sz << "\n" ;
  n = ::select( sock+1, &readfds, nullptr, nullptr, &timeval ) ;
//cout << "select.sock(" << sock << ") returned " << n << endl ;
  if (n <= 0)
    return n ; // timeout == 0; error == -1

  n = sizeof(src_addr) ;
  return recvfrom( sock, (char*)recv_buf, buf_sz, 0, &src_addr, &n ) ;
} // :: udp_recvfrom

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
#include "rbmessage.h"
#include <thread>
#include <vector>
#include <deque>
#include <mutex>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include <semaphore.h>

class TopicUpdate {
    public:
    uint32_t id ;
    std::string  topic ;
    std::string  txt ;
    int priority ;
    int nPendingNotifications ;
    
    TopicUpdate( uint32_t id_, const string &topic_, const string &txt_, int priority_ ) 
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

bool comparePtrToUpdate(TopicUpdate* a, TopicUpdate* b) { return (*a < *b); }

class Subscriber {
    protected:
        std::string  _id ;
        vector<TopicUpdate*>  _updates ;
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
            std::sort(_updates.begin(), _updates.end(), comparePtrToUpdate);

            // letting the os file cacheing handle the writes efficiently
            // if it were a problem, i'd buffer the writes here into large bulk writes
            //
            ofstream outf ;
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

class Topic {
    public:
    std::string  id ;
    vector<Subscriber*>  observers ; // i'd MUCh rather use proper observables, but this is what i've got

    Topic( const std::string &id_ ) : id( id_ ) {}
} ; // class Topic

typedef map<string,Topic*>                      TopicMap ;
typedef map<string,Topic*>::value_type          TopicMap_pair ;
typedef map<string,Topic*>::iterator            TopicMap_iter ;

typedef map<uint32_t,TopicUpdate*>              UpdateMap ;
typedef map<uint32_t,TopicUpdate*>::value_type  UpdateMap_pair ;
typedef map<uint32_t,TopicUpdate*>::iterator    UpdateMap_iter ;

typedef map<string,Subscriber*>                 SubscriberMap ;
typedef map<string,Subscriber*>::value_type     SubscriberMap_pair ;
typedef map<string,Subscriber*>::iterator       SubscriberMap_iter ;

class RBServer {
    protected:
        Args        _args ;
        std::string _err ;
        bool        _done ;
        SOCKET      _sock ;
        struct sockaddr_in _addr ;
        vector<RBMessage*> _msgVec ;
        deque <RBMessage*> _workQ ;
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
        vector<std::thread> _threadPool ;
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
        string id, line, topic ;
        inf >> id ;
        getline( inf, line ) ;
        size_t p1 = line.find( '\"' );
        if (p1 == string::npos) {
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
            cout << "error:  topic not found [" << topic << "].  unable to process.  dropped.\n" ;
        }
    }
} // RBServer :: update

// threaded work method
void RBServer :: work_t() {
    if (_args.verbose >= 2) {
        cout << "worker_thread." << std::this_thread::get_id() << " started \n" ;
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
                cout << "sem.error: " << err << "\n" ;
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
        string topic, txt ;
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
        cout << "worker_thread." << std::this_thread::get_id() << " exiting \n" ;
    }
} // RBServer :: work_t

void RBServer :: write_t() {
    if (_args.verbose >= 2) {
        cout << "output_thread." << std::this_thread::get_id() << " started \n" ;
    }
    while (!_done) {        
        usleep(100'000) ;
        if (!_done) {
            for (auto s : _subscriberMap) {
                s.second->release_updates() ;
            }
            // remove updates no longer needed from pending queue
            std::unique_lock<std::mutex> guard(_update_mtx);
            vector<uint32_t> dead_ids ;
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
        cout << "output_thread." << std::this_thread::get_id() << " exiting \n" ;
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
        cout << "bad packet recvd.  amt: " << amt << "  pkt.sz: " << msg->pkt()->totalSize() << "\n" ;
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

    cout << std::fixed << std::setprecision(1) 
         << _stats.packetsPerSec << " pkts/sec   " 
         << _stats.bytesRecvPerSec << " rB/sec" 
         << endl ;
} // RBServer :: report_stats

int16_t RBServer :: start() {
    _done = false ;
    _stats.reset() ;

    open() ;

    // allocate N threads ; N == (#cpus - 2); leave 2 cpus for system and main thread
    // no need to assign affinity; let the os manage it
    int num_cpus = std::thread::hardware_concurrency();
    if (_args.verbose >= 1) {
        cout << "number of cpus: " << num_cpus << "\n" ;
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

//-----------------------------------------------------------------------------
int main( int argc, const char *argv[] ) {
    string      reason ;

    if (args.parse( argc, argv, reason ) != 0) {
        show_args( argv[0] ) ;
        cout << "\n  error: " << reason << endl << endl ;
        return -1 ;
    }
    banner() ;

    RBServer    comms( args ) ;
    RBMessage  *msg = nullptr ;
    int16_t     rc = 0 ;
    int32_t     next_stats_tm = timeGetTime() + 2500 ;

    comms.initialize() ;
    if (comms.start() != 0) {
        cout << "failure to start communications\n" ;
        cout << "reason: " << comms.err() << "\n" ;
        return -1 ;
    }

    while (!comms.is_done()) {
        if (msg == nullptr) {
            msg = comms.get_buffer() ;
        }
        rc = udp_recvfrom( comms.sock(), (char*)msg->pkt(), msg->sz(), msg->addr(), 500 ) ;
        if (rc > 0) {
            comms.process( msg, rc ) ;
            msg = nullptr ;
        } else if (rc == 0) { // timeout
//            cout << "." ;
        } else {
            cout << "err: " << rc << endl ;
        }
        timeGetTime() ;
        if (__now > next_stats_tm) {
            comms.tick() ;
            comms.report_stats() ;
            next_stats_tm = __now + 2500 ;
        }
    }
    comms.stop() ;

    return 0 ; 
} // :: main
