/*
    rbserver.cpp
    main application file for roblox server sample

    developer   r_mcinnis@solidice.com
    date        june 02, 2020
*/
#include <iostream>
#include <unistd.h>
#include <chrono>
#include "server.h"
#include "utils.h"

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
            // do_idle() ;
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
