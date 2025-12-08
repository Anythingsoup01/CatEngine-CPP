#pragma once

#include <memory>
#include <stdint.h>

namespace CatEngine
{
	struct Buffer
	{
		void* Data = nullptr;
		uint64_t Size = 0;

		Buffer() = default;
		Buffer(uint64_t size)
		: Size(size) 
		{
			Allocate(size);
		}

		Buffer(const void* data, uint64_t size)
			: Data((void*)data), Size(size)
		{
		}

		Buffer(const Buffer&) = default;

		static Buffer Copy(const Buffer& other)
		{
			Buffer buffer;
            buffer.Allocate(other.Size);
			memcpy(buffer.Data, other.Data, other.Size);
			return buffer;
		}

		static Buffer Copy(const void* data, uint64_t size)
		{
			Buffer buffer;
            buffer.Allocate(size);
			memcpy(buffer.Data, data, size);
			return buffer;
		}
		
		void Allocate(uint64_t size)
		{
            delete[] (uint8_t*)Data;
            Data = nullptr;
            Size = size;

            if (size == 0)
                return;

			Data = new uint8_t[size];
		}

		void Release()
		{
	        delete[] (uint8_t*)Data;
			Data = nullptr;
			Size = 0;
		}

        template<typename T>
        T Read(uint64_t offset = 0)
        {
            return *(T*)((uint8_t*)Data + offset);
        }

        template<typename T>
        T Read(uint64_t offset = 0) const
        {
            return *(T*)((uint8_t*)Data + offset);
        }

        void Write(const void* data, uint64_t size, uint64_t offset = 0)
        {
            CE_API_ASSERT(offset + size <= Size, "Buffer overflow");
            memcpy((uint8_t*)Data + offset, data, size);
        }

        template<typename T>
        T* As()
        {
            return (T*)Data;
        }

		operator bool() const
		{
			return (bool)Data;
		}
	};

	struct ScopedBuffer : public Buffer
	{
	    ~ScopedBuffer()
		{
			Release();
		}

        static ScopedBuffer Copy(const void* data, uint64_t size)
        {
            ScopedBuffer buffer;
            buffer.Allocate(size);
            memcpy(buffer.Data, data, size);
            return buffer;
        }

	private:
	};

}
