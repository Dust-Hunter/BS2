// =====================================================================
// RMMIXinstruction.h - Header file for the RMMIXinstruction class.
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
// Header File for the RMMIXinstruction class.
//
// =====================================================================

#pragma once

#ifndef RMMIXINSTRUCTION_H_
#define RMMIXINSTRUCTION_H_

#include <string>
#include <fstream>
#include <iostream>

// An RMMIX Instruction contains an op code and 0-3 operands (parameters)
// The op code could be stored as a byte, but this is a simulator, not real 
// hardware, so we store everything in an array, modeled on argc & argv (in main)

class RMMIXinstruction {
public:
    static const int maxNumFields = 4;
    int fields[ maxNumFields ]; // field[0] = op, field[1-3] = args[1-3], just like argc
    int numFields;

    RMMIXinstruction(); // blank, simple constructor

    void reset( // (Re-) Initializer
            int opCode,
            int numOps,
            int op1 = 0,
            int op2 = 0,
            int op3 = 0);

    RMMIXinstruction( // Constructor with parameters
            int opCode,
            int numOps,
            int op1 = 0,
            int op2 = 0,
            int op3 = 0) {
        reset( opCode, numOps, op1, op2, op3);
    }

    bool empty() const {
        return ( 0 == numFields);
    }

    // Convenience method
    int opCode( ) const { return fields[0]; }
    
    // Two debug methods 
    std::string dump() const;

    void dump(std::ostream &outStream) const // prints fields to outStream
    {
        outStream << dump();
    }

}; // end RMMIXinstruction


#endif /*RMMIXINSTRUCTION_H_*/
