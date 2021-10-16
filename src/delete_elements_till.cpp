#include "pch.h"
#include "delete_elements_till.h"

#include <utility>


#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <iostream>
#include <iterator>

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
				std::vector<size_t> prev_indexes(numbers.size());
				size_t max_sequence_len{ 1 };
				size_t last_ind{ 0 };

				{
					std::vector<size_t> sequences_lens(numbers.size(), 1);
					for (size_t i{ 1 }; i < numbers.size(); ++i)
					{
						for (size_t j{ 0 }; j < i; ++j)
						{
							auto const current_seq_len = sequences_lens[j] + 1;
							if (comparator(numbers[j], numbers[i])
								&& current_seq_len > sequences_lens[i])
							{
								sequences_lens[i] = current_seq_len;
								prev_indexes[i] = j;

								if (sequences_lens[i] > max_sequence_len)
								{
									max_sequence_len = sequences_lens[i];
									last_ind = i;
								}
							}
						}
					}
				}

				std::vector<element_t> subsequence;
				subsequence.resize(max_sequence_len);
				auto subsequence_current = subsequence.rbegin();
				for (size_t i{ 0 }; i < max_sequence_len; ++i, ++subsequence_current)
					*subsequence_current = numbers[ std::exchange(last_ind, prev_indexes[last_ind]) ];

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