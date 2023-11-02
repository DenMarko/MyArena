#pragma once
#include <stdint.h>
#include <type_traits>
#include <malloc.h>

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
				free(p_Data);
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
			p_Data = (T*)malloc(capacity * sizeof(T));
			p_Data[0] = def_value;
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

			T *data = (T*)realloc(p_Data, iSize * sizeof(T));
			if (data != nullptr)
			{
				p_Data = data;
				capacity = iSize;
			} else {
				T *pData = (T*)malloc(iSize * sizeof(T));
				if (pData != nullptr)
				{
					for (uint32_t i = 0; i < size && i < iSize; i++)
					{
						pData[i] = p_Data[i];
					}

					free(p_Data);
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
		}

		void erase(uint32_t index)
		{
			if (index >= size)
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
	};
}