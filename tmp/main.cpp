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

// самое простое - решение полным перебором с рекусией.
// todo: У нынешнего алгоритма точно есть дублирование логики. 
//		Если мы удалим сначала 1-й элемет, вызовем функцию и удалим второй, то эта же работа повторится когда мы удалим второй элемент, вызовем функцию и удалим первый. 
//		Нужно же найти другой(отличный от того  что описан ниже с кешированем) способ не допускать вызовов в уже проверенных последовательностях удаления. Потому что нужно даже не делать вызова для уже проверенных последовательностей.
//		Думаю, что этот способ такой: во время первого вызова функции, после прохода с первым элементом, первый элемент более не должен попадать в массив, т.к. все комбинации с ним уже проверены. Ну то есть отбрасываем часть, которая левее текущего элемента всегда.
//		Этот вопрос можно решить создав таблицу, в которой будут кешироваться результаты(
//			по аналогии с оптимизацией для вычисления цисел фибоначчи, только у нас последовательность удаления 3-го 1-го и 2-го эквивалентна удалению 1-го 2-го и 3-го и т.д. ну то есть обычным map-ом не обойтись)

// todo: можно ли проще можно ли за O(n*log(n)поОснованию2) или он и так за такое время, если да, то можно ли быстрее? 
//			Возможно ли использовать динаммическое программирование, или более разумный разделяй и влавствуй? 
//			Или возможно нужный алгоритм есть в тех, что дальше(см грокаем)?
// todo: template itreators?
std::vector<int> recursive_delete_elements_untill(std::vector<int> elements, std::function<bool(int&, int&)> comparator)
{
	if (std::is_sorted(elements.begin(), elements.end(), comparator))
		return std::move(elements);

	auto current = elements.begin();
	auto end = elements.end();

	std::vector<int> max_elements;
	for (; current != end; ++current)
	{
		std::vector<int> copy_without_current;
		copy_without_current.reserve(elements.size() - 1);
		// копируем содержимое элементов, исключая текущий
		{
			// Думаю, что этот способ такой: во время первого вызова функции, после прохода с первым элементом, первый элемент более не должен попадать в массив, т.к. все комбинации с ним уже проверены. Ну то есть отбрасываем часть, которая левее текущего элемента всегда.
			// Нет! Отбрасывать левую часть не получится, потому что тогда мы теряем часть промежуточных решений(сотрированных массивов). Эта потеря происходит у последовательностей, включающих элементы привее текущего(для примера потести на {1,2,3} представив решение в виде дерева)
			// todo: Видимо придётся кешировать резульаты?
			//			todo: какой ключ использовать для кешировния? 
			//			значение указателя не подходит, но подолошло бы число - индекс исходного массива, 
			//				но тогда надо как-то понять в рекурсивных вызовах какой индекс у элемента в исходном массиве
			//			или же можно сделать число - индекс локального массива и сделать вектор со вложенным вектором со вложенным вектором(вложенность по количеству элементов, но некоторые столбы будут разреженными, потому что дублируют результат, но лучше выбрать тогда не вектор векторов векторов...).
			//			Думаю да, но есть проблема см ниже -> Будем смотреть не по вычёркиваемым чисам, а по оставшимся, то есть смотреть на ситуацию с точки зрения, что при вычёркивании 1-го и 2-го и 2-го и 1-го у нас и там и там остаётся 3-й?
			//			См ниже, сюда -> Только проблема в том, что нам придётся бегать по этому вектору векторов проверяя ВСЕ оставшиеся индексы.
			//				Обходной путь, который не подходит, т.к. не учитывает элементы не в конце -> Думаю, что из-за специфики порядка вычёркивания(от начала до конца) можно не хранить вектор векторов, а достаточно хранить цифру - количество элементов в конце, которое уже было проверено.
			//				TODO: Поможет????? Поэтому доработаем обходной путь, и пусть будет 2 ключа - количество элементов перед текущим и после текущего. Вроде как это позволит нам не завязываться на последовательность индексов, при этом результат будет тот же.
			if (current != elements.begin())
			{
				if (/*де факто выполняется, т.к. элементы не отсортированы elements.size() > 1 && */
					current == (elements.begin() + 1))// перед текущим только первый элемент и через пару итераторов его не вставить
					copy_without_current.emplace_back(elements.front());
				else// если бы текущий элемент был первым, то мы бы вышли за границы массива
					copy_without_current.insert(copy_without_current.end(), elements.begin(), current - 1);
			}

			auto last_element_iter = elements.end() - 1;
			if (current != last_element_iter)
			{
				if (/*де факто выполняется, т.к. элементы не отсортированы elements.size() > 1 && */
					current == (last_element_iter - 1))// после текущено только последний элемент и через пару итераторов его не вставить
					copy_without_current.emplace_back(elements.back());
				else
					copy_without_current.insert(copy_without_current.end(), current + 1, elements.end());
			}
		}

		auto result = recursive_delete_elements_untill(std::move(copy_without_current), comparator);
		if (std::is_sorted(result.begin(), result.end(), comparator) && result.size() > max_elements.size())
			max_elements = std::move(result);
	}

	return std::move(max_elements);
}


int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "Russian");

	std::vector<int> container{ 1,4,2,3,4 }; // we need 1, 2, 3, 4

	auto result = recursive_delete_elements_untill(std::move(container), std::less<int>());

	return EXIT_SUCCESS;
}