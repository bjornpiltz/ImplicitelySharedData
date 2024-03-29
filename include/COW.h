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
#include <memory>

/**
 * This is an implementation of the copy-on write idiom 
 * (or implicit sharing: http://doc.qt.io/qt-5/implicit-sharing.html).
 * 
 * It can also be used as an opaque pointer with forward declared types.
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
       // a copy if other instances point to the same data.
       return d->resize(size()+1);
   }

 * TODO: document exception safety, re-entrancy(i.e. thread safety)
 * 
 * The class is safe to use in a multi threaded environment since we 
 * use atomic counters for the reference counting.
 *
 * The class is exception safe, provided the class T can make the
 * same promise.
 *
 * The class has the binary footprint of two pointers and the default constructor
 * does not allocate any memory on the heap. (all default constructed objects point to
 * the same static sharedNull object)
 */
template<typename T>
class COW final
{
public:
    COW() noexcept(noexcept(T()));

    template<typename Arg0, typename... Args>
    explicit COW(Arg0&& arg0, Args&& ... args);// Forwarding constructor

          T* operator->();
    const T* operator->()const noexcept;

          T& data();
    const T& constData()const noexcept;

    void swap(COW&& other)noexcept;
    void detach();

private:
    std::shared_ptr<T> pointer;
    static const std::shared_ptr<T>& sharedNull() noexcept(noexcept(T()));

    friend class BasicTest_Count_Test;
    friend class BasicTest_DefaultConstructed_Test;
    int count()const;
};



//////////////////////////////////////////////////////////////////////////////
//                    Implementation details follow:                        //
//////////////////////////////////////////////////////////////////////////////

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
    return (int)pointer.use_count();
}

template<typename T>
inline void COW<T>::swap(COW&& other)noexcept
{
    pointer.swap(other);
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
    if (!pointer.unique())
        pointer = std::make_shared<T>(*pointer);
}

template<typename T>
template<typename Arg0, typename... Args>
inline COW<T>::COW(Arg0&& arg0, Args&&... args)
    : pointer(std::make_shared<T>(std::forward<Arg0>(arg0), std::forward<Args>(args)...))
{
}

template<typename T>
inline COW<T>::COW() noexcept(noexcept(T()))
    : pointer(sharedNull())
{
}

template<typename T>
const std::shared_ptr<T>& COW<T>::sharedNull()noexcept(noexcept(T()))
{
    static std::shared_ptr<T> sharedNull{std::make_shared<T>()};
    return sharedNull;
}