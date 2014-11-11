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

#define JobFinished -3
#define noJobLeft -1
#define hasNotBeenBooted -2

#define _trapNumber 0
#define _trapData 1
#define _trapStatus 2
#define _inputData 3
#define _regToUpdate 4

// =====================================================================
//             OPERATING SYSTEM DATA STRUCTURES
//
// Put all data structures that the Operating System needs HERE!
//
// At the moment, our operating system only need two int variables,
// but we'll almost certainly need a lot more later, especially when we
// get to multiple processes. Feel free to add whatever is necessary!

objectCodeDecompiler* decompiler;
int currentJobIndex;
std::vector<std::vector<RMMIXinstruction>>* programmMem;
std::vector<std::vector<int>>* registerMem;
std::vector<char*>* argVector;
std::vector<int>* subJobVector;
std::vector<objectCodeDecompiler*>* obcVector;
std::vector<std::vector<int>>* trapRegMem;

//change this if possible cause is terrible and I feel bad for doing it
std::ofstream os0;
std::ofstream os1;
std::ofstream os2;
std::ofstream os3;
std::vector<std::ofstream*>* osVector;




// =====================================================================
//             INTERRUPT HANDLERS
// =====================================================================

// =====================================================================
//       Halt and Fatal - Interrupts that end a process

void rmminixOS::handleHALT( )
{

    int status = theCPU->registers[ theCPU->trapData ];
    rmmixHardware::logStream << "Simulation Halt! Status = ";
    
    if(rmminixOS::loadNextProgramm()){
     return;
    }else if(switchProgramm()){
     return;
    }else{
exit(status);
}


} // end handleHALT

void rmminixOS::handleFATAL( )
{
    rmmixHardware::logStream << "FATAL Interrupt!!" << std::endl;
    std::cerr << "FATAL Interrupt!!" << std::endl;
    // This is OK if we only want to run one program -
    // We need to extend this to handle multiprogramming!
   
	if(rmminixOS::loadNextProgramm()){
     return;
    }else{
	//current programm failed so we have to mark is as finished
	setPCof(currentJobIndex,JobFinished);
		if(switchProgramm()){
			return;
		}else{
			  exit( theCPU->trapData );
		}
   
    }
    
} // end handleFATAL

bool rmminixOS::isCurrentJobDone(){
//check if the pc is at the last instruction of the current Programm	
	if(theCPU->registers[ 0 ]==programmMem->at(currentJobIndex).size()){
		return true;
	}else{
		return false;
	}

}

void rmminixOS::removeCurrentJob(){
	//close the old os stream
	assert( hardwareComponents[(currentJobIndex+1)*2] ); // is not null
	
	//close the file in which the ostream is writing, change this line if something more readable is possible
	//osVector->at(currentJobIndex)->close();
        	
	setPCof(currentJobIndex,JobFinished);

}

bool rmminixOS::switchProgramm(){
	std::cout<<"current job is "<<currentJobIndex<<std::endl;
	//check if the current job is done
	if(isCurrentJobDone()){
		removeCurrentJob();
	}

	//check if another job is waiting
	int nextJobIndex = getNextWaitingJob();
       	if(nextJobIndex!=noJobLeft&&nextJobIndex!=currentJobIndex){
		std::cout<<"changing to "<<nextJobIndex<<std::endl;
		executeJobChange(nextJobIndex);
	
		return true;
	}else{
		
		return false;
	}

}

void rmminixOS::restoreRegState(int nextJobIndex){
	for(int i=0;i<registerMem->at(0).size();i++){
		theCPU->registers.at(i)=registerMem->at(nextJobIndex).at(i);
		
	}
restoreTrapRegs(nextJobIndex);

}



void rmminixOS::executeJobChange(int nextJobIndex){
	//check if the next job is the current Job, in that case nothing has to be done	
	
	
	//save the pc of the old job, but only if the pc says the jo is done
	//also in that case save the registers
	if(getPCof(currentJobIndex)!=JobFinished){
	
	saveRegisters();
	}

	restoreRegState(nextJobIndex);
	//hardwareComponents[0]->trapNumber=0;
	
	//check if the next job has been booted
	if(hasBeenBooted(nextJobIndex)){
		std::cout<<"booting "<<nextJobIndex<<std::endl;	
		bootProgramm(nextJobIndex);
		std::cout<<"booting worked"<<std::endl;
		
	}else{
		
		restoreInstructionMem(nextJobIndex);
		currentJobIndex=nextJobIndex;
		
	
	}
}

void rmminixOS::clearInterrups(int jobIndex){
trapRegMem->at(jobIndex).at(_trapNumber) = 0;
trapRegMem->at(jobIndex).at(_trapData) = 0;
trapRegMem->at(jobIndex).at(_trapStatus) = 0;

}

