// =====================================================================
// RMMIXcodes.h - Header file for the RMMIX Job Description Language 
//                (JDL) classes & namespace:
//                   Keywords and corresponding codes, such as
//                   op codes and their names, are defined here.
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

#pragma once

#include <cassert>
#include <map>

namespace RMMIX_JDL {

    // ====================================>>>   The  SymbolTableClass
    // First, we need a container to store the symbols and codes.
    // A vector would be good for converting codes (numbers) to symbols,
    // a map would would be good for converting symbols to codes, 
    // neither is perfect for both.  We define a type "SymbolTable" - currently
    // implemented as a map from string to int. Changes in the future are possible.

    typedef std::map< std::string, int > SymbolTable;

    // Use standard find(), at(), operator[](), and so forth whereever possible.
    // Here a few helper functions for looking up strings in the table

    inline int lookup(const SymbolTable &table,
                      const std::string &target) { // return negative if not found
        auto itr = table.find( target );
        return ( ( table.end() == itr ) ? -1 : itr->second );
    }

    inline const std::string lookup(const SymbolTable &table, int target ) { 
        for ( auto symbolPair : table )
            if ( symbolPair.second == target )
                return symbolPair.first;
        return std::string( "" );
    }

    // ====================================>>>   The  Operation Codes (OpCodes)
    // First, the op codes (numeric values).

    enum opCode_type  {
        NOP = 0x0,
        MOV = 0x1,
        MOVI = 0x2,
        ADD = 0x3,
        ADDI = 0x4,
        SUB = 0x5,
        SUBI = 0x6,
        MUL = 0x7,
        MULI = 0x8,
        DIV = 0x9,
        DIVI = 0xa,
        JMPI = 0xb,
        BEQZI = 0xc,
        BNEZI = 0xd,
        BNEGI = 0xe,
        TRAP = 0xf,
        LDWI = 0x10,
        LDW = 0x11,
        STWI = 0x12,
        STW = 0x13,
        maxOpCode = 0x14 // not an op code!
    };

    // Next the symbolic names of the op codes,
    // Stored in a SymbolTable.  
    // Note use of C++11 Initializer list!
    const SymbolTable opCodes{
        { "NOP", NOP },
        { "MOV", MOV },
        { "MOVI", MOVI },
        { "ADD", ADD },
        { "ADDI", ADDI },
        { "SUB", SUB },
        { "SUBI", SUBI },
        { "MUL", MUL },
        { "MULI", MULI },
        { "DIV", DIV },
        { "DIVI", DIVI },
        { "JMPI", JMPI },
        { "BEQZI", BEQZI },
        { "BNEZI", BNEZI },
        { "BNEGI", BNEGI },
        { "TRAP", TRAP },
        { "LDWI", LDWI },
        { "LDW", LDW },
        { "STWI", STWI },
        { "STW", STW }           
    }; // end opCodes (constant symbol table)

    // Function to check of an opCode is legal                                

    inline bool opCodeOK(opCode_type opCode) {
        return ((NOP <= opCode) && (opCode < maxOpCode));
    }

    inline bool opCodeOK( int opCode ) { 
        return opCodeOK( opCode_type( opCode )); 
    }
    
    // Function returns the number of operands to expect for a given op code
    // (thus - 0 <= return value <= 3 ).

    inline int numberOfOperands(opCode_type opCode ) {
        assert(opCodeOK(opCode));
        switch (opCode) {
            case NOP: return 0;
            case JMPI: return 1;
            case TRAP: // except for TRAP dump, which takes only 1 op...  ;-(
            case MOV:
            case MOVI:
            case BEQZI:
            case BNEZI:
            case BNEGI:
            case LDWI:
            case LDW:
            case STWI:
            case STW: return 2;
            case ADD:
            case ADDI:
            case SUB:
            case SUBI:
            case MUL:
            case MULI:
            case DIV:
            case DIVI: return 3;
            default: assert(false); // we should never get here
        }; // end switch on opCOde 
    }; // end numberOfOperands

    inline int numberOfOperands( int opCode ) {
        return numberOfOperands( opCode_type( opCode ) );
    }

    // ====================================>>>   The  Pseudo Operation Codes (OpCodes)

    enum pseudoOpCode_type { 
        JMP = 0xb,
        BEQZ = 0xc,
        BNEZ = 0xd,
        BNEG = 0xe
    }; // end pseudoOpCode_type

    const SymbolTable pseudoOpCodes{
        { "JMP", JMP },
        { "BEQZ", BEQZ },
        { "BNEZ", BNEZ },
        { "BNEG", BNEG }
    }; // end pseudoOpCodes 

    // ====================================>>>   The Trap Codes

    enum trapCode_type { 
        // Software Trap Codes - used in (rmmix assembly language) programs
        HALT = 1,
        GETW = 2,
        PUTW = 3,
        // Internal (Hardware) Trap Codes - not intended to be used in programs
        FATAL      = 65,
        GETW_READY = 66,
        PUTW_READY = 67,
        maxTrapCode = 68 // should be greater than max(op)
    }; // end trapCode_type

    const SymbolTable trapCodes{
        { "halt", HALT  }, 
        { "getw", GETW  },
        { "putw", PUTW  },
        { "FATAL",      FATAL  },
        { "GETW READY", GETW_READY },
        { "PUTW READY", PUTW_READY }
    }; // end pseudoOpCodes 

    inline bool trapCodeOK(trapCode_type trapCode) {
        return ((HALT <= trapCode) && (trapCode < maxTrapCode));
    }

    inline bool trapCodeOK( int trapCode) {
        return trapCodeOK( trapCode_type( trapCode ) );
    }

} // end RMMIX_JDL namespace

