#pragma once

namespace CatRuntime
{
    struct Vector2
    {
        float x, y;
    };

    struct Vector3
    {
        float x, y, z;

        Vector2 xy() { return {x, y}; }
    };

    struct Vector4
    {
        float x, y, z, a;

        Vector3 xyz() { return {x, y, z}; }
    };
}
