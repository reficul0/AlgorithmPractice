#pragma once

#ifndef ALGO_CACHE_H
#define ALGO_CACHE_H

namespace algo
{
	struct CacheByElementsCountInCacheDesc;
	struct CacheByCachedDataKey;
	
	template<typename ElementT>
	class Cache
	{
	public:
		using cache_t = std::vector<int>;
		using key_t = boost::tuple<size_t/*crossed_off_element_id*/, size_t/*crossed_off_elements*/>;

		Cache(size_t elements_count)
			: max_elements_cache(std::make_pair(
				boost::make_tuple(0, 0),
				0
			))
			, cache_()
		{
			assert(elements_count != 0);
			cache_.resize(elements_count);
		}

		bool is_cached(key_t key)
		{
			auto &crossed_off_element_cache = cache_[key.get<0>()];
			
			boost::shared_lock<boost::shared_mutex> read_lock(crossed_off_element_cache.mutex);
			auto cache = crossed_off_element_cache.container.find(key.get<1>());
			return cache != crossed_off_element_cache.container.end();
		}
		void cache(key_t key, cache_t cache)
		{
			auto &crossed_off_element_cache = cache_[key.get<0>()];

			boost::upgrade_lock<boost::shared_mutex> read_lock(crossed_off_element_cache.mutex);
			auto cached_elements = crossed_off_element_cache.container.find(key.get<1>());
			if(cached_elements != crossed_off_element_cache.container.end())
				// это значение уже закешировано
				return;
			{
				boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(read_lock);
				cached_elements = crossed_off_element_cache.container.emplace_hint(
					cached_elements,
					std::make_pair(
						key.get<1>(),
						std::move(cache)
					)
				);
			}
			if(cached_elements->second.size() > max_elements_cache.second)
				max_elements_cache = std::make_pair(key, cached_elements->second.size());
		}
		/**
		 * \warning multithread unsafe
		 */
		cache_t const& get_max_elements_cache() const
		{
			assert(max_elements_cache.second != 0);
			return cache_[max_elements_cache.first.get<0>()].container.at(max_elements_cache.first.get<1>());
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

		using cache_pool_t = std::vector<
			SynchronizedContainer<std::unordered_map<size_t, cache_t>>
		>;
		
		cache_pool_t cache_;
		std::pair<key_t, size_t> max_elements_cache;
	};
}

#endif
