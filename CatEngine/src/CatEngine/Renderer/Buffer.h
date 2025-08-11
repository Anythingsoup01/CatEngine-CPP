#pragma once

namespace CatEngine
{

    enum class ShaderDataType
    {
        None = 0,
        Vec,
        Vec2,
        Vec3,
        Vec4,
        Mat3,
        Mat4,
        U32,
        Int,
        Int2,
        Int3,
        Int4,
        Bool,
    };

    static uint32_t ShaderDataTypeSize(ShaderDataType type)
    {
        switch (type)
        {
            case ShaderDataType::Vec:  return 4 * 1;
            case ShaderDataType::Vec2: return 4 * 2;
            case ShaderDataType::Vec3: return 4 * 3;
            case ShaderDataType::Vec4: return 4 * 4;
            case ShaderDataType::Mat3: return 4 * 3 * 3;
            case ShaderDataType::Mat4: return 4 * 4 * 4;
            case ShaderDataType::Int:  return 4 * 1;
            case ShaderDataType::Int2: return 4 * 2;
            case ShaderDataType::Int3: return 4 * 3;
            case ShaderDataType::Int4: return 4 * 4;
            case ShaderDataType::Bool: return 1;
            default: CE_API_ASSERT(false, "Unknown ShaderDataType!"); return 0;
       }
    }

    struct BufferElements
    {
        ShaderDataType Type;
        uint32_t Size;
        uint32_t Offset;
        bool Normalized;

        BufferElements() {}

        BufferElements(ShaderDataType type, bool normalized = false)
            : Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized) {}

        uint32_t GetComponentCount() const
        {
            switch (Type)
            {
                case ShaderDataType::Vec:  return 1;
                case ShaderDataType::Vec2: return 2;
                case ShaderDataType::Vec3: return 3;
                case ShaderDataType::Vec4: return 4;
                case ShaderDataType::Mat3: return 3 * 3;
                case ShaderDataType::Mat4: return 4 * 4;
                case ShaderDataType::Int:  return 1;
                case ShaderDataType::Int2: return 2;
                case ShaderDataType::Int3: return 3;
                case ShaderDataType::Int4: return 4;
                case ShaderDataType::Bool: return 1;
                default: CE_API_ASSERT(false, "Unknown ShaderDataType!"); return 0;
            }

        }
    };

    class BufferLayout
    {
    public:
        BufferLayout() = default;

        BufferLayout(const std::initializer_list<BufferElements>& elements)
            : m_Elements(elements)
        {
            CalculateOffsetAndStride();
        }

        inline const std::vector<BufferElements>& GetElements() const { return m_Elements; }
        inline uint32_t GetStride() const { return m_Stride; }

        std::vector<BufferElements>::iterator begin() { return m_Elements.begin(); }
        std::vector<BufferElements>::iterator end() { return m_Elements.end(); }
        std::vector<BufferElements>::const_iterator begin() const { return m_Elements.begin(); }
        std::vector<BufferElements>::const_iterator end() const { return m_Elements.end(); }
    private:
        void CalculateOffsetAndStride()
        {
            uint32_t offset = 0;
            m_Stride = 0;
            for (auto& element : m_Elements)
            {
                element.Offset = offset;
                offset += element.Size;
                m_Stride += element.Size;
            }
        }
    private:
        std::vector<BufferElements> m_Elements;
        uint32_t m_Stride;
    };

    class VertexBuffer
    {
    public:
        ~VertexBuffer() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual const BufferLayout& GetLayout() const = 0;
        virtual void SetLayout(const BufferLayout& layout) = 0;
        
        static Ref<VertexBuffer> Create(float* vertices, uint32_t size);
    };

    class IndexBuffer
    {
    public:
        ~IndexBuffer() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual uint32_t GetCount() const = 0;

        static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);
    };
}
