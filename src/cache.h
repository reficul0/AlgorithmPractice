#pragma once

#ifndef ALGO_CACHE_H
#define ALGO_CACHE_H

namespace algo
{
	class Cache
	{
	public:
		using erased_elements_t = std::set<size_t, std::greater<size_t>>;
		using best_case_detector_t = std::function<bool(erased_elements_t const&, erased_elements_t const&)>;
		
		Cache(best_case_detector_t best_case_detector)
			: is_best_case_(std::move(best_case_detector))
			, is_best_case_found_(false)
		{
		}

		void cache(erased_elements_t erased_elements)
		{
			boost::upgrade_lock<boost::shared_mutex> read_lock(erased_elements_.mutex);
			if (is_best_case_found_ == false
				|| is_best_case_(erased_elements, erased_elements_.container))
			{
				boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(read_lock);
				erased_elements_.container = std::move(erased_elements);
				is_best_case_found_ = true;
			}
		}
		/**
		 * \warning multithread unsafe
		 * \throws std::exception if there is no best cases
		 */
		erased_elements_t const& get_best_case_erased_elements() const
		{
			if(is_best_case_found_ == false)
				throw std::logic_error("There is no best case.");
			return erased_elements_.container;
		}

		template<typename ContainerT>
		static auto apply_erasure(ContainerT &&container, erased_elements_t const &erasing_elements_ids)
			-> decltype(std::forward<ContainerT>(container))
		{
			// todo: оптимизация, если элементы идут один за другим, удалить их скопом через 2 итератора
			for (auto &erasing_element_pos : erasing_elements_ids)
				container.erase(container.begin() + erasing_element_pos);

			return std::forward<ContainerT>(container);
		}
		template<typename ElementT>
		static std::vector<ElementT> apply_erasure_copy(std::vector<ElementT> container, erased_elements_t const &erasing_elements_ids)
		{
			return apply_erasure(std::move(container), erasing_elements_ids);
		}
		
		template<typename ElementT>
		std::vector<ElementT>& get_best_case(std::vector<ElementT> &container) const
		{
			return apply_erasure(container, get_best_case_erased_elements());
		}
	private:
		template<typename ContainerT>
		struct SynchronizedContainer
		{
			boost::shared_mutex mutex;
			ContainerT container;
			
			SynchronizedContainer()
			{}
			SynchronizedContainer(SynchronizedContainer &&other) noexcept
				: container(std::move(other.container))
			{
			}
		};

		best_case_detector_t is_best_case_;
		SynchronizedContainer<erased_elements_t> erased_elements_;
		bool is_best_case_found_;
	};
}

#endif
