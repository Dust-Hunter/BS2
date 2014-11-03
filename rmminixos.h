// =====================================================================
// rmminixos.h - Header file for the rmminix operating system (simulator)
//
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
// Header file rmminixos.h
//
// This file defines the rmminixOS Namespace - i.e. the rmminix o s sim
//
// =====================================================================

#ifndef RMMINIXOS_H_
#define RMMINIXOS_H_

#include "rmmixHardware.h"
#include <string>

namespace rmminixOS {
    /**
     * boots a programm
     * @param currentProgIndex provides information which programm is to be
     * booted, importent for the i/o components...I think so
     * @param filename the name of the file that should contain the programm code
     * @return true if everything worked as intened, false if any errors accured
     */
    bool boot(int currentProgIndex,char *filename,char *argv[]);

    //    Load
    // Loads instructions into Instruction memory, by reading an object file.
    // Expects a decompiler (a wrapper around a std::ifstream object)
    // Returns true if and only if everything loaded OK.
    bool load( objectCodeDecompiler& decompiler );
    
    /**
     * creates the filename for the output file
     * 
     */
    std::string createOutputFilename(int jobnumber)const;

    // Programmable Interrupt (TRAP) Handlers
    void handleHALT( );

    void handleGETW( );

    void handlePUTW( );

    // Internal Interrupt Handlers
    void handleFATAL( );

    void handleGETW_READY(  );

    void handlePUTW_READY(  );

} // end of rmmixOS namespace

#endif /* RMMINIXOS_H_ */
