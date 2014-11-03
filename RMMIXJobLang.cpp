// =====================================================================
// RMMIXJobLang.h - Source Code file for the RMMIX Job Description Language 
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

#include <cctype> // for isspace
#include <sstream> // for std::stringstream

#include "RMMIXJobLang.h"

// Before the methods start, a few utilities (functions)
// in an anonymous namespace (so that they are not exported)
namespace {

void cleanupLine( std::string& line ) // static by the way
{
    if (line.empty()) return; // unnecessary but saves time
    
    const char comma = ',';
    const char space = ' ';
    const char commentDelimiter = '%';
    auto iter = line.begin(); // Note C++11 "auto" declaration

    // remove leading white space
    while ((iter != line.end())
            && std::isspace(*iter)) // includes spaces, tabs, etc.
        iter = line.erase(iter);
    if (line.empty()) return; // unnecessary but saves time

    // search for a comment character commentDelimiter == '%')
    // and replace commas with spaces along the way
    while ((iter != line.end())
            && (commentDelimiter != *iter)) {
        if (comma == *iter) *iter = space;
        ++iter;
    }; // end while not at end or at comment character
    // Either iter == line.end OR RMMIXcommentDelimiter == *iter
    if (iter != line.end())
        line.erase(iter, line.end());

    // remove trailing white space
    while ( !line.empty() && std::isspace(line.back()) )
        line.pop_back();


} // end cleanupLineBuffer

bool containsJobControl(const std::string &line, const std::string &pattern)
{
    // basic sanity check
    if (line.empty()) return false; // unnecessary but saves time
    if ('$' != line[0]) return false;
    // OK, there could be the right job control line here...
    std::stringstream sstrm(line);
    std::string contents;
    if (!(sstrm >> contents)) return false;
    // else... 
    bool result = (contents == pattern);
    return result;
} // end containsJobControl

int decodeOpSymbol( const std::string& opSym ) {
    int opCode = RMMIX_JDL::lookup( RMMIX_JDL::opCodes, opSym );
    if ( RMMIX_JDL::opCodeOK( opCode ) ) 
        return opCode;
    else {
        opCode = RMMIX_JDL::lookup( RMMIX_JDL::pseudoOpCodes, opSym );
        if ( RMMIX_JDL::opCodeOK( opCode ) ) 
            return opCode;
        else // if not a valid pseudoOpCode
            return -1;
    }; // if not in opCodes table
} // end decodeOpSymbol


} // end anonymous namespace

// Begin Methods for theJobLangCompiler class and its extensions

bool JobLangCompiler::refillLineBuffer()
{   
    // Start with a clean slate.
    lastOperationOK = true; // until something goes wrong...
    lineBuffer.clear();
    lineBuffer.str("");
    // Get a line from the file, clean it up (remove comments, etc.).
    // If the line (after cleaning) is blank, repeat (unless we hit eof)
    std::string line;
    do {
        if ( inStream.eof() ) 
            return (lastOperationOK = false); // no point going on if at eof...
        // else, if not yet at eof, get a line of text (not a token!)
        lastOperationOK = ( std::getline( inStream, line ) );
        if ( lastOperationOK ) {
            lineNumber++; // count lines!
            cleanupLine( line );
        };
    } while( lastOperationOK && line.empty() ); 
    
    // if last operation OK, store line in lineBuffer
    if (lastOperationOK) {
        lineBuffer.clear( );
        lineBuffer.str( line );
    };
//    std::cout << "End of refillLineBuffer, lineBuffer = <" << lineBuffer.str()
//              << ">." << std::endl;
    return lastOperationOK; // == true
} // end refillLineBuffer

// Instruction reader

JobLangCompiler& JobLangCompiler::operator>>(RMMIXinstruction& instruction) {
    lastOperationOK = true; // until something goes wrong
    if (gotoState( codeReaderState)) 
        if ( refillLineBuffer( ) ) {
            lastOperationOK = ! containsJobControl( lineBuffer.str(), 
                                                    "$RUN" );
            if (lastOperationOK)
                getInstruction(instruction); 
        };
    return (*this);
}

// Input reader

