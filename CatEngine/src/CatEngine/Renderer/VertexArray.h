#pragma once

namespace CatEngine
{
    class VertexArray
    {
    public:
        ~VertexArray() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        static Ref<VertexArray> Create();
    };
}
