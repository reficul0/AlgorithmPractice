#include "pch.h"

#define _SILENCE_FPOS_SEEKPOS_DEPRECATION_WARNING					

#include <list>
#include <unordered_map>
#include <unordered_set>
#include <boost/pool/pool_alloc.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread_pool.hpp>

#include "program_options.h"

// todo: удаляет не наименьшее число элементов.
void delete_elements_untill(std::vector<int> &elements, std::function<bool(int&, int&)> comparator)
{
	auto beginning_of_unsorted_part = std::is_sorted_until(elements.begin(), elements.end(), comparator);

	// todo: нужно удалять неподходящие элементы не по одному, а сразу все
	// auto unsorted_until_iter = std::is_sorted_until(beginning_of_unsorted_part, elements.end(), predicate);
	while (beginning_of_unsorted_part != elements.end())
	{
		elements.erase(beginning_of_unsorted_part);
		beginning_of_unsorted_part = std::is_sorted_until(elements.begin(), elements.end(), comparator);
	}
}

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "Russian");

	std::vector<int> container{ 1,4,2,3,4 }; // we need 1, 2, 3, 4
	delete_elements_untill(container, std::greater<int>());

	return EXIT_SUCCESS;
}