
//========================================
//
// These are thunks that convert __cdecl 
// and __stdcall calls into thiscall calls
//
//========================================

/*

thiscall functions are weird on different compilers:
(from http://www.ownedcore.com/forums/world-of-warcraft/world-of-warcraft-bots-programs/wow-memory-editing/281008-gcc-thiscall-calling-convention-linux-win32-mingw.html#post1804011 )

MS Visual C++:
* Arguments are pushed on the stack in reverse order.
* The object pointer is passed in ECX.
* The callee pops arguments when returning.
* Primitive data types, except floating point values, are returned in EAX or EAX:EDX depending on the size.
* float and double are returned in fp0, i.e. the first floating point register.
* Simple data structures with 8 bytes or less in size are returned in EAX:EDX.
* All classes and structures are returned in memory, regardless of size.
* When a return is made in memory the caller passes a pointer to the memory location as the first parameter (hidden). The callee populates the memory, and returns the pointer. The callee pops the hidden pointer together with the rest of the arguments.
* If the method takes variable number of arguments, i.e. is declared with the ... operator, then the calling convention instead becomes that of cdecl. The object pointer is pushed on the stack as the first argument instead of passed in ECX, and all arguments are popped from the stack by the caller when the function returns.

MinGW g++ / Win32:
* Arguments are pushed on the stack in reverse order.
* The object pointer is pushed on the stack as the first parameter.
* The caller pops arguments after return.
* Primitive data types, except floating point values, are returned in EAX or EAX:EDX depending on the size.
* float and double are returned in fp0, i.e. the first floating point register.
* Objects with 8 bytes or less in size are returned in EAX:EDX.
* Objects larger than 8 bytes are returned in memory.
* Classes that have a destructor are returned in memory regardless of size.
* When a return is made in memory the caller passes a pointer to the memory location as the first parameter (hidden). The callee populates the memory, and returns the pointer. The callee pops the hidden pointer from the stack when returning.
* Classes that have a destructor are always passed by reference, even if the parameter is defined to be by value.

GCC g++ / Linux:
* Arguments are pushed on the stack in reverse order.
* The object pointer is pushed on the stack as the first parameter.
* The caller pops arguments after return.
* Primitive data types, except floating point values, are returned in EAX or EAX:EDX depending on the size.
* float and double are returned in fp0, i.e. the first floating point register.
* All structures and classes are returned in memory regardless of complexity and size.
* When a return is made in memory the caller passes a pointer to the memory location as the first parameter (hidden). The callee populates the memory, and returns the pointer. The callee pops the hidden pointer from the stack when returning.
* Classes that have a destructor are always passed by reference, even if the parameter is defined to be by value. 

*/

#ifndef THUNK_H
#define THUNK_H

#ifdef __linux__

#include <sys/mman.h>

#ifndef PAGESIZE
#define PAGESIZE 4096
#endif
#define PAGE_EXECUTE_READWRITE (PROT_READ | PROT_WRITE | PROT_EXEC)

#include <stdio.h>
typedef unsigned long ADDRTYPE;

int VirtualProtect(const void *addr, size_t len, int prot, unsigned int *prev)
{
	ADDRTYPE p = (ADDRTYPE) addr & ~(PAGESIZE-1);
	int ret = mprotect((void*)p, (ADDRTYPE)addr - p + len, prot);
	if(ret)
		perror("VirtualProtect");
	// just make something up. this seems like a probable previous value.
	*prev = PROT_READ | PROT_EXEC;
	return ret == 0;
}


#elif _WIN32

#define VC_EXTRALEAN 
#define WIN32_LEAN_AND_MEAN 

#include <windows.h>

#endif

template<typename To, typename From>
inline To union_cast(From fr) throw()
{
	union
	{
		From f;
		To t;
	} uc;
	uc.f = fr;
	return uc.t;
}

// this class forwards a __stdcall 
// function call into a thiscall function.
//
// __stdcall functions store the params on the 
// stack from right to left. The callee is 
// responsible for cleaning up the stack after 
// the call.
//
// This will only work for mvsc thiscall functions 
// because they are __stdcall by default, while gcc is not.
// Everything else besides msvc uses __cdecl by default 
// for thiscall functions.
//
// The calls should go like this:
// 'some function' [call]-> our asm (disguised as __stdcall) [jmp]-> our thiscall hook (can be __cdecl or __stdcall)
//
// Since our asm is disguised as an __stdcall, 
// 'some function' won't clean up the stack for us.
// In the asm step, we have no clue how many 
// params that we need to clean up, so we 
// need to force the thiscall hook to be what 
// 'some function' expects. (a __stdcall)
//
// We want to force __stdcall because we want 
// the compiler to generate code for the callee 
// to clean up the stack.
//
// TODO: Force __stdcall on the callback and fix the asm
//
// TODO: might need to align the stack on 16 bytes in the asm
//		 or else SSE inside the hook func won't work
//
template <class T>
class CThunkStdCall
{
public:
	typedef void (__stdcall T::*ThunkType)();