void rmminixOS::restoreInstructionMem(int nextJobIndex){
	theCPU->instructionMemory.clear();
	for(int i = 0;i<programmMem->at(nextJobIndex).size();i++){
		theCPU->instructionMemory[i] = programmMem->at(nextJobIndex).at(i);
	}
}

bool rmminixOS::hasBeenBooted(int nextJob){
	if(getPCof(nextJob)==hasNotBeenBooted){
		return true;
	}else{
		return false;
	}

}

void rmminixOS::saveRegisters(){
	for(int i=0;i<registerMem->at(0).size();i++){ 
	
		//if the cpu was set to idle via -1 dont save this,
		//the currect pc was saves bevore the cpu was set to idle
		if(i!=0&&registerMem->at(currentJobIndex).at(i)!=-1){
			registerMem->at(currentJobIndex).at(i)= theCPU->registers.at(0);
		}
	}
	saveTrapRegs();

}

int rmminixOS::getNextWaitingJob(){
	//check the entire list
	for(int i=0;i<registerMem->size();i++){
		if(getPCof((currentJobIndex+1+i)%registerMem->size())!=JobFinished){
			return (currentJobIndex+1+i)%registerMem->size();
		}
	}
	return noJobLeft;
}


std::string rmminixOS::getOutputFilename(int jobIndex){

	std::string filename = "Mainjob";
	filename.append(std::to_string(jobIndex));
        filename.append("Subjob");
        filename.append(std::to_string(subJobVector->at(currentJobIndex)));
	filename.append(".txt");	
	return filename;

}

void rmminixOS::restoreTrapRegs(int nextJobIndex){
theCPU->trapNumber = trapRegMem->at(nextJobIndex).at(_trapNumber);
theCPU->trapData = trapRegMem->at(nextJobIndex).at(_trapData); 
theCPU->trapStatus = trapRegMem->at(nextJobIndex).at(_trapStatus);


}

void rmminixOS::saveTrapRegs(){
trapRegMem->at(currentJobIndex).at(_trapNumber) = theCPU->trapNumber;
trapRegMem->at(currentJobIndex).at(_trapData) = theCPU->trapData;
trapRegMem->at(currentJobIndex).at(_trapStatus) = theCPU->trapStatus;

}

void rmminixOS::storeInput(int input,int inputDeviceNumber){

//calculate the job index depending on the input device
int jobIndex = inputToJobIndex(inputDeviceNumber);
//check if the job that is running atm is the one that wanted the input
if(jobIndex==currentJobIndex){
//current job wants the input
 theCPU->registers[ trapRegMem->at(jobIndex)[_regToUpdate]] = input;
                                 
}else{
//this is needed in case the job hat wanted that input is not the one that is running atm
	registerMem->at(jobIndex)[trapRegMem->at(jobIndex)[_regToUpdate]] = input;
}





}


int rmminixOS::inputToJobIndex(int deviceNumber){
return ((deviceNumber+1)/2)-1;
}

int rmminixOS::outputToJobIndex(int deviceNumber){
return (deviceNumber/2)-1;
}	
 

bool rmminixOS::loadNextProgramm(){

	//close the old os stream
	assert( hardwareComponents[(currentJobIndex+1)*2] ); // is not null
        //close the file in which the ostream is writing, change this line if something more readable is possible
	osVector->at(currentJobIndex)->close();  
//dynamic_cast<std::ofstream*>((dynamic_cast<rmmixOutputDevice*>((hardwareComponents[(currentJobIndex+1)*2 ])))->outputSink)->close();
	
	//try loading another programm	
	//check for another job line
	if(decompiler->gotoState( JobLangCompiler::codeReaderState  )){
			
		if(!rmminixOS::load(*decompiler,currentJobIndex)){
		return false;		
		}
		
		subJobVector->at(currentJobIndex)=subJobVector->at(currentJobIndex)+1;
		
		
		//OPEN A NEW OS STREAM
		 
    		
    		//osVector->at(currentJobIndex)->open(getOutputFilename(currentJobIndex));  
		
		//rebind io components
 		assert( hardwareComponents[ ((currentJobIndex+1)*2)-1 ] ); // is not null
        	hardwareComponents[((currentJobIndex+1)*2)-1 ]->bind( decompiler );
    		assert( hardwareComponents[(currentJobIndex+1)*2] ); // is not null
                hardwareComponents[(currentJobIndex+1)*2 ]->bind( (osVector->at(currentJobIndex)) ); // bind to std out
		//trap number fuer neustart auf initzialwert setzten		
		hardwareComponents[0]->trapNumber=0;

		
		return true;
	}else{
		//failed to relaod
		return false;	
	}

}

