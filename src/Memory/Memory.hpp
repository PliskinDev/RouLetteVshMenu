#pragma once
#include <stdint.h>
#include <stdio.h>
#include <ppu_asm_intrinsics.h> // __ALWAYS_INLINE
#include <sys/process.h>
#include "Utils/SystemCalls.hpp"

struct opd_s
{
   uint32_t func;
   uint32_t toc;
};

struct importStub_s
{
   int16_t ssize;
   int16_t header1;
   int16_t header2;
   int16_t imports;
   int32_t zero1;
   int32_t zero2;
   const char* name;
   uint32_t* fnid;
   opd_s** stub;
   int32_t zero3;
   int32_t zero4;
   int32_t zero5;
   int32_t zero6;
};

struct exportStub_s
{
   int16_t ssize;
   int16_t header1;
   int16_t header2;
   int16_t exports; // number of exports
   int32_t zero1;
   int32_t zero2;
   const char* name;
   uint32_t* fnid;
   opd_s** stub;
};

uint32_t GetCurrentToc();
int WriteProcessMemory(uint32_t pid, void* address, const void* data, size_t size);
int ReadProcessMemory(uint32_t pid, void* address, void* data, size_t size);

template<typename T>
inline T VshGetMem(uint32_t address)
{
   T data;
   ReadProcessMemory(sys_process_getpid(), (void*)address, &data, sizeof(T));
   return data;
}

template<typename T>
inline void VshSetMem(uint32_t address, T data)
{
   WriteProcessMemory(sys_process_getpid(), (void*)address, &data, sizeof(T));
}

template <typename R, typename... Args>
__ALWAYS_INLINE R CallByAddr(uint32_t addr, Args... args)
{
	volatile opd_s opd = { addr, GetCurrentToc() };
	R(*fn)(Args...) = (R(*)(Args...))&opd;
	return fn(args...);
}

template <typename R, typename... Args>
__ALWAYS_INLINE R CallByOpd(opd_s opd, Args... args)
{
	if (opd.sub && opd.toc) 
	{
		R(*fn)(Args...) = (R(*)(Args...))&opd;
		return fn(args...);
	}
	return (R)0;
}

template<typename R, typename... Args>
__ALWAYS_INLINE R CallVmtMethodByClassAddr(uint32_t addr, int index, Args... args)
{
	opd_s** opd = reinterpret_cast<opd_s**>(addr);
	R(*fn)(Args...) = (R(*)(Args...))opd[index];
	return fn(args...);
}