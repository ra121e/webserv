#ifndef SHAREDPOINTER_HPP
#define SHAREDPOINTER_HPP
#include <cstddef>

template<typename T>
class SharedPointer
{
private:
	T*		ptr;
	int*	ref_count;

public:
	SharedPointer();
	SharedPointer(T* _ptr);
	SharedPointer(const SharedPointer& other);
	SharedPointer&	operator=(const SharedPointer& other);
	~SharedPointer();

	T&				operator*() const;
	T*				operator->() const;
};

template<typename T>
SharedPointer<T>::SharedPointer()
	: ptr(NULL), ref_count(new int(1))
{
}

template<typename T>
SharedPointer<T>::SharedPointer(T* _ptr)
	: ptr(_ptr), ref_count(new int(1))
{
}

template<typename T>
SharedPointer<T>::SharedPointer(const SharedPointer& other)
	: ptr(other.ptr), ref_count(other.ref_count)
{
	if (ref_count) ++(*ref_count);
}

template<typename T>
SharedPointer<T>&	SharedPointer<T>::operator=(const SharedPointer& other)
{
	if (this != &other)
	{
		if (ref_count && --(*ref_count) == 0)
		{
			delete ptr;
			delete ref_count;
		}
		ptr = other.ptr;
		ref_count = other.ref_count;
		if (ref_count) ++(*ref_count);
	}
	return *this;
}

template<typename T>
SharedPointer<T>::~SharedPointer()
{
	if (ref_count && --(*ref_count) == 0)
	{
		delete ptr;
		delete ref_count;
	}
}

template<typename T>
T&	SharedPointer<T>::operator*() const
{
	return *ptr;
}

template<typename T>
T*	SharedPointer<T>::operator->() const
{
	return ptr;
}

#endif
