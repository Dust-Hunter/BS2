// =====================================================================
// test.cpp - source code for (main() for) an RMMIX test program
//
// Author: Prof. Ronald Charles Moore
// Fachbereich Informatik - Dept. of Computer Science
// Hochschule Darmstadt   - University of Applied Sciences
//                          Darmstadt, Germany
//
// This File is part of the RMMIX Assembler/Simulator Package.
// Version 0.7, October 2014
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
// Source File for the rmmix test program.
// Unit Testing using Google Test
// This program is used to try out functions, classes and methods intended
// for the RMMIX assembler and simulator programs.  It is not essential
// for using the assembler and simulator - it is only for use when
// developing with the source code.
//
// By the way - this used to work with Google's C++ Testing Framework,
// but has been converted to Prof. Moore's own "UnitTesting.h" framework,
// which is a poort substitute, but is easier to install (it's just one
// header file).  Some tests could not be ported, and have been
// commented out (for historical interest).
// =====================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <vector> // needed for utility function acceptInput

#include "UnitTesting.h"

#include "RMMIXJobLang.h"
#include "RMMIXinstruction.h"

/*****
 * Utility Fuction parseObjFile
 * Tests if object file can be decompiled
 ****/
void parseObjFile( const char* fileName,
                   const char* jobName   ) {

    std::cout << std::endl << "TEST job control parsing from file "
                                                       << fileName << std::endl;

    // test what happens if a file can be opened.
    objectCodeDecompiler decompiler(fileName);
    ASSERTION_TEST(decompiler.good(), "File should be good" );

    EQUALITY_TEST( std::string(fileName), decompiler.filename ,
                   "decompiler should know its file name" );
    EQUALITY_TEST( std::string("<uninitialized>"), decompiler.jobname,
                  "decompiler should not yet know its job name" );

    // Now, see if we can "jump" to the $JOB...
    ASSERTION_TEST(decompiler.gotoState(JobLangCompiler::codeReaderState),
                   "Decompiler can jump to $JOB line" );
    EQUALITY_TEST( std::string(jobName), decompiler.jobname,
                  "decompiler should know its job name" );

    // ...and to $RUN
    ASSERTION_TEST(decompiler.gotoState(JobLangCompiler::inputReaderState),
                   "decompiler went to $RUN" );

    // ...and to $JOB again (this should NOT work)
    ASSERTION_TEST( ! decompiler.gotoState(JobLangCompiler::codeReaderState),
                    "decompiler cannot go back to $JOB" );

    // ... and now we should be at eof
    ASSERTION_TEST( decompiler.eof(), "Decompiler should be at EOF" );
    ASSERTION_TEST( ! decompiler.good(), "Decompiler should no longer be good" );

} // end parseObjFile

/*****
 * Utility Fuction acceptInput
 * Tests if input section of file can be read
 * (can be used with job files or obj files!)
 ****/
void acceptInput( JobLangCompiler* compiler, const std::vector< int >& answers ) {

    std::cout << std::endl << "TEST  input section of file " << compiler->filename
                                                                   << std::endl;

    int number = 123456789; // conspicuous number, never a legit input

    ASSERTION_TEST( compiler->good() , "Compiler should be good before testing." );
    ASSERTION_TEST( ! compiler->eof() , "Compiler should be not be at eof before testing." );

    for ( auto answer : answers ) {
        bool numberOK = ( *compiler >> number );
        ASSERTION_TEST( numberOK, "Was able to read input OK" );
        if (numberOK )
            EQUALITY_TEST( answer, number , "Expected Input" );
    }; // end new C++11-style range for loop!

    ASSERTION_TEST( ! compiler->eof() ,
         "Should not be at end of  file after reading all input numbers." );

    delete compiler ; // !!!

} // end acceptInput


/*****
 * Utility Fuction acceptInstructions
 * Tests if instructions can be compiled (assembled) from job file
 * (can be used with job files or obj files!)
 ****/
