#pragma once
#ifndef SG_NPLOT_HPP
#define SG_NPLOT_HPP

#include "vector"
#include "tuple"
#include "algorithm"
#include "SGUtil.h"

template<typename T, size_t N>
class NPlot
{
public:

	//Pass in the dimension of the array in all N directions - size is max size before looping
	NPlot(size_t size)
	{
		static_assert(std::is_arithmetic<T>::value, "Not an arithmetic type!");
		static_assert(N > 0, "Dimension of plot must be > 0!");

		//Allocate the space for each dimension
		for (int i = 0; i < N; i++)
		{
			m_Data[i].resize(size);
		}

		m_MaxSize = size;
	}

	template<typename... Args>
	void addPoint(Args... args)
	{
		static_assert(sizeof...(args) == N, "Point must be of dimensions N!");
		auto t = std::make_tuple(args...);

		//If about to overrun, shift back
		if (m_Size == m_MaxSize)
		{
			for (int i = 0; i < N; i++)
			{
				memmove_s(&(m_Data[i])[0], m_MaxSize * sizeof(T), &(m_Data[i])[1], (m_MaxSize - 1) * sizeof(T));
			}
			m_Size--;
		}

		//Iterate tuple and add elements
		tupleElements(t);

		m_Size++;
	}

	const T* NData(size_t dim) const 
	{ 
		assert(dim < N && dim >= 0, "Nth data must be in bounds!");
		return &(m_Data[dim])[0]; 
	};

	T MaxN(size_t dim) const
	{
		T maxValue = std::numeric_limits<T>::min();
		for (int i = 0; i < m_Data[dim].size(); i++)
		{
			if ((m_Data[dim])[i] > maxValue)
			{
				maxValue = (m_Data[dim])[i];
			}
		}
		return maxValue;
	}

	T MinN(size_t dim) const
	{
		T minValue = std::numeric_limits<T>::max();
		for (int i = 0; i < m_Data[dim].size(); i++)
		{
			if ((m_Data[dim])[i] < minValue)
			{
				minValue = (m_Data[dim])[i];
			}
		}
		return minValue;
	}

	size_t size() const { return m_Size; }
	size_t maxSize() const { return m_MaxSize; }

	//Doesn't free data as is constant size, but resets
	void clear() 
	{ 
		m_Size = 0;
	}

private:

	//Recursively add elements
	template<size_t I = 0, typename... Args>
	constexpr void tupleElements(std::tuple<Args...> t)
	{
		//If we have iterated through all elements
		if
			constexpr (I == sizeof...(Args))
		{
			//Last case, if nothing is left to iterate, then exit the function
			return;
		}
		else {
			//Add the element
			(m_Data[I])[m_Size] = std::get<I>(t);

			//Next element
			tupleElements<I + 1>(t);
		}
	}

	size_t m_MaxSize = 0;
	size_t m_Size = 0;
	std::array<std::vector<T>, N> m_Data;
};

#endif