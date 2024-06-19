#include "thread_alloc.hpp"

namespace CppAD
{

#define CPPAD_MAX_NUM_CAPACITY 100

const thread_alloc::capacity_t *thread_alloc::capacity_info(void)
{
	CPPAD_ASSERT_FIRST_CALL_NOT_PARALLEL;
	static const capacity_t capacity;
	return &capacity;
}

bool thread_alloc::set_get_hold_memory(bool set, bool new_value)
{
	static bool value = false;
	if (set)
		value = new_value;
	return value;
}

thread_alloc::thread_alloc_info *thread_alloc::thread_info(size_t thread, bool clear)
{
	static thread_alloc_info *all_info[CPPAD_MAX_NUM_THREADS];
	static thread_alloc_info zero_info;

	CPPAD_ASSERT_FIRST_CALL_NOT_PARALLEL;

	CPPAD_ASSERT_UNKNOWN(thread < CPPAD_MAX_NUM_THREADS);

	thread_alloc_info *info = all_info[thread];
	if (clear)
	{
		if (info != nullptr)
		{
#ifndef NDEBUG
			CPPAD_ASSERT_UNKNOWN(info->count_inuse_ == 0 && info->count_available_ == 0);
			for (size_t c = 0; c < CPPAD_MAX_NUM_CAPACITY; c++)
			{
				CPPAD_ASSERT_UNKNOWN(info->root_inuse_[c].next_ == nullptr &&
				                     info->root_available_[c].next_ == nullptr);
			}
#endif
			if (thread != 0)
				::operator delete(reinterpret_cast<void *>(info));
			info = nullptr;
			all_info[thread] = info;
		}
	}
	else if (info == nullptr)
	{
		if (thread == 0)
			info = &zero_info;
		else
		{
			size_t size = sizeof(thread_alloc_info);
			void *v_ptr = ::operator new(size);
			info = reinterpret_cast<thread_alloc_info *>(v_ptr);
		}
		all_info[thread] = info;

		// initialize the information record
		for (size_t c = 0; c < CPPAD_MAX_NUM_CAPACITY; c++)
		{
			info->root_inuse_[c].next_ = nullptr;
			info->root_available_[c].next_ = nullptr;
		}
		info->count_inuse_ = 0;
		info->count_available_ = 0;
	}
	return info;
}

size_t thread_alloc::set_get_num_threads(size_t number_new)
{
	static size_t number_user = 1;

	CPPAD_ASSERT_UNKNOWN(number_new <= CPPAD_MAX_NUM_THREADS);
	CPPAD_ASSERT_UNKNOWN(!in_parallel() || (number_new == 0));

	// case where we are changing the number of threads
	if (number_new != 0)
		number_user = number_new;

	return number_user;
}

size_t thread_alloc::set_get_thread_num(size_t (*thread_num_new)(void), bool set)
{
	static size_t (*thread_num_user)(void) = nullptr;

	if (set)
	{
		thread_num_user = thread_num_new;
		return 0;
	}

	if (thread_num_user == nullptr)
		return 0;

	size_t thread = thread_num_user();
	CPPAD_ASSERT_KNOWN(thread < set_get_num_threads(0),
	                   "parallel_setup: thread_num() >= num_threads");
	return thread;
}
} // namespace CppAD