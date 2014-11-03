// =====================================================================
// RMMIXJobLang.h - Header file for the RMMIX Job Description Language 
//                  (JDL) classes & namespace
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

#include <cassert>      // assert()
#include <string>       // std::string
#include <fstream>      // std::ifstream
#include <ostream>      // for std::cerr
#include <sstream>      // std::stringstream

#include "RMMIXcodes.h"
#include "RMMIXinstruction.h"

/*  class: JobLangCompiler
 ******************************************************* 
 *  Virtual base class - wrapper for a file input stream.
 *  Packages everything common between two extensions -
 *  one extension reads assembly language (used in the assembler),
 *  the other extension reads object (i.e. assembled) code (used in the simulator).
 */
class JobLangCompiler {
public:
    std::string filename { "<uninitialized>" };
    std::string jobname  { "<uninitialized>" };
    
    // protected:
    // The file input stream that supplies the "raw" input
    std::ifstream inStream;

    // The reader can be in one of several states: 
    // ready to read code, ready to read input (numbers), etc.

    enum stateType : char { // C++11 (embedded) enum class!
        startState, codeReaderState, inputReaderState, endState
    };

    stateType state = startState; // C++11 initialization!

    // We keep track of whether the last input operation was successful
    // (required by operator bool() )
    bool lastOperationOK = true; // C++11 initialization!

    // Internal buffers - each line of input is read into  lineBuffer,
    // where comments are removed, blank lines are skipped (etc.).
    std::stringstream lineBuffer;

    // we need to keep track of the line number in order to give 
    // better error messages
    int lineNumber = 0;        // C++11 initialization!
    
    // This next buffer is used to store error messages whenever
    // (input) errors are detected.
    std::stringstream errorBuffer; // implicitly initialized to blank.
    
    // Utilities - 
    // Get new line of text and call cleanupLineBuffer as long as necessary
    bool refillLineBuffer(); 
    

public:
    // Constructor(s)
    JobLangCompiler(const char* fileName)
    : filename( fileName ), inStream(fileName, std::ifstream::in) {   };
    
    // Use the implicit destructor - it closes inStream!!
    // But make it a "virtual destructor" so we can delete ppointers to
    // this base class.
    virtual ~JobLangCompiler( ) { /* nothing to do here*/ };

    virtual void buildJumpTable( ) = 0; 

    // Methods from std::ifstream, which we can just "inherit"

    bool eof() const {
        return inStream.eof();
    }

    bool good() const {
        return inStream.good();
    }

    // Methods we can ALMOST "inherit" 

    // operator bool conversion - is used (most importantly) so we can write
    // things like   while ( langRdr >> object ) object.doSomething();

    operator bool() const {
        return ( lastOperationOK /* && inStream */ );
    }

    // Note that we do NOT inherit open(), or close() - use the constructor
    // and destructor for opening and closing a file! Do not reuse a reader

    // HERE ARE THE KEY METHODS - EVERYTHING ELSE IS HERE TO ALLOW THESE!
    // Instruction reader
    JobLangCompiler& operator>>(RMMIXinstruction& instruction);
    // Input reader
    JobLangCompiler& operator>>(int& inputNumber);

    // Of course, sometimes the input is bad - this exception
    // is thrown in this case - a reference to errorBuffer.
    virtual std::string  RMMIX_JDLexception( const std::string& string ) = 0;
    
    // Get a string out of the lineBuffer (if possible).
    // Set lastOpertionOK according to whether successful or not,
    // then return lastOperationOK. Do not read a new line!
    bool getToken( std::string& token );
    
    // protected:
    // Methods used to build operator>> (see above)

    // The following methods are different in each specialization
    // (derived class) and are thus virtual functions.
    // Note that all of these return true iff the operation was OK, and that
    // they set lastOperationOK to their return value (before returning).
    virtual bool getLabel( int &instructionNumber ) { return false; }
    virtual bool getOpCode(int &opCode) = 0;
    virtual bool getOperand(int &operand ) = 0;
    virtual bool getNumber(int &inputNumber) = 0;

    // If not already in newState, skip over unused input and get ready for
    // instructions or input (as the case may be).
    // This is where $JOB and $RUN are parsed (which is the same
    // in both objectCodeDecompiler and assemblyCompiler below).
    bool gotoState(stateType newState);
    virtual void prepareForState( stateType newState ) = 0; // extension specific
    
    // Finally, the instruction parser...
    bool getInstruction(RMMIXinstruction& instruction);

};

/*  class: objectCodeDecompiler
 ******************************************************* 
 *  Extends JobLangReader (see above) and 
 *  reads object code (already assembled)
 *  writes reverse assembled code (used in the simulator).
 */
class objectCodeDecompiler : public JobLangCompiler {
public:
    // Constructor(s)

    objectCodeDecompiler(const char* fileName)
    : JobLangCompiler(fileName) {
    };

    virtual std::string   RMMIX_JDLexception( const std::string& suffix ) {
        errorBuffer.str("");
        errorBuffer.clear();
        assert( 0 == errorBuffer.str().size() );
        assert( errorBuffer.good() );
        errorBuffer  << "Error in RMMIX JDL Object Code File "
                 << filename << ", line number " << lineNumber
                 << ": " << suffix << std::endl;
        return errorBuffer.str();
    }

    virtual void buildJumpTable( ) { };

    // protected:
    // Necessary virtual methods
    virtual bool getNumber(int &inputNumber);

    virtual bool getOpCode(int &opCode) {
        return getNumber(opCode);
    }

    virtual bool getOperand(int &operand) {
        return getNumber(operand);
    }

    virtual void prepareForState( stateType newState ) {
        /* do nothing */
    }; 


};

/*  class: assemblyCompiler
 ******************************************************* 
 *  Extends JobLangReader (see above) and 
 *  reads assembly language,
 *  writes object code (used in the assembler)
 */
class assemblyCompiler : public JobLangCompiler {
public:
    // Constructor(s)

    assemblyCompiler(const char* fileName)
    : JobLangCompiler(fileName), // call base class constructor
      jumpTable( ) // this shouldn't be necessary, but it can't hurt?
    {   };

    virtual std::string   RMMIX_JDLexception( const std::string& suffix ) { 
        errorBuffer.str("");
        errorBuffer.clear();
        assert( 0 == errorBuffer.str().size() );
        assert( errorBuffer.good() );
        errorBuffer   << "Error in RMMIX JDL Assembly Code File "
                  << filename << ", line number " << lineNumber
                  << ": " << suffix << std::endl;
        return  errorBuffer.str() ;
    }

    // private:
    int instructionNumber = 0;
    RMMIX_JDL::SymbolTable      jumpTable;
    virtual void prepareForState( stateType newState ); 
    virtual void buildJumpTable( ); 
        
    // protected:
    // Necessary virtual methods
    virtual bool getNumber(int &inputNumber);
    virtual bool getOpCode(int &opCode);

    virtual bool getOperand(int &operand);


    
};
