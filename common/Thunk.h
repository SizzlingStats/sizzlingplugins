
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

// this class converts a static __stdcall function into 
// a thiscall function. take a look at the pdf in our 
// files on the calling conventions if you want to 
// know the specifics.
template <class T>
class CThunk
{
public:
	typedef void (T::*ThunkType)();

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
		int protect;
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

#endif // THUNK_H
