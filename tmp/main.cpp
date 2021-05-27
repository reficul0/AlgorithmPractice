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

// todo: template?
// todo: перевести с реальной рекурсии на цикл, или же можно сделать хвостовую оптимизацию?
// todo: сделать многопоточной. Как разделить по потокам эту задачу оптимальнее всего? Не забыть блокировать доступ к кешу. ВОзможно придётся переделать кеш, чтобы сделать через read/write блокировки.
std::vector<int> recursive_delete_elements_untill_with_caching(std::vector<int> elements, std::function<bool(int&, int&)> comparator, std::function<bool(size_t, size_t)> &cache_it, size_t shift_from_beginning = 0) noexcept
{
	if (std::is_sorted(elements.begin(), elements.end(), comparator))
		return std::move(elements);

	std::vector<int> max_elements;
	for (size_t current_shift = 0; current_shift != elements.size(); ++current_shift)
	{
		auto current_shift_from_beginning = shift_from_beginning + current_shift;
		auto shift_to_end = elements.size() - current_shift;

		// предполагаем, что точно выполним работу. Поэтому говорим, что задача выполнена ещё до её непосредственного выполнения.
		// функция noexcept. 
		// todo: Если же конструкторы копирования могут кидать исключения, или же исключение может реально вызываться в процессе выполнения задачи, то это надо обработать отдельно, или же переделать кеширование с проверки(read) и кеширвоания после выполнения работы(write).
		bool was_it_cached = cache_it(current_shift_from_beginning, shift_to_end);
		if (was_it_cached)
			continue;

		std::vector<int> copy_without_current;
		copy_without_current.reserve(elements.size() - 1);

		// Кастомное копирование содержимого элементов, исключая текущий
		// Альтернатива вида ниже может приводить к избыточному копированию элементов, если текущий не в конце. 
		//		А ещё приводит к избыточному использованию памяти на 1 лишний элемент, а если же делать shrink_to_size, 
		//			то всё станет ещё хуже(избыточная деаллокация памяти и копирование).
		//		Скорее всего элемент не в конце, вероятность чего (N-1)/N. Против веротяности в конце 1/N.
		//			std::vector<int> copy_without_current = elements;
		//			copy_without_current.erase(elements.begin() + current_shift)
		{
			if (current_shift != 0)
			{
				if (/*де факто выполняется, т.к. элементы не отсортированы elements.size() > 1 && */
					current_shift == 1)// перед текущим только первый элемент и через пару итераторов его не вставить
					copy_without_current.emplace_back(elements.front());
				else// если бы текущий элемент был первым, то мы бы вышли за границы массива
					copy_without_current.insert(copy_without_current.end(), elements.begin(), elements.begin() + current_shift);
			}

			auto last_element_id = elements.size() - 1;
			if (current_shift != last_element_id)
			{
				if (/*де факто выполняется, т.к. элементы не отсортированы elements.size() > 1 && */
					current_shift == (last_element_id - 1))// после текущено только последний элемент и через пару итераторов его не вставить
					copy_without_current.emplace_back(elements.back());
				else
					copy_without_current.insert(copy_without_current.end(), elements.begin() + current_shift + 1, elements.end());
			}
		}

		auto result = recursive_delete_elements_untill_with_caching(std::move(copy_without_current), comparator, cache_it, current_shift_from_beginning);
		if (std::is_sorted(result.begin(), result.end(), comparator) && result.size() > max_elements.size())
			max_elements = std::move(result);
	}

	return std::move(max_elements);
}

