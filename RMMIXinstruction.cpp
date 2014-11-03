// =====================================================================
// RMMIXinstruction.cpp - Source code for the RMMIXinstruction class.
//
// Author: Prof. Ronald Charles Moore
// Fachbereich Informatik - Dept. of Computer Science
// Hochschule Darmstadt   - University of Applied Sciences
//                          Darmstadt, Germany
// This File is part of the RMMIX Assembler/Simulator Package.
// Version 0.6, September 2013
//
// This package has been developed to be used ONLY with the course
// named "Betriebssysteme" (Operating Systems), given by 
// Prof. Moore.
//
// No warranty of any kind is given to anyone.  
// Only students currently enrolled and actively involved in the 
// "Betriebssysteme" course are granted permission to work with this
// package, but they are given unlimited permission to do as much or
// as little with this package as they choose, up to but not including
// distribution of the package or any of its contents.
//
// Please report bugs to <ronald.moore@h-da.de>
//
// =====================================================================
// 
// Source File for RMMIXinstruction.h, i.e. for the RMMIXinstruction class.
// See RMMIXinstruction.h for more information
// 
// =====================================================================

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cassert> 

#include "RMMIXcodes.h"

#include "RMMIXinstruction.h"

// constructor 

RMMIXinstruction::RMMIXinstruction()
{
    // -999 is used here as conspicuously false value (for debugging)
    // ...should never see the light of day...
    numFields = 0;
    fields[0] = fields[1] = fields[2] = fields[3] = -999;
}; // end constructor

void RMMIXinstruction::reset( int opCode, int numOps,
                              int op1, int op2, int op3)
{
    assert( RMMIX_JDL::opCodeOK( opCode ) );
    assert( (0 <= numOps) && (numOps < 4) );
    numFields = numOps + 1;
    fields[0] = opCode;
    fields[1] = op1;
    fields[2] = op2;
    fields[3] = op3;
}; // end class RMMIXinstruction constructor

// TO DO: REMOVE - DEPRECATED

std::string RMMIXinstruction::dump() const
{
    std::ostringstream outStream; // starts empty
    outStream << std::flush;
    outStream << "Instruction: " << std::flush;
    if (0 < numFields)
        outStream << " op = "
            << RMMIX_JDL::lookup( RMMIX_JDL::opCodes, fields[ 0 ] )
            << ", " << numFields << " fields" << std::flush;
    for (int i = 0; i < numFields; ++i)
        outStream << " [" << i << "]=" << fields[ i ] << std::flush;
    return outStream.str();
}; // end dump()

