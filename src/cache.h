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
		struct CachedData
		{
			key_t key;
			cache_t data;
			size_t crossed_off_element_id() const
			{
				return key.get<0>();
			}
			size_t crossed_off_elements() const
			{
				return key.get<1>();
			}
			typename cache_t::size_type data_size() const
			{
				return data.size();
			}
			CachedData(key_t key, cache_t cache) noexcept(std::is_nothrow_move_constructible<cache_t>::value)
				: data(std::move(cache))
				, key(key)
			{
			}
			
			CachedData(CachedData &&other) noexcept(std::is_nothrow_move_constructible<cache_t>::value)
				: CachedData(other.key, std::move(other.data))
			{
			}
			CachedData& operator=(CachedData &&other) noexcept(std::is_nothrow_move_constructible<cache_t>::value)
			{
				if(this != &other)
				{
					data = std::move(other.data);
					key = other.key;
				}
				return *this;
			}
		};

		using cache_pool_t = boost::multi_index::multi_index_container<
			CachedData,
			boost::multi_index::indexed_by<
				boost::multi_index::ordered_non_unique<
					boost::multi_index::tag<CacheByElementsCountInCacheDesc>,
				    boost::multi_index::const_mem_fun<CachedData, size_t, &CachedData::data_size>,
					std::greater<>
				>,
				boost::multi_index::hashed_unique<
					boost::multi_index::tag<CacheByCachedDataKey>,
				    boost::multi_index::composite_key<
						CachedData,
						boost::multi_index::const_mem_fun<CachedData, size_t, &CachedData::crossed_off_element_id>,
						boost::multi_index::const_mem_fun<CachedData, size_t, &CachedData::crossed_off_elements>
					>
				>
			>
		>;

		bool is_cached(key_t key)
		{
			boost::shared_lock<decltype(cache_mutex_)> cache_lock(cache_mutex_);
			auto &elements_by_key = cache_.template get<CacheByCachedDataKey>();
			auto data = elements_by_key.find(key);
			return data != elements_by_key.end();
		}
		void cache(key_t key, cache_t cache)
		{
			boost::unique_lock<decltype(cache_mutex_)> cache_lock(cache_mutex_);
			auto &elements_by_key = cache_.template get<CacheByCachedDataKey>();
			elements_by_key.emplace(
				CachedData(key, std::move(cache))
			);
		}
		/**
		 * \warning multithread unsafe
		 */
		cache_pool_t const& get_cache() const
		{
			return cache_;
		}
	private:
		boost::shared_mutex mutable cache_mutex_;
		cache_pool_t cache_;
	};
}

#endif