std::vector<int> recursive_delete_elements_untill_with_caching2(std::vector<int> elements, std::function<bool(int&, int&)> comparator, std::function<bool(size_t, size_t)> &cache_it, size_t shift_from_beginning = 0, size_t elements_deleted = 0) noexcept
{
	if (std::is_sorted(elements.begin(), elements.end(), comparator))
		return std::move(elements);

	std::vector<int> max_elements;

	auto current_elements_deleted = elements_deleted + 1;
	for (size_t current_shift = 0; current_shift != elements.size(); ++current_shift)
	{
		auto current_element_id = shift_from_beginning + current_shift;

		// предполагаем, что точно выполним работу. Поэтому говорим, что задача выполнена ещё до её непосредственного выполнения.
		// функция noexcept. 
		// todo: Если же конструкторы копирования могут кидать исключения, или же исключение может реально вызываться в процессе выполнения задачи, то это надо обработать отдельно, или же переделать кеширование с проверки(read) и кеширвоания после выполнения работы(write).
		bool was_it_cached = cache_it(current_element_id, current_elements_deleted);
		if (was_it_cached)
			continue;

		std::vector<int> copy_without_current;
		copy_without_current.reserve(elements.size() - 1);

		// Кастомное копирование содержимого элементов, исключая текущий
		// Альтернатива вида ниже может приводить к избыточному копированию элементов, если текущий не в конце. 
		//		А ещё приводит к избыточному использованию памяти на 1 лишний элемент, а если же делать shrink_to_size, 
		//			то всё станет ещё хуже(избыточная деаллокация памяти и копирование).
		//		Скорее всего элемент не в конце, вероятность чего (N-1)/N. Против веротяности в конце 1/N.
		//			std::vector<int> copy_without_current = elements;
		//			copy_without_current.erase(elements.begin() + current_shift)
		{
			if (current_shift != 0)
			{
				if (/*де факто выполняется, т.к. элементы не отсортированы elements.size() > 1 && */
					current_shift == 1)// перед текущим только первый элемент и через пару итераторов его не вставить
					copy_without_current.emplace_back(elements.front());
				else// если бы текущий элемент был первым, то мы бы вышли за границы массива
					copy_without_current.insert(copy_without_current.end(), elements.begin(), elements.begin() + current_shift);
			}

			auto last_element_id = elements.size() - 1;
			if (current_shift != last_element_id)
			{
				if (/*де факто выполняется, т.к. элементы не отсортированы elements.size() > 1 && */
					current_shift == (last_element_id - 1))// после текущено только последний элемент и через пару итераторов его не вставить
					copy_without_current.emplace_back(elements.back());
				else
					copy_without_current.insert(copy_without_current.end(), elements.begin() + current_shift + 1, elements.end());
			}
		}

		auto result = recursive_delete_elements_untill_with_caching2(std::move(copy_without_current), comparator, cache_it, current_element_id, current_elements_deleted);
		if (std::is_sorted(result.begin(), result.end(), comparator) && result.size() > max_elements.size())
			max_elements = std::move(result);
	}

	return std::move(max_elements);
}

class ICache
{
public:
	virtual ~ICache() = default;
	virtual bool is_cached(size_t crossed_of_element_id, size_t crossed_of_elements) = 0;
	virtual void cache(size_t crossed_of_element_id, size_t crossed_of_elements, std::vector<int> result) = 0;
};

void async_recursive_delete_elements_untill_with_caching2(
	std::vector<int> elements, 
	std::function<bool(int&, int&)> comparator, 
	ICache *cache,
	boost::asio::io_service *service, 
	size_t shift_from_beginning = 0, 
	size_t elements_deleted = 0
) noexcept
{
	if (std::is_sorted(elements.begin(), elements.end(), comparator))
	{
		cache->cache(shift_from_beginning, elements_deleted, std::move(elements));
		return;
	}

	auto current_elements_deleted = elements_deleted + 1;
	for (size_t current_shift = 0; current_shift != elements.size(); ++current_shift)
	{
		auto current_element_id = shift_from_beginning + current_shift;

		// предполагаем, что точно выполним работу. Поэтому говорим, что задача выполнена ещё до её непосредственного выполнения.
		// функция noexcept. 
		// todo: Если же конструкторы копирования могут кидать исключения, или же исключение может реально вызываться в процессе выполнения задачи, то это надо обработать отдельно, или же переделать кеширование с проверки(read) и кеширвоания после выполнения работы(write).
		bool was_it_cached = cache->is_cached(current_element_id, current_elements_deleted);
		if (was_it_cached)
			continue;

		std::vector<int> copy_without_current;
		copy_without_current.reserve(elements.size() - 1);

		// Кастомное копирование содержимого элементов, исключая текущий
		// Альтернатива вида ниже может приводить к избыточному копированию элементов, если текущий не в конце. 
		//		А ещё приводит к избыточному использованию памяти на 1 лишний элемент, а если же делать shrink_to_size, 
		//			то всё станет ещё хуже(избыточная деаллокация памяти и копирование).
		//		Скорее всего элемент не в конце, вероятность чего (N-1)/N. Против веротяности в конце 1/N.
		//			std::vector<int> copy_without_current = elements;
		//			copy_without_current.erase(elements.begin() + current_shift)
		{
			if (current_shift != 0)
			{
				if (/*де факто выполняется, т.к. элементы не отсортированы elements.size() > 1 && */
					current_shift == 1)// перед текущим только первый элемент и через пару итераторов его не вставить
					copy_without_current.emplace_back(elements.front());
				else// если бы текущий элемент был первым, то мы бы вышли за границы массива
					copy_without_current.insert(copy_without_current.end(), elements.begin(), elements.begin() + current_shift);
			}

			auto last_element_id = elements.size() - 1;
			if (current_shift != last_element_id)
			{
				if (/*де факто выполняется, т.к. элементы не отсортированы elements.size() > 1 && */
					current_shift == (last_element_id - 1))// после текущено только последний элемент и через пару итераторов его не вставить
					copy_without_current.emplace_back(elements.back());
				else
					copy_without_current.insert(copy_without_current.end(), elements.begin() + current_shift + 1, elements.end());
			}
		}

		service->post(
			[comparator, cache, moved_copy_without_current = std::move(copy_without_current), service, current_element_id, current_elements_deleted]()
			{ 
				async_recursive_delete_elements_untill_with_caching2(std::move(moved_copy_without_current), comparator, cache, service, current_element_id, current_elements_deleted);
			}
		);
	}
	
}

