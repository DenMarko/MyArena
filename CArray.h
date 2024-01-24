#pragma once
#include <stdint.h>
#include <type_traits>
#include "CMemory.h"

namespace Utilite
{
	template<typename T> class CArray
	{
		static_assert(std::is_arithmetic<T>::value, "The data type must be arithmetic.");
	private:
		T *p_Data;
		uint32_t size;
		uint32_t capacity;

		T def_value = T();

	public:
		class iterator
		{
		protected:
			T* m_Ptr;
		public:
			iterator()
			{
				m_Ptr = nullptr;
			}

			iterator(T * ptr)
			{
				m_Ptr = ptr;
			}

			T * base()
			{
				return m_Ptr;
			}

			const T * base() const
			{
				return m_Ptr;
			}

			T & operator*()
			{
				return *m_Ptr;
			}

			T * operator->()
			{
				return m_Ptr;
			}

			iterator & operator++()
			{
				++m_Ptr;
				return (*this);
			}

			iterator operator++(int)
			{
				iterator tmp = *this;
				++m_Ptr;
				return tmp;
			}

			iterator & operator--()
			{
				--m_Ptr;
				return (*this);
			}

			iterator operator--(int)
			{
				iterator tmp = *this;
				--m_Ptr;
				return tmp;
			}

			bool operator==(T * right) const
			{
				return (m_Ptr == right);
			}

			bool operator==(const iterator & right) const
			{
				return (m_Ptr == right.m_Ptr);
			}

			bool operator!=(T * right) const
			{
				return (m_Ptr != right);
			}

			bool operator!=(const iterator & right) const
			{
				return (m_Ptr != right.m_Ptr);
			}

			iterator & operator+=(size_t offset)
			{
				m_Ptr += offset;
				return (*this);
			}

			iterator & operator-=(size_t offset)
			{
				m_Ptr -= offset;
				return (*this);
			}

			iterator operator+(size_t offset) const
			{
				iterator tmp(*this);
				tmp.m_Ptr += offset;
				return tmp;
			}

			iterator operator-(size_t offset) const
			{
				iterator tmp(*this);
				tmp.m_Ptr -= offset;
				return tmp;
			}

			T & operator[](size_t offset)
			{
				return (*(*this + offset));
			}

			const T & operator[](size_t offset) const
			{
				return (*(*this + offset));
			}

			bool operator<(const iterator & right) const
			{
				return m_Ptr < right.m_Ptr;
			}

			bool operator>(const iterator & right) const
			{
				return m_Ptr > right.m_Ptr;
			}

			bool operator<=(const iterator & right) const
			{
				return m_Ptr <= right.m_Ptr;
			}

			bool operator>=(const iterator & right) const
			{
				return m_Ptr >= right.m_Ptr;
			}

			size_t operator-(const iterator & right) const
			{
				return m_Ptr - right.m_Ptr;
			}
		};
	public:
		CArray() : size(0), capacity(0x1000)
		{
			init();
		}

		CArray(uint32_t iSize) : size(0), capacity(iSize)
		{
			init();
		}

		~CArray()
		{
			if (p_Data)
			{
				mem::Free(p_Data);
				p_Data = nullptr;
				size = 0;
				capacity = 0;
			}
		}

		void clear()
		{
			if (p_Data)
			{
				size = 0;
				p_Data[size] = def_value;
			}
		}

		void init()
		{
			p_Data = (T*)mem::Malloc(capacity * sizeof(T));
			p_Data[0] = def_value;
		}

		iterator begin()
		{
			return iterator(p_Data);
		}

		iterator end()
		{
			return iterator(p_Data + size);
		}

		T& operator[](uint32_t index)
		{
			if (index >= 0 && index < size)
				return p_Data[index];

			return def_value;
		}

		const T& operator[](uint32_t index) const
		{
			if (index >= 0 && index < size)
				return p_Data[index];

			return def_value;
		}

		const uint32_t Capacity() const { return capacity; }
		const uint32_t Size() const { return size; }

		void resize(uint32_t iSize)
		{
			if (iSize == capacity)
				return;

			T *data = (T*)mem::Realloc(p_Data, iSize * sizeof(T));
			if (data != nullptr)
			{
				p_Data = data;
				capacity = iSize;
			} else {
				T *pData = (T*)mem::Malloc(iSize * sizeof(T));
				if (pData != nullptr)
				{
					for (uint32_t i = 0; i < size && i < iSize; i++)
					{
						pData[i] = p_Data[i];
					}

					mem::Free(p_Data);
					p_Data = pData;
					capacity = iSize;
				} else { throw std::bad_alloc(); }
			}
		}

		void push(const T& val)
		{
			if (size >= capacity)
				this->resize(capacity + sizeof(T));

			p_Data[size] = val;
			size++;
			p_Data[size] = def_value;
		}

		void push(const T *val, uint32_t count)
		{
			if(val == nullptr || count == 0)
				return;

			if ((size + count) >= capacity)
				this->resize((size + count) + sizeof(T));

			for (uint32_t i = 0; i < count; i++)
			{
				p_Data[size] = val[i];
				size++;
			}
			p_Data[size] = def_value;
		}

		iterator erase(iterator p_where)
		{
			if(p_where < p_Data || p_where >= (p_Data + size))
				return iterator(0);

			uint32_t ofs = p_where - begin();

			if (size > 1)
			{
				T* theend = p_Data + size;
				for(T* ptr = p_where.base() + 1; ptr < theend; ++ptr)
					*(ptr - 1) = *ptr;
			}

			--size;

			return begin() + ofs;
		}

		void erase(uint32_t index)
		{
			if (index < 0 || index >= size)
				return;

			for (uint32_t i = index; i < (size - 1); i++)
				p_Data[i] = p_Data[i + 1];

			size--;
		}

		void erase(uint32_t first_element, uint32_t last_element)
		{
			if(first_element >= size || last_element >= size || first_element > last_element)
				return;

			uint32_t count = last_element - first_element + 1;
			for(uint32_t i = first_element; i + count < size; i++)
				p_Data[i] = p_Data[i + count];

			size -= count;
		}

		T* data()
		{
			return p_Data;
		}

		CArray<T>& operator= (CArray<T>& arr)
		{
			clear();
			push(arr.data(), arr.Size());

			return (*this);
		}
	};
}