void acceptInstructions( JobLangCompiler* compiler,
                         const std::vector< const char* >& instrs ) {

    std::cout << std::endl << "TEST assembling instructions from file "
                                            << compiler->filename << std::endl;

    ASSERTION_TEST( compiler->good() , "Compiler should be good before testing." );
    ASSERTION_TEST( ! compiler->eof() ,
                          "Compiler  should be not be at eof before testing." );

    RMMIXinstruction instruction;

    for ( auto instrString : instrs ) {
       try {
            bool instructionOK = ( *compiler >> instruction );
            ASSERTION_TEST( instructionOK, "Could read instruction" );
            if (instructionOK ) {
                EQUALITY_TEST( std::string( instrString ), instruction.dump(),
                              "Read correct instruction" );
            }; // end if instruction OK
        } catch ( std::stringstream& errorPackage ) {
             ASSERTION_TEST( false , "Caught Exception of  ( std::stringstream type" );
        } catch ( ... ) {
            ASSERTION_TEST( false , "Caught Exception of ... type" );
        };
    }; // end range for over all instructions in vector

    ASSERTION_TEST( ! compiler->eof() ,
        "Should not be at end of  file after reading all input instructions." );

    ASSERTION_TEST( ! (*compiler >> instruction),
                      "Should not be able to continue reading instructions " );

    delete compiler ; // !!!

} // end acceptInstructions

