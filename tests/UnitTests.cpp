#include "pch.h"

#define BOOST_TEST_MODULE AlgorithmTests
#include <boost/chrono/ceil.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/thread/future.hpp>

#include "delete_elements_till.h"

using int_container_t = std::vector<int>;

void run_test(int_container_t elements, size_t expected, size_t expected_strict)
{
	using namespace boost::lambda;
	
	decltype(elements) max_len_seq;
	max_len_seq.reserve(elements.size());
	algo::delete_till_copy(elements.begin(), elements.end(), std::back_inserter(max_len_seq), boost::lambda::_1 <= boost::lambda::_2);
	BOOST_TEST_PASSPOINT();
	
	decltype(elements)  max_len_seq_strict;
	max_len_seq_strict.reserve(elements.size());
	algo::delete_till_copy(elements.begin(), elements.end(), std::back_inserter(max_len_seq_strict), boost::lambda::_1 < boost::lambda::_2);
	BOOST_TEST_PASSPOINT();
	
	BOOST_TEST(max_len_seq.size() == expected);
	BOOST_TEST(max_len_seq_strict.size() == expected_strict);
};
void run_test(int_container_t elements, size_t expected)
{
	run_test(std::move(elements), expected, expected);
};

BOOST_AUTO_TEST_CASE(ProofByInduction)
{
	boost::unit_test::unit_test_log.set_threshold_level(boost::unit_test::log_level::log_test_units);
	
	// Док-во по индукции
	run_test({ 1 }, 1);
	BOOST_TEST_PASSPOINT();
	
	run_test({ 1,2 }, 2);
	BOOST_TEST_PASSPOINT();
	run_test({ 2,1 }, 1);
	BOOST_TEST_PASSPOINT();

	run_test({ 1,3,2 }, 2);
	BOOST_TEST_PASSPOINT();
	run_test({ 1,2,3 }, 3);
	BOOST_TEST_PASSPOINT();
	run_test({ 2,1,3 }, 2);
	BOOST_TEST_PASSPOINT();
	run_test({ 2,3,1 }, 2);
	BOOST_TEST_PASSPOINT();
	run_test({ 3,1,2 }, 2);
	BOOST_TEST_PASSPOINT();
	run_test({ 3,2,1 }, 1);

	// рандомные тесты
	run_test({ 1,3,2,4 }, 3);
	BOOST_TEST_PASSPOINT();
	run_test({ 2,1,3,2 }, 2);
	BOOST_TEST_PASSPOINT();

	run_test({ 1,3,2,4,5 }, 4);
	BOOST_TEST_PASSPOINT();

	run_test({ 3,2,3,4,5 }, 4);
	BOOST_TEST_PASSPOINT();

	run_test({ 4,2,3,1,5 }, 3);
	BOOST_TEST_PASSPOINT();

	run_test({ 4,5,1,3,6 }, 3);
	BOOST_TEST_PASSPOINT();

	run_test({ 1,3,6,4,5 }, 4);
	BOOST_TEST_PASSPOINT();

	run_test({ 1,4,2,3,4 }, 4);
	BOOST_TEST_PASSPOINT();

	run_test({ 2,1,1,2,1 }, 3, 2);

}

BOOST_AUTO_TEST_CASE(DeleteOneElementFromALotOfValues)
{
	int_container_t a_lot_of_values(1000, 2);
	BOOST_TEST_PASSPOINT();
	
	a_lot_of_values.push_back(1);
	BOOST_TEST_PASSPOINT();
	
	run_test(std::move(a_lot_of_values), 1000, 1);
}