JobLangCompiler& JobLangCompiler::operator>>(int& inputNumber) {
    lastOperationOK = true; // until something goes wrong...
    if (gotoState( inputReaderState)) 
        if ( refillLineBuffer( ) ) {
            lastOperationOK =  ! containsJobControl( lineBuffer.str(), 
                                                     "$END" );
            if (lastOperationOK)
                getNumber(inputNumber); 
        };
    return (*this);
}

bool JobLangCompiler::getToken( std::string& token )
{
    lastOperationOK = ( lineBuffer >> token ); 
    return lastOperationOK;
} // end getToken


bool objectCodeDecompiler::getNumber(int &inputNumber)
{
    lastOperationOK = (  lineBuffer >> std::hex >> inputNumber  ); 
//    std::cout << "end of obj get number, num = " << inputNumber
//              << ", OK = " << lastOperationOK << std::endl;
    return lastOperationOK;
} // end getNumber

bool assemblyCompiler::getNumber(int &inputNumber)
{
    lastOperationOK = ( lineBuffer >> std::dec >> inputNumber ); 
//    std::cout << "end of asm get number, num = " << inputNumber
//              << ", OK = " << lastOperationOK << std::endl;
    return lastOperationOK;
} // end getNumber


bool assemblyCompiler::getOpCode(int &opCode)
{
    std::string opSym;
    lastOperationOK = getToken( opSym );
    if ( lastOperationOK ) {
        opCode = decodeOpSymbol( opSym );
        lastOperationOK = RMMIX_JDL::opCodeOK( opCode );
        if ( !lastOperationOK ) {
            // if it's not a valid op code,
            // we (apparently) found an instruction label.
            int jumpLocation = RMMIX_JDL::lookup( jumpTable, opSym );
            if ( jumpLocation < 0 ) {
                jumpTable[ opSym ] = instructionNumber;
            }
            else if ( jumpLocation != instructionNumber ) {
                std::stringstream suffix;
                suffix << "Ambiguous Label: " << opSym
                       << " can mean instruction " << instructionNumber
                       << " or " << jumpLocation
                       << ", line: "
                       << lineBuffer.str();
                throw RMMIX_JDLexception( suffix.str() );
            };
            // if we're here, label found, and OK.
            // After a label, we expect an op symbol
            lastOperationOK = getToken( opSym );
            // label can also be on the next line...
            if ( !lastOperationOK ) {
                lastOperationOK = refillLineBuffer() && getToken( opSym );
            }; // end if next token not found on same line as label
            // Now our patience is exhausted...
            if ( ! lastOperationOK ) {
                throw RMMIX_JDLexception( 
                        std::string( "Apparent label " )+ opSym +
                        std::string( " not followed by instruction, current line buffer = ")
                        + lineBuffer.str() );
            } else { // if lastOperationOK
                opCode = decodeOpSymbol( opSym );
                lastOperationOK = RMMIX_JDL::opCodeOK( opCode );
            }; // end if token found after label
        }; // end if label found
    }; // end is getToken worked
    // Update instructionNumber - this would be better in getInstruction,
    // but we only want to do it for assemblyCompiler class... so... do it here.
    if (lastOperationOK) ++instructionNumber;
    return lastOperationOK;
} // end getOpCode

bool assemblyCompiler::getOperand(int &operand)
{
    std::string opSym;
    lastOperationOK = getToken( opSym );
    if ( lastOperationOK ) {
        operand = RMMIX_JDL::lookup( RMMIX_JDL::trapCodes, opSym );
        if( ! RMMIX_JDL::trapCodeOK( operand ) )
        {
            // Is opSym a number?
            lastOperationOK = ( std::stringstream( opSym ) >> operand );
            if ( !lastOperationOK ) { // not a number
                int landingPad = RMMIX_JDL::lookup( jumpTable, opSym );
                operand = landingPad - instructionNumber;
                lastOperationOK = true;
            }; // end if not a number
        };
    }; // end is getToken worked
    return lastOperationOK;

} // end getOperand

