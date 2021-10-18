#pragma once

#ifndef ALGO_DELETE_ELEMENTS_TILL_H
#define ALGO_DELETE_ELEMENTS_TILL_H

namespace algo
{
	// ComparatorT example: std::function<bool(ElementT const&, ElementT const&)>
	template<typename IteratorT, typename OutRBeginIteratorT, typename ComparatorT, typename ElementT = std::decay_t<decltype(*std::declval<IteratorT>())>>
	inline void delete_till_copy(
		IteratorT begin,
		IteratorT end,
		OutRBeginIteratorT out,
		ComparatorT comparator
	) noexcept
	{
		if (begin == end)
			return;

		auto elements_count = std::distance(begin, end);
		boost::container::flat_map<ElementT const*, ElementT const*> prev_indexes;
		prev_indexes.reserve(elements_count);

		size_t max_sequence_len{ 1 };
		ElementT const *last_ind{ &*begin };

		{
			boost::container::flat_map<ElementT const*, size_t> sequences_lens;
			sequences_lens.reserve(elements_count);
			for (auto element = begin; element != end; ++element)
				sequences_lens.emplace(&*element, 1);

			auto i = begin;
			for (++i; i != end; ++i)
			{
				auto const *addressof_i = &*i;
				for (auto j = begin; j != i; ++j)
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

		for (size_t i{ 0 }; i < max_sequence_len; ++i, ++out)
			*out = *std::exchange(last_ind, prev_indexes[last_ind]);
	}
}

#endif