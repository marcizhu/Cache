/*
 *  This file is part of the Cache library (https://github.com/marcizhu/Cache)
 *
 *  Copyright (C) 2020-2021 Marc Izquierdo
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 */

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