	void InitThunk(ThunkType method, const T *pthis)
	{
		union uPtr 
		{
			BYTE *byte;
			DWORD *dword;
		};

		uPtr pAsm86;
		pAsm86.byte = asm86;
		// to make a trampoline to a thiscall, we take the 'this' pointer 
		// and shove it in ecx. then we can jump to the thiscall function.
		*pAsm86.byte++	= 0xB9;			// mov ecx, %pthis
		*pAsm86.dword++	= (DWORD)pthis;	// dword ptr pthis
		*pAsm86.byte++	= 0xE9;			// jmp method
		// when decoding this jump offset, the program counter 
		// will be at the instruction after this one, because that's how it works.
		// so the jump offset is the addr of the function we want to jump to, minus 
		// the current position of the program counter.
		*pAsm86.dword++	= union_cast<DWORD>(method) - (DWORD)(asm86 + sizeof(asm86)); // offset to func

		// need to enable execution access for this memory or else we will get a seg fault
		DWORD protect;
		VirtualProtect( asm86, sizeof(asm86), PAGE_EXECUTE_READWRITE, &protect );

		//This function is valid only on multiprocessor computers.
		// if the asm86 contents were already in the cache before we modified it, it 
		// has to be reloaded. so we need to flush the instruction cache.
		// msdn says that x86 and x64 chips don't need this though because they 
		// serialize the instructions on each jmp.
		//FlushInstructionCache(GetCurrentProcess(), asm86, sizeof(asm86));
	}

	// get the asm disguised as a __stdcall func
	template <typename K>
	K GetThunk()
	{
		return reinterpret_cast<K>(&asm86);
	}

private:
	BYTE asm86[10];
};

// this class forwards a __cdecl 
// function call into a thiscall function.
//
// in __cdecl, the caller cleans up the stack
// and arguments are ordered from right to left
// on the stack
//
// there are special cases for __cdecl when 
// returning structs/classes/large values 
// with msvc and gcc, but i'm not doing those
// yet because i don't need them
//
// THE CALLBACK METHOD MUST BE __cdecl ELSE CRASH
//
template <class T>
class CThunkCDecl
{
public:
	typedef void (__cdecl T::*ThunkType)();

	CThunkCDecl():
		m_oldRetVal(0)
	{
	}

	void InitThunk(ThunkType method, const T *pthis)
	{
		union uPtr 
		{
			BYTE *byte;
			DWORD *dword;
		};

		uPtr pAsm86;
		pAsm86.byte = asm86;
		// for __cdecl, we need to push the 'this' pointer
		// on the end of the stack before the return value,
		// then call our function and clean up the 'this'
		// pointer on the stack

		// pop the old return value into our mem
		*pAsm86.byte++ = 0x8F;			// pop m32
		*pAsm86.byte++ = 0x05;			// modr/m byte
		*pAsm86.dword++ = (DWORD)(&m_oldRetVal); // m32

		// push our 'this' pointer
		*pAsm86.byte++ = 0x68;			// push imm32
		*pAsm86.dword++ = (DWORD)(pthis);	// imm32

		// push our return addr
		*pAsm86.byte++ = 0x68;			// push imm32
		*pAsm86.dword++ = (DWORD)(asm86 + 21);	// imm32

		*pAsm86.byte++ = 0xE9;			// jmp method
		// when decoding this jump offset, the program counter 
		// will be at the instruction after this one, because that's how it works.
		// so the jump offset is the addr of the function we want to jump to, minus 
		// the current position of the program counter.
		//
		// offset to func ( -10 is for the instructions after which are 10 bytes )
		*pAsm86.dword++	= union_cast<DWORD>(method) - (DWORD)(asm86 + sizeof(asm86) - 9);
		
		// clean up the 'this' ptr and jump back 
		// to the original caller

		// clean up 'this'
		*pAsm86.byte++ = 0x83;		// add esp, 4
		*pAsm86.byte++ = 0xC4;		// modr/m byte
		*pAsm86.byte++ = 0x04;

		// jump back to caller
		*pAsm86.byte++ = 0xFF;		// jmp [&m_oldRetVal]
		*pAsm86.byte++ = 0x25;		// modr/m byte
		*pAsm86.dword++ = (DWORD)(&m_oldRetVal);

		// need to enable execution access for this memory or else we will get a seg fault
		DWORD protect;
		VirtualProtect( asm86, sizeof(asm86), PAGE_EXECUTE_READWRITE, &protect );

		//This function is valid only on multiprocessor computers.
		// if the asm86 contents were already in the cache before we modified it, it 
		// has to be reloaded. so we need to flush the instruction cache.
		// msdn says that x86 and x64 chips don't need this though because they 
		// serialize the instructions on each jmp.
		//FlushInstructionCache(GetCurrentProcess(), asm86, sizeof(asm86));
	}

	// get the asm disguised as a __cdecl func
	template <typename K>
	K GetThunk()
	{
		return reinterpret_cast<K>(&asm86);
	}

private:
	uint32 m_oldRetVal;
	// TODO: i think i need to align this on 16 bytes for gcc-4.5 and SSE
	BYTE asm86[30];
};

#endif // THUNK_H
