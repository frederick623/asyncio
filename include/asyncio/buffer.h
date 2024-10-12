
#pragma once 

#include <algorithm>
#include <iostream>
#include <iterator> 
#include <cstddef>
#include <cstring>
#include <string>
#include <vector>

#include <asyncio/asyncio_ns.h>

ASYNCIO_NS_BEGIN

template<typename T>
concept StringLike = std::is_convertible_v<T, std::string_view>;

struct Buffer
{
	friend class Stream;

	template<std::size_t N>
	Buffer(const char(&ar)[N])
	: buffer_(std::begin(ar), std::end(ar))
	{
	}

	Buffer(ssize_t n, const char& x)
	: buffer_(n, x)
	{
	}

	Buffer(void* data, ssize_t n)
	{
		buffer_.resize(n);
		std::memcpy(&buffer_, data, n);
	}

	template <typename T>
	requires StringLike<T>
	Buffer(T x)
	: buffer_(std::begin(x), std::end(x))
	{
	}

	auto data() 
	{
		return buffer_;
	}

	auto size() const
	{
		return buffer_.size();
	}

	void resize(ssize_t size)
	{
		buffer_.resize(size);
	}

	bool empty()
	{
		return buffer_.empty();
	}

private:
	friend std::ostream& operator<<(std::ostream& os, const Buffer& v) 
	{
		std::copy(v.buffer_.begin(), v.buffer_.end(), std::ostream_iterator<char>(std::cout, ""));
		return os;
	}
	
	std::vector<char> buffer_;
};
// using Buffer = std::vector<char>;
ASYNCIO_NS_END
