
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

int VirtualProtect(const void *addr, size_t len, int prot, int *prev)
{
	ADDRTYPE p = (ADDRTYPE) addr & ~(PAGESIZE-1);
	int ret = mprotect((void*)p, (ADDRTYPE)addr - p + len, prot);
	if(ret)
		perror("VirtualProtect");
	// just make something up. this seems like a probable previous value.
	*prev = PROT_READ | PROT_EXEC;
	return ret == 0;
}


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
		// when decoding this final instruction of the jump offset, the program counter 
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

	// can't declare the function const 
	// because the compiler complains about 
	// the cast
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
template <class T>
class CThunkCDecl
{
public:
	typedef void (__cdecl T::*ThunkType)();

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
		// on the end of the stack before eax,
		// then jump to our function
		*pAsm86.byte++ = 0x58;			// pop eax
		*pAsm86.byte++ = 0x68;			// push pThis
		*pAsm86.dword++	= (DWORD)pthis;	// dword ptr pthis
		*pAsm86.byte++ = 0x50;			// push eax
		*pAsm86.byte++ = 0xE9;			// jmp method
		// when decoding this final instruction of the jump offset, the program counter 
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

	// can't declare the function const 
	// because the compiler complains about 
	// the cast
	template <typename K>
	K GetThunk()
	{
		return reinterpret_cast<K>(&asm86);
	}

private:
	BYTE asm86[12];
};

#endif // THUNK_H
