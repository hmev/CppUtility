#pragma once

template <typename T>
class Nullable
{
protected:
	T* val;
	Nullable() { val = nullptr; }
public:
	Nullable(const T& t) { val = new T(t); }
	Nullable(const Nullable<T>& t) { val = new T(t.value()); }
	~Nullable() { delete val; }

	bool isNull() const 
	{ 
		return val == nullptr; 
	}
	
	T value()  const 
	{ 
		return *val; 
	}

	T operator.() const
	{
		// NOT SAFE
		return *value;
	}

	T& operator.()
	{
		// NOT SAFE
		return *value;
	}

public:
	static Nullable<T> null() { return Nullable<T>(); }
};