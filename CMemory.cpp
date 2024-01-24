#include "CMemory.h"
#include <Windows.h>
#include <Psapi.h>

#define IS_2_POW_N(X) (((X) & (X - 1)) == 0)
#define PTR_SZ sizeof(void*)

constexpr size_t DEFAULT_ALIGNMENT = 16;
namespace mem
{
	CMemory mMemory;

	void* memmove(void* dest, const void* src, size_t count)
	{
		char* dest_ptr = static_cast<char*>(dest);
		const char* src_ptr = static_cast<const char*>(src);

		if (dest_ptr < src_ptr || dest_ptr >= src_ptr + count)
		{
			while (count-- > 0)
			{
				*dest_ptr++ = *src_ptr++;
			}
		} else {
			dest_ptr += count;
			src_ptr += count;
			while (count-- > 0)
			{
				*(--dest_ptr) = *(--src_ptr);
			}
		}

		return dest;
	}

	void* aligned_offset_malloc(size_t size, size_t align, size_t offset)
	{
		uintptr_t ptr, retptr, gap;

		if (!IS_2_POW_N(align))
		{
			errno = 22;
			return NULL;
		}
		if (offset >= size && offset != 0)
			size = offset + 1;

		align = (align > PTR_SZ ? align : PTR_SZ) - 1;

		gap = (0 - offset) & (PTR_SZ - 1);

		if ((ptr = (uintptr_t)malloc(PTR_SZ + gap + align + size)) == (uintptr_t)NULL)
			return NULL;

		retptr = ((ptr + PTR_SZ + gap + align + offset) & ~align) - offset;
		((uintptr_t*)(retptr - gap))[-1] = ptr;

		return (void*)retptr;
	}

	void* aligned_malloc(size_t size, size_t alignment)
	{
		return aligned_offset_malloc(size, alignment, 0);
	}

	void aligned_free(void* memblock)
	{
		uintptr_t ptr;

		if (memblock == NULL)
			return;

		ptr = (uintptr_t)memblock;

		ptr = (ptr & ~(PTR_SZ - 1)) - PTR_SZ;

		ptr = *((uintptr_t*)ptr);
		free((void*)ptr);
	}

	void* aligned_offset_realloc(void* memblock, size_t size, size_t align, size_t offset)
	{
		uintptr_t ptr, retptr, gap, stptr, diff;
		uintptr_t movsz, reqsz;
		int bFree = 0;

		if (memblock == NULL)
		{
			return aligned_offset_malloc(size, align, offset);
		}
		if (size == 0)
		{
			aligned_free(memblock);
			return NULL;
		}
		if (offset >= size && offset != 0)
		{
			errno = 22;
			return NULL;
		}

		stptr = (uintptr_t)memblock;

		stptr = (stptr & ~(PTR_SZ - 1)) - PTR_SZ;

		stptr = *((uintptr_t*)stptr);

		if (!IS_2_POW_N(align))
		{
			errno = 22;
			return NULL;
		}

		align = (align > PTR_SZ ? align : PTR_SZ) - 1;
		gap = (0 - offset) & (PTR_SZ - 1);

		diff = (uintptr_t)memblock - stptr;
		movsz = _msize((void*)stptr) - ((uintptr_t)memblock - stptr);
		movsz = movsz > size ? size : movsz;
		reqsz = PTR_SZ + gap + align + size;

		if ((stptr + align + PTR_SZ + gap) < (uintptr_t)memblock)
		{
			if ((ptr = (uintptr_t)malloc(reqsz)) == (uintptr_t)NULL)
				return NULL;
			bFree = 1;
		}
		else
		{
			if ((ptr = (uintptr_t)_expand((void*)stptr, reqsz)) == (uintptr_t)NULL)
			{
				if ((ptr = (uintptr_t)malloc(reqsz)) == (uintptr_t)NULL)
					return NULL;
				bFree = 1;
			}
			else
				stptr = ptr;
		}

		if (ptr == ((uintptr_t)memblock - diff) && !(((size_t)memblock + gap + offset) & ~(align)))
		{
			return memblock;
		}

		retptr = ((ptr + PTR_SZ + gap + align + offset) & ~align) - offset;
		mem::memmove((void*)retptr, (void*)(stptr + diff), movsz);
		if (bFree)
			free((void*)stptr);

		((uintptr_t*)(retptr - gap))[-1] = ptr;
		return (void*)retptr;
	}

