/*
    rbclient.cpp
    main application file for roblox client sample

    developer   r_mcinnis@solidice.com
    date        june 02, 2020
*/
#include <iostream>
#include "inputfile.h"
#include "client.h"

using namespace std;

#define APP_DESC  "roblox sample client"  
#define VERSION   "v0.1"

Args  args ;

void banner() {
    cout << APP_DESC << " " << VERSION << " \n" ;
} // :: banner

void show_args( const char *app_name ) {
    cout << APP_DESC << "  " << VERSION << " \n" ;
    cout << app_name << " [options] <input_filename.txt> \n" ;
    cout << "  -d N     millisecond delay between messages, max 60000 (default: " << args.nDelay << "ms) \n" ;
    cout << "  -r N     repeat N times; 0 == continous, max 1000 (default: no repeat) \n" ;
    cout << "  -h host  server ip address (default: " << args.ipaddr << ") \n" ;
    cout << "  -p port  port (range: 1024..65535; default: " << args.port << ") \n" ;
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

    InputFile   inf ;
    if (!inf.open( args.infilename )) {
        cout << "error opening input file.  terminating.\n" ;
        return -1 ;
    }

    RBClient    comms( args ) ;
    RBUpdate    update ;

    if (!comms.start()) {
        cout << "failure to start communications\n" ;
        cout << "reason: " << comms.err() << "\n" ;
        return -1 ;
    }

    cout << "continous: " << args.continous << "  nRepeats: " << args.nRepeats << "\n" ;

    while ((args.continous || (args.nRepeats > 0)) && !comms.is_done()) {
        while (!inf.eof()) {
            if (!inf.read_line( update )) {
                continue ;
            }
            if (comms.send( update ) == 0) {
                cout << "error sending data to server\n" ; 
                return -1 ;
            }
            usleep( args.nDelay * 1000 ) ;
        }
        if (!args.continous)  args.nRepeats-- ;
        inf.reset() ;
        cout << "input stream reset." << endl ;
    }
    comms.stop() ;

    return 0 ; 
} // :: main
