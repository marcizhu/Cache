#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

namespace detail
{
	template<typename... Ts>
	constexpr auto tuple_indices(const std::tuple<Ts...>&)
	{
		return std::make_index_sequence<sizeof...(Ts)>();
	}

	template<typename T, typename... Args, std::size_t... Indices>
	constexpr T construct_from_tuple(const std::tuple<Args...>& arguments, std::index_sequence<Indices...>)
	{
		return T(std::forward<Args>(std::get<Indices>(arguments))...);
	}

	template<typename T, typename... Args>
	constexpr T construct_from_tuple(const std::tuple<Args...>& args)
	{
		return construct_from_tuple<T>(args, tuple_indices(args));
	}

	template<typename T, typename... Args>
	constexpr T construct_from_tuple(std::tuple<Args...>&& args)
	{
		return construct_from_tuple<T>(std::move(args), tuple_indices(args));
	}
}
