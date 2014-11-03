// =====================================================================
// rmmixas.cpp - source code for (main() for) the RMMIX Assembler
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
// Source File for the rmmixas assembler.  See printUsage() for more 
// information on how to use the assembler program.
// 
// =====================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>


#include "RMMIXJobLang.h"

void printVersion() {
    std::cout << std::endl << "% RMMIX Assembler Version 0.6" 
              << std::endl << std::endl;
} // end printVersion

void printUsage(const std::string &argv0) {
    printVersion();
    std::cerr <<
            "Usage: " << argv0 << " [OPTION] [FILE]... \n"
            "Translate FILE(s) from RMMIXAL Job Format into RMMIX Object Format,\n"
            "Concatenate the translations to standard output.\n"
            "Options:\n"
            "\n"
            "      --help     display this help and exit\n"
            "      --version  output version information and exit\n"
            "\n"
            "There must be at least one FILE\n"
            "\n"
            "Report bugs to <ronald.moore@h-da.de>.\n"
            "\n";
} // end printUsage

std::ostream& printNumber( int number, std::ostream& ostream ) {
    if ( number < 0 ) {
        ostream << '-';
        number = number * -1;
    }; // end if negative
    return( ostream << std::hex << number );
} // end printNumber

std::ostream& printInstruction( const RMMIXinstruction& instruction, 
                                std::ostream& ostream ) {
    for ( int field = 0; field < instruction.numFields; ++field )
        printNumber( instruction.fields[ field ], ostream ) << ' ';
    return ostream;
}

int main(int argc, char *argv[]) {

    try {
        // argc == 1 means no arguments
        // (because argv[0] is the name of the program)
        if (argc == 1)
            printUsage(argv[ 0 ]);
        else
            for (int argnum = 1; argnum < argc; argnum++) {
                std::string arg(argv[ argnum ]);
                if (arg == "--version")
                    printVersion();
                else if (arg == "--help")
                    printUsage(argv[ 0 ]);
                else // argument is not an option, should be a file name
                {
                    // std::cerr << "Opening file " << arg << "  ..." << std::endl;
                    assemblyCompiler compiler( arg.c_str() );
                    if ( ! compiler.good() )
                        std::cerr << "Error opening file <" << arg
                                  << ">." << std::endl;
                    else if ( compiler.eof() )
                        std::cerr << "File <" << arg
                                  << "> is empty." << std::endl;
                    else { // if compiler object is OK
                        bool notFinished = true;
                        while ( notFinished ) {
                            notFinished = compiler.gotoState( 
                                             JobLangCompiler::codeReaderState );
                            if (notFinished) {
                                std::cout << "$JOB " << compiler.jobname 
                                          << std::endl;
                        
                               RMMIXinstruction instruction;
                               while ( notFinished ) {
                                   try {
                                       notFinished = ( compiler >> instruction );
                                        if ( notFinished )
                                            printInstruction( instruction, 
                                                              std::cout )
                                                << std::endl;
                                   } catch ( std::string exception ) {
                                       std::cerr << exception << std::endl;
                                   };
                                }; // end while notFinished

                                if ( compiler.good() )
                                    notFinished = compiler.gotoState( 
                                            JobLangCompiler::inputReaderState );
                                if ( notFinished ) {
                                    std::cout << "$RUN" << std::endl;

                                    int inputNumber;
                                    while ( notFinished ) {
                                        try {
                                            notFinished = 
                                                    ( compiler >> inputNumber );
                                            if ( notFinished )
                                                printNumber( inputNumber, 
                                                                     std::cout )
                                                << std::endl;
                                        } catch ( std::string exception ) {
                                            std::cerr << exception << std::endl;
                                        };
                                    }; // end while notFinished

                                    notFinished = compiler.good() && ! compiler.eof();
                                    if ( notFinished ) 
                                        std::cout << "$END" << std::endl;
                                }; // end if notFinished
                            }; // end if notFinished
                        }; // end while notFinished
                    }; // end if compiler object is ok
                }; // end if argument is not an option, should be a file name
            }; // end for all arguments
    } catch (std::string err) {
        std::cerr << "FATAL ERROR " << err << std::endl
                  << "Exiting..." << std::endl;
        return -5;
    }; // end catch

    return 0;

} // end main (for rmmixas)
