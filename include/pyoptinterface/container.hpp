#pragma once

#include <vector>
#include <algorithm>
#include <concepts>

#include "ankerl/unordered_dense.h"
#include "pyoptinterface/core.hpp"

// index as the key, variable as the value
// index is monotone increasing
// if we want to delete	a index, we can set the value to -1
template <std::integral T>
class MonotoneVector
{
  private:
	std::vector<T> m_data;
	T m_start = 0;
	bool m_is_dirty = false;
	IndexT m_first_dirty_index = 0;
	std::size_t m_cardinality = 0;

  public:
	MonotoneVector() = default;
	MonotoneVector(T start) : m_start(start)
	{
	}

	IndexT add_index()
	{
		m_data.push_back(m_start);
		if (!m_is_dirty)
		{
			m_is_dirty = true;
			m_first_dirty_index = m_data.size() - 1;
		}
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
		if (!m_is_dirty)
		{
			m_is_dirty = true;
			m_first_dirty_index = index;
		}
		else if (index < m_first_dirty_index)
		{
			m_first_dirty_index = index;
		}
		m_data[index] = -1;
		m_cardinality -= 1;
	}
	bool has_index(const IndexT &index) const
	{
		return m_data[index] >= 0;
	}
	T &get_index(const IndexT &index)
	{
		if (index >= m_first_dirty_index)
		{
			update();
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
	void update()
	{
		// from the first element
		// if element >= 0 then the count++
		if (!m_is_dirty)
		{
			return;
		}

		T counter;
		if (m_first_dirty_index == 0)
		{
			counter = m_start;
		}
		else
		{
			counter = m_data[m_first_dirty_index - 1];
		}
		for (IndexT i = m_first_dirty_index; i < m_data.size(); i++)
		{
			if (m_data[i] >= 0)
			{
				m_data[i] = counter;
				counter++;
			}
		}
		m_is_dirty = false;
		m_first_dirty_index = 0;
	}
	void clear()
	{
		m_data.clear();
		m_is_dirty = false;
		m_first_dirty_index = 0;
		m_cardinality = 0;
	}
};
