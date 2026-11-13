#pragma once

#include <cstddef>
#include <memory>
#include <stdint.h>

namespace CatEngine {
struct Buffer {
  void *Data = nullptr;
  uint64_t Size = 0;

  Buffer() = default;
  Buffer(uint64_t size) { Allocate(size); }

  Buffer(const void *data, uint64_t size) : Data((void *)data), Size(size) {}

  Buffer(const Buffer &) = default;

  static Buffer Copy(const Buffer &other) {
    Buffer buffer;
    buffer.Allocate(other.Size);
    memcpy(buffer.Data, other.Data, other.Size);
    return buffer;
  }

  static Buffer Copy(const void *data, uint64_t size) {
    Buffer buffer;
    buffer.Allocate(size);
    memcpy(buffer.Data, data, size);
    return buffer;
  }

  void Allocate(uint64_t size) {
    delete[] (uint8_t *)Data;
    Data = nullptr;
    Size = size;

    if (size == 0)
      return;

    Data = new uint8_t[size];
  }

  void Release() {
    delete[] (uint8_t *)Data;
    Data = nullptr;
    Size = 0;
  }

  template <typename T> T Read(uint64_t offset = 0) {
    return *(T *)((uint8_t *)Data + offset);
  }

  template <typename T> T Read(uint64_t offset = 0) const {
    return *(T *)((uint8_t *)Data + offset);
  }

  void Write(const void *data, uint64_t size, uint64_t offset = 0) {
    if (size == 0)
      size = 8;

    CE_API_ASSERT(offset + size <= Size, "Buffer overflow");
    memcpy((uint8_t *)Data + offset, data, size);
  }

  template <typename T> T *As() { return (T *)Data; }

  operator bool() const { return (bool)Data; }
};

struct ScopedBuffer : public Buffer {
  ~ScopedBuffer() { Release(); }

  static ScopedBuffer Copy(const void *data, uint64_t size) {
    ScopedBuffer buffer;
    buffer.Allocate(size);
    memcpy(buffer.Data, data, size);
    return buffer;
  }

private:
};

struct VectorBuffer {
  std::vector<uint8_t> Data;
  void *PointerValue = nullptr;

  VectorBuffer() = default;

  // Initialize with a pre-existing raw byte blob [count][elements...]
  VectorBuffer(const std::vector<uint8_t> &data) : Data(data) {}

  VectorBuffer(const VectorBuffer &) = default;
  VectorBuffer(VectorBuffer &&) noexcept = default;

  VectorBuffer &operator=(const VectorBuffer &) = default;
  VectorBuffer &operator=(VectorBuffer &&) noexcept = default;

  // --- Vector Metadata Helpers ---

  bool IsEmpty() const { return Data.size() < sizeof(size_t); }

  uint64_t GetCount() const {
    if (IsEmpty())
      return 0;
    return *reinterpret_cast<const size_t *>(Data.data());
  }

  void SetCount(size_t count, uint32_t elementSize) {
    size_t newTotalSize = sizeof(size_t) + (count * elementSize);
    Data.resize(newTotalSize);
    // Write the new count into the first 8 bytes
    *reinterpret_cast<size_t *>(Data.data()) = count;
  }

  // --- Element Access ---

  void *GetElementPointer(uint64_t index, uint32_t elementSize) {
    if (IsEmpty())
      return nullptr;
    size_t count = GetCount();
    if (index >= count)
      return nullptr;

    uint64_t byteOffset = sizeof(size_t) + (index * elementSize);
    if (byteOffset + elementSize > Data.size())
      return nullptr;

    return static_cast<void *>(Data.data() + byteOffset);
  }

  const void *GetElementPointer(uint64_t index, uint32_t elementSize) const {
    return const_cast<VectorBuffer *>(this)->GetElementPointer(index, elementSize);
  }

  // --- Dynamic Modifications (for UI [+] / [-] buttons) ---

  void PushBackDefault(uint32_t elementSize) {
    size_t currentCount = GetCount();
    // Zero-initialize a new element block at the end
    size_t oldSize = Data.size();
    if (oldSize == 0) {
      // First initialization: 8-byte header + 1 element
      size_t newSize = sizeof(size_t) + elementSize;
      Data.resize(newSize, 0);
      *reinterpret_cast<size_t *>(Data.data()) = 1;
    } else {
      size_t newSize = oldSize + elementSize;
      Data.resize(newSize, 0);
      // Increment the count header
      *reinterpret_cast<size_t *>(Data.data()) = currentCount + 1;
    }
  }

  void PopBack(uint32_t elementSize) {
    size_t currentCount = GetCount();
    if (currentCount == 0)
      return;

    if (currentCount == 1) {
      // Drop down to an empty vector blob holding just count = 0
      Data.resize(sizeof(size_t), 0);
      *reinterpret_cast<size_t *>(Data.data()) = 0;
    } else {
      // Shrink by element size and decrement count
      Data.resize(Data.size() - elementSize);
      *reinterpret_cast<size_t *>(Data.data()) = currentCount - 1;
    }
  }

  // --- Utility Methods ---

  void Release() {
    Data.clear();
    Data.shrink_to_fit();
  }

  operator bool() const { return !IsEmpty() && GetCount() > 0; }
};

} // namespace CatEngine