bool JobLangCompiler::gotoState(stateType newState)
{
    // We are only prepared to switch into codeReaderState or inputReaderState
    assert((codeReaderState == newState) || (inputReaderState == newState));

    std::string token;
    const std::string targetKeyword( (codeReaderState == newState) ? 
                                     "$JOB" : "$RUN");

    // If we're already in the right state, there's nothing to do
    while ( newState != state ) {
        lastOperationOK = containsJobControl( lineBuffer.str(), targetKeyword );
        if ( ! lastOperationOK ) {
            // We found something else, not what we were looking for
            // If we can read another line, good, keep searching, 
            // otherwise return false
            if ( ! refillLineBuffer() ) 
                return ( lastOperationOK = false );
        } else { // if we found targetKeyWord
            prepareForState( newState );
            state = newState;
            if ( codeReaderState == newState ) {
                if ( !getToken( token ))
                    std::cerr << "Serious logic error at file "
                            << __FILE__ << ":" << __LINE__
                            << std::endl;
                if ( !getToken( token ) ) 
                    throw RMMIX_JDLexception(
                            std::string( "Could not find job name in line: " )
                            + lineBuffer.str() );
                // else, if job name found...
                jobname = token;
                buildJumpTable( );
            }; // end if looking for "$JOB"
        }; 
        // else if we've found some other token - just skip over it!
    }; // end while not yet in newState
    return lastOperationOK;
} // end gotoState

void assemblyCompiler::prepareForState( stateType newState ) {
    if ( codeReaderState == newState ) {
        // start with a clean slate
        instructionNumber = 0;
        jumpTable.clear(); 
    };
}; // end prepareForState();

void assemblyCompiler::buildJumpTable( ) {
 
    if ( !good() || eof() ) return; // cannot proceed if not good or at eof

    // Store state, so we can restore it later
    int originalLineNumber = lineNumber;
    int originalInstrNumber = instructionNumber;
    int placeMarker = inStream.tellg(); // so we can rewind to here
    std::string lineBufferString = lineBuffer.str( );
    
    RMMIXinstruction tempInstruction;
    bool moreToDo = true; // until we find we're finished
    while ( moreToDo ) {
        try {
            moreToDo = ( (*this) >> tempInstruction );
        } catch ( std::string errorString ) {
            // Suppress errors in this phase!!
//            std::cout << "Suppressing exception <" << errorString 
//                      << std::endl;
        }; 
    }; // end while moreToDo
    // restore state
    lineNumber = originalLineNumber;
    instructionNumber = originalInstrNumber;
    inStream.seekg( placeMarker );
    lastOperationOK = true;
    lineBuffer.str( ) = lineBufferString;

}; // end buildJumpTable

bool JobLangCompiler::getInstruction(RMMIXinstruction& instruction)
{
    // The opcode and various fields of the instruction are stored first in 
    // local variables, and put into the instruction only if everything was
    // parsed correctly.
    int opCode = RMMIX_JDL::NOP;
    lastOperationOK = getOpCode( opCode );

    if ( ! lastOperationOK )  
        throw RMMIX_JDLexception( 
                      std::string( "Could not find legal operation in line: " )
                      + lineBuffer.str()  );
    else { // if lastOperationOK 
        int numOperands = int(RMMIX_JDL::numberOfOperands( opCode ));
        int operands[3] = {0, 0, 0};
        for (int opNum = 0; lastOperationOK && (opNum < numOperands); ++opNum) {
            lastOperationOK = getOperand( operands[ opNum ] );
        }; // end for 0 <= opNum < numOperands

        if ( ! lastOperationOK ) 
                throw RMMIX_JDLexception( 
                      std::string("Could not find required operands in line: " )
                      + lineBuffer.str()  );
        else { // if all required operands present
            // check if there are TOO MANY operands
            int dummy;
            lastOperationOK = ( ! getOperand( dummy ) );
            if ( ! lastOperationOK ) 
                throw RMMIX_JDLexception(  
                        std::string( "Too many operands in line: " )
                        +lineBuffer.str() );
            else  // if exactly the right number of oprands present            
                instruction.reset( opCode, numOperands,
                                   operands[0], operands[1], operands[2] );
        }; // end if all required operand found
    }; // return if OK
//    std::cout << "Done getting instruction, OK = "  << lastOperationOK;
//    if ( lastOperationOK ) std::cout << " instr = " << instruction.dump();
//    std::cout << std::endl;
    return lastOperationOK;
} // end getInstruction


