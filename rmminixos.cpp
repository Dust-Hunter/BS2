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
#define _clear -1

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
std::vector<bool>* waitingForIOStatus;
int fatalInterruptIndex = _clear;

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
    
    if(rmminixOS::loadNextProgramm(currentJobIndex)){
     return;
    }else if(switchProgramm()){
	return;
    }else{
//check if another job is there but cannot be switched into cause the job is waiting for io operations
	for(int i=0;i<waitingForIOStatus->size();i++){
		if(!waitingForIOStatus->at(i)){
			theCPU->trapNumber = theCPU->trapData = theCPU->trapStatus = 0;
			theCPU->registers[0]=-1;
			return;
		}
	}
exit(status);
}


} // end handleHALT

void rmminixOS::handleFATAL()
{
    int jobIndex;
    //check whcih job caused the fatal interrupt
    //if the index is clear, then the fatal interrupt was caused by the cpu which means it was the current Job
    if(fatalInterruptIndex==_clear){
	jobIndex=currentJobIndex;
    }else{
        //the interrut as caused by an io operation and the related jobindex was saved in fatalInterruptIndex
	jobIndex=fatalInterruptIndex;
    } 
theCPU->trapNumber=0;
    //reset the index
    fatalInterruptIndex=_clear;
    rmmixHardware::logStream << "FATAL Interrupt!!" << std::endl;
    std::cerr << "FATAL Interrupt!!" << std::endl;
    // This is OK if we only want to run one program -
    // We need to extend this to handle multiprogramming!
   
	if(rmminixOS::loadNextProgramm(jobIndex)){
     return;
    }else{

	//current programm failed so we have to mark is as finished
	setPCof(jobIndex,JobFinished);


		if(switchProgramm()){
		
		return;
		}else{
			//check if another job is there but cannot be switched into cause the job is waiting for io operations
	for(int i=0;i<waitingForIOStatus->size();i++){
		if(!waitingForIOStatus->at(i)){
			
			theCPU->registers[0]=-1;
			
			return;
		}
	}
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
	if(isCurrentJobDone()){
		removeCurrentJob();
	}

	//check if another job is waiting
	int nextJobIndex = getNextWaitingJob();
	
       	if(nextJobIndex!=noJobLeft&&nextJobIndex!=currentJobIndex){
		executeJobChange(nextJobIndex,false);
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



void rmminixOS::executeJobChange(int nextJobIndex,bool afterIdle){
	//check if the next job is the current Job, in that case nothing has to be done	
	//dont save the registers if the currentJob is done or this function was called afer
	//the cpu was idle, in that case the pc is -1 and we dont want to save that ofc
	//the register have been saves before the cpu was set to idle
	if(getPCof(currentJobIndex)!=JobFinished&& (!afterIdle)){
	
	saveRegisters();
	}

	restoreRegState(nextJobIndex);
	//hardwareComponents[0]->trapNumber=0;
	
	//check if the next job has been booted
	if(hasBeenBooted(nextJobIndex)){
		bootProgramm(nextJobIndex);
		
		
	}else{
		
		restoreInstructionMem(nextJobIndex);
		currentJobIndex=nextJobIndex;
		
	
	}
}

void rmminixOS::clearInterrupts(int jobIndex){
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
		registerMem->at(currentJobIndex).at(i)= theCPU->registers.at(i);
	}
saveTrapRegs();

}

int rmminixOS::getNextWaitingJob(){
	//check the entire list

	for(int i=0;i<registerMem->size();i++){
		if(getPCof((currentJobIndex+1+i)%registerMem->size())!=JobFinished){
			if(waitingForIOStatus->at((currentJobIndex+1+i)%registerMem->size())){
					return (currentJobIndex+1+i)%registerMem->size();
				}
		}		

		//if(getPCof((currentJobIndex+1+i)%registerMem->size())!=JobFinished&&waitingForIOStatus->at((currentJobIndex+1+i)%registerMem->size())){
						
		//	return (currentJobIndex+1+i)%registerMem->size();
		//}
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

registerMem->at(jobIndex)[trapRegMem->at(jobIndex)[_regToUpdate]] = input;


}


int rmminixOS::inputToJobIndex(int deviceNumber){
return ((deviceNumber+1)/2)-1;
}

int rmminixOS::outputToJobIndex(int deviceNumber){
return (deviceNumber/2)-1;
}	
 

bool rmminixOS::loadNextProgramm(int jobIndex){

	//close the old os stream
	assert( hardwareComponents[(jobIndex+1)*2] ); // is not null
    
	//try loading another programm	
	//check for another job line
	if(obcVector->at(jobIndex)->gotoState( JobLangCompiler::codeReaderState  )){
			
		if(!rmminixOS::load(*(obcVector->at(jobIndex)),jobIndex)){
		
		return false;		
		}
		
		subJobVector->at(jobIndex)=subJobVector->at(jobIndex)+1;
		
		
		//OPEN A NEW OS STREAM
		 
    		

		//rebind io components
 		assert( hardwareComponents[ ((jobIndex+1)*2)-1 ] ); // is not null
        	hardwareComponents[((jobIndex+1)*2)-1 ]->bind( (obcVector->at(jobIndex)) );
    		assert( hardwareComponents[(jobIndex+1)*2] ); // is not null
                hardwareComponents[(jobIndex+1)*2 ]->bind( (osVector->at(jobIndex)) ); // bind to std out
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
	bool loadingForCurrentJob= programmIndex==currentJobIndex;
	programmMem->at(programmIndex).clear();
        // Parses each line after $JOB to $RUN
        while ( decompiler >> instruction ) {
	if(loadingForCurrentJob){
	theCPU->instructionMemory[ instructionNumber ] = instruction;
	}    
        ++instructionNumber;
	programmMem->at(programmIndex).push_back(instruction);

        }; // until no more lines or found $RUN
	if(loadingForCurrentJob){
	// if we're here, then we could load the program.
        theCPU->registers[ 0 ] = 0;
	}
	setPCof(programmIndex,0);    
        
	

	
	

        return true;
    } catch ( std::string error  ) { // if an exception was thrown, something went wrong.
        std::cerr << "Error while loading file named " << decompiler.filename
                  << std::endl << error << std::endl;
        return false;
    }; // end if $JOB accepted
} // end load


bool rmminixOS::bootProgramm(int programmIndex){
  
    theCPU->instructionMemory.clear();
 
    int tempCurrentJobIndex = currentJobIndex;
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
	currentJobIndex=programmIndex;
    // Load the instruction memory
    if ( ! rmminixOS::load( *decompiler,programmIndex )) {
        std::cerr << "BOOT LOAD FAILED - File name " << argVector->at(programmIndex) << std::endl;
	currentJobIndex = tempCurrentJobIndex;
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
    
   
   

    //hardwareComponents[((programmIndex+1)*2)]->bind( (osVector->at(currentJobIndex)) ); // bind to std out
hardwareComponents[((programmIndex+1)*2)]->bind(osVector->at(currentJobIndex));
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
	waitingForIOStatus = new std::vector<bool>();
	
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
	waitingForIOStatus->push_back(true);
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
		
	int tempJobIndex = currentJobIndex;	
	
    assert( theCPU ); // i.e. assert that theCPU is not a null pointer
    // Save data we will need later
    if ( 0 <= theCPU->registers[0] ) {
     //save the register into which 		
    trapRegMem->at(currentJobIndex)[_regToUpdate] = theCPU->trapData;	
    waitingForIOStatus->at(currentJobIndex) = false;
    //try to switch to another job, if no other job
     
     if(!switchProgramm()){
	
	// Put the CPU in an idle state until PUTW_READY signal
	//save the registers...mainly for the pc
	saveRegisters();
	
        theCPU->registers[ 0 ] = -1; // make the cpu wait!
	
     }
    
	
    };

    // Signal the input device
    // Note that the choice of device is hard-coded - we always read from
    // device 1! (This will have to change)
    assert( hardwareComponents[((tempJobIndex+1)*2)-1] );
    if ( 0 == hardwareComponents[((tempJobIndex+1)*2)-1]->trapNumber ) {
        hardwareComponents[((tempJobIndex+1)*2)-1]->trapNumber = RMMIX_JDL::GETW;
        // hardwareComponents[1]->trapData = ???
        // hardwareComponents[1]->trapStatus = ???
	
	clearInterrupts(tempJobIndex);
        if(tempJobIndex==currentJobIndex){
	theCPU->trapNumber = theCPU->trapData = theCPU->trapStatus = 0;
	}

	
    };
    // else do nothing (but wait until the input device is ready!)

} // end handleGETW

// Input, Phase 2 (Device signals completion)
void rmminixOS::handleGETW_READY( )
{
	
	
	
    assert( theCPU ); // i.e. assert that theCPU is not a null pointer
 int inputDevice = theCPU->trapData;
    // The hardware has signaled that the get-word operation is done.
    if ( 0 != theCPU->trapStatus ) {
        // trigger fatal interrupt (crash current process)
	hardwareComponents[ inputDevice ]->trapData = hardwareComponents[ inputDevice ]->trapStatus = hardwareComponents[ inputDevice ]->trapNumber = 0;
	waitingForIOStatus->at(inputToJobIndex(inputDevice))=true;
	theCPU->trapNumber = RMMIX_JDL::FATAL;
	fatalInterruptIndex = inputToJobIndex(inputDevice); //the os needs to know wich job caused the fatal interrupt
    } else { // if OK status
	
       
	//assert( 1 == inputDevice ); // THIS MUST BE CHANGED LATER!
        assert( hardwareComponents[ inputDevice ] ); // not null
	
        // Get data from input device
	storeInput(hardwareComponents[ inputDevice ]->trapData,inputDevice);
	
	clearInterrupts(inputToJobIndex(inputDevice));
	
	theCPU->trapData=theCPU->trapStatus=theCPU->trapNumber=0;
	hardwareComponents[ inputDevice ]->trapData = hardwareComponents[ inputDevice ]->trapStatus = hardwareComponents[ inputDevice ]->trapNumber = 0;
	
	waitingForIOStatus->at(inputToJobIndex(inputDevice))=true;
	//check if the cpu was ideling cause no other job was there
        if(theCPU->registers[0]== -1){
		executeJobChange(inputToJobIndex(inputDevice),true);
	
	
	}
    };
} // end handleGETW_READY

// Output, Phase 1 (CPU requests output)
void rmminixOS::handlePUTW( )
{

osVector->at(currentJobIndex)->open(getOutputFilename(currentJobIndex));

	int tempJobIndex=currentJobIndex;
	int tempTrapData = theCPU->registers[ theCPU->trapData ];
    assert( theCPU ); // i.e. assert that theCPU is not a null pointer
    // Save data we will need later
    if ( 0 <= theCPU->registers[0] ) {
        waitingForIOStatus->at(currentJobIndex) = false;
    //try to switch to another job, if no other job
     if(!switchProgramm()){
	// Put the CPU in an idle state until PUTW_READY signal
	saveRegisters();
        theCPU->registers[ 0 ] = -1; // make the cpu wait!
	
     }
       
    };

    // Signal the output device
    // Note that the choice of device is hard-coded - we always write to
    // device 2!

    assert( hardwareComponents[((tempJobIndex+1)*2)] );
    if ( 0 == hardwareComponents[((tempJobIndex+1)*2)]->trapNumber ) {
        hardwareComponents[((tempJobIndex+1)*2)]->trapNumber = RMMIX_JDL::PUTW;
        hardwareComponents[((tempJobIndex+1)*2)]->trapData = tempTrapData;
        hardwareComponents[((tempJobIndex+1)*2)-1]->trapStatus = 0;
	 // Clear interrupts
        clearInterrupts(tempJobIndex);
	 if(tempJobIndex==currentJobIndex){
	theCPU->trapNumber = theCPU->trapData = theCPU->trapStatus = 0;
	}

	
    }; // end if output device ready
    // else do nothing (but wait until the input device is ready!)

} // end handlePUTW

// Output, Phase 2 (Device signals completion)
void rmminixOS::handlePUTW_READY( )
{


assert( theCPU ); // i.e. assert that theCPU is not a null pointer
  int outputDevice = theCPU->trapData;
    // The hardware has signaled that the put-word operation is done.
    if ( 0 != theCPU->trapStatus ) {
        // trigger fatal interrupt (crash current process)
	 osVector->at(outputToJobIndex(outputDevice))->close();
        theCPU->trapNumber = RMMIX_JDL::FATAL;
	fatalInterruptIndex = outputToJobIndex(outputDevice); //os need to know which job caused the fatal interrupt
    } else { // if OK status
        // Check if the device number is OK
      
	osVector->at(outputToJobIndex(outputDevice))->close();
        //assert( 1 == outputDevice ); // This will change later!
        assert( hardwareComponents[ outputDevice ] ); // not null
	clearInterrupts(outputToJobIndex(outputDevice));
	theCPU->trapData=theCPU->trapStatus=theCPU->trapNumber=0;
	hardwareComponents[ outputDevice ]->trapData = hardwareComponents[ outputDevice ]->trapStatus = hardwareComponents[outputDevice ]->trapNumber = 0;
	//this only happens if only 1 job is left and it was waiting
       waitingForIOStatus->at(outputToJobIndex(outputDevice))=true;
	//check if the cpu was ideling cause no other job was there
        if(theCPU->registers[0]== -1){
		executeJobChange(outputToJobIndex(outputDevice),true);
	
	}



    };

} // end handlePUTW_READY


