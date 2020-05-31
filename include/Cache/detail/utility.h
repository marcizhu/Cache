#pragma once

#include <tuple>
#include <utility>

namespace detail
{
	template<typename T, typename... Args, std::size_t... Indices>
	constexpr T construct_from_tuple(const std::tuple<Args...>& arguments, std::index_sequence<Indices...>)
	{
		return T(std::get<Indices>(arguments)...);
	}

	template<typename T, typename... Args>
	constexpr T construct_from_tuple(const std::tuple<Args...>& args)
	{
		return construct_from_tuple<T>(args, std::make_index_sequence<sizeof...(Args)>());
	}

	template<typename T, typename... Args>
	constexpr T construct_from_tuple(std::tuple<Args...>&& args)
	{
		return construct_from_tuple<T>(std::move(args), std::make_index_sequence<sizeof...(Args)>());
	}
}