class ThreadsafeCache
	: public ICache
{
	boost::shared_mutex cache_mutex_;
	std::unordered_map<size_t, std::unordered_map<size_t, std::vector<int>>> cache_;
public:
	~ThreadsafeCache() override = default;
	bool is_cached(size_t crossed_off_element_id, size_t crossed_off_elements)  override
	{
		try
		{
			boost::shared_lock<decltype(cache_mutex_)> cache_lock(cache_mutex_);
			auto &elements = cache_.at(crossed_off_element_id).at(crossed_off_elements);

			return true;
		}
		catch (const std::exception&)
		{
		}

		return false;
	}
	void cache(size_t crossed_of_element_id, size_t crossed_of_elements, std::vector<int> result) override
	{
		boost::unique_lock<decltype(cache_mutex_)> cache_lock(cache_mutex_);
		cache_[crossed_of_element_id][crossed_of_elements] = std::move(result);
	}

	decltype(cache_) const& get_cache()
	{
		return cache_;
	}
};

void run_tests(std::function<std::vector<int>(std::vector<int>, std::function<bool(int&, int&)>, std::function<bool(size_t, size_t)>&, size_t, size_t)> testee, std::function<bool(size_t, size_t)> cache_it, std::function<void()> clear_cache)
{
	// Док-во по индукции

	auto test = testee({ 1 }, std::less<int>(), cache_it, 0, 0);
	assert(test.size() == 1);
	clear_cache();

	test = testee({ 1,2 }, std::less<int>(), cache_it, 0, 0);
	assert(test.size() == 2);
	clear_cache();

	test = testee({ 2,1 }, std::less<int>(), cache_it, 0, 0);
	assert(test.size() == 1);
	clear_cache();

	test = testee({ 1,3,2 }, std::less<int>(), cache_it, 0, 0);
	assert(test.size() == 2);
	clear_cache();

	test = testee({ 1,2,3 }, std::less<int>(), cache_it, 0, 0);
	assert(test.size() == 3);
	clear_cache();

	test = testee({ 2,1,3 }, std::less<int>(), cache_it, 0, 0);
	assert(test.size() == 2);
	clear_cache();

	test = testee({ 2,3,1 }, std::less<int>(), cache_it, 0, 0);
	assert(test.size() == 2);
	clear_cache();

	test = testee({ 3,1,2 }, std::less<int>(), cache_it, 0, 0);
	assert(test.size() == 2);
	clear_cache();

	test = testee({ 3,2,1 }, std::less<int>(), cache_it, 0, 0);
	assert(test.size() == 1);
	clear_cache();


	test = testee({ 1,3,2,4 }, std::less<int>(), cache_it, 0, 0);
	assert(test.size() == 3);
	clear_cache();

	test = testee({ 2,1,3,2 }, std::less<int>(), cache_it, 0, 0);
	assert(test.size() == 2);
	clear_cache();

	// рандомные тесты
	test = testee({ 1,3,2,4,5 }, std::less<int>(), cache_it, 0, 0);
	assert(test.size() == 4);
	clear_cache();

	test = testee({ 3,2,3,4,5 }, std::less<int>(), cache_it, 0, 0);
	assert(test.size() == 4);
	clear_cache();

	test = testee({ 4,2,3,1,5 }, std::less<int>(), cache_it, 0, 0);
	assert(test.size() == 3);
	clear_cache();

	test = testee({ 4,5,1,3,6 }, std::less<int>(), cache_it, 0, 0);
	assert(test.size() == 3);
	clear_cache();

	test = testee({ 1,3,6,4,5 }, std::less<int>(), cache_it, 0, 0);
	assert(test.size() == 4);
	clear_cache();

	test = testee({ 1,4,2,3,4 }, std::less<int>(), cache_it, 0, 0);
	assert(test.size() == 4);
	clear_cache();
}
void run_tests_for_async()
{
	auto run_test = [](std::vector<int> elements, size_t expected)
	{
		std::unique_ptr<boost::asio::io_service> service;
		std::unique_ptr<boost::asio::io_service::strand> strand;
		std::unique_ptr<boost::asio::io_service::work> work;

		service = std::make_unique<decltype(service)::element_type>();
		strand = std::make_unique<decltype(strand)::element_type>(*service);
		work = std::make_unique<decltype(work)::element_type>(*service);

		auto thread_fun = [](decltype(service) &service)
		{
			try { service->run(); }
			catch (boost::thread_interrupted const &)
			{
			}
		};
		boost::thread_group threads;
		for (size_t i = 0; i < 10; i++)
		{
			threads.add_thread(new boost::thread(
				thread_fun,
				std::ref(service)
			)
			);
		}

		std::unique_ptr<ThreadsafeCache> cache = std::make_unique<ThreadsafeCache>();

		async_recursive_delete_elements_untill_with_caching2(elements, std::less<int>(), cache.get(), service.get());

		// пусть не сообщает сервису о задачах
		work.reset();
		threads.join_all();

		std::vector<int> const *max_elements = nullptr;
		for (auto &cache_pair : cache->get_cache())
		{
			auto local_max_elements = std::max_element(cache_pair.second.begin(), cache_pair.second.end(),
				[](decltype(*cache_pair.second.begin()) left, decltype(*cache_pair.second.begin()) right)
			{
				return left.second.size() < right.second.size();
			}
			);

			if (!max_elements || max_elements->size() < local_max_elements->second.size())
				max_elements = &local_max_elements->second;
		}

		assert(max_elements && max_elements->size() == expected);

		// остановим сервис
		service->stop();
		// удаляем все неисполненные задачи
		service.reset();
	};

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

	run_test({ 1,3,2,4,5 },4);

	run_test({ 3,2,3,4,5 },4);

	run_test({ 4,2,3,1,5 },3);

	run_test({ 4,5,1,3,6 },3);

	run_test({ 1,3,6,4,5 },4);

	run_test({ 1,4,2,3,4 },4);
}


