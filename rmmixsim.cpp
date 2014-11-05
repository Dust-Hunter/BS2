// =====================================================================
// rmmixsim.cpp - source code for (main() for) the RMMIX Simulator
// (Primitive Version - only runs one program, takes input from stdin)
//
// Author: Prof. Ronald Charles Moore
// Fachbereich Informatik - Dept. of Computer Science
// Hochschule Darmstadt   - University of Applied Sciences
//                          Darmstadt, Germany
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
// Source File for the rmmix simulator.  See printUsage() for more
// information on how to use the simulator.
//
// =====================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cassert>

#include "rmmixHardware.h"  // for the hardware models (simulator)
#include "rmminixos.h"      // for the rmminix operating system (simulator)

void printVersion()
{
    std::cout << std::endl << "% RMMIX Simulator Version 0.6" << std::endl << std::endl;
} // end printVersion

void printUsage(const std::string &argv0) {
    printVersion();
    std::cout <<
            "Usage: " << argv0 << " [OPTION] | [object file name] \n"
            "Takes the name of a file containing one or more jobs in RMMIX JDL \n"
            "(Job Description Language) Object Format and simulates an RMMIX \n"
            "machine running those jobs.\n"
            "Options:\n"
            "\n"
            "      --help     display this help and exit\n"
            "      --version  output version information and exit\n"
            "\n"
            "Report bugs to <ronald.moore@h-da.de>.\n"
            "\n";
} // end printUsage

void SetUpHardware( int argc ) {

    // Preliminaries
    rmmixHardware::logStream.open( "rmmix.log" );
    assert ( rmmixHardware::logStream.good() );

    // Set up CPU
    theCPU = new rmmixCPU( 0 ); // devince number zero
    assert( theCPU );
    hardwareComponents[0] = theCPU;

    // NOTE: This code is written to allow multiple input files
    // BUT THIS HAS NOT BEEN TESTED YET!
    for ( int arg = 1; arg < argc; arg++ ) { // for all args, except for 0

        int outputDeviceNumber = 2 * arg;
        int inputDeviceNumber  = outputDeviceNumber -1;

        // Set up input device
        rmmixInputDevice* inputer = new rmmixInputDevice( inputDeviceNumber );
        assert( inputer ); // is not null
        hardwareComponents[ inputDeviceNumber ] = inputer;

        // Set up output Device
        rmmixOutputDevice* outputer = new rmmixOutputDevice( outputDeviceNumber );
        assert( outputer ); // is not null
        hardwareComponents[ outputDeviceNumber ] = outputer;

    }; // end for all arguments on the command line,

} // end SetUpHardware

int main(int argc, char *argv[])
{

    try {

        // take care of any options on the command line (--help, etc)
        bool specialArgsFound = false; // until found
        for (int argnum = 1; argnum < argc; argnum++) {
            std::string arg(argv[ argnum ]);
            if (arg == "--version") {
                specialArgsFound = true;
                printVersion();
            }
            else if (arg == "--help") {
                specialArgsFound = true;
                printUsage(argv[ 0 ]);
            }; // else, hopefully it's a file name
        }; // end for all arguments
        if ( specialArgsFound ) return 0; // Everythings's OK, go home

        if ( 2 != argc) { // somethings's wrong, go home
            printUsage(argv[ 0 ]);
            return ( -1 );
        };

        SetUpHardware( argc );

        
        
        if(rmminixOS::boot(argc,argv)){
        //onley run the sim if booting when smooth 
            
        // Run The Simulation
        const int forever = 1024 * 1024; // infinite loop protection...
        for ( rmmixHardware::clock = 0;
              rmmixHardware::clock < forever;
              rmmixHardware::clock++ )
            for ( auto component : hardwareComponents ) // C++11 for all loop!
                component.second->run( );
        
        }
        

    } catch (std::string err) {
        std::cerr << std::flush << "Caught exception: " << err << std::endl
                << std::flush << "Exiting..." << std::endl
                << std::flush;
        return -4;
    } catch (...) {
        std::cerr << std::flush << "Caught unexpected exception: " << std::endl
                << std::flush << "Exiting..." << std::endl
                << std::flush;
        return -5;
    }; // end catch
    
    
    return ( 0 );
} // end main (for rmmixsim)
