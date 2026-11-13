#pragma once

namespace CatRuntime {

struct Vector2 {
public:
  Vector2() = default;
  Vector2(float x, float y) : X(x), Y(y) {}

public:
  float X;
  float Y;
};

struct Vector3 {
public:
  Vector3() = default;
  Vector3(float x, float y, float z) : X(x), Y(y), Z(z) {}

  Vector2 xy() { return Vector2(X, Y); }

public:
  float X;
  float Y;
  float Z;
};

struct Vector4 {
public:
  Vector4() = default;
  Vector4(float x, float y, float z, float a) : X(x), Y(y), Z(z), A(a) {}

  Vector2 xy() { return Vector2(X, Y); }
  Vector3 xyz() { return Vector3(X, Y, Z); }

public:
  float X;
  float Y;
  float Z;
  float A;
};

} // namespace CatRuntime
