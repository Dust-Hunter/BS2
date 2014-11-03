// =====================================================================
// rmmixHardware.h - Header file for essential hardware classes.
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
// Header file rmmixHardware.h
//
// This file defines the rmmixHardware name space
// which models the RMMIX hardware.
//
// =====================================================================

#ifndef RMMIXHARDWARE_H_
#define RMMIXHARDWARE_H_

#include <fstream> // for the log file
#include <vector>
#include <map>

#include "RMMIXinstruction.h" // needed for the RMMIXinstruction class
#include "RMMIXJobLang.h"  // needed for commpiler, decompiler classes
                           // and indirectly for ob codes, trap codes...

class rmmixHardware { // abstract class for deriving hardware subclasses
public:
    static std::ofstream  logStream; // There is only one, not one per instance!

    // Interrupts are done with the following "lines".
    // Every component (instance of the class) has its own copies
    // of these three "lines" (or "ports" or "special purpose registers");
    //  o  trapNumber says what has happened
    //  o  trapData is for communication (if needed)
    //  o  trapStatus is for signaling errors (while handling interrupts)
    int   trapNumber  = 0;    // Note new C++11 member initialization
    int   trapData    = 0;
    int   trapStatus  = 0;    // Zero == OK, non-zero == problem

    // the clock is used by the simulator (useful for logging)
    // and is global (there is only one clock, shared by all components)
    static int            clock;

    // every hardware component should have a unique device number
    int                   deviceNumber;

    // constructor
    rmmixHardware( int devNum ) : deviceNumber( devNum ) { };

    // virtual destructor
    virtual ~rmmixHardware( ) { };

    // Every hardware component CAN overload the log method
    virtual std::ostream& log( ) {
        return ( logStream << clock  << ": dev " << deviceNumber << ' ' );
    };

    // Every hardware component MUST overload the run method!
    virtual void run( ) = 0;

    // Every hardware component MUST overload the bind method!
    virtual void bind( void *pointer ) = 0;

}; // end class rmmixHardware

class rmmixCPU : public rmmixHardware {
public:
    // ====================================>>>  The Registers
    const int numberOfRegisters = 32; // see RMMIX presentation

    std::vector< int > registers;

    // By the way, it's plural because "register" is a keyword inc C (& C++)

    // note: We're cheating here a bit - we should make sure that int = 32 bits
    // (and not 16 or 64 or whatever)
    // but for our purposes here it doesn't really matter.  Trust me.

    // ===================================>>> The Instruction Memory
    const int instructionMemorySize = 1024;

    std::vector< RMMIXinstruction > instructionMemory;

    // ===================================>>> The Data Memory
    const int dataMemorySize = 1024; // see RMMIX presentation

    std::vector< int > dataMemory; // size = dataMemorySize

    // Constructor & Destructor
    rmmixCPU( int devNum )
    : rmmixHardware( devNum ),
      registers( numberOfRegisters ),
      instructionMemory( instructionMemorySize ),
      dataMemory( dataMemorySize )
    { };
    virtual ~rmmixCPU( ) { };

    virtual std::ostream& log( ) {
        return ( rmmixHardware::log() << "CPU " );
    };

    // ===================================>>>> A L U
    // Arithmetical Logic Unit
    void executeInstruction(RMMIXinstruction& instruction);

    // take care of traps (a.k.a. interrupts )
    void handleInterrupt( );

    // perform do one clock tick
    virtual void run( );

    // The CPU does not really support the bind method
    virtual void bind( void *pointer ) {
        std::cerr << "ERROR: CPU cannot be bound to a pointer" << std::endl;
        exit( -7 );
    };

}; // end rmmixCPU


class rmmixInputDevice : public rmmixHardware {
public:
    objectCodeDecompiler*    decompiler;
    const int                inputDelay = 10; // clock ticks
    int                      countDownTimer = 0;

    rmmixInputDevice( int                    devNum,
                      objectCodeDecompiler*  deco = nullptr )
    : rmmixHardware( devNum ), decompiler( deco )
    { };

    virtual ~rmmixInputDevice( ) { };

    virtual std::ostream& log( ) {
        return ( rmmixHardware::log() << "Input " );
    };
    // perform do one clock tick
    virtual void run( );

    virtual void bind( void *pointer ) {
        objectCodeDecompiler*  deco = static_cast<objectCodeDecompiler*>( pointer );
        assert( deco ); // is not null
        assert( deco->good() );
        decompiler = deco;
    };

};

class rmmixOutputDevice : public rmmixHardware {
public:
    std::ostream*    outputSink;
    const int        outputDelay = 10; // clock ticks
    int              countDownTimer = 0;
    int              buffer;

    rmmixOutputDevice( int           devNum,
                      std::ostream*  sink = nullptr )
    : rmmixHardware( devNum ), outputSink( sink )
    { };

    virtual ~rmmixOutputDevice( ) { };

    virtual std::ostream& log( ) {
        return ( rmmixHardware::log() << "Output " );
    };
    // perform do one clock tick
    virtual void run( );

    virtual void bind( void *pointer ) {
        std::ostream*  sink = static_cast<std::ostream*>( pointer );
        assert( sink ); // is not null
        assert( sink->good() );
        outputSink = sink;
    };

};

// =================== Global Variables!!!
extern rmmixCPU* theCPU;
extern std::map< int, rmmixHardware* > hardwareComponents; // global list of hardware


#endif /* RMMIXHARDWARE_H_ */
