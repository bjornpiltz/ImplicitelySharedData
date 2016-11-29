#include "gtest/gtest.h"
#include "SharedInt.h"
#include "Singleton.h"

GTEST_TEST(BasicTest, DefaultConstructed)
{
    SharedInt::AllowAllocations(false);
    EXPECT_EQ(0, SharedInt::ReferenceCount());

    SharedInt a, b, c;
    EXPECT_EQ(0, a.value());
    EXPECT_EQ(0, b.value());
    EXPECT_EQ(0, c.value());

    EXPECT_EQ(1, SharedInt::ReferenceCount());

    // No heap memory has been allocated so far.
    EXPECT_EQ(0, SharedInt::AllocatedCount());

    //All three variables point to the shared null
    EXPECT_EQ(a.d.pointer, b.d.pointer);
    EXPECT_EQ(b.d.pointer, c.d.pointer);
}

GTEST_TEST(BasicTest, StandardUsage)
{
    {
        EXPECT_EQ(0, SharedInt::AllocatedCount());

        // ReferenceCount will include the "shared null" object,
        // so it will always be 1 larger than expected.
        EXPECT_EQ(1, SharedInt::ReferenceCount());

        SharedInt x(1), y(2), z(3);

        // No variables have been allocated on the heap.
        EXPECT_EQ(0, SharedInt::AllocatedCount());

        EXPECT_EQ(3+1, SharedInt::ReferenceCount());

        // Check their values
        EXPECT_EQ(1, x.value());
        EXPECT_EQ(2, y.value());
        EXPECT_EQ(3, z.value());

        y = x;
        EXPECT_EQ(2+1, SharedInt::ReferenceCount());

        z = y;
        EXPECT_EQ(1+1, SharedInt::ReferenceCount());

        // Now two have been freed, and all three point to the same value(1).
        EXPECT_EQ(1, x.value());
        EXPECT_EQ(1, y.value());
        EXPECT_EQ(1, z.value());

        // The following line changes x, but leaves y and z unchanged.
        x.setValue(4);

        EXPECT_EQ(2+1, SharedInt::ReferenceCount());
        EXPECT_EQ(4, x.value());
        EXPECT_EQ(1, y.value());
        EXPECT_EQ(1, z.value());
    }

    // Check that all memory has been freed
    EXPECT_EQ(1, SharedInt::ReferenceCount());
}

GTEST_TEST(BasicTest, Arrays)
{
    SharedInt::AllowAllocations(false);

    static const int Size = 256;

    // The following line triggers no heap allocation:
    SharedInt array[Size];
    EXPECT_EQ(1, SharedInt::ReferenceCount());

    // Check that a single element has the size of one pointer: 
    EXPECT_EQ(sizeof(void*), sizeof(array)/Size);

    std::vector<SharedInt> vector(100);
    EXPECT_EQ(1, SharedInt::ReferenceCount());
}

struct Data { int value = 0; };
GTEST_TEST(BasicTest, Failure)
{
    COW<Data> d;

    // Access should work on a default constructed object.
    d->value = 1;
}

GTEST_TEST(BasicTest, Count)
{
    COW<int> a(2);
    EXPECT_EQ(2, a.constData());
    EXPECT_EQ(1, a.count());

    COW<int> b = COW<int>(4);
    EXPECT_EQ(4, b.constData());
    EXPECT_EQ(1, b.count());

    COW<int> c = a;
    EXPECT_EQ(2, a.count());
    EXPECT_EQ(2, c.count());
    EXPECT_EQ(c.constData(), a.constData());

    a.data() = a.constData();
    EXPECT_EQ(1, a.count());
    EXPECT_EQ(1, c.count());
    EXPECT_EQ(2, a.constData());
    EXPECT_EQ(4, b.constData());
    EXPECT_EQ(2, c.constData());

    c.data() = a.constData();
    EXPECT_EQ(1, a.count());
    EXPECT_EQ(1, c.count());
}

static int ctor_count = 0;
static int copy_count = 0;
static int forwarding_count = 0;
static int assignment_count = 0;

GTEST_TEST(BasicTest, perfectForwarding)
{
    struct ConstructorTester
    {
        ConstructorTester()
        {
            ctor_count++;
        }
        ConstructorTester(const ConstructorTester&)
        {
            copy_count++;
        }
        ConstructorTester(ConstructorTester&&)
        {
            forwarding_count++;
        }
        ConstructorTester& operator=(const ConstructorTester&)
        {
            assignment_count++;
            return *this;
        }
        static ConstructorTester get()
        {
            ConstructorTester tmp;
            return std::move(tmp);
        }
    };

    EXPECT_EQ(0, ctor_count);
    EXPECT_EQ(0, copy_count);
    EXPECT_EQ(0, forwarding_count);
    EXPECT_EQ(0, assignment_count);

    COW<ConstructorTester> a, b, c;

    EXPECT_EQ(1, ctor_count);
    EXPECT_EQ(0, copy_count);
    EXPECT_EQ(0, forwarding_count);
    EXPECT_EQ(0, assignment_count);

    COW<ConstructorTester> d(ConstructorTester::get());
    EXPECT_EQ(2, ctor_count);
    EXPECT_EQ(0, copy_count);
    EXPECT_NE(0, forwarding_count);
    EXPECT_EQ(0, assignment_count);

    a = c;

    EXPECT_EQ(2, ctor_count);
    EXPECT_EQ(0, copy_count);
    EXPECT_EQ(0, assignment_count);
    
    b.data() = d.constData();

    EXPECT_EQ(2, ctor_count);
    EXPECT_EQ(1, copy_count);
    EXPECT_EQ(1, assignment_count);
}

struct ThereCanBeOnlyOne
{
    int value = 1;
    static int count;
    ThereCanBeOnlyOne()
    {
        EXPECT_EQ(count, 0);
        count++;
    }
//  MOCK_METHOD0(MakeSureDtorIsCalled, void());
    ~ThereCanBeOnlyOne()
    {
        EXPECT_EQ(count, 1);
        //MakeSureDtorIsCalled();
        --count;
    }
    ThereCanBeOnlyOne(const ThereCanBeOnlyOne&)=delete;
    ThereCanBeOnlyOne& operator=(const ThereCanBeOnlyOne&)=delete;
};

int ThereCanBeOnlyOne::count=0;

GTEST_TEST(BasicTest, Singleton)
{
    EXPECT_EQ(ThereCanBeOnlyOne::count, 0);
    ThereCanBeOnlyOne& one = Singleton<ThereCanBeOnlyOne>::get();
    EXPECT_EQ(ThereCanBeOnlyOne::count, 1);
    EXPECT_EQ(one.value, 1);
    ThereCanBeOnlyOne& two = Singleton<ThereCanBeOnlyOne>::get();
    EXPECT_EQ(ThereCanBeOnlyOne::count, 1);
    one.value = 2;
    EXPECT_EQ(one.value, 2);
    EXPECT_EQ(two.value, 2);

    // TODO: make the following line work,
    // Otherwise we are not testing that the dtor is called.
//  EXPECT_CALL(one, MakeSureDtorIsCalled());
}