int main(int argc, char** argv)
{
    std::cout << std::endl << "TEST TestRMMIX_JDL: OpCodes" << std::endl;

    EQUALITY_TEST( 0, int(RMMIX_JDL::NOP), "NOP should be numerically should be zero ");
    EQUALITY_TEST( std::string("NOP"),
                   RMMIX_JDL::lookup( RMMIX_JDL::opCodes, RMMIX_JDL::NOP ),
                   "NOP Symbol should be symbolically NOP " );

    ASSERTION_TEST( RMMIX_JDL::opCodeOK(RMMIX_JDL::NOP), "NOP is OK");
    ASSERTION_TEST( ! RMMIX_JDL::opCodeOK( -1 ) , "Op Code -1 is NOT OK");

    EQUALITY_TEST( 0, RMMIX_JDL::numberOfOperands(RMMIX_JDL::NOP), "NOP has 0 operands");
    EQUALITY_TEST( 2, RMMIX_JDL::numberOfOperands(RMMIX_JDL::MOVI), "MOVI has 2 operands");


    // Test case TestRMMIX_JDL; test pseudoOpCodes
    std::cout << std::endl << "TEST TestRMMIX_JDL: pseudoOpCodes" << std::endl;

    EQUALITY_TEST( 0xb, int(RMMIX_JDL::JMP),
                   "JMP should be numerically should be 11 ");
    EQUALITY_TEST( std::string("JMP"),
                   RMMIX_JDL::lookup( RMMIX_JDL::pseudoOpCodes, RMMIX_JDL::JMP ),
                   "JMP op code should be symbolically JMP ");
    EQUALITY_TEST( std::string("BNEG"),
                   RMMIX_JDL::lookup( RMMIX_JDL::pseudoOpCodes, RMMIX_JDL::BNEG ),
                   "JMP op code should be symbolically JMP " );



    // Test case TestRMMIX_JDL; test trapCodes
    std::cout << std::endl << "TEST TestRMMIX_JDL, trapCodes" << std::endl;

    EQUALITY_TEST( 1, int(RMMIX_JDL::HALT), "HALT should be numerically should be 1 " );
    EQUALITY_TEST( std::string("halt"),
               RMMIX_JDL::lookup( RMMIX_JDL::trapCodes, RMMIX_JDL::HALT ),
               "HALT trap code should be symbolically HALT " );

    EQUALITY_TEST(3, int(RMMIX_JDL::PUTW), "PUTW should be numerically should be 3 " );
    EQUALITY_TEST( std::string("putw"),
               RMMIX_JDL::lookup( RMMIX_JDL::trapCodes, RMMIX_JDL::PUTW ),
               "PUTW trap code should be symbolically PUTW " );

    EQUALITY_TEST(65, int(RMMIX_JDL::FATAL), "FATAL should be numerically should be 65 " );
    EQUALITY_TEST( std::string("FATAL"),
               RMMIX_JDL::lookup( RMMIX_JDL::trapCodes, RMMIX_JDL::FATAL ),
               "FATAL trap code should be symbolically FATAL " );

    ASSERTION_TEST(RMMIX_JDL::trapCodeOK(RMMIX_JDL::FATAL), "DUMP is an OK trap code" );
    ASSERTION_TEST(RMMIX_JDL::trapCodeOK(RMMIX_JDL::PUTW), "PUTW is an OK trap code" );
    ASSERTION_TEST( ! RMMIX_JDL::trapCodeOK( 0 ),
                    "Zero is not an OK trap code" );

    // TestCase TestRMMIX_JDL; test symbolTableLookup
    std::cout << std::endl << "TEST TestRMMIX_JDL, symbolTableLookup" << std::endl;

    EQUALITY_TEST( 3, RMMIX_JDL::lookup(RMMIX_JDL::opCodes, "ADD"),
                  "lookup( ADD ) -> 3" );
    EQUALITY_TEST( -1, RMMIX_JDL::lookup(RMMIX_JDL::opCodes, "FOO BAR"),
                "lookup( FOO BAR ) -> -1" );

    // Test TestRMMIXInstruction; test Basis
    std::cout << std::endl << "TEST TestRMMIXInstruction, Basis" << std::endl;

    // This test is named "Basis",
    // and belongs to the "TestRMMIXInstruction" test case.

    // Check blank constructor
    RMMIXinstruction ins0;
    ASSERTION_TEST(ins0.empty(), "simple instruction constructor returns empty instructions" );

    RMMIXinstruction ins1( 1, 3, 30, 40, 50);
    ASSERTION_TEST( ! ins1.empty(), "instruction constructor takes args" );

    std::string ins1dump = ins1.dump();
    EQUALITY_TEST( ins1dump,
     std::string( "Instruction:  op = MOV, 4 fields [0]=1 [1]=30 [2]=40 [3]=50"),
                              "instruction 1, 3, 30, 40, 50 constructed OK"   );

    // EXPECT_ASSERTION_FAILURE(ins0.reset( -42,   0 ));
    // EXPECT_ASSERTION_FAILURE(ins0.reset(  42,   0 ));
    // EXPECT_ASSERTION_FAILURE(ins0.reset(   1, -42 ));
    // EXPECT_ASSERTION_FAILURE(ins0.reset(   1,  42 ));
    // EXPECT_ASSERTION_FAILURE( { RMMIXinstruction ins2(  1, -42 ); } );
    // EXPECT_ASSERTION_FAILURE( { RMMIXinstruction ins3( 42,   0 ); } );

    // Test Case RMMIXJobLang; test objectCodeDecompiler class
    std::cout << std::endl << "TEST TestRMMIXJobLang, assemblyCompiler" << std::endl;

    // test what happens if a job file cannot be opened.
    assemblyCompiler asmFile("nonExistantFileName.bad");
    ASSERTION_TEST( !asmFile.good(), "Non-existant File bad after opening" );
    // test what happens if a job file CAN be opened
    assemblyCompiler test0Reader("tests/test0.job");
    ASSERTION_TEST(test0Reader.good(), "Existant File good after opening" );


    std::cout << std::endl << "TEST TestRMMIXJobLang, objectCodeDecompiler" << std::endl;

    // test what happens if an obj file cannot be opened.
    objectCodeDecompiler objFile("nonExistantFileName.bad");
    ASSERTION_TEST(  ! objFile.good(), "Object file good after opening" );

    // parse test files
    parseObjFile("tests/test0.ref",  "test0" );
    parseObjFile("tests/test0a.ref", "test0a" );
    parseObjFile("tests/test0b.ref", "test0b" );
    parseObjFile("tests/test0c.ref", "test0c" );
    parseObjFile("tests/test1.ref",  "test1" );
    parseObjFile("tests/test2a.ref", "test2a" );
    parseObjFile("tests/test2b.ref", "test2b" );

    // check input from ref files
    std::cout << std::endl << "TEST TestRMMIXJobLang, objInputAcceptor" << std::endl;
    acceptInput( new objectCodeDecompiler("tests/test1.ref"),  std::vector< int >{ 42 } );
    acceptInput( new objectCodeDecompiler("tests/test2a.ref"), std::vector< int >{ 42, 24 } );
    acceptInput( new objectCodeDecompiler("tests/test2b.ref"), std::vector< int >{ 42, 24 } );
    acceptInput( new objectCodeDecompiler("tests/test3.ref"),  std::vector< int >{ 24, 42, 40, -1 } );

    // check input from job files
    std::cout << std::endl << "TEST TestRMMIXJobLang, asmInputAcceptor " << std::endl;
    acceptInput( new assemblyCompiler("tests/test1.job"),  std::vector< int >{ 42 } );
    acceptInput( new assemblyCompiler("tests/test2a.job"), std::vector< int >{ 42, 24 } );
    acceptInput( new assemblyCompiler("tests/test2b.job"), std::vector< int >{ 42, 24 } );
    acceptInput( new assemblyCompiler("tests/test3.job"),  std::vector< int >{ 24, 42, 40, -1 } );


    std::cout << std::endl << "TEST TestRMMIXJobLang, objInstructionsAcceptor " << std::endl;

    // check input from ref files
    acceptInstructions( new objectCodeDecompiler( "tests/test0b.ref" ),
                        std::vector< const char* >{
                      "Instruction:  op = MOV, 3 fields [0]=1 [1]=1 [2]=2",
                      "Instruction:  op = MOVI, 3 fields [0]=2 [1]=3 [2]=42" }
                      ) ;

    acceptInstructions( new objectCodeDecompiler( "tests/test1.ref" ),
                        std::vector< const char* >{
                      "Instruction:  op = TRAP, 3 fields [0]=15 [1]=2 [2]=10",
                      "Instruction:  op = TRAP, 3 fields [0]=15 [1]=3 [2]=10",
                      "Instruction:  op = MOVI, 3 fields [0]=2 [1]=30 [2]=0",
                      "Instruction:  op = TRAP, 3 fields [0]=15 [1]=1 [2]=30"}
                     ) ;

  acceptInstructions( new objectCodeDecompiler( "tests/test2b.ref" ),
                        std::vector< const char* >{
                      "Instruction:  op = TRAP, 3 fields [0]=15 [1]=2 [2]=10",
                      "Instruction:  op = TRAP, 3 fields [0]=15 [1]=2 [2]=11",
                      "Instruction:  op = SUB, 4 fields [0]=5 [1]=12 [2]=10 [3]=11",
                      "Instruction:  op = BNEGI, 3 fields [0]=14 [1]=12 [2]=1",
                      "Instruction:  op = MOV, 3 fields [0]=1 [1]=11 [2]=10",
                      "Instruction:  op = TRAP, 3 fields [0]=15 [1]=3 [2]=11",
                      "Instruction:  op = MOVI, 3 fields [0]=2 [1]=30 [2]=0",
                      "Instruction:  op = TRAP, 3 fields [0]=15 [1]=1 [2]=30"
                                                  } );


    std::cout << std::endl << "TEST TestRMMIXJobLang, asmInstructionsAcceptor " << std::endl;

    // check input from job files
    acceptInstructions( new assemblyCompiler( "tests/test0b.job" ),
                        std::vector< const char* >{
                      "Instruction:  op = MOV, 3 fields [0]=1 [1]=1 [2]=2",
                      "Instruction:  op = MOVI, 3 fields [0]=2 [1]=3 [2]=42" }
                      );

    acceptInstructions( new assemblyCompiler( "tests/test1.job" ),
                        std::vector< const char* >{
                      "Instruction:  op = TRAP, 3 fields [0]=15 [1]=2 [2]=10",
                      "Instruction:  op = TRAP, 3 fields [0]=15 [1]=3 [2]=10",
                      "Instruction:  op = MOVI, 3 fields [0]=2 [1]=30 [2]=0",
                      "Instruction:  op = TRAP, 3 fields [0]=15 [1]=1 [2]=30"}
                      );

    acceptInstructions( new assemblyCompiler( "tests/test2b.job" ),
                        std::vector< const char* >{
                      "Instruction:  op = TRAP, 3 fields [0]=15 [1]=2 [2]=10",
                      "Instruction:  op = TRAP, 3 fields [0]=15 [1]=2 [2]=11",
                      "Instruction:  op = SUB, 4 fields [0]=5 [1]=12 [2]=10 [3]=11",
                      "Instruction:  op = BNEGI, 3 fields [0]=14 [1]=12 [2]=1",
                      "Instruction:  op = MOV, 3 fields [0]=1 [1]=11 [2]=10",
                      "Instruction:  op = TRAP, 3 fields [0]=15 [1]=3 [2]=11",
                      "Instruction:  op = MOVI, 3 fields [0]=2 [1]=30 [2]=0",
                      "Instruction:  op = TRAP, 3 fields [0]=15 [1]=1 [2]=30"
                                                  } );

    std::cout << std::endl
              << "\tFinished with all tests." << std::endl
              <<  ( testing::AllTestsSuccessful ? "\tAll tests passed!"
                                                : "\tSOME TESTS FAILED!!!" )
              << std::endl << std::endl;

   return 0; // Everything OK
}; // end main


