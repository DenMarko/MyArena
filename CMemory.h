#pragma once

#include <new>

enum class align_val_t : size_t {};

class CMemory
{
public:
	CMemory();

	void _initialize();
	void _destroy();

	unsigned int stat_calls;

public:
	static constexpr size_t MEM_SIZE_MAX = 128 * sizeof(void *);

public:
	void* mem_alloc(size_t size);
	void* mem_alloc(size_t size, size_t alignment);
	void* mem_alloc(size_t size, const std::nothrow_t&) noexcept;
	void* mem_alloc(size_t size, size_t alignment, const std::nothrow_t&) noexcept;
	void* mem_realloc(void* ptr, size_t size);
	void* mem_realloc(void* ptr, size_t size, size_t alignment);
	void mem_free(void* ptr);
	void mem_free(void* ptr, size_t alignment);

	void* small_alloc(size_t size) noexcept;
	void  small_free (void* ptr) noexcept;
};

extern CMemory mMemory;

extern void* operator new(size_t size);
extern void* operator new(size_t size, const std::nothrow_t&) noexcept;
extern void* operator new(size_t size, align_val_t alignment);
extern void* operator new(size_t size, align_val_t alignment, const std::nothrow_t&) noexcept;
extern void operator delete(void* ptr) noexcept;
extern void operator delete(void* ptr, align_val_t alignment) noexcept;
extern void operator delete(void* ptr, size_t) noexcept;
extern void operator delete(void* ptr, size_t, align_val_t alignment) noexcept;

template <typename T, typename... Args>
inline T* m_new(Args&&... args)
{
	auto ptr = static_cast<T*>(mMemory.mem_alloc(sizeof(T)));
	return new (ptr) T(std::forward<Args>(args)...);
}

template <class T>
inline void m_delete(T*& ptr) noexcept
{
	delete ptr;
	ptr = nullptr;
}

template <class T>
inline T* m_alloc(size_t count)
{
	return (T*)mMemory.mem_alloc(count * sizeof(T));
}

template <class T>
inline void m_free(T*& ptr) noexcept
{
	if (ptr)
	{
		mMemory.mem_free((void*)ptr);
		ptr = nullptr;
	}
}

inline void* m_malloc            (size_t size) { return mMemory.mem_alloc       (size); }
inline void* m_realloc(void* ptr, size_t size) { return mMemory.mem_realloc(ptr, size); }
