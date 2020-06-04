/*
    inputfile.h
    InputFile class definition

    developer   r_mcinnis@solidice.com
    date        june 02, 2020
*/
#pragma once
#ifndef INPUTFILE_H
#define INPUTFILE_H

#include <iostream>
#include <fstream>
#include <unistd.h>
#include "rbupdate.h"

#define MAXLENGTH  256

class InputFile {
  protected:
    std::string   _fname ;
    std::ifstream _inf ;
    char          _buffer[ MAXLENGTH+1 ] ;

  public:
                InputFile() ;
                ~InputFile() ;
    bool        eof() ;
    bool        open( const std::string &infname ) ;
    bool        read_line( RBUpdate &dst ) ;
    void        reset() ;
} ; // class InputFile

#endif
