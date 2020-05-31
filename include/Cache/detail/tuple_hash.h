#pragma once

#include <tuple>      // std::tuple
#include <utility>    // std::get, std::make_index_sequence
#include <functional> // std::hash

namespace std
{
	template<typename... Ts>
	struct hash<std::tuple<Ts...>>
	{
		using argument_type = std::tuple<Ts...>;
		using result_type = std::size_t;

		result_type operator()(const argument_type& argument) const { return hash_tuple(argument, std::make_index_sequence<sizeof...(Ts)>()); }

	private:
		template<std::size_t I, std::size_t... Is>
		result_type hash_tuple(const argument_type& tuple, std::index_sequence<I, Is...>) const
		{
			auto value = std::get<I>(tuple);
			auto current = std::hash<decltype(value)>{}(value);
			auto seed = hash_tuple(tuple, std::index_sequence<Is...>());

			// http://www.boost.org/doc/libs/1_35_0/doc/html/boost/hash_combine_id241013.html
			return current + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}

		result_type hash_tuple(const argument_type&, std::index_sequence<>) const { return 0; }
	};
}
