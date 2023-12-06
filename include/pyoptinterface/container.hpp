#pragma once

#include <vector>
#include <algorithm>
#include <concepts>

#include "pyoptinterface/core.hpp"

// index as the key, variable as the value
// index is monotone increasing
// if we want to delete	a index, we can set the value to -1
template <std::signed_integral T>
class MonotoneVector
{
  private:
	std::vector<T> m_data;
	T m_start = 0;
	T m_update_start = 0;
	std::size_t m_last_correct_index = 0;
	std::size_t m_cardinality = 0;

  public:
	MonotoneVector() = default;
	MonotoneVector(T start) : m_start(start), m_update_start(start)
	{
	}

	IndexT add_index()
	{
		m_data.push_back(m_start);
		IndexT index = m_data.size() - 1;
		m_cardinality += 1;
		return index;
	}
	void delete_index(const IndexT &index)
	{
		if (m_data[index] < 0)
		{
			return;
		}
		if (index < m_last_correct_index)
		{
			m_update_start = m_data[index];
			m_last_correct_index = index;
		}
		m_data[index] = -1;
		m_cardinality -= 1;
	}
	bool has_index(const IndexT &index)
	{
		return get_index(index) >= 0;
	}
	T &get_index(const IndexT &index)
	{
		if (index >= m_data.size())
		{
			throw std::runtime_error("Index out of range");
		}
		if (m_data[index] >= 0 && index > m_last_correct_index)
		{
			update_to(index);
		}
		return m_data[index];
	}
	std::size_t num_active_indices() const
	{
		return m_cardinality;
	}
	std::vector<IndexT> get_active_indices() const
	{
		std::vector<IndexT> indices;
		indices.reserve(m_cardinality);
		for (IndexT i = 0; i < m_data.size(); i++)
		{
			if (m_data[i] >= 0)
			{
				indices.push_back(i);
			}
		}
		return indices;
	}
	void update_to(IndexT index)
	{
		// we ensure that m_data[index] >= 0
		T counter = m_update_start;
		constexpr int STEP_STATE = 1;
		constexpr int JUMP_STATE = 2;
		int state;
		std::size_t jump_start_index;
		if (m_data[m_last_correct_index] < 0)
		{
			state = JUMP_STATE;
			jump_start_index = m_last_correct_index;
		}
		else
		{
			state = STEP_STATE;
			counter++;
		}
		for (IndexT i = m_last_correct_index + 1; i <= index;)
		{
			switch (state)
			{
			case JUMP_STATE:
				if (m_data[i] >= 0)
				{
					m_data[jump_start_index] = -i;
					state = STEP_STATE;
					m_data[i] = counter;
					counter++;
					i++;
				}
				else
				{
					T new_i = -m_data[i];
					if (new_i > index)
					{
						i = new_i;
					}
					else
					{
						i++;
					}
				}
				break;
			case STEP_STATE:
				if (m_data[i] < 0)
				{
					state = JUMP_STATE;
					jump_start_index = i;
				}
				else
				{
					m_data[i] = counter;
					counter++;
					i++;
				}
				break;
			default:
				throw std::runtime_error("Unknown state");
			}
		}
		m_last_correct_index = index;
		m_update_start = m_data[index];
	}
	void clear()
	{
		m_data.clear();
		m_update_start = m_start;
		m_last_correct_index = 0;
		m_cardinality = 0;
	}
};
