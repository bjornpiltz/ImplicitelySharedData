#include "gtest/gtest.h"
#include "COW.h"

struct Private { int data=0; };

// This should be allowed even if ForwardDeclared is never defined.
struct ForwardDeclaration
{
    COW<struct ForwardDeclarared> d;
};

#ifndef FAIL_LEVEL

// Now we define ForwardDeclarared, which is why the following works.
struct ForwardDeclarared {};

GTEST_TEST(WontFail, empty)
{
    ForwardDeclaration object1, object2;
    object1 = object2;
    object2.d.detach();
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
GTEST_TEST(WillFail, NotDefinedType)
{
    // The following line would be an error like:
    // error C2079: 'COW<ForwardDeclaration>::SharedNull::data' uses undefined struct 'ForwardDeclaration'
    ForwardDeclaration object;
}
#endif