#pragma once
#include "COW.h"

class SharedInt
{
public:
    SharedInt();
    SharedInt(int i);

    int value()const;
    void setValue(int);

private:
    COW<struct PrivateInt> d;

    static void AllowAllocations(bool on=true);
    static std::size_t AllocatedCount();
    static std::size_t ReferenceCount();

    friend class BasicTest_DefaultConstructed_Test;
    friend class BasicTest_StandardUsage_Test;
    friend class BasicTest_Arrays_Test;
};
