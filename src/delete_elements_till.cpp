#include "pch.h"
#include "delete_elements_till.h"

#include <utility>

namespace algo
{
	namespace details
	{
		namespace
		{
			void delete_elements_till(
				std::vector<element_t> const &elements,
				std::function<bool(element_t&, element_t&)> comparator,
				Cache *cache,
				boost::asio::thread_pool &executor,
				Cache::erased_elements_t erased_elements_ids = Cache::erased_elements_t{}
			) noexcept
			{
				auto elements_without_erased = Cache::apply_erasure_copy(elements, erased_elements_ids);
				if (elements_without_erased.empty())
					return;
				if (elements_without_erased.size()==1 
					|| std::is_sorted(elements_without_erased.begin(), elements_without_erased.end(), comparator))
				{
					cache->cache(std::move(erased_elements_ids));
					return;
				}
				for (size_t current_element_id = 0; current_element_id < elements.size(); ++current_element_id)
				{
					if (// не допускаем повторных вычислений(вычёркиваем только элементы выше диагонали воображаемой матрицы)
						(erased_elements_ids.empty()==false && *erased_elements_ids.begin() > current_element_id)
						// не вычисляем, если элемент уже вычеркнут
						|| erased_elements_ids.find(current_element_id) != erased_elements_ids.end())
						continue;
					
					auto erased_ids = std::make_shared<Cache::erased_elements_t>(erased_elements_ids);
					erased_ids->emplace(current_element_id);

					boost::asio::post(executor, 
						[&elements, comparator, cache, &executor, moved_erased_ids = std::move(erased_ids)]
						{
							return delete_elements_till(elements, comparator, cache, executor, std::move(*moved_erased_ids));
						}
					);
				}

			}
		}
	}

	void delete_elements_till(
		std::vector<element_t> const &elements,
		std::function<bool(element_t&, element_t&)> comparator,
		Cache *cache,
		boost::asio::thread_pool &executor
	) noexcept
	{
		boost::asio::post(
			executor,
			[&elements, comparator, cache, &executor]
			{
				return details::delete_elements_till(elements, comparator, cache, executor);
			}
		);
	}
}