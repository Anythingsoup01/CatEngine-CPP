#pragma once

namespace CatEngine {
class ProjectSerializer {
public:
  bool Serialize(const std::filesystem::path &filePath);
  bool Deserialize(const std::filesystem::path &filePath);

private:
};
} // namespace CatEngine
