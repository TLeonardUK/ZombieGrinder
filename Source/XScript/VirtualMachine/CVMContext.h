/* *****************************************************************

		CVMContext.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CVMCONTEXT_H_
#define _CVMCONTEXT_H_

#include "Generic/Types/String.h"
#include <stack>

#include "XScript/VirtualMachine/CInstructionBuffer.h"
#include "XScript/VirtualMachine/CVMValue.h"
#include "XScript/VirtualMachine/CVMValueStack.h"

//#include "Engine/Profiling/ProfilingManager.h"

struct ScopeProfiler;
class CVirtualMachine;

// If set contexts will be given profile blocks so its easier to track VM stuff in profiler :3.
#ifndef MASTER_BUILD
//#define VM_PROFILE_BLOCKS
#endif

// =================================================================
//	An activation context is a single "function call" held on a
//	contexts stack. It hosts its own registers and such.
// =================================================================
class CActivationContext
{
public:

	// Method that we are running.
	CVMLinkedSymbol* Method;

	// Instruction pointer / registers. Obvious no? :3
	//int IP;
	// Pointer to next instruction. TODO: Safety check for overrun.
	u8* IP;

	// Offset into stack that our registers are located at.
	int Register_Offset;

	// Offset into stack that our locals are located at.
	int Local_Offset;

	// Register of parent activation context to put return
	// value into.
	int Result_Register;

	// Ths handle for the class object of this method, NULL for static functions.
	CVMValue This;

	// If true execution will finish when returning from this context.
	bool Finish_On_Return;

	// Debug information traced in scripts.
	//int Current_File_Name_Index;
	//int Current_Line;
	//int Current_Column;

	// Profile scope.
	ScopeProfiler* Profile_Block;

	INLINE void Clear()
	{
		This.Clear();
	}

};

// =================================================================
//	Stores an execution state of a virtual machine. Essentially
//	a single "thread".
// =================================================================
class CVMContext
{
public:
	CVMValueStack<CActivationContext> Activation_Stack;
	CVMValueStack<CVMValue>			  Stack;
	CVirtualMachine*				  VM;
	void*							  MetaData;

	// Latent execution.
	bool							  Sleeping;
	float							  Wakeup_Time;

	// Who owns this context at the moment.
	int								  Locked;
	Thread*							  Lock_Thread;
	int								  Lock_Depth;
	bool							  Lock_Set_By_Release;

	CVMContext(CVirtualMachine* vm, int stack_size, int activation_stack_size, void* meta)
		: Stack(vm, stack_size)
		, Activation_Stack(vm, activation_stack_size)
		, VM(vm)
		, MetaData(meta)
		, Sleeping(false)
		, Locked(0)
		, Lock_Thread(NULL)
		, Lock_Depth(0)
		, Lock_Set_By_Release(false)
	{
	}

};

#endif