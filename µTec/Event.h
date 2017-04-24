#pragma once

//Copyright Stefan Reinalter
//https://blog.molecular-matters.com/2011/09/19/generic-type-safe-delegates-and-events-in-c/
namespace µTec
{

template<typename R>
class Event {};


//0 argument method
template<typename R>
class Event <R()>
{
public:
	Event(): instance(nullptr),method(nullptr){};
	~Event(){};

	template<typename C, R (C::*func)()>
	void Bind(C* instance)
	{
		this->instance = instance;
		this->method = &Invoker<C, func>;
	}

	R Invoke() const
	{
		return method(instance);
	}

private:

	template<typename C, R (C::*func)()>
	static R Invoker(void* instance)
	{
		return (static_cast<C*>(instance)->*func)();
	}

	void* instance;
	R (*method) (void*);
};

//1 argument method
template<typename R,typename a0>
class Event <R(a0)>
{
public:
	Event() : instance(nullptr), method(nullptr){};
	~Event(){};

	template<typename C, R (C::*func)(a0)>
	void Bind(C* instance)
	{
		this->instance = instance;
		this->method = &Invoker<C, func>;
	}

	R Invoke(a0 arg0) const
	{
		return method(instance,arg0);
	}

private:

	template<typename C, R (C::*func)(a0)>
	static R Invoker(void* instance,a0 arg0)
	{
		return (static_cast<C*>(instance)->*func)(arg0);
	}

	void* instance;
	R (*method) (void*,a0);
};

//2 argument method
template<typename R, typename a0, typename a1>
class Event <R(a0,a1)>
{
public:
	Event() : instance(nullptr), method(nullptr){};
	~Event(){};

	template<typename C, R(C::*func)(a0,a1)>
	void Bind(C* instance)
	{
		this->instance = instance;
		this->method = &Invoker<C, func>;
	}

	R Invoke(a0 arg0,a1 arg1) const
	{
		return method(instance, arg0,arg1);
	}

private:

	template<typename C, R(C::*func)(a0,a1)>
	static R Invoker(void* instance, a0 arg0,a1 arg1)
	{
		return (static_cast<C*>(instance)->*func)(arg0,arg1);
	}

	void* instance;
	R(*method) (void*, a0,a1);
};

// 3 argument method
template<typename R, typename a0, typename a1, typename a2>
class Event <R(a0, a1, a2)>
{
public:
	Event() : instance(nullptr), method(nullptr){};
	~Event(){};

	template<typename C, R(C::*func)(a0, a1, a2)>
	void Bind(C* instance)
	{
		this->instance = instance;
		this->method = &Invoker<C, func>;
	}

	R Invoke(a0 arg0, a1 arg1, a2 arg2) const
	{
		return method(instance, arg0, arg1, arg2);
	}

private:

	template<typename C, R(C::*func)(a0, a1, a2)>
	static R Invoker(void* instance, a0 arg0, a1 arg1, a2 arg2)
	{
		return (static_cast<C*>(instance)->*func)(arg0, arg1, arg2);
	}

	void* instance;
	R(*method) (void*, a0, a1, a2);
};

// 4 argument method
template<typename R, typename a0, typename a1, typename a2, typename a3>
class Event <R(a0, a1, a2, a3)>
{
public:
	Event() : instance(nullptr), method(nullptr){};
	~Event(){};

	template<typename C, R(C::*func)(a0, a1, a2, a3)>
	void Bind(C* instance)
	{
		this->instance = instance;
		this->method = &Invoker<C, func>;
	}

	R Invoke(a0 arg0, a1 arg1, a2 arg2, a3 arg3) const
	{
		return method(instance, arg0, arg1, arg2, arg3);
	}

private:

	template<typename C, R(C::*func)(a0, a1, a2, a3)>
	static R Invoker(void* instance, a0 arg0, a1 arg1, a2 arg2, a3 arg3)
	{
		return (static_cast<C*>(instance)->*func)(arg0, arg1, arg2, arg3);
	}

	void* instance;
	R(*method) (void*, a0, a1, a2, a3);
};


}