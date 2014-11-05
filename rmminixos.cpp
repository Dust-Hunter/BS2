// =====================================================================
// rmminixos.cpp - Source code for essential symbol tables.
//
// Author: Prof. Ronald Charles Moore
// Fachbereich Informatik - Dept. of Computer Science
// Hochschule Darmstadt   - University of Applied Sciences
//                          Darmstadt, Germany
// This File is part of the RMMIX Assembler/Simulator Package.
// Version 0.7, September 2014
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
// Source File for rmminixos.h
// See rmminixos.h for more information
//
// =====================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>

// we need to know about the hardware to use it...
#include "rmmixHardware.h"

// we need this to know what to implement...
#include "rmminixos.h"

#include "RMMIXJobLang.h" // for objectCodeDecompiler class

// =====================================================================
//             OPERATING SYSTEM DATA STRUCTURES
//
// Put all data structures that the Operating System needs HERE!
//
// At the moment, our operating system only need two int variables,
// but we'll almost certainly need a lot more later, especially when we
// get to multiple processes. Feel free to add whatever is necessary!

static int instructionNumberToRestore = -1;
static int registerNumberToUpdate     = -1;

// =====================================================================
//             INTERRUPT HANDLERS
// =====================================================================

// =====================================================================
//       Halt and Fatal - Interrupts that end a process

void rmminixOS::handleHALT( )
{
    int status = theCPU->registers[ theCPU->trapData ];
    rmmixHardware::logStream << "Simulation Halt! Status = "
                             << status << std::endl;
    // This is OK if we only want to run one program -
    // We need to extend this to handle multiprogramming!
    exit(status); // brutal!
} // end handleHALT

void rmminixOS::handleFATAL( )
{
    rmmixHardware::logStream << "FATAL Interrupt!!" << std::endl;
    std::cerr << "FATAL Interrupt!!" << std::endl;
    // This is OK if we only want to run one program -
    // We need to extend this to handle multiprogramming!
    exit( theCPU->trapData );
} // end handleFATAL

// =====================================================================
//           Load -
//     Loads instructions into Instruction memory, by reading an object file.
//     It SHOULD be possible to use this multiple times, but that has not
//     been tested.
//     Returns true if and only if successful

bool rmminixOS::load( objectCodeDecompiler& decompiler )
{
    assert( theCPU ); // i.e. assert that theCPU is not a null pointer

    if ( ! decompiler.gotoState( JobLangCompiler::codeReaderState  ) ) {
        std::cerr << "Could not find $JOB control line in object file named "
                  << decompiler.filename << std::endl;
        return false;
    }
    else try // if ready to read object code (i.e. $JOB found)
    {
        RMMIXinstruction instruction;
        int instructionNumber = 0;

        // Parses each line after $JOB to $RUN
        while ( decompiler >> instruction ) {
            theCPU->instructionMemory[ instructionNumber ] = instruction;
            ++instructionNumber;
        }; // until no more lines or found $RUN

        // if we're here, then we could load the program.
        theCPU->registers[ 0 ] = 0;
        return true;
    } catch ( std::string error  ) { // if an exception was thrown, something went wrong.
        std::cerr << "Error while loading file named " << decompiler.filename
                  << std::endl << error << std::endl;
        return false;
    }; // end if $JOB accepted
} // end load

// =====================================================================
//           Boot -
//     Construct a decompiler, and bind it to the input device.
//     Bind standard out to the output device.
//     This will have to be changed when we go to multiple I/O devices...
//     Returns true if and only if everything booted OK
bool rmminixOS::boot(int argc,char *argv[]) {

    
    // SET UP INPUT
    // try to open a decompiler with a given file name
    objectCodeDecompiler* decompiler = new objectCodeDecompiler(argv[1]);
    // We call new instead of using a local variable so that the
    // decompiler object survives the call to this function
     // (it will be used later by the intput device object).

    // Basic error checking (argv arguments are often bad, so be careful)
    assert( decompiler ); // is not null
    if ( ! decompiler->good() ) {
        std::cerr << "Could not open object file with name "  << argv[1]
                  << std::endl;
        return false;
    };
    if ( decompiler->eof() ) {
        std::cerr << "Object file with name "  << decompiler->filename
                 << " appears to be empty."   << std::endl;
        return false;
    };

    // Load the instruction memory
    if ( ! rmminixOS::load( *decompiler )) {
        std::cerr << "BOOT LOAD FAILED - File name " << argv[1] << std::endl;
        return false;
    } else { // if load was successful
        assert( decompiler->good() ); // should still be OK
        assert( ! decompiler->eof() ); // should not be at eof (or can it?)

        assert( hardwareComponents[ 1 ] ); // is not null
        hardwareComponents[1]->bind( decompiler );
    }; // end if load successful

    // SET UP OUTPUT
    assert( hardwareComponents[2 ] ); // is not null
    
    hardwareComponents[2]->bind( &(std::cout) ); // bind to std out
    
    return true;
} // end boot