// =====================================================================
//           Load -
//     Loads instructions into Instruction memory, by reading an object file.
//     It SHOULD be possible to use this multiple times, but that has not
//     been tested.
//     Returns true if and only if successful

bool rmminixOS::load( objectCodeDecompiler& decompiler,int programmIndex )
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
	    programmMem->at(programmIndex).push_back(instruction);

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


bool rmminixOS::bootProgramm(int programmIndex){
   
    theCPU->instructionMemory.clear();
   

    // SET UP INPUT
    // try to open a decompiler with a given file name
    decompiler = new objectCodeDecompiler(argVector->at(programmIndex));
    // We call new instead of using a local variable so that the
    // decompiler object survives the call to this function
     // (it will be used later by the intput device object).

    // Basic error checking (argv arguments are often bad, so be careful)
    assert( decompiler ); // is not null
    if ( ! decompiler->good() ) {
        std::cerr << "Could not open object file with name "  << argVector->at(programmIndex)
                  << std::endl;
        return false;
    };
    if ( decompiler->eof() ) {
        std::cerr << "Object file with name "  << decompiler->filename
                 << " appears to be empty."   << std::endl;
        return false;
    };

    // Load the instruction memory
    if ( ! rmminixOS::load( *decompiler,programmIndex )) {
        std::cerr << "BOOT LOAD FAILED - File name " << argVector->at(programmIndex) << std::endl;
        return false;
    } else { // if load was successful

        assert( decompiler->good() ); // should still be OK
        assert( ! decompiler->eof() ); // should not be at eof (or can it?)
        obcVector->push_back(decompiler);


        assert( hardwareComponents[ ((programmIndex+1)*2)-1 ] ); // is not null
        hardwareComponents[((programmIndex+1)*2)-1]->bind( obcVector->back() );

    }; // end if load successful

    // SET UP OUTPUT
    assert( hardwareComponents[(programmIndex+1)*2] ); // is not null
    currentJobIndex=programmIndex;
   
   // osVector->at(currentJobIndex)->open(getOutputFilename());   

    //hardwareComponents[((programmIndex+1)*2)]->bind( (osVector->at(currentJobIndex)) ); // bind to std out
hardwareComponents[((programmIndex+1)*2)]->bind(&(std::cout));
    setPCof(programmIndex,0);

    


    
return true;
}

void rmminixOS::setPCof(int programmIndex,int newPC){
registerMem->at(programmIndex).at(0)=newPC;
return;
}

int rmminixOS::getPCof(int programmIndex){
return registerMem->at(programmIndex).at(0);

}
// =====================================================================
//           Boot -
//     Construct a decompiler, and bind it to the input device.
//     Bind standard out to the output device.
//     This will have to be changed when we go to multiple I/O devices...
//     Returns true if and only if everything booted OK
bool rmminixOS::boot(int argc,char *argv[]) {
        currentJobIndex = 0;
       

	programmMem = new std::vector<std::vector<RMMIXinstruction>>();
	argVector = new std::vector<char*>();
	registerMem = new std::vector<std::vector<int>>();
	subJobVector = new std::vector<int>(5,0);
	obcVector = new std::vector<objectCodeDecompiler*>();
	trapRegMem = new std::vector<std::vector<int>>();
	
	//this should be changed if a better solution pops up
	//put atm i can think of anything else
	//Problem: ofstream's dont have copy constructor that are needed by the vector
	//and vec->push_back(std::ofstream()) or without () doesnt work       	
		
	osVector = new std::vector<std::ofstream*>();
	osVector->push_back(&os0);
	osVector->push_back(&os1);
	osVector->push_back(&os2);
	osVector->push_back(&os3);
	
	
	for(int i=0;i<argc-1;i++){
        programmMem->push_back(std::vector<RMMIXinstruction>());
	//Note: where in argc the first programm has the index 1, in argVector it will be 0	
	trapRegMem->push_back(std::vector<int>(4,0));
	argVector->push_back(argv[i+1]);
	registerMem->push_back(std::vector<int>(32,0));
	setPCof(i,hasNotBeenBooted);
	
        }
 
	return bootProgramm(0);
      
} // end boot


// =====================================================================
//                                  Input and Output
// Note that I/O is two phased - the CPU sends a request to the device
// (either a GETW or a PUTW interrupt), then the device sends an interrupt
// to say that it is finished (either a GETW_READY ora PUTW_READY).

