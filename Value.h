#ifndef value_h_included
#define value_h_included

#ifndef rodb_h_included
#error Please include "rodb.h", don't include Value.h directly
#endif

#include <iostream>
#include <stdexcept>
#include <cassert>

namespace rodb
{
	
#define rodb_to_string_(s) #s
#define rodb_to_string(s) rodb_to_string_(s)

#ifdef CONFIG_NO_LOCATION_INFO
#define rodb_location_info ""
#else
#define rodb_location_info  " in " __FILE__ ", line " rodb_to_string(__LINE__)
#endif

#ifdef CONFIG_NO_EXCEPTIONS
#define rodb_assert_or_throw(e, m) do { assert(e); } while(false)
#else
#define rodb_assert_or_throw(e, m) do { if (!(e)) throw std::runtime_error(m rodb_location_info); } while (false)
#endif

template <typename T> inline T const *offset_ptr(T const *p, size_t by)
{
	return reinterpret_cast<T const *>(reinterpret_cast<char const *>(p) + by);
}

class Value
{
public:
	enum Type
	{
		BOOL = 'b',
		INT = 'i',
		FLOAT = 'f',
		STRING = 's',

		ARRAY = 'a',
		MAP = 'm',
	};
	
	Type type() const
	{
		return static_cast<Type>(header().type_);
	}
	
	bool is_bool() const
	{
		return type() == BOOL;
	}
	
	bool is_int() const
	{
		return type() == INT;
	}
	
	bool is_float() const
	{
		return type() == FLOAT;
	}
	
	bool is_string() const
	{
		return type() == STRING;
	}
	
	bool is_array() const
	{
		return type() == ARRAY;
	}
	
	bool is_map() const
	{
		return type() == MAP;
	}
	
	bool is_scalar() const
	{
		return is_bool() || is_int() || is_float() || is_string();
	}
	
	bool is_compound() const
	{
		return is_array() || is_map();
	}
	
	operator bool() const
	{
		rodb_assert_or_throw(is_bool(), "Value is not convertible to bool");
		return *reinterpret_cast<int32_t const *>(payload()) != 0;
	}
	
	operator int() const
	{
		rodb_assert_or_throw(is_int(), "Value is not convertible to int");
		return static_cast<int>(*reinterpret_cast<int32_t const *>(payload()));
	}

	operator unsigned() const
	{
		rodb_assert_or_throw(is_int(), "Value is not convertible to unsigned");
		return static_cast<unsigned>(*reinterpret_cast<int32_t const *>(payload()));
	}

	operator float() const
	{
		rodb_assert_or_throw(is_float(), "Value is not convertible to float");
		return *reinterpret_cast<float const *>(payload());
	}

	operator char const *() const
	{
		rodb_assert_or_throw(is_string(), "Value is not convertible to string");
		return static_cast<char const *>(payload());
	}

	size_t size() const
	{
		return is_scalar() ? 1 : *reinterpret_cast<int32_t const *>(payload());
	}

	// Array only
	Value operator [](size_t index) const
	{
		rodb_assert_or_throw(is_array(), "Value is not an array");
		rodb_assert_or_throw(index < size(), "Index is out of bounds");
		
		int32_t const *offsets = reinterpret_cast<int32_t const *>(payload()) + 1;
		return Value(offset_ptr(offsets + size(), offsets[index]));
	}

	// The "int" version is for the "array[0]" situation.  With only "size_t" and "char const *"
	// overloads there's an ambiguity problem.  It's very tidious to write "array[(int)0]" all the time.
	Value operator [](int index) const
	{
		return operator []((size_t)index);
	}

	// Map only
	bool has_key(char const *key) const
	{
		Value k = keys();
		for (size_t i = 0; i < k.size(); ++i)
			if (strcmp(k[i], key) == 0)
				return true;

		return false;
	}

	Value keys() const
	{
		rodb_assert_or_throw(is_map(), "Value is not a map");
		return Value(payload(8));
	}

	Value values() const
	{
		rodb_assert_or_throw(is_map(), "Value is not a map");
		return Value(payload(8 + static_cast<uint32_t const *>(payload())[1]));
	}

	Value operator [](char const *key) const
	{
		rodb_assert_or_throw(is_map(), "Value is not a map");
		
		Value k = keys();
		for (size_t i = 0; i < k.size(); ++i)
			if (strcmp(k[i], key) == 0)
				return values()[i];

		rodb_assert_or_throw(false, "Key is not in the map");
	}

private:
	struct Header
	{
		uint32_t type_;
		uint32_t size_;
	};
	
	explicit Value(void const *data): data_(reinterpret_cast<char const *>(data))
	{
		assert(sizeof(Type) == 4); // TODO: This should be a static assert.
		rodb_assert_or_throw(is_scalar() || is_compound(), "Value must be scalar or compound");
	}

	Header const &header() const
	{
		return *header_ptr();
	}

	Header const *header_ptr() const
	{
		return reinterpret_cast<Header const *>(data_);
	}
	
	void const *payload(size_t extra_offset = 0) const
	{
		return offset_ptr(header_ptr() + 1, extra_offset);
	}

	char const *const data_;
	
	// BFF
	friend class Database;
	friend std::ostream &operator <<(std::ostream &stream, Value const &value);
};

template <typename T> inline bool operator ==(Value const &left, T right)
{
	return (T)left == right;
}

template <typename T> inline bool operator ==(T left, Value const &right)
{
	return left == (T)right;
}

// Special handling for "Value == char const *"
template <> inline bool operator ==(Value const &left, char const *right)
{
	return strcmp(left, right) == 0;
}

// Special handling for "char const * == Value"
template <> inline bool operator ==(char const *left, Value const &right)
{
	return strcmp(left, right) == 0;
}

inline std::ostream &operator <<(std::ostream &stream, Value const &value)
{
	return stream
		<< "Type: " << "'" << (char)value.type() << "'" << "\n"
		<< "Size: " << (value.is_compound() ? value.size() : 0);
}

}

#endif