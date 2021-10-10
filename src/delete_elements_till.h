#pragma once

#ifndef ALGO_DELETE_ELEMENTS_TILL_H
#define ALGO_DELETE_ELEMENTS_TILL_H

#include "cache.h"

using element_t = int;

namespace algo
{
	
	/**
	 * \brief 
	 * \param elements 
	 * \param comparator 
	 * \param cache 
	 * \param executor
	 * \code
	 * {
	 * std::vector<int> container{ 1,4,2,3,4 }; // we need 1, 2, 3, 4
	 * std::unique_ptr<algo::Cache<int>> cache = std::make_unique<algo::Cache<int>>();
	 * {
	 *		boost::asio::thread_pool threads(10);
	 *		algo::delete_elements_till(container, std::less<int>(), cache.get(), threads);
	 *		threads.join();
	 * }
	 * \endcode 
	 */
	void delete_elements_till(
		std::vector<element_t> elements,
		std::function<bool(element_t&, element_t&)> comparator,
		Cache<element_t> *cache,
		boost::asio::thread_pool &executor
	) noexcept;
}

#endif