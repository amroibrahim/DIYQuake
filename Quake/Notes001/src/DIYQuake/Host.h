#pragma once

#include <string>
#include <cstdint>

typedef std::uint8_t byte_t;

struct QuakeParameters
{
	int iArgumentsCount;
	char** szArgumentsValue;
};


class Host
{
public:
	Host();
	~Host();
};