int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "Russian");

	std::vector<int> container{ 1,4,2,3,4 }; // we need 1, 2, 3, 4
	//delete_elements_untill(container, std::greater<int>());

	//auto result = recursive_delete_elements_untill(std::move(container), std::less<int>());

	{
		std::unordered_map<size_t, std::unordered_map<size_t, bool>> cache;
		std::function<bool(size_t, size_t)> calculated_cache_impl = [&cache](size_t element_id, size_t shift_to_end) mutable
		{
			return std::exchange(cache[element_id][shift_to_end], true);
		};

		auto result = recursive_delete_elements_untill_with_caching(std::move(container), std::less<int>(), calculated_cache_impl);
		cache.clear();

		run_tests(&recursive_delete_elements_untill_with_caching2, calculated_cache_impl, [&cache]() { cache.clear(); });
	}

	{
		run_tests_for_async();

		std::unique_ptr<boost::asio::io_service> service;
		std::unique_ptr<boost::asio::io_service::strand> strand;
		std::unique_ptr<boost::asio::io_service::work> work;

		service = std::make_unique<decltype(service)::element_type>();
		strand = std::make_unique<decltype(strand)::element_type>(*service);
		work = std::make_unique<decltype(work)::element_type>(*service);

		auto thread_fun = [](decltype(service) &service)
		{
			try { service->run(); }
			catch (boost::thread_interrupted const &)
			{
			}
		};
		boost::thread_group threads;
		for (size_t i = 0; i < 10; i++)
		{
			threads.add_thread(new boost::thread(
					thread_fun,
					std::ref(service)
				)
			);
		}

		std::unique_ptr<ThreadsafeCache> cache = std::make_unique<ThreadsafeCache>();

		async_recursive_delete_elements_untill_with_caching2({1,3,2}, std::less<int>(), cache.get(), service.get());

		// пусть не сообщает сервису о задачах
		work.reset();
		threads.join_all();


		std::vector<int> const *max_elements = nullptr;
		for (auto &cache_pair : cache->get_cache())
		{
			auto local_max_elements = std::max_element(cache_pair.second.begin(), cache_pair.second.end(),
				[](decltype(*cache_pair.second.begin()) left, decltype(*cache_pair.second.begin()) right)
				{
					return left.second.size() < right.second.size();
				}
			);

			if (!max_elements || max_elements->size() < local_max_elements->second.size())
				max_elements = &local_max_elements->second;
		}

		// остановим сервис
		service->stop();
		// удаляем все неисполненные задачи
		service.reset();
	}

	return EXIT_SUCCESS;
}