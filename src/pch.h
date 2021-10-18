// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.

#ifndef PCH_H
#define PCH_H

#define NOMINMAX
#ifdef _WIN32
	// Исключаем редко используемые компоненты из заголовков Windows
	#define WIN32_LEAN_AND_MEAN     
	#include <windows.h>  
#endif // _WIN32

#define _SILENCE_FPOS_SEEKPOS_DEPRECATION_WARNING					

#include <list>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <iostream>
#include <iterator>

#include <boost/pool/pool_alloc.hpp>

#include <boost/container/flat_map.hpp>

#include <boost/concept_check.hpp>

#endif //PCH_H
