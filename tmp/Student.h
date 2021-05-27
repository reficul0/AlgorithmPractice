#pragma once
#include <iostream>
#include <numeric>
#include <algorithm>
#include <string>
#include <unordered_map>

#include <boost/optional.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

#include "logging.h"
#include "Demonstratator.h"

// lab 4 var.№1 Task: Student class

class Student
{
public:
	std::string full_name;
	std::string group;
	std::unordered_map<
		std::string/*имя предмета*/,
		boost::optional<uint8_t>/*оценка*/
	> performance;

	~Student() noexcept = default;
	Student() noexcept(std::is_nothrow_default_constructible<decltype(performance)::allocator_type>::value)
	{}
	explicit Student(decltype(performance)::allocator_type const &allocator) noexcept
		: performance(allocator)
	{
	}

	Student(Student &&other)
		: full_name(std::move(other.full_name))
		, group(std::move(other.group))
		, performance(std::move(other.performance))
	{
		
	}

	Student(decltype(group) group_, decltype(full_name) full_name_, decltype(performance) performance_)
		: full_name(std::move(full_name_))
		, group(std::move(group_))
		, performance(std::move(performance_))
	{
		
	}
};

inline std::ostream& operator<<(std::ostream &destination, Student const &source)
{
	static constexpr char const * delimiter = "\n\t";
	destination << "Student={" << delimiter
		<< "group={\"" << source.group << "\"};" << delimiter
		<< "full_name={\"" << source.full_name << "\"};" << delimiter;
	destination << "performance[]={";
	std::transform(
		source.performance.begin(),
		--source.performance.end(),
		std::ostream_iterator<std::string>(destination, ","),
		[](decltype(*source.performance.begin()) grade)
		{
			return std::string(delimiter) + "\t{" + grade.first + "," + (grade.second ? std::to_string(*grade.second) : "Nan") + "}";
		}
	);
	auto &last = *--source.performance.end();
	destination << delimiter << "\t{" + last.first + "," + (last.second ? std::to_string(*last.second) : "Nan") + "}";
	destination << delimiter << "};" << std::endl;
	destination << "}";
	return destination;
}

using namespace boost::multi_index;

using Students = multi_index_container<
	Student,
	indexed_by<
		hashed_non_unique<
			tag<struct ByGroup>, member<Student, decltype(Student::group), &Student::group>
		>
	>
>;

template<>
class Demonstratator<Student>
{
public:
	static void demonstrate(configuration::Configuration const*)
	{
		Students students;
		
		students.emplace(Students::value_type
			{
				"з29-А",
				"Student F.N.",
				{
					{"Математика", 5},
					{"МЛиТА", 5},
					{"БИТП", 5},
					{"Философия", 5},
					{"МиТП", 5}
				}
			}
		);
		students.emplace(Students::value_type
			{
				"з29-А",
				"Student",
				{
					{"Математика", 3},
					{"МЛиТА", 3},
					{"БИТП", 4},
					{"Философия", 5},
					{"МиТП", 4}
				}
			}
		);
		students.emplace(Students::value_type
			{
				"з29-А",
				"Student",
				{
					{"Математика", 3},
					{"МЛиТА", 3},
					{"БИТП", 3},
					{"Философия", 4},
					{"МиТП", 2}
				}
			}
		);
		
		students.emplace(Students::value_type
			{
				"з30-З",
				"Student42",
				{
					{"Высшая математика", 3},
					{"Физика", 4},
					{"Системный анализ", 4},
					{"Функциональный анализ", 3},
					{"Линейная алгебра и аналитическая геометрия", 4}
				}
			}
		);
		students.emplace(Students::value_type
			{
				"з30-З",
				"Student43",
				{
					{"Высшая математика", 4},
					{"Физика", 4},
					{"Системный анализ", 4},
					{"Функциональный анализ", 4},
					{"Линейная алгебра и аналитическая геометрия", 5}
				}
			}
		);

		students.emplace(Students::value_type
			{
				"а272",
				"Student23",
				{
					{"Высшая математика", 2},
					{"Физика", 4},
					{"Системный анализ", 4},
					{"Функциональный анализ", 3},
					{"Линейная алгебра и аналитическая геометрия", 3}
				}
			}
		);
		students.emplace(Students::value_type
			{
				"а272",
				"Student2",
				{
					{"Высшая математика", 3},
					{"Физика", 4},
					{"Системный анализ", 3},
					{"Функциональный анализ", 3},
					{"Линейная алгебра и аналитическая геометрия", 3}
				}
			}
		);
		students.emplace(Students::value_type
			{
				"а272",
				"Student2",
				{
					{"Высшая математика", 3},
					{"Физика", 3},
					{"Системный анализ", 3},
					{"Функциональный анализ", 3},
					{"Линейная алгебра и аналитическая геометрия", 3}
				}
			}
		);
		
		students.emplace(Students::value_type
			{
				"32455",
				"Student27",
				{
					{"Высшая математика", 4},
					{"Физика", 1},
					{"Системный анализ", 2},
					{"Функциональный анализ", 3},
					{"Линейная алгебра и аналитическая геометрия", boost::none}
				}
			}
		);
		
		students.emplace(Students::value_type
			{
				"32456",
				"Student289",
				{
					{"Высшая математика", 4},
					{"Физика", 1},
					{"Системный анализ", 2},
					{"Функциональный анализ", 3},
					{"Линейная алгебра и аналитическая геометрия", 0}
				}
			}
		);

		auto &students_by_group = students.get<ByGroup>();

		static constexpr char const *students_delimiter = ";\n";
		
		std::wcout << logging::wformat_header(L"Все студенты:") << std::endl;
		std::copy(
			students_by_group.begin(),
			students_by_group.end(),
			std::ostream_iterator<Student>(std::cout, students_delimiter)
		);
		
		std::wcout << logging::wformat_header(L"Студенты, имеющие средний бал выше 4:") << std::endl;
		bool any_student = false;
		std::copy_if(
			students_by_group.begin(),
			students_by_group.end(),
			std::ostream_iterator<Student>(std::cout, students_delimiter),
			[&any_student](decltype(*students_by_group.begin()) student)
			{
				double const grades_count = student.performance.size();
				double const average_score = grades_count
					? std::accumulate(
							student.performance.begin(), student.performance.end(), 
							double{ 0 }, 
							[](double &result, decltype(*student.performance.begin()) grade)
							{
								return result += (grade.second ? *grade.second : 0);
							}
						) / grades_count
					: 0;

				if (average_score > 4.)
					return any_student = true;
				
				return false;
			}
		);
		if(!any_student)
			std::cout << "Студентов, удовлетворяющих фильтру, не обнаружено." << std::endl;
	}
};