// Input, Phase 1 (CPU requests input)
void rmminixOS::handleGETW(  )
{
	std::cout<<"call get W"<<std::endl;
    assert( theCPU ); // i.e. assert that theCPU is not a null pointer
    // Save data we will need later
    if ( 0 <= theCPU->registers[0] ) {
	//save the pc
        setPCof(currentJobIndex,theCPU->registers[ 0 ]);
     trapRegMem->at(currentJobIndex)[_regToUpdate] = theCPU->trapData;
	// Put the CPU in an idle state until PUTW_READY signal
        theCPU->registers[ 0 ] = -1; // make the cpu wait!
	
    };

    // Signal the input device
    // Note that the choice of device is hard-coded - we always read from
    // device 1! (This will have to change)
    assert( hardwareComponents[((currentJobIndex+1)*2)-1] );
    if ( 0 == hardwareComponents[((currentJobIndex+1)*2)-1]->trapNumber ) {
        hardwareComponents[((currentJobIndex+1)*2)-1]->trapNumber = RMMIX_JDL::GETW;
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
	
	std::cout<<"getWReady"<<std::endl;
	
    assert( theCPU ); // i.e. assert that theCPU is not a null pointer

    // The hardware has signaled that the get-word operation is done.
    if ( 0 != theCPU->trapStatus ) {
        // trigger fatal interrupt (crash current process)
	     
	theCPU->trapNumber = RMMIX_JDL::FATAL;
    } else { // if OK status
	
        int inputDevice = theCPU->trapData;
	//assert( 1 == inputDevice ); // THIS MUST BE CHANGED LATER!
        assert( hardwareComponents[ inputDevice ] ); // not null
	
        // Get data from input device
	storeInput(hardwareComponents[ inputDevice ]->trapData,inputDevice);
	
	clearInterrups(inputToJobIndex(inputDevice));
	
	theCPU->trapData=theCPU->trapStatus=theCPU->trapNumber=0;
	hardwareComponents[ inputDevice ]->trapData = hardwareComponents[ inputDevice ]->trapStatus = hardwareComponents[ inputDevice ]->trapNumber = 0;
	
	//this only happens if only 1 job is left and it was waiting
        if(theCPU->registers[0]== -1){
		theCPU->registers[0]=getPCof(currentJobIndex);
	
	}
	
    
    };
} // end handleGETW_READY

// Output, Phase 1 (CPU requests output)
void rmminixOS::handlePUTW( )
{
osVector->at(currentJobIndex)->open(getOutputFilename(currentJobIndex));
	std::cout<<"putW called by"<<currentJobIndex<<std::endl;
    assert( theCPU ); // i.e. assert that theCPU is not a null pointer
    // Save data we will need later
    if ( 0 <= theCPU->registers[0] ) {
        //save the pc
        setPCof(currentJobIndex,theCPU->registers[ 0 ]);
      
        // Put the CPU in an idle state until PUTW_READY signal
	// Put the CPU in an idle state until PUTW_READY signal
        theCPU->registers[ 0 ] = -1; // make the cpu wait!
       
    };

    // Signal the output device
    // Note that the choice of device is hard-coded - we always write to
    // device 2!

    assert( hardwareComponents[((currentJobIndex+1)*2)] );
    if ( 0 == hardwareComponents[((currentJobIndex+1)*2)]->trapNumber ) {
        hardwareComponents[((currentJobIndex+1)*2)]->trapNumber = RMMIX_JDL::PUTW;
        hardwareComponents[((currentJobIndex+1)*2)]->trapData = theCPU->registers[ theCPU->trapData ];
        hardwareComponents[((currentJobIndex+1)*2)-1]->trapStatus = 0;
	 // Clear interrupts
        theCPU->trapNumber = theCPU->trapData = theCPU->trapStatus = 0;

	
    }; // end if output device ready
    // else do nothing (but wait until the input device is ready!)

} // end handlePUTW

// Output, Phase 2 (Device signals completion)
void rmminixOS::handlePUTW_READY( )
{
	std::cout<<"putwE"<<std::endl;
	
    assert( theCPU ); // i.e. assert that theCPU is not a null pointer

    // The hardware has signaled that the put-word operation is done.
    if ( 0 != theCPU->trapStatus ) {
        // trigger fatal interrupt (crash current process)
	std::cout<<(os0.good())<<" os0 state"<<std::endl;
        theCPU->trapNumber = RMMIX_JDL::FATAL;
    } else { // if OK status
        // Check if the device number is OK
        int outputDevice = theCPU->trapData;
        //assert( 1 == outputDevice ); // This will change later!
        assert( hardwareComponents[ outputDevice ] ); // not null
	  clearInterrups(outputToJobIndex(outputDevice));
	theCPU->trapData=theCPU->trapStatus=theCPU->trapNumber=0;
hardwareComponents[ outputDevice ]->trapData = hardwareComponents[ outputDevice ]->trapStatus = hardwareComponents[outputDevice ]->trapNumber = 0;
	//this only happens if only 1 job is left and it was waiting
        if(theCPU->registers[0] == -1){
		theCPU->registers[0]=getPCof(currentJobIndex);
	}
osVector->at(currentJobIndex)->close();
    };

} // end handlePUTW_READY


