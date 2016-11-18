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
    template<typename... Args>
    explicit COW(Args ... args);// Forwarding constructor
    COW(const COW& other)noexcept;
    COW& operator=(const COW& other)noexcept;
    ~COW();

          T* operator->();
    const T* operator->()const noexcept;

          T& data();
    const T& constData()const noexcept;

    void swap(COW& other)noexcept;
    void detach();

private:
    typedef std::atomic<uint64_t> Count;

    T* pointer = nullptr;

    // The rest is memory management.
    // 
    // In addition to the space needed by T, we transparently allocate 
    // space for a delete function and a count variable before the space
    // needed by the actual object.
    // 
    // Header                            pointer points here
    // |                                 | 
    // +----------------+----------------+----------------+----------------
    // |    deleter     | atomic_counter +  actual data T ...
    // +----------------+----------------+----------------+----------------
    typedef void(*Deleter)(COW<T>*);
    struct Header
    {
        Deleter deleter;
        Count count;
    };

    inline Header& header()const
    {
        static_assert(sizeof(Deleter) + sizeof(Count) == sizeof(Header), "We have a problem.");
        return *reinterpret_cast<Header*>(reinterpret_cast<unsigned char *>(pointer) - sizeof(Header));
    }
    
    template<typename... Args>
    static T* Create(Args ... args)
    {
        // We have been asked to create an object of type T.
        void* memory = std::malloc(sizeof(Header) + sizeof(T));
        Header* h = reinterpret_cast<Header*>(memory);

        new(&h->count)Count(1);
        h->deleter = &Destroy;

        T* data = reinterpret_cast<T*>(h+1);
        new (data) T(std::forward<Args>(args)...);
        return data;
    }

    static void Destroy(COW<T>* data)noexcept
    {
        Header& h = data->header();
        h.count.~Count();
        data->pointer->~T();
        std::free(&h);
    }
private:
    COW(Header* header, T* data)noexcept;
    friend class BasicTest_Count_Test;

    struct SharedNull
    {
        Header header = { nullptr, { 2 } };// We set the count to 2 so delete is never called.
        T data;
        COW instance{ &header, &data };
    };

};

template<typename T>
inline COW<T>::COW()
    : pointer(Singleton<SharedNull>::get().instance.pointer)
{
    ++header().count;
}

template<typename T>
template<typename... Args>
inline COW<T>::COW(Args... args)
    : pointer(Create(std::forward<Args>(args)...))
{
}


template<typename T>
inline COW<T>::COW(Header*, T* data)noexcept
    : pointer(data)
{
    //assert(pointer==reinterpret_cast<T*>(header+1));
}

template<typename T>
inline COW<T>::COW(const COW & other)noexcept
    : pointer(other.pointer)
{
    ++header().count;
}

template<typename T>
inline COW<T>::~COW()
{
    auto& h = header();
    if (--h.count==0)
        h.deleter(this);
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
    detach();
    return pointer;
}

template<typename T>
inline const T* COW<T>::operator->()const noexcept
{
    return pointer;
}

template<typename T>
inline T& COW<T>::data()
{
    detach();
    return *pointer;
}

template<typename T>
inline const T& COW<T>::constData()const noexcept
{
    return *pointer;
}

template<typename T>
inline void COW<T>::detach()
{
    auto& count = header().count;
    if (count>1)
    {
        pointer = Create<const T&>(*pointer);
        --count;
    }
}

template<typename T>
inline void COW<T>::swap(COW& other)noexcept
{
    std::swap(pointer, other.pointer);
}
