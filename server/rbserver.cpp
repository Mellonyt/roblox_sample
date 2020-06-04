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
#include <vector>
#include <mutex>

class RBServer {
    protected:
        Args        _args ;
        std::string _err ;
        bool        _done ;
        SOCKET      _sock ;
        struct sockaddr_in _addr ;
        vector<RBMessage*> _msgVec ;
        std::mutex  _mtx ;
        int16_t     _nMsgBuffers ;
        LineStats   _stats ;

        int16_t     close() ;
        int16_t     open() ;

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

    // allocate N threads ; N == (#cpus - 2); leave 2 cpus for system and main thread
    // no need to assign affinity; let the os manage it

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

//    sockaddr_in  &sa = (sockaddr_in&)msg->addr() ;
//    cout << "p: " << inet_ntoa( sa.sin_addr ) << ":" << sa.sin_port << "  m: " << msg->pkt()->data << endl;

    // put the msg pkt back
    std::unique_lock<std::mutex> guard(_mtx);
    _msgVec.push_back( msg ) ;
    msg = nullptr ;

    // increment pkt count
    _stats.nPackets++ ;

    return 0 ;
} // RBServer :: process

void RBServer :: report_stats() {
    cout << "nPackets/sec: " << _stats.packetsPerSec << endl ;
} // RBServer :: report_stats

int16_t RBServer :: start() {
    _done = false ;
    _stats.reset() ;

    open() ;
    return 0 ;
} // RBServer :: start 

int16_t RBServer :: stop() {
    _done = true ;
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
    int32_t     next_stats_tm = timeGetTime() + 1000 ;

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
        } else if (rc == 0) {
//            cout << "." ;
        } else {
            cout << "err: " << rc << endl ;
        }
        timeGetTime() ;
        if (__now > next_stats_tm) {
            comms.tick() ;
            comms.report_stats() ;
            next_stats_tm = __now ;
        }
    }
    comms.stop() ;

    return 0 ; 
} // :: main
