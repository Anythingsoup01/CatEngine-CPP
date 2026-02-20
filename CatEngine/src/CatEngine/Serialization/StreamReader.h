#pragma once

#include "CatEngine/Core/Buffer.h"

#include <map>

namespace CatEngine {

class StreamReader {
public:
  virtual ~StreamReader() = default;

  virtual bool IsStreamGood() const = 0;
  virtual uint64_t GetStreamPosition() = 0;
  virtual void SetStreamPosition(uint64_t position) = 0;
  virtual bool ReadData(char *destination, size_t size) = 0;

  operator bool() const { return IsStreamGood(); }

  void ReadBuffer(Buffer &buffer, uint32_t size = 0);
  void ReadString(std::string &str);

  template <typename T> void ReadRaw(T &type) {
    bool success = ReadData((char *)&type, sizeof(T));
    CE_ASSERT(success);
  }

  template <typename T> void WriteObject(T &obj) { T::Deserialize(this, obj); }

  template <typename Key, typename Value>
  void ReadMap(std::map<Key, Value> &map, uint32_t size = 0) {
    if (size == 0)
      ReadRaw<uint32_t>(size);

    for (uint32_t i = 0; i < size; i++) {
      Key key;
      if constexpr (std::is_trivial<Key>())
        WriteRaw<Key>(key);
      else
        WriteObject<Key>(key);

      if constexpr (std::is_trivial<Value>())
        WriteRaw<Value>(map[key]);
      else
        WriteObject<Value>(map[key]);
    }
  }

  template <typename Key, typename Value>
  void ReadMap(std::unordered_map<Key, Value> &map, uint32_t size = 0) {
    if (size == 0)
      ReadRaw<uint32_t>(size);

    for (uint32_t i = 0; i < size; i++) {
      Key key;
      if constexpr (std::is_trivial<Key>())
        WriteRaw<Key>(key);
      else
        WriteObject<Key>(key);

      if constexpr (std::is_trivial<Value>())
        WriteRaw<Value>(map[key]);
      else
        WriteObject<Value>(map[key]);
    }
  }

  template <typename Value>
  void ReadMap(std::unordered_map<std::string, Value> &map, uint32_t size = 0) {
    if (size == 0)
      ReadRaw<uint32_t>(size);

    for (uint32_t i = 0; i < size; i++) {
      std::string key;
      ReadString(key);

      if constexpr (std::is_trivial<Value>())
        WriteRaw<Value>(map[key]);
      else
        WriteObject<Value>(map[key]);
    }
  }

  template <typename T>
  void ReadArray(std::vector<T> &array, uint32_t size = 0) {
    if (size == 0)
      ReadRaw<uint32_t>(size);

    array.resize(size);

    for (uint32_t i = 0; i < size; i++) {
      if constexpr (std::is_trivial<T>())
        WriteRaw<T>(array[i]);
      else
        WriteObject<T>(array[i]);
    }
  }

  template <> void ReadArray(std::vector<std::string> &array, uint32_t size) {
    if (size == 0)
      ReadRaw<uint32_t>(size);

    array.resize(size);

    for (uint32_t i = 0; i < size; i++)
      ReadString(array[i]);
  }
};

} // namespace CatEngine