// =====================================================================
//                                  Input and Output
// Note that I/O is two phased - the CPU sends a request to the device
// (either a GETW or a PUTW interrupt), then the device sends an interrupt
// to say that it is finished (either a GETW_READY ora PUTW_READY).

// Input, Phase 1 (CPU requests input)
void rmminixOS::handleGETW(  )
{
    assert( theCPU ); // i.e. assert that theCPU is not a null pointer
    // Save data we will need later
    if ( 0 <= theCPU->registers[0] ) {
        instructionNumberToRestore = theCPU->registers[ 0 ];
        registerNumberToUpdate = theCPU->trapData;
        // Put the CPU in an idle state until PUTW_READY signal
        theCPU->registers[ 0 ] = -1; // make the cpu wait!
    };

    // Signal the input device
    // Note that the choice of device is hard-coded - we always read from
    // device 1! (This will have to change)
    assert( hardwareComponents[1] );
    if ( 0 == hardwareComponents[1]->trapNumber ) {
        hardwareComponents[1]->trapNumber = RMMIX_JDL::GETW;
        // hardwareComponents[1]->trapData = ???
        // hardwareComponents[1]->trapStatus = ???

        // Clear interrupts
        theCPU->trapNumber = theCPU->trapData = theCPU->trapStatus = 0;
    };
    // else do nothing (but wait until the input device is ready!)

} // end handleGETW

// Input, Phase 2 (Device signals completion)
void rmminixOS::handleGETW_READY( )
{
    assert( theCPU ); // i.e. assert that theCPU is not a null pointer

    // The hardware has signaled that the get-word operation is done.
    if ( 0 != theCPU->trapStatus ) {
        // trigger fatal interrupt (crash current process)
        theCPU->trapNumber = RMMIX_JDL::FATAL;
    } else { // if OK status
        int inputDevice = theCPU->trapData;
        assert( 1 == inputDevice ); // THIS MUST BE CHANGED LATER!
        assert( hardwareComponents[ inputDevice ] ); // not null
        // Get data from input device
        theCPU->registers[ registerNumberToUpdate ]
                                 =  hardwareComponents[ inputDevice ]->trapData;
        // Restore system state
        theCPU->registers[ 0 ] = instructionNumberToRestore; // end idling
        // Clear interrupts
        theCPU->trapNumber = theCPU->trapData = theCPU->trapStatus = 0;
    };
} // end handleGETW_READY

// Output, Phase 1 (CPU requests output)
void rmminixOS::handlePUTW( )
{
    assert( theCPU ); // i.e. assert that theCPU is not a null pointer
    // Save data we will need later
    if ( 0 <= theCPU->registers[0] ) {
        instructionNumberToRestore = theCPU->registers[ 0 ];
        // Put the CPU in an idle state until PUTW_READY signal
        theCPU->registers[ 0 ] = -1; // make the cpu wait!
    };

    // Signal the output device
    // Note that the choice of device is hard-coded - we always write to
    // device 2!
    assert( hardwareComponents[2] );
    if ( 0 == hardwareComponents[2]->trapNumber ) {
        hardwareComponents[2]->trapNumber = RMMIX_JDL::PUTW;
        hardwareComponents[2]->trapData = theCPU->registers[ theCPU->trapData ];
        hardwareComponents[1]->trapStatus = 0;

        // Clear interrupts
        theCPU->trapNumber = theCPU->trapData = theCPU->trapStatus = 0;
    }; // end if output device ready
    // else do nothing (but wait until the input device is ready!)

} // end handlePUTW

// Output, Phase 2 (Device signals completion)
void rmminixOS::handlePUTW_READY( )
{
    assert( theCPU ); // i.e. assert that theCPU is not a null pointer

    // The hardware has signaled that the put-word operation is done.
    if ( 0 != theCPU->trapStatus ) {
        // trigger fatal interrupt (crash current process)
        theCPU->trapNumber = RMMIX_JDL::FATAL;
    } else { // if OK status
        // Check if the device number is OK
        int outputDevice = theCPU->trapData;
        assert( 2 == outputDevice ); // This will change later!
        assert( hardwareComponents[ outputDevice ] ); // not null

        // Restore system state
        theCPU->registers[ 0 ] = instructionNumberToRestore; // end idling
        // Clear interrupts
        theCPU->trapNumber = theCPU->trapData = theCPU->trapStatus = 0;
    };

} // end handlePUTW_READY


