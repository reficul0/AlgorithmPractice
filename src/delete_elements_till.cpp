#include "pch.h"
#include "delete_elements_till.h"

#include <utility>


#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <iostream>
#include <iterator>
#include <boost/container/flat_map.hpp>

namespace algo
{
	namespace details
	{
		namespace
		{
			std::vector<element_t> delete_till_copy(
				std::vector<element_t> const &numbers,
				std::function<bool(element_t const&, element_t const&)> comparator
			) noexcept
			{
				if (numbers.size() < 2)
					return numbers;
				
				boost::container::flat_map<element_t const*, element_t const*> prev_indexes;
				prev_indexes.reserve(numbers.size());
				
				size_t max_sequence_len{ 1 };
				element_t const *last_ind{ &numbers.front() };

				{
					boost::container::flat_map<element_t const*, size_t> sequences_lens;
					sequences_lens.reserve(numbers.size());
					for(auto const &element: numbers)
						sequences_lens.emplace(&element, 1);
					
					auto i = numbers.begin();
					for (++i; i != numbers.end(); ++i)
					{
						auto const *addressof_i = &*i;
						for (auto j = numbers.begin(); j != i; ++j)
						{
							auto const *addressof_j = &*j;
							
							size_t seq_len_j = sequences_lens.at(addressof_j) + 1;
							size_t &seq_len_ref_i = sequences_lens.at(addressof_i);
							if (comparator(*addressof_j, *addressof_i)
								&& seq_len_j > seq_len_ref_i)
							{
								if (seq_len_j > max_sequence_len)
								{
									max_sequence_len = seq_len_j;
									last_ind = addressof_i;
								}
								seq_len_ref_i = seq_len_j;
								prev_indexes.insert_or_assign(addressof_i, addressof_j);
							}
						}
					}
				}

				std::vector<element_t> subsequence;
				subsequence.resize(max_sequence_len);
				auto subsequence_current = subsequence.rbegin();
				for (size_t i{ 0 }; i < max_sequence_len; ++i, ++subsequence_current)
				{
					*subsequence_current = *std::exchange(last_ind, prev_indexes[last_ind]);
				}

				return std::move(subsequence);
			}
		}
	}

	std::vector<element_t> delete_till_copy(
		std::vector<element_t> const &elements,
		std::function<bool(element_t const&, element_t const&)> comparator
	) noexcept
	{
		return details::delete_till_copy(elements, std::move(comparator));
	}
}