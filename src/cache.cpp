#include "pch.h"
#include "cache.h"

namespace algo
{
	template<typename ElementT>
	class Cache<ElementT>::Impl
	{
	public:
		bool is_cached(key_t key)
		void cache(key_t key, cache_t cache)

	};

	template<typename ElementT>
	Cache<ElementT>::Cache()
		: impl_(std::make_unique<Impl>())
	{
	}
	template<typename ElementT>
	Cache<ElementT>::~Cache() = default;

	template<typename ElementT>
	bool Cache<ElementT>::is_cached(key_t key)
	{
		return impl_->is_cached(key);
	}
	template<typename ElementT>
	void Cache<ElementT>::cache(key_t key, cache_t cache)
	{
		return impl_->cache(key, std::move(cache));
	}

	template<typename ElementT>
	typename Cache<ElementT>::cache_pool_t const& Cache<ElementT>::get_cache() const
	{
		return impl_->get_cache();
	}
}
