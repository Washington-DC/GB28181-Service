#include "pch.h"
#include "Utils.h"
#include <fmt/chrono.h>

std::string LocalTime(time_t time)
{
	return fmt::format("{:%Y-%m-%d %H:%M:%S}", fmt::localtime(time));
}
