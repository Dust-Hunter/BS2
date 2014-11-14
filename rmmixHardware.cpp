// =====================================================================
// rmmixalHardware.cpp - Source code for essential symbol tables.
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
// Source File for rmmixalHardware.h
// See rmmixalHardware.h for more information
//
// =====================================================================

#include <iostream>
#include <fstream>
#include <string>   // for std::string
#include <sstream>  // for std::stringstream

// we need some basic knowledge about op codes & the like
#include "RMMIXJobLang.h"

// we need to know about the hardware to model it...
#include "rmmixHardware.h"

// we need to know something about the os to handle interrupts.
#include "rmminixos.h"


// Declare (allocate) the hardware models!!

// ====================================>>>  The Globals
std::ofstream  rmmixHardware::logStream; // There is only one, not one per instance!
int  rmmixHardware::clock       = 0;



rmmixCPU* theCPU = nullptr; // C++11!
std::map< int, rmmixHardware* > hardwareComponents; // global list of hardware

// ===================================>>>> C P U
// perform whatever instructions are loaded into InstructionMemory

void rmmixCPU::run( )
{


    if ( trapNumber )
        handleInterrupt( );
    else if ( registers[ 0 ] < 0 ) {
        log()<<"idle"<<std::endl;
		
    }
    else { // if instruction pointer is positive and no interrupt needs handling
         RMMIXinstruction instruction = instructionMemory[ registers[ 0 ] ];
	
	 executeInstruction(instruction);

    }; // end if instruction Pointer OK and no interrupt needs handling
} // end of run( )

// take care of traps (a.k.a. interrupts )

void rmmixCPU::handleInterrupt( )
{
    log() << " Interrupt handler - number " << trapNumber
          << " = " << RMMIX_JDL::lookup( RMMIX_JDL::trapCodes, trapNumber )
          << ", data " << trapData << std::endl;

    switch (trapNumber) {

    case RMMIX_JDL::HALT:
	rmminixOS::handleHALT(  );
	break;

    case RMMIX_JDL::GETW:
        rmminixOS::handleGETW( );
        break;

    case RMMIX_JDL::PUTW:
	 rmminixOS::handlePUTW(  );
         break;

    case RMMIX_JDL::FATAL:
        rmminixOS::handleFATAL(  );
        break;

    case RMMIX_JDL::GETW_READY:
        rmminixOS::handleGETW_READY(  );
        break;

    case RMMIX_JDL::PUTW_READY:
        rmminixOS::handlePUTW_READY(  );
        break;

    default: std::string err("Unknown Interrupt passed to HandleInterrupt");
        throw err;
    }; // end switch on trapNumber

} // end of handleInterrupt( )

// Arithmetical Logic Unit

void rmmixCPU::executeInstruction(RMMIXinstruction& instruction)
{

    log() << "execute @ addr " << registers[0]
          << " : " << instruction.dump() << std::endl;
    switch (instruction.fields[0]) // i.e. switch on opcode
    {
    case RMMIX_JDL::NOP: break; // nothing to do here

        // the simple arithmetic instructions

    case RMMIX_JDL::MOV:
        registers[ instruction.fields[1] ] = registers[ instruction.fields[2] ];
        break;
    case RMMIX_JDL::MOVI:
        registers[ instruction.fields[1] ] = instruction.fields[2];
        break;
    case RMMIX_JDL::ADD:
        registers[ instruction.fields[1] ] = registers[ instruction.fields[2] ]
                + registers[ instruction.fields[3] ];
        break;
    case RMMIX_JDL::ADDI:
        registers[ instruction.fields[1] ] = registers[ instruction.fields[2] ]
                + instruction.fields[3];
        break;
    case RMMIX_JDL::SUB:
        registers[ instruction.fields[1] ] = registers[ instruction.fields[2] ]
                - registers[ instruction.fields[3] ];
        break;
    case RMMIX_JDL::SUBI:
        registers[ instruction.fields[1] ] = registers[ instruction.fields[2] ]
                - instruction.fields[3];
        break;
    case RMMIX_JDL::MUL:
        registers[ instruction.fields[1] ] = registers[ instruction.fields[2] ]
                * registers[ instruction.fields[3] ];
        break;
    case RMMIX_JDL::MULI:
        registers[ instruction.fields[1] ] = registers[ instruction.fields[2] ]
                * instruction.fields[3];
        break;

        // slightly trickier arithmetic instructions
    case RMMIX_JDL::DIV:
        if (0 != registers[ instruction.fields[3] ])
            registers[ instruction.fields[1] ] = registers[ instruction.fields[2] ]
                / registers[ instruction.fields[3] ];
        else { // if division by zero
            assert( 0 == trapNumber );
            trapNumber = RMMIX_JDL::FATAL;
            trapData = 0;
        };
        break;
    case RMMIX_JDL::DIVI:
        if (0 != instruction.fields[3])
            registers[ instruction.fields[1] ] = registers[ instruction.fields[2] ]
                / instruction.fields[3];
        else { // if division by zero
            assert( 0 == trapNumber );
            trapNumber = RMMIX_JDL::FATAL;
            trapData = 0;
        };
        break;

        // The (conditional and unconditional) Jump Instructions
    case RMMIX_JDL::JMPI:
        registers[ 0 ] += instruction.fields[1];
        break;
    case RMMIX_JDL::BEQZI:
        if (0 == registers[ instruction.fields[1] ])
            registers[ 0 ] += instruction.fields[2];
        break;
    case RMMIX_JDL::BNEZI:
        if (0 != registers[ instruction.fields[1] ])
            registers[ 0 ] += instruction.fields[2];
        break;
    case RMMIX_JDL::BNEGI:
        if (0 > registers[ instruction.fields[1] ])
            registers[ 0 ] += instruction.fields[2];
        break;

        // Trigger an interrupt handler!
    case RMMIX_JDL::TRAP: // every instruction ends by incrementing the program counter
        assert( 0 == trapNumber );
        trapNumber = instruction.fields[1];
        trapData   = instruction.fields[2];
        break;

        // Data Memory Operations
        // WARNING: No Virtual Memory Yet!!!
    case RMMIX_JDL::LDWI:
        registers[ instruction.fields[1] ] = dataMemory[ instruction.fields[2] ];
        break;
    case RMMIX_JDL::LDW:
        registers[ instruction.fields[1] ] = dataMemory[ registers[ instruction.fields[2] ] ];
        break;
    case RMMIX_JDL::STWI:
        dataMemory[ instruction.fields[2] ] = registers[ instruction.fields[1] ];
        break;
    case RMMIX_JDL::STW:
        dataMemory[ registers[ instruction.fields[2] ] ] = registers[ instruction.fields[1] ];
        break;

        // If we ever get here, something's very wrong!
    default:
        std::string err("Illegal Op Code ");
        throw err;

    }; // end switch on opCOde

    // every instruction ends by incrementing the program counter
    registers[ 0 ]++;
} // end of executeInstricution( )

