//
//  __SINGLETON_HPP__
//
// The MIT License (MIT)

// Copyright (c) <2013> <Jiang Bian jbian@uams.edu>

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIElS OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

// Usage:
//   class Foo : public Singleton<Foo> {
//       friend class Singleton<Foo>;

//   private:

//       Foo(const std::string& hello) : hello_(hello)  {
//           std::cout << hello << std::endl;
//       }

//       Foo() : hello_(nullptr) {
//           std::cout << "Foo()" << std::endl;
//       }

//       const std::string& hello_;
//   };

//   int main(int, const char**) {
//     const std::string hello("hello world");
//     Foo::getInstance(hello);
//     Foo::getInstance();
//   }

#ifndef __SINGLETON_HPP__
#define __SINGLETON_HPP__

#include <mutex>
#include <functional>
#include <memory>
#include <utility>

using std::call_once;
using std::once_flag;

template <class T>
class Singleton  {
public:
	template <typename... Args>
	static T& getInstance(Args&&... args) {

		call_once(
			get_once_flag(),
			[](Args&&... args)
			{
				instance_.reset(new T(std::forward<Args>(args)...));
			},
			std::forward<Args>(args)...);

		return *instance_.get();
	}

protected:
	explicit Singleton<T>() {}
	~Singleton<T>() {}

private:
	static std::unique_ptr<T> instance_;
	static once_flag& get_once_flag() {
		static once_flag once_;
		return once_;
	}

	Singleton(const Singleton&) = delete ;
	const Singleton& operator=(const Singleton&) = delete;
};

template<class T> std::unique_ptr<T> Singleton<T>::instance_ = nullptr;


/* Singleton ne fonctionne pas en cas d'appel en dehors du main a cause de std::unique_ptr  */
template <class T>
class Singleton_Static  {
public:
	template <typename... Args>
	static T& getInstance(Args&&... args) {

		call_once(
			get_once_flag(),
			[](Args&&... args)
			{
				instance_ = new T(std::forward<Args>(args)...);
			},
			std::forward<Args>(args)...);

		return *instance_;
	}

protected:
	explicit Singleton_Static<T>() {}
	~Singleton_Static<T>() { if(instance_!=nullptr) { delete instance_ ; } }

private:
	static T *instance_;
	static once_flag& get_once_flag() {
		static once_flag once_;
		return once_;
	}

	Singleton_Static(const Singleton_Static&) = delete ;
	const Singleton_Static& operator=(const Singleton_Static&) = delete;
};
template<class T> T* Singleton_Static<T>::instance_ = nullptr;

#endif
