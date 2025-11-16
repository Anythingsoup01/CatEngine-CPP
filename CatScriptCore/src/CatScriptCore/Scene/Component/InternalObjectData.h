#pragma once

#include <cstdint>

#include <stdio.h>

namespace CatRuntime
{
    class InternalObjectData
    {
    public:
        InternalObjectData() { m_InstanceID = 0; }
        InternalObjectData(uint64_t id) { m_InstanceID = id; }


    protected:
        uint64_t m_InstanceID = 0;
    private:
    };
}