void rmmixInputDevice::run( ) {

    assert( (0 == trapNumber) || (RMMIX_JDL::GETW == trapNumber));
    if ( RMMIX_JDL::GETW == trapNumber ) {
        countDownTimer = inputDelay;
        // clear interrupt
        trapNumber = trapData = trapStatus = 0;
        log() << "starting delay" << std::endl;
	} else if ( countDownTimer ) {
        countDownTimer--;
        log() << "Delay down to " << countDownTimer << std::endl;
        if ( 0 == countDownTimer ) {
            assert( theCPU );
            // Is the CPU ready for this interrupt?
            if ( 0 != theCPU->trapNumber )
                countDownTimer = 1; // wait one more cycle...
            else { // if the CPU is ready
                theCPU->trapNumber = RMMIX_JDL::GETW_READY;
                trapNumber = 0; // clear my trapnumber

                // Read Number from decompiler object into MY trapData word!!!
                bool OK = ( *decompiler >> trapData );
		 // tell the CPU that she can pick up the data
                theCPU->trapStatus = ( !OK ); // if OK, set status to zero...
                theCPU->trapNumber = RMMIX_JDL::GETW_READY;
                theCPU->trapData = deviceNumber; // Tell the CPU which input
                                                 // Device is finished.
		log() << "signaled trap " << theCPU->trapNumber
                     << ", data = " <<       theCPU->trapData
                    << ", status = " <<      theCPU->trapStatus
                    << std::endl;
	
            }; // end if the CPU is ready
        }; // end if we just counted down to zero
    } else { // if countDownTimer == 0, do nothing
        log() << "idle." << std::endl;
    }


}

void rmmixOutputDevice::run( ) {

	
    assert( (0 == trapNumber) || (RMMIX_JDL::PUTW == trapNumber));
    if ( RMMIX_JDL::PUTW == trapNumber ) {
	 
	countDownTimer = outputDelay;
        buffer = trapData;


        // clear interrupt
        trapNumber = trapData = trapStatus = 0;
        log() << "starting delay, buffered reg[" << trapData
               << "] = " << buffer << std::endl;
    } else if ( countDownTimer ) {
	        
	countDownTimer--;
        log() << "Delay down to " << countDownTimer << std::endl;
        if ( 0 == countDownTimer ) {
            assert( theCPU );
            // Is the CPU ready for this interrupt?
            if ( 0 != theCPU->trapNumber )
                countDownTimer = 1; // wait one more cycle...
		
            else { // if the CPU is ready
		
                theCPU->trapNumber = RMMIX_JDL::PUTW_READY;
                theCPU->trapData = deviceNumber; // Tell the CPU which output
                   
                // Here is the actual output...
		
                bool OK = ( *outputSink << buffer << std::endl );
		
		
		theCPU->trapStatus = ( !OK ); // if OK, set status to zero...
                log() << "signaling trap " << theCPU->trapNumber
                      << ", status = "     << theCPU->trapStatus
                      << std::endl;
            }; // end if the CPU is ready
        }; // end if we just counted down to zero
    } else { // if countDownTimer == 0, do nothing
        log() << "idle." << std::endl;
	
    }


}



