#ifndef _H_UTILS
#define _H_UTILS

template<intmax_t _val>
struct _abs
{
	static constexpr intmax_t value = _val < 0 ? -_val : _val;
};

template<intmax_t _a, intmax_t _b>
struct _gcdx
{
	static constexpr intmax_t value = _gcdx<_b, _a % _b>::value;
};

template<intmax_t _a>
struct _gcdx<_a, 0>
{
	static constexpr intmax_t value = _a;
};

template<intmax_t _a, intmax_t _b>
struct _gcd
{
	static constexpr intmax_t value = _gcdx<_abs<_a>::value, _abs<_b>::value>::value;
};

template<>
struct _gcd<0, 0>
{
	static constexpr intmax_t value = 1;
};

template<intmax_t _a, intmax_t _b>
struct _lcm
{
	static constexpr intmax_t value = (_a / _gcd<_a, _b>::value) * _b;
};

void ltrim(std::string& str);
void rtrim(std::string& str);
void trim(std::string& str);
void lcase(std::string &str);
void ucase(std::string &str);

std::string toHEX(uint16_t n);

#endif
