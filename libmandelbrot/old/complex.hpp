#ifndef COMPLEX_H
#define COMPLEX_H

template<typename T>
struct Complex
{
	T r = 0.0f;
	T i = 0.0f;

	Complex() = default;
	Complex(T r, T i) : r(r), i(i) {}
	Complex& operator+=(Complex rhs)
	{
		r += rhs.r;
		i += rhs.i;
		return *this;
	}
	Complex& operator+(const Complex& rhs)
	{
		*this += rhs;
		return *this;
	}
	Complex& operator*=(Complex rhs)
	{
		Complex temp = *this;
		r = temp.r * rhs.r - temp.i * rhs.i;
		i = temp.r * rhs.i + temp.i * rhs.r;
		return *this;
	}
	Complex& operator*(const Complex& rhs)
	{
		*this *= rhs;
		return *this;
	}
	inline T absSquared()
	{
		return r * r + i * i;
	}
};

#endif //!COMPLEX_H