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
              msg = "more than one config file specified" ;
              return -1 ;
          }
          infilename = argv[i] ;
        } else {
            char opt = tolower(argv[i][1]) ;
            switch (opt) {
                case 'p' : { // port
                    if (!to_numeric( argv[++i], port, 65535 ) || (port < 1024)) {
                        msg = "invalid port" ;
                        return -2 ;
                    }
                    break ;
                }
                case 'v' : { // verbose
                    if (!to_numeric( argv[++i], verbose, 5 )) {
                        msg = "invalid verbose level." ;
                        return -3 ;
                    }
                    break ;
                }
                default:
                  msg = "unknown argument" ;
                  return -4 ; // invalid arg
            }
        }
    }
    if ((infilename == "") || !file_exists( infilename )) {
        msg = "config file not specified" ;
        return -1 ;
    }
    if (!file_exists( infilename )) {
        msg = "config file does not exist" ;
        return -1 ;
    }
    return 0 ;
} // Args :: parse

