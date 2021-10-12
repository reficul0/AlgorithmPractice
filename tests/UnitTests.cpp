#include "pch.h"

#define BOOST_TEST_MODULE AlgorithmTests
#include <boost/chrono/ceil.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/thread/future.hpp>

#include "delete_elements_till.h"

void run_test(std::vector<int> elements, size_t expected)
{
	std::unique_ptr<algo::Cache> cache = std::make_unique<algo::Cache>(
		[](algo::Cache::erased_elements_t const& lhs, algo::Cache::erased_elements_t const& rhs)
		{
			return lhs.size() < rhs.size();
		}
	);

	{
		boost::asio::thread_pool threads(3);
		algo::delete_elements_till(elements, std::less<int>(), cache.get(), threads);
		threads.join();
	}

	auto &elements_by_count_in_cache_desc = cache->get_best_case(elements);
	BOOST_TEST(elements_by_count_in_cache_desc.size() == expected);
};

BOOST_AUTO_TEST_CASE(ProofByInduction)
{
	boost::unit_test::unit_test_log.set_threshold_level(boost::unit_test::log_level::log_test_units);
	// 10 29467 - 41000 us
	// 3  17000 - 23000 us

	// Док-во по индукции
	run_test({ 1 }, 1);

	run_test({ 1,2 }, 2);
	run_test({ 2,1 }, 1);

	run_test({ 1,3,2 }, 2);
	run_test({ 1,2,3 }, 3);
	run_test({ 2,1,3 }, 2);
	run_test({ 2,3,1 }, 2);
	run_test({ 3,1,2 }, 2);
	run_test({ 3,2,1 }, 1);

	// рандомные тесты
	run_test({ 1,3,2,4 }, 3);
	run_test({ 2,1,3,2 }, 2);

	run_test({ 1,3,2,4,5 }, 4);

	run_test({ 3,2,3,4,5 }, 4);

	run_test({ 4,2,3,1,5 }, 3);

	run_test({ 4,5,1,3,6 }, 3);

	run_test({ 1,3,6,4,5 }, 4);

	run_test({ 1,4,2,3,4 }, 4);

	run_test({ 2,1,1,2,1 }, 3);

}

BOOST_AUTO_TEST_CASE(DeleteOneElementFromALotOfValues)
{
	std::vector<int> a_lot_of_values(20, 2);
	a_lot_of_values.push_back(1);
	run_test(std::move(a_lot_of_values), 20);
}

//BOOST_AUTO_TEST_CASE()
//{
//	std::vector<int> container{ 1,4,2,3,4 }; // we need 1, 2, 3, 4
//
//	{
//
//		std::unique_ptr<algo::Cache<int>> cache = std::make_unique<algo::Cache<int>>();
//
//		{
//			boost::asio::thread_pool threads(10);
//			algo::delete_elements_till(container, std::less<int>(), cache.get(), threads);
//			threads.join();
//		}
//
//		std::vector<int> const *max_elements = nullptr;
//		for (auto const& cache_pair : cache->get_cache())
//		{
//			const auto local_max_elements = std::max_element(
//				cache_pair.second.begin(), cache_pair.second.end(),
//				[](decltype(*cache_pair.second.begin()) left, decltype(*cache_pair.second.begin()) right)
//				{
//					return left.second.size() < right.second.size();
//				}
//			);
//
//			if (!max_elements || max_elements->size() < local_max_elements->second.size())
//				max_elements = &local_max_elements->second;
//		}
//	}
//
//	return EXIT_SUCCESS;
//}