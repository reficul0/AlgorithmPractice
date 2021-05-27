#include "stdafx_tools.h"
#include "deleters.h"

namespace tools
{
	namespace memory
	{
		template <>
		void Deleter<boost::thread>::operator()(boost::thread *thread) const
		{
			if (!thread)
				return;

			if (thread->joinable())
			{
				thread->interrupt();
				thread->join();
			}
			delete thread;
			thread = nullptr;
		}
	}
}
