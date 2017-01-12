/* *****************************************************************

		CBindingHelper.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CBINDINGHELPER_H_
#define _CBINDINGHELPER_H_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMNativeFunctionTrampoline.h"
#include "XScript/VirtualMachine/CVMNativeMethodTrampoline.h"

class CVMBindingHelper
{
private:
	CVirtualMachine* m_vm;

public:
	CVMBindingHelper(CVirtualMachine* vm)
		: m_vm(vm)
	{
	}
	
	template<typename RT>
	void Bind_Function(const char* class_name, const char* name, typename CVMNativeFunctionTrampoline_0<RT>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeFunctionTrampoline_0<RT>(func_ptr));
	}
	template<typename RT, typename P1>
	void Bind_Function(const char* class_name, const char* name, typename CVMNativeFunctionTrampoline_1<RT,P1>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeFunctionTrampoline_1<RT,P1>(func_ptr));
	}
	template<typename RT, typename P1, typename P2>
	void Bind_Function(const char* class_name, const char* name, typename CVMNativeFunctionTrampoline_2<RT,P1,P2>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeFunctionTrampoline_2<RT,P1,P2>(func_ptr));
	}
	template<typename RT, typename P1, typename P2, typename P3>
	void Bind_Function(const char* class_name, const char* name, typename CVMNativeFunctionTrampoline_3<RT,P1,P2,P3>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeFunctionTrampoline_3<RT,P1,P2,P3>(func_ptr));
	}
	template<typename RT, typename P1, typename P2, typename P3, typename P4>
	void Bind_Function(const char* class_name, const char* name, typename CVMNativeFunctionTrampoline_4<RT,P1,P2,P3,P4>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeFunctionTrampoline_4<RT,P1,P2,P3,P4>(func_ptr));
	}
	template<typename RT, typename P1, typename P2, typename P3, typename P4, typename P5>
	void Bind_Function(const char* class_name, const char* name, typename CVMNativeFunctionTrampoline_5<RT,P1,P2,P3,P4,P5>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeFunctionTrampoline_5<RT,P1,P2,P3,P4,P5>(func_ptr));
	}
	template<typename RT, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	void Bind_Function(const char* class_name, const char* name, typename CVMNativeFunctionTrampoline_6<RT,P1,P2,P3,P4,P5,P6>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeFunctionTrampoline_6<RT,P1,P2,P3,P4,P5,P6>(func_ptr));
	}
	template<typename RT, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	void Bind_Function(const char* class_name, const char* name, typename CVMNativeFunctionTrampoline_7<RT,P1,P2,P3,P4,P5,P6,P7>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeFunctionTrampoline_7<RT,P1,P2,P3,P4,P5,P6,P7>(func_ptr));
	}
	template<typename RT, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
	void Bind_Function(const char* class_name, const char* name, typename CVMNativeFunctionTrampoline_8<RT,P1,P2,P3,P4,P5,P6,P7,P8>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeFunctionTrampoline_8<RT,P1,P2,P3,P4,P5,P6,P7,P8>(func_ptr));
	}
	template<typename RT, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
	void Bind_Function(const char* class_name, const char* name, typename CVMNativeFunctionTrampoline_9<RT, P1, P2, P3, P4, P5, P6, P7, P8, P9>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeFunctionTrampoline_9<RT, P1, P2, P3, P4, P5, P6, P7, P8, P9>(func_ptr));
	}
	template<typename RT, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10>
	void Bind_Function(const char* class_name, const char* name, typename CVMNativeFunctionTrampoline_10<RT, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeFunctionTrampoline_10<RT, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10>(func_ptr));
	}
	
	template<typename RT>
	void Bind_Method(const char* class_name, const char* name, typename CVMNativeMethodTrampoline_0<RT>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeMethodTrampoline_0<RT>(func_ptr));
	}
	template<typename RT, typename P1>
	void Bind_Method(const char* class_name, const char* name, typename CVMNativeMethodTrampoline_1<RT,P1>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeMethodTrampoline_1<RT,P1>(func_ptr));
	}
	template<typename RT, typename P1, typename P2>
	void Bind_Method(const char* class_name, const char* name, typename CVMNativeMethodTrampoline_2<RT,P1,P2>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeMethodTrampoline_2<RT,P1,P2>(func_ptr));
	}
	template<typename RT, typename P1, typename P2, typename P3>
	void Bind_Method(const char* class_name, const char* name, typename CVMNativeMethodTrampoline_3<RT,P1,P2,P3>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeMethodTrampoline_3<RT,P1,P2,P3>(func_ptr));
	}
	template<typename RT, typename P1, typename P2, typename P3, typename P4>
	void Bind_Method(const char* class_name, const char* name, typename CVMNativeMethodTrampoline_4<RT,P1,P2,P3,P4>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeMethodTrampoline_4<RT,P1,P2,P3,P4>(func_ptr));
	}
	template<typename RT, typename P1, typename P2, typename P3, typename P4, typename P5>
	void Bind_Method(const char* class_name, const char* name, typename CVMNativeMethodTrampoline_5<RT,P1,P2,P3,P4,P5>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeMethodTrampoline_5<RT,P1,P2,P3,P4,P5>(func_ptr));
	}
	template<typename RT, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	void Bind_Method(const char* class_name, const char* name, typename CVMNativeMethodTrampoline_6<RT,P1,P2,P3,P4,P5,P6>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeMethodTrampoline_6<RT,P1,P2,P3,P4,P5,P6>(func_ptr));
	}
	template<typename RT, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	void Bind_Method(const char* class_name, const char* name, typename CVMNativeMethodTrampoline_7<RT,P1,P2,P3,P4,P5,P6,P7>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeMethodTrampoline_7<RT,P1,P2,P3,P4,P5,P6,P7>(func_ptr));
	}
	template<typename RT, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
	void Bind_Method(const char* class_name, const char* name, typename CVMNativeMethodTrampoline_8<RT,P1,P2,P3,P4,P5,P6,P7,P8>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeMethodTrampoline_8<RT,P1,P2,P3,P4,P5,P6,P7,P8>(func_ptr));
	}
	template<typename RT, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
	void Bind_Method(const char* class_name, const char* name, typename CVMNativeMethodTrampoline_9<RT, P1, P2, P3, P4, P5, P6, P7, P8, P9>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeMethodTrampoline_9<RT, P1, P2, P3, P4, P5, P6, P7, P8, P9>(func_ptr));
	}
	template<typename RT, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10>
	void Bind_Method(const char* class_name, const char* name, typename CVMNativeMethodTrampoline_10<RT, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10>::FunctionPtr func_ptr)
	{
		m_vm->Bind_Method_Internal(class_name, name, new CVMNativeMethodTrampoline_10<RT, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10>(func_ptr));
	}


};

#endif