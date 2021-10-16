#include "pch.h"

#define BOOST_TEST_MODULE AlgorithmTests
#include <boost/chrono/ceil.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/thread/future.hpp>

#include "delete_elements_till.h"

void run_test(std::vector<int> elements, size_t expected, size_t expectd_strict)
{
	auto max_len_seq = algo::delete_till_copy(elements, [](auto left, auto right) { return left <= right; });
	auto max_len_seq_strict = algo::delete_till_copy(elements, [](auto left, auto right) { return left < right; });
	BOOST_TEST(max_len_seq.size() == expected);
	BOOST_TEST(max_len_seq_strict.size() == expectd_strict);
};
void run_test(std::vector<int> elements, size_t expected)
{
	run_test(std::move(elements), expected, expected);
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

	run_test({ 2,1,1,2,1 }, 3, 2);

}

BOOST_AUTO_TEST_CASE(DeleteOneElementFromALotOfValues)
{
	std::vector<int> a_lot_of_values(10000, 2);
	a_lot_of_values.push_back(1);
	run_test(std::move(a_lot_of_values), 10000, 1);
}