/*
Copyright(c) 2016 Bjoern Piltz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include "Singleton.h"
#include <atomic>
#include <memory>
#include <cstdlib>

/**
 * This is an implementation of the copy-on write idiom 
 * (or implicit sharing: http://doc.qt.io/qt-5/implicit-sharing.html).
 * 
 * It can also be used as an opaque pointer with forward declarated types.
 * 
 * The standard use case looks like this.
 *
 * Header file:

   // SomeClass is reference counted.
   struct SomeClass
   {
      int size()const;
      void resize();
   
   private:
      COW<struct Private> data;
   }
 *
 * Implementation:

   // private part
   struct Private
   {
      std::vector<char> data;
   }

   void SomeClass::size()const
   {
       return d->size();
   }

   void SomeClass::resize()
   {
       // The following line detaches this instance, i.e. makes
       // a copy of other instances point to the same data.
       return d->resize(size()+1);
   }

 * TODO: document exception safety, reentrancy(i.e. thread safety)
 * 
 * The class is safe to use in a multi threaded environment since we 
 * use atomic counters for the reference counting.
 *
 * The class should be exception safe, provided the class T can make the
 * same promise.
 *
 * The class has the binary footprint of one pointer and the default constructor
 * does not allocate any memory on the heap. (all default constructed objects point to
 * the same static sharedNull object)
 */
template<typename T>
class COW
{
public:
    COW();// noexcept if T() is noexcept
    COW(const COW& other)noexcept;
    COW& operator=(const COW& other)noexcept;
    ~COW();

    template<typename... Args>
    explicit COW(Args ... args);// Forwarding constructor

          T* operator->();
    const T* operator->()const noexcept;

          T& data();
    const T& constData()const noexcept;

    void swap(COW& other)noexcept;
    void detach();

private:
    struct ReferenceCounted* pointer = nullptr;

private:
    friend class BasicTest_Count_Test;
    friend class BasicTest_DefaultConstructed_Test;
    int count()const;
};



//////////////////////////////////////////////////////////////////////////////
//                    Implementation details follow:                        //
//////////////////////////////////////////////////////////////////////////////

/**
 * This helper class holds the atomic count and a function pointer do a 'deleter'.
 */
struct ReferenceCounted
{
    ReferenceCounted()=delete;
    void(*deleter)(ReferenceCounted*);
    std::atomic<uint64_t> count;
};

template<typename T>
struct ReferenceCountedData : public ReferenceCounted
{
    T data;
    static void Destroy(ReferenceCounted* pointer)noexcept
    {
        delete reinterpret_cast<ReferenceCountedData*>(pointer);
    }
    template<typename... Args>
    ReferenceCountedData(Args ... args)
        : ReferenceCounted{ &Destroy, {1} }
        , data(std::forward<Args>(args)...)
    {
    }
    struct SharedNull : public ReferenceCountedData<T>
    {
        SharedNull()
        {
            count++;// We set the count to 2 so delete is never called.
        }
    };
};

template<typename T>
inline COW<T>::COW(const COW & other)noexcept
    : pointer(other.pointer)
{
    ++pointer->count;
}

template<typename T>
inline COW<T>& COW<T>::operator=(const COW& other)noexcept
{
    COW(other).swap(*this);
    return *this;
}

template<typename T>
inline T* COW<T>::operator->()
{
    return &data();
}

template<typename T>
inline const T* COW<T>::operator->()const noexcept
{
    return &constData();
}

template<typename T>
inline int COW<T>::count()const
{
    // This function should only ever be accessed through the unit tests.
    return (int)pointer->count;
}

template<typename T>
inline void COW<T>::swap(COW& other)noexcept
{
    std::swap(pointer, other.pointer);
}

template<typename T>
inline T& COW<T>::data()
{
    detach();
    return reinterpret_cast<ReferenceCountedData<T>*>(pointer)->data;
}

template<typename T>
inline const T& COW<T>::constData()const noexcept
{
    return reinterpret_cast<ReferenceCountedData<T>*>(pointer)->data;
}

template<typename T>
inline void COW<T>::detach()
{
    auto& count = pointer->count;
    if (count>1)
    {
        pointer = new ReferenceCountedData<T>(constData());
        --count;
    }
}

template<typename T>
template<typename... Args>
inline COW<T>::COW(Args... args)
    : pointer(new ReferenceCountedData<T>(std::forward<Args>(args)...))
{
}

template<typename T>
inline COW<T>::COW()
    : pointer(&Singleton<typename ReferenceCountedData<T>::SharedNull>::get())
{
    ++pointer->count;
}

template<typename T>
inline COW<T>::~COW()
{
    if (--pointer->count==0)
        pointer->deleter(pointer);
}

