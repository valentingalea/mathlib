#pragma once

template<
    size_t Begin, 
    size_t End>
struct static_for
{
	template<class Func>
	void operator ()(Func &&f) //TODO: constrain with `enable_if callable`
	{
		f(Begin);

		static_for<Begin + 1, End>()(
			std::forward<Func>(f));
	}
};

template<size_t N>
struct static_for<N, N>
{
	template<class Func>
	constexpr void operator ()(Func &&)
	{
	}
};