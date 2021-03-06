/* *****************************************************************

		CVMNativeFunctionTrampoline.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CVMNATIVEFUNCTIONTRAMPOLINE_H_
#define _CVMNATIVEFUNCTIONTRAMPOLINE_H_

#include "XScript/VirtualMachine/CVMValue.h"
#include "XScript/VirtualMachine/CVMNativeTrampoline.h"

// Here be dragons.

// --------------------------------------------------------------------------------------------------
// Zero parameter trampoline.
// --------------------------------------------------------------------------------------------------
template <typename RT>
class CVMNativeFunctionTrampoline_0 : public CVMNativeTrampoline
{
public:
	typedef RT (*FunctionPtr)(CVirtualMachine*);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_0(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		CVMValue retval = m_function_ptr(vm);
		vm->Set_Return_Value(retval);
	}
};
template <>
class CVMNativeFunctionTrampoline_0<void> : public CVMNativeTrampoline
{
public:
	typedef void (*FunctionPtr)(CVirtualMachine*);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_0(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		m_function_ptr(vm);
	}
};

// --------------------------------------------------------------------------------------------------
// One parameter trampoline.
// --------------------------------------------------------------------------------------------------
template <typename RT, typename P1>
class CVMNativeFunctionTrampoline_1 : public CVMNativeTrampoline
{
public:
	typedef RT (*FunctionPtr)(CVirtualMachine*, P1);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_1(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		P1 param_1;
		vm->Get_Parameter(0, &param_1);

		CVMValue retval = m_function_ptr(vm, param_1);
		vm->Set_Return_Value(retval);
	}
};
template <typename P1>
class CVMNativeFunctionTrampoline_1<void, P1> : public CVMNativeTrampoline
{
public:
	typedef void (*FunctionPtr)(CVirtualMachine*, P1);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_1(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		P1 param_1;
		vm->Get_Parameter(0, &param_1);

		m_function_ptr(vm, param_1);
	}
};

// --------------------------------------------------------------------------------------------------
// Two parameter trampoline.
// --------------------------------------------------------------------------------------------------
template <typename RT, typename P1, typename P2>
class CVMNativeFunctionTrampoline_2 : public CVMNativeTrampoline
{
public:
	typedef RT (*FunctionPtr)(CVirtualMachine*, P1, P2);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_2(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		P1 param_1;
		P2 param_2;
		vm->Get_Parameter(0, &param_1);
		vm->Get_Parameter(1, &param_2);

		CVMValue retval = m_function_ptr(vm, param_1, param_2);
		vm->Set_Return_Value(retval);
	}
};
template <typename P1, typename P2>
class CVMNativeFunctionTrampoline_2<void, P1, P2> : public CVMNativeTrampoline
{
public:
	typedef void (*FunctionPtr)(CVirtualMachine*, P1, P2);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_2(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		P1 param_1;
		P2 param_2;
		vm->Get_Parameter(0, &param_1);
		vm->Get_Parameter(1, &param_2);

		m_function_ptr(vm, param_1, param_2);
	}
};

// --------------------------------------------------------------------------------------------------
// Three parameter trampoline.
// --------------------------------------------------------------------------------------------------
template <typename RT, typename P1, typename P2, typename P3>
class CVMNativeFunctionTrampoline_3 : public CVMNativeTrampoline
{
public:
	typedef RT (*FunctionPtr)(CVirtualMachine*, P1, P2, P3);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_3(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		P1 param_1;
		P2 param_2;
		P3 param_3;
		vm->Get_Parameter(0, &param_1);
		vm->Get_Parameter(1, &param_2);
		vm->Get_Parameter(2, &param_3);

		CVMValue retval = m_function_ptr(vm, param_1, param_2, param_3);
		vm->Set_Return_Value(retval);
	}
};
template <typename P1, typename P2, typename P3>
class CVMNativeFunctionTrampoline_3<void, P1, P2, P3> : public CVMNativeTrampoline
{
public:
	typedef void (*FunctionPtr)(CVirtualMachine*, P1, P2, P3);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_3(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		P1 param_1;
		P2 param_2;
		P3 param_3;
		vm->Get_Parameter(0, &param_1);
		vm->Get_Parameter(1, &param_2);
		vm->Get_Parameter(2, &param_3);

		m_function_ptr(vm, param_1, param_2, param_3);
	}
};

// --------------------------------------------------------------------------------------------------
// Four parameter trampoline.
// --------------------------------------------------------------------------------------------------
template <typename RT, typename P1, typename P2, typename P3, typename P4>
class CVMNativeFunctionTrampoline_4 : public CVMNativeTrampoline
{
public:
	typedef RT (*FunctionPtr)(CVirtualMachine*, P1, P2, P3, P4);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_4(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		P1 param_1;
		P2 param_2;
		P3 param_3;
		P4 param_4;
		vm->Get_Parameter(0, &param_1);
		vm->Get_Parameter(1, &param_2);
		vm->Get_Parameter(2, &param_3);
		vm->Get_Parameter(3, &param_4);

		CVMValue retval = m_function_ptr(vm, param_1, param_2, param_3, param_4);
		vm->Set_Return_Value(retval);
	}
};
template <typename P1, typename P2, typename P3, typename P4>
class CVMNativeFunctionTrampoline_4<void, P1, P2, P3, P4> : public CVMNativeTrampoline
{
public:
	typedef void (*FunctionPtr)(CVirtualMachine*, P1, P2, P3, P4);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_4(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		P1 param_1;
		P2 param_2;
		P3 param_3;
		P4 param_4;
		vm->Get_Parameter(0, &param_1);
		vm->Get_Parameter(1, &param_2);
		vm->Get_Parameter(2, &param_3);
		vm->Get_Parameter(3, &param_4);

		m_function_ptr(vm, param_1, param_2, param_3, param_4);
	}
};

// --------------------------------------------------------------------------------------------------
// Five parameter trampoline.
// --------------------------------------------------------------------------------------------------
template <typename RT, typename P1, typename P2, typename P3, typename P4, typename P5>
class CVMNativeFunctionTrampoline_5 : public CVMNativeTrampoline
{
public:
	typedef RT (*FunctionPtr)(CVirtualMachine*, P1, P2, P3, P4, P5);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_5(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		P1 param_1;
		P2 param_2;
		P3 param_3;
		P4 param_4;
		P5 param_5;
		vm->Get_Parameter(0, &param_1);
		vm->Get_Parameter(1, &param_2);
		vm->Get_Parameter(2, &param_3);
		vm->Get_Parameter(3, &param_4);
		vm->Get_Parameter(4, &param_5);

		CVMValue retval = m_function_ptr(vm, param_1, param_2, param_3, param_4, param_5);
		vm->Set_Return_Value(retval);
	}
};
template <typename P1, typename P2, typename P3, typename P4, typename P5>
class CVMNativeFunctionTrampoline_5<void, P1, P2, P3, P4, P5> : public CVMNativeTrampoline
{
public:
	typedef void (*FunctionPtr)(CVirtualMachine*, P1, P2, P3, P4, P5);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_5(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		P1 param_1;
		P2 param_2;
		P3 param_3;
		P4 param_4;
		P5 param_5;
		vm->Get_Parameter(0, &param_1);
		vm->Get_Parameter(1, &param_2);
		vm->Get_Parameter(2, &param_3);
		vm->Get_Parameter(3, &param_4);
		vm->Get_Parameter(4, &param_5);

		m_function_ptr(vm, param_1, param_2, param_3, param_4, param_5);
	}
};

// --------------------------------------------------------------------------------------------------
// Six parameter trampoline.
// --------------------------------------------------------------------------------------------------
template <typename RT, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
class CVMNativeFunctionTrampoline_6 : public CVMNativeTrampoline
{
public:
	typedef RT (*FunctionPtr)(CVirtualMachine*, P1, P2, P3, P4, P5, P6);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_6(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		P1 param_1;
		P2 param_2;
		P3 param_3;
		P4 param_4;
		P5 param_5;
		P6 param_6;
		vm->Get_Parameter(0, &param_1);
		vm->Get_Parameter(1, &param_2);
		vm->Get_Parameter(2, &param_3);
		vm->Get_Parameter(3, &param_4);
		vm->Get_Parameter(4, &param_5);
		vm->Get_Parameter(5, &param_6);

		CVMValue retval = m_function_ptr(vm, param_1, param_2, param_3, param_4, param_5, param_6);
		vm->Set_Return_Value(retval);
	}
};
template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
class CVMNativeFunctionTrampoline_6<void, P1, P2, P3, P4, P5, P6> : public CVMNativeTrampoline
{
public:
	typedef void (*FunctionPtr)(CVirtualMachine*, P1, P2, P3, P4, P5, P6);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_6(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		P1 param_1;
		P2 param_2;
		P3 param_3;
		P4 param_4;
		P5 param_5;
		P6 param_6;
		vm->Get_Parameter(0, &param_1);
		vm->Get_Parameter(1, &param_2);
		vm->Get_Parameter(2, &param_3);
		vm->Get_Parameter(3, &param_4);
		vm->Get_Parameter(4, &param_5);
		vm->Get_Parameter(5, &param_6);

		m_function_ptr(vm, param_1, param_2, param_3, param_4, param_5, param_6);
	}
};

// --------------------------------------------------------------------------------------------------
// Seven parameter trampoline.
// --------------------------------------------------------------------------------------------------
template <typename RT, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
class CVMNativeFunctionTrampoline_7 : public CVMNativeTrampoline
{
public:
	typedef RT (*FunctionPtr)(CVirtualMachine*, P1, P2, P3, P4, P5, P6, P7);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_7(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		P1 param_1;
		P2 param_2;
		P3 param_3;
		P4 param_4;
		P5 param_5;
		P6 param_6;
		P7 param_7;
		vm->Get_Parameter(0, &param_1);
		vm->Get_Parameter(1, &param_2);
		vm->Get_Parameter(2, &param_3);
		vm->Get_Parameter(3, &param_4);
		vm->Get_Parameter(4, &param_5);
		vm->Get_Parameter(5, &param_6);
		vm->Get_Parameter(6, &param_7);

		CVMValue retval = m_function_ptr(vm, param_1, param_2, param_3, param_4, param_5, param_6, param_7);
		vm->Set_Return_Value(retval);
	}
};
template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
class CVMNativeFunctionTrampoline_7<void, P1, P2, P3, P4, P5, P6, P7> : public CVMNativeTrampoline
{
public:
	typedef void (*FunctionPtr)(CVirtualMachine*, P1, P2, P3, P4, P5, P6, P7);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_7(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		P1 param_1;
		P2 param_2;
		P3 param_3;
		P4 param_4;
		P5 param_5;
		P6 param_6;
		P7 param_7;
		vm->Get_Parameter(0, &param_1);
		vm->Get_Parameter(1, &param_2);
		vm->Get_Parameter(2, &param_3);
		vm->Get_Parameter(3, &param_4);
		vm->Get_Parameter(4, &param_5);
		vm->Get_Parameter(5, &param_6);
		vm->Get_Parameter(6, &param_7);

		m_function_ptr(vm, param_1, param_2, param_3, param_4, param_5, param_6, param_7);
	}
};

// --------------------------------------------------------------------------------------------------
// Eight parameter trampoline.
// --------------------------------------------------------------------------------------------------
template <typename RT, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
class CVMNativeFunctionTrampoline_8 : public CVMNativeTrampoline
{
public:
	typedef RT (*FunctionPtr)(CVirtualMachine*, P1, P2, P3, P4, P5, P6, P7, P8);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_8(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		P1 param_1;
		P2 param_2;
		P3 param_3;
		P4 param_4;
		P5 param_5;
		P6 param_6;
		P7 param_7;
		P8 param_8;
		vm->Get_Parameter(0, &param_1);
		vm->Get_Parameter(1, &param_2);
		vm->Get_Parameter(2, &param_3);
		vm->Get_Parameter(3, &param_4);
		vm->Get_Parameter(4, &param_5);
		vm->Get_Parameter(5, &param_6);
		vm->Get_Parameter(6, &param_7);
		vm->Get_Parameter(7, &param_8);

		CVMValue retval = m_function_ptr(vm, param_1, param_2, param_3, param_4, param_5, param_6, param_7, param_8);
		vm->Set_Return_Value(retval);
	}
};
template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
class CVMNativeFunctionTrampoline_8<void, P1, P2, P3, P4, P5, P6, P7, P8> : public CVMNativeTrampoline
{
public:
	typedef void (*FunctionPtr)(CVirtualMachine*, P1, P2, P3, P4, P5, P6, P7, P8);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_8(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		P1 param_1;
		P2 param_2;
		P3 param_3;
		P4 param_4;
		P5 param_5;
		P6 param_6;
		P7 param_7;
		P8 param_8;
		vm->Get_Parameter(0, &param_1);
		vm->Get_Parameter(1, &param_2);
		vm->Get_Parameter(2, &param_3);
		vm->Get_Parameter(3, &param_4);
		vm->Get_Parameter(4, &param_5);
		vm->Get_Parameter(5, &param_6);
		vm->Get_Parameter(6, &param_7);
		vm->Get_Parameter(7, &param_8);

		m_function_ptr(vm, param_1, param_2, param_3, param_4, param_5, param_6, param_7, param_8);
	}
};


// --------------------------------------------------------------------------------------------------
// Nine parameter trampoline.
// --------------------------------------------------------------------------------------------------
template <typename RT, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
class CVMNativeFunctionTrampoline_9 : public CVMNativeTrampoline
{
public:
	typedef RT(*FunctionPtr)(CVirtualMachine*, P1, P2, P3, P4, P5, P6, P7, P8, P9);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_9(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		P1 param_1;
		P2 param_2;
		P3 param_3;
		P4 param_4;
		P5 param_5;
		P6 param_6;
		P7 param_7;
		P8 param_8;
		P9 param_9;
		vm->Get_Parameter(0, &param_1);
		vm->Get_Parameter(1, &param_2);
		vm->Get_Parameter(2, &param_3);
		vm->Get_Parameter(3, &param_4);
		vm->Get_Parameter(4, &param_5);
		vm->Get_Parameter(5, &param_6);
		vm->Get_Parameter(6, &param_7);
		vm->Get_Parameter(7, &param_8);
		vm->Get_Parameter(8, &param_9);

		CVMValue retval = m_function_ptr(vm, param_1, param_2, param_3, param_4, param_5, param_6, param_7, param_8, param_9);
		vm->Set_Return_Value(retval);
	}
};
template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
class CVMNativeFunctionTrampoline_9<void, P1, P2, P3, P4, P5, P6, P7, P8, P9> : public CVMNativeTrampoline
{
public:
	typedef void(*FunctionPtr)(CVirtualMachine*, P1, P2, P3, P4, P5, P6, P7, P8, P9);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_9(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		P1 param_1;
		P2 param_2;
		P3 param_3;
		P4 param_4;
		P5 param_5;
		P6 param_6;
		P7 param_7;
		P8 param_8;
		P9 param_9;
		vm->Get_Parameter(0, &param_1);
		vm->Get_Parameter(1, &param_2);
		vm->Get_Parameter(2, &param_3);
		vm->Get_Parameter(3, &param_4);
		vm->Get_Parameter(4, &param_5);
		vm->Get_Parameter(5, &param_6);
		vm->Get_Parameter(6, &param_7);
		vm->Get_Parameter(7, &param_8);
		vm->Get_Parameter(8, &param_9);

		m_function_ptr(vm, param_1, param_2, param_3, param_4, param_5, param_6, param_7, param_8, param_9);
	}
};


// --------------------------------------------------------------------------------------------------
// Ten parameter trampoline.
// --------------------------------------------------------------------------------------------------
template <typename RT, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10>
class CVMNativeFunctionTrampoline_10 : public CVMNativeTrampoline
{
public:
	typedef RT(*FunctionPtr)(CVirtualMachine*, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_10(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		P1 param_1;
		P2 param_2;
		P3 param_3;
		P4 param_4;
		P5 param_5;
		P6 param_6;
		P7 param_7;
		P8 param_8;
		P9 param_9;
		P10 param_10;
		vm->Get_Parameter(0, &param_1);
		vm->Get_Parameter(1, &param_2);
		vm->Get_Parameter(2, &param_3);
		vm->Get_Parameter(3, &param_4);
		vm->Get_Parameter(4, &param_5);
		vm->Get_Parameter(5, &param_6);
		vm->Get_Parameter(6, &param_7);
		vm->Get_Parameter(7, &param_8);
		vm->Get_Parameter(8, &param_9);
		vm->Get_Parameter(9, &param_10);

		CVMValue retval = m_function_ptr(vm, param_1, param_2, param_3, param_4, param_5, param_6, param_7, param_8, param_9, param_10);
		vm->Set_Return_Value(retval);
	}
};
template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10>
class CVMNativeFunctionTrampoline_10<void, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10> : public CVMNativeTrampoline
{
public:
	typedef void(*FunctionPtr)(CVirtualMachine*, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10);
private:
	FunctionPtr m_function_ptr;
public:
	CVMNativeFunctionTrampoline_10(FunctionPtr func) : m_function_ptr(func) { }
	void Invoke(CVirtualMachine* vm)
	{
		P1 param_1;
		P2 param_2;
		P3 param_3;
		P4 param_4;
		P5 param_5;
		P6 param_6;
		P7 param_7;
		P8 param_8;
		P9 param_9;
		P10 param_10;
		vm->Get_Parameter(0, &param_1);
		vm->Get_Parameter(1, &param_2);
		vm->Get_Parameter(2, &param_3);
		vm->Get_Parameter(3, &param_4);
		vm->Get_Parameter(4, &param_5);
		vm->Get_Parameter(5, &param_6);
		vm->Get_Parameter(6, &param_7);
		vm->Get_Parameter(7, &param_8);
		vm->Get_Parameter(8, &param_9);
		vm->Get_Parameter(9, &param_10);

		m_function_ptr(vm, param_1, param_2, param_3, param_4, param_5, param_6, param_7, param_8, param_9, param_10);
	}
};

#endif

