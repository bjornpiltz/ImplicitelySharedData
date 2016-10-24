#pragma once
#include "COW.h"

class SharedInt
{
public:
    SharedInt();
    SharedInt(int i);

    int value()const;
    void setValue(int);


    COW<struct PrivateInt> d;

    static void AllowAllocations(bool on=true);
    static std::size_t AllocatedCount();
    static std::size_t ReferenceCount();
};
