/*
    args.cpp
    Args class implementation

    developer   r_mcinnis@solidice.com
    date        june 02, 2020
*/
#include "args.h"
#include "utils.h"

Args::Args() {

} // Args [ctor]

int16_t Args::parse( int argc, const char *argv[], std::string &msg ) {
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') { // input filename
          if (infilename != "") {
              // more then one filename?  invalid
              msg = "more than one input file specified" ;
              return -1 ;
          }
          infilename = argv[i] ;
        } else {
            char opt = tolower(argv[i][1]) ;
            switch (opt) {
                case 'd' : { // delay
                    if (!to_numeric( argv[++i], nDelay, 30000 )) {
                        msg = "invalid delay specified" ;
                        return -2 ;
                    }
                    break ;
                }
                case 'h' : { // server ip address
                    i++ ;
                    ipaddr = argv[i] ;
                    if (!valid_ipaddr( argv[i] )) {
                        msg = "invalid server ip (dot) address" ;
                        return -3 ;
                    }
                    break ;
                }
                case 'p' : { // server port
                    if (!to_numeric( argv[++i], port, 65535 ) || (port < 1024)) {
                        msg = "invalid server port" ;
                        return -4 ;
                    }
                    break ;
                }
                case 'r' : { // repeats
                    if (!to_numeric( argv[++i], nRepeats, 1000 )) {
                        msg = "invalid repeat specified" ;
                        return -5 ;
                    }
                    continous = (nRepeats == 0) ;
                    break ;
                }
                default:
                  msg = "unknown argument" ;
                  return -6 ; // invalid arg
            }
        }
    }
    if ((infilename == "") || !file_exists( infilename )) {
        msg = "input file not specified" ;
        return -1 ;
    }
    if (!file_exists( infilename )) {
        msg = "input file does not exist" ;
        return -1 ;
    }
    return 0 ;
} // Args :: parse

