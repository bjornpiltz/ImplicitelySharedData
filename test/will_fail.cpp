#include "gtest/gtest.h"
#include "COW.h"

struct Private { int data=0; };

#ifndef FAIL_LEVEL

GTEST_TEST(WontFail, empty)
{
}

#elif FAIL_LEVEL==1

struct ConstCorrectness
{
    void inspect()const
    {
        d->data = 0;// Error non const access in const function
    }
    COW<Private> d;
};

#elif FAIL_LEVEL==2
GTEST_TEST(WillFail, NoImplicitConVersion)
{
    COW<int> b = 4; // no implicit conversion should be allowed
}
#elif FAIL_LEVEL==3
I can't think of another failing compile test right now.
#endif