#pragma once

#include "CatEngine/Core/Buffer.h"
#include "StreamReader.h"
#include "StreamWriter.h"

#include <filesystem>
#include <fstream>

namespace CatEngine {

////////////////////////////////////////
//  FileStreamWriter  //////////////////
////////////////////////////////////////

class FileStreamWriter : public StreamWriter {
public:
  FileStreamWriter(const std::filesystem::path &path);
  FileStreamWriter(const FileStreamWriter &) = delete;
  virtual ~FileStreamWriter();

  virtual bool IsStreamGood() const final { return m_Stream.good(); }
  virtual uint64_t GetStreamPosition() final { return m_Stream.tellp(); }
  virtual void SetStreamPosition(uint64_t position) final {
    m_Stream.seekp(position);
  }
  virtual bool WriteData(const char *data, size_t position) final;

private:
  std::filesystem::path m_Path;
  std::ofstream m_Stream;
};

////////////////////////////////////////
//  FileStreamReader  //////////////////
////////////////////////////////////////

class FileStreamReader : public StreamReader {
public:
  FileStreamReader(const std::filesystem::path &path);
  FileStreamReader(const FileStreamReader &) = delete;
  virtual ~FileStreamReader();

  virtual bool IsStreamGood() const final { return m_Stream.good(); }
  virtual uint64_t GetStreamPosition() override { return m_Stream.tellg(); }
  virtual void SetStreamPosition(uint64_t position) override {
    m_Stream.seekg(position);
  }
  virtual bool ReadData(char *destination, size_t size) override;

private:
  std::filesystem::path m_Path;
  std::ifstream m_Stream;
};

} // namespace CatEngine
