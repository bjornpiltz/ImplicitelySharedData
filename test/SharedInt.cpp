#include "SharedInt.h"
#include "gtest/gtest.h"
#include <atomic>

struct PrivateInt
{
    PrivateInt(int i=0)
        : value(i)
    {
        ++referenceCount;
    }
    PrivateInt(const PrivateInt&)
    {
        ++referenceCount;
    }
    ~PrivateInt()
    {
        --referenceCount;
    }
    int value;

    // Test diagnostics:
    void* operator new(std::size_t count)
    {
        EXPECT_TRUE(allowAllocations);
        ++allocationCount;
        return ::operator new(count);
    }
    void* operator new[](std::size_t count)
    {
        EXPECT_TRUE(allowAllocations);
        ++allocationCount;
        return ::operator new[](count);
    }
    inline void* operator new(std::size_t count, void* where)// placement new
    {
        EXPECT_TRUE(allowAllocations);
        ++allocationCount;
        return where;
    }

    static bool allowAllocations;
    static std::atomic<std::size_t>  allocationCount;
    static std::atomic<std::size_t>  referenceCount;
};

bool PrivateInt::allowAllocations = true;
std::atomic<std::size_t> PrivateInt::allocationCount = { 0 };
std::atomic<std::size_t> PrivateInt::referenceCount = { 0 };

void SharedInt::AllowAllocations(bool on)
{
    PrivateInt::allowAllocations = on;
}

std::size_t SharedInt::ReferenceCount()
{
    return PrivateInt::referenceCount;
}

std::size_t SharedInt::AllocatedCount()
{
    return PrivateInt::allocationCount;
}

SharedInt::SharedInt()
{
}

SharedInt::SharedInt(int i)
    : d(i)
{
}

int SharedInt::value()const
{
    return d->value;
}

void SharedInt::setValue(int value)
{
    const auto& c(d);
    if(c->value==value)
        return;
    d->value = value;
}