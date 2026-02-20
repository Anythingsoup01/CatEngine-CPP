#include "StreamReader.h"
#include "cepch.h"

namespace CatEngine {

void StreamReader::ReadBuffer(Buffer &buffer, uint32_t size) {
  buffer.Size = size;
  if (size == 0)
    ReadData((char *)&buffer.Size, sizeof(uint64_t));

  buffer.Allocate(buffer.Size);
  ReadData((char *)buffer.Data, buffer.Size);
}

void StreamReader::ReadString(std::string &str) {
  size_t size;
  ReadData((char *)&size, sizeof(size_t));

  str.resize(size);
  ReadData((char *)str.data(), sizeof(char) * size);
}

} // namespace CatEngine
