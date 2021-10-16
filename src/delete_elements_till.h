#pragma once

#ifndef ALGO_DELETE_ELEMENTS_TILL_H
#define ALGO_DELETE_ELEMENTS_TILL_H

using element_t = int;

namespace algo
{
	std::vector<element_t> delete_till_copy(
		std::vector<element_t> const &elements,
		std::function<bool(element_t const&, element_t const&)> comparator
	) noexcept;
}

#endif