	void* aligned_realloc(void* memblock, size_t size, size_t alignment)
	{
		return aligned_offset_realloc(memblock, size, alignment, 0);
	}

	size_t aligned_msize(void* memblock)
	{
		uintptr_t ptr;

		if (memblock == NULL)
			return 0;

		ptr = (uintptr_t)memblock;

		ptr = (ptr & ~(PTR_SZ - 1)) - PTR_SZ;

		ptr = *((uintptr_t*)ptr);
		return _msize((void*)ptr);
	}

	CMemory::CMemory()
	{
	}

	size_t CMemory::mem_usage()
	{
		PROCESS_MEMORY_COUNTERS pmc = {};

		if (HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId()))
		{
			GetProcessMemoryInfo(h, &pmc, sizeof pmc);
			CloseHandle(h);
		}

		return pmc.PagefileUsage;
	}

	void CMemory::mem_compact()
	{
		RegFlushKey(HKEY_CLASSES_ROOT);
		RegFlushKey(HKEY_CURRENT_USER);
	}

	void* CMemory::mem_alloc(size_t size)
	{
		return aligned_malloc(size, DEFAULT_ALIGNMENT);
	}

	void* CMemory::mem_alloc(size_t size, size_t alignment)
	{
		return aligned_malloc(size, alignment);
	}

	void* CMemory::mem_alloc(size_t size, const std::nothrow_t&) noexcept
	{
		return aligned_malloc(size, DEFAULT_ALIGNMENT);
	}

	void* CMemory::mem_alloc(size_t size, size_t alignment, const std::nothrow_t&) noexcept
	{
		return aligned_malloc(size, alignment);
	}

	void* CMemory::small_alloc(size_t size) noexcept
	{
		return aligned_malloc(size, DEFAULT_ALIGNMENT);
	}

	void CMemory::small_free(void* ptr) noexcept
	{
		aligned_free(ptr);
	}

	void* CMemory::mem_realloc(void* ptr, size_t size)
	{
		return aligned_realloc(ptr, size, DEFAULT_ALIGNMENT);
	}

	void* CMemory::mem_realloc(void* ptr, size_t size, size_t alignment)
	{
		return aligned_realloc(ptr, size, alignment);
	}

	void CMemory::mem_free(void* ptr)
	{
		aligned_free(ptr);
	}

	void CMemory::mem_free(void* ptr, size_t alignment)
	{
		aligned_free(ptr);
	}
};

void* operator new(size_t size)
{
	return mem::mMemory.mem_alloc(size);
}

void* operator new(size_t size, const std::nothrow_t&) noexcept
{
	return mem::mMemory.mem_alloc(size);
}

void* operator new(size_t size, align_val_t alignment)
{
	return mem::mMemory.mem_alloc(size, static_cast<size_t>(alignment));
}

void* operator new(size_t size, align_val_t alignment, const std::nothrow_t&) noexcept
{
	return mem::mMemory.mem_alloc(size, static_cast<size_t>(alignment));
}

void operator delete(void* ptr) noexcept
{
	mem::mMemory.mem_free(ptr);
}

void operator delete(void* ptr, align_val_t alignment) noexcept
{
	mem::mMemory.mem_free(ptr, static_cast<size_t>(alignment));
}

void operator delete(void* ptr, size_t) noexcept
{
	mem::mMemory.mem_free(ptr);
}

void operator delete(void* ptr, size_t, align_val_t alignment) noexcept
{
	mem::mMemory.mem_free(ptr, static_cast<size_t>(alignment));
}
