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
				std::vector<element_t> elements,
				std::function<bool(element_t&, element_t&)> comparator,
				Cache<element_t> *cache,
				boost::asio::thread_pool &executor,
				size_t shift_from_beginning = 0,
				size_t elements_deleted = 0
			) noexcept
			{
				if (std::is_sorted(elements.begin(), elements.end(), comparator))
				{
					cache->cache(boost::make_tuple(shift_from_beginning, elements_deleted), std::move(elements));
					return;
				}

				auto current_elements_deleted = elements_deleted + 1;
				for (size_t current_shift = 0; current_shift != elements.size(); ++current_shift)
				{
					auto current_element_id = shift_from_beginning + current_shift;

					// предполагаем, что точно выполним работу. Поэтому говорим, что задача выполнена ещё до её непосредственного выполнения.
					// функция noexcept. 
					// todo: Если же конструкторы копирования могут кидать исключения, или же исключение может реально вызываться в процессе выполнения задачи, то это надо обработать отдельно, или же переделать кеширование с проверки(read) и кеширвоания после выполнения работы(write).
					if (bool was_it_cached = cache->is_cached(boost::make_tuple(current_element_id, current_elements_deleted)))
						continue;

					auto copy_without_current = std::make_shared<std::vector<element_t>>();
					copy_without_current->reserve(elements.size() - 1);

					// Кастомное копирование содержимого элементов, исключая текущий
					// Альтернатива вида ниже может приводить к избыточному копированию элементов, если текущий не в конце. 
					//		А ещё приводит к избыточному использованию памяти на 1 лишний элемент, а если же делать shrink_to_size, 
					//			то всё станет ещё хуже(избыточная деаллокация памяти и копирование).
					//		Скорее всего элемент не в конце, вероятность чего (N-1)/N. Против веротяности в конце 1/N.
					//			std::vector<element_t> copy_without_current = elements;
					//			copy_without_current.erase(elements.begin() + current_shift)
					{
						if (current_shift != 0)
						{
							if (/*де факто выполняется, т.к. элементы не отсортированы elements.size() > 1 && */
								current_shift == 1)// перед текущим только первый элемент и через пару итераторов его не вставить
								copy_without_current->emplace_back(elements.front());
							else// если бы текущий элемент был первым, то мы бы вышли за границы массива
								copy_without_current->insert(copy_without_current->end(), elements.begin(), elements.begin() + current_shift);
						}

						auto last_element_id = elements.size() - 1;
						if (current_shift != last_element_id)
						{
							if (/*де факто выполняется, т.к. элементы не отсортированы elements.size() > 1 && */
								current_shift == (last_element_id - 1))// после текущего только последний элемент и через пару итераторов его не вставить
								copy_without_current->emplace_back(elements.back());
							else
								copy_without_current->insert(copy_without_current->end(), elements.begin() + current_shift + 1, elements.end());
						}
					}

					boost::asio::post(
						executor,
						[copy_without_current, comparator, cache, &executor, current_element_id,
							current_elements_deleted]
						{
							return delete_elements_till(std::move(*copy_without_current), comparator, cache, executor, current_element_id,
								current_elements_deleted);
						}
					);
				}

			}
		}
	}

	void delete_elements_till(
		std::vector<element_t> elements,
		std::function<bool(element_t&, element_t&)> comparator,
		Cache<element_t> *cache,
		boost::asio::thread_pool &executor
	) noexcept
	{
		details::delete_elements_till(std::move(elements), std::move(comparator), cache, executor);
	}
}