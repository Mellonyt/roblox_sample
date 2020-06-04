/*
    inputfile.cpp
    InputFile class implementation

    developer   r_mcinnis@solidice.com
    date        june 02, 2020
*/
#include "inputfile.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <string.h>

using namespace std ;

InputFile :: InputFile() {
} // InputFile [ctor]

InputFile :: ~InputFile() {
    _inf.close() ;
} // InputFile [dtor]

bool InputFile :: eof() { 
    return _inf.good() == false ; 
} // InputFile :: eof

bool InputFile :: open( const std::string &infname ) { 
    _inf.open( infname ) ;
    _fname = infname ;
    return _inf.good() ;
} // InputFile :: open

bool InputFile::read_line( RBUpdate &dst ) {
    dst.clear() ;

    _inf.getline( _buffer, MAXLENGTH ) ;

    char *p1 = strchr( _buffer, '\"' ) ;
    if (p1 == nullptr) return false ;

    char *p2 = strchr( &p1[1], '\"' ) ;
    if (p2 == nullptr) return false ;

    p2[0] = '\0' ;
    dst.topic = &p1[1] ;

    p1 = strchr( &p2[1], '\"' );
    if (p1 == nullptr) return false ;
    p2 = strchr( &p1[1], '\"' ) ;
    if (p2 == nullptr) return false ;
    p2[0] = '\0' ;
    dst.txt = &p1[1] ;

    p1 = strchr( &p2[1], ':' ) ;
    if (p1 == nullptr) return false ;
    dst.priority = atoi( &p1[1] ) ;

    return true ;
} // InputFile :: read_line

void InputFile::reset() {
    _inf.close() ;
    open( _fname ) ;
} // InputFile :: reset

