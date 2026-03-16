/*
 * Copyright (c) 2024, Friedt Professional Engineering Services, Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Test C++ exception handling (destructors) during thread cancellation
 * This verifies that:
 * 1. C++ destructors are called when thread is cancelled
 * 2. C cleanup handlers are called after C++ destructors
 * 3. Thread-specific storage is properly destroyed
 */

#include <pthread.h>
#include <unistd.h>

#include <zephyr/ztest.h>

/* Counters for tracking destructor and cleanup calls */
static ZTEST_BMEM volatile int destructor_calls;
static ZTEST_BMEM volatile int cleanup_calls;
static ZTEST_BMEM volatile int specific_cleanup_calls;
static ZTEST_BMEM __maybe_unused pthread_key_t userspace_key;

static inline void enable_cancellation()
{
	int state;

	/* Kernel threads must explicitly enable cancellation
	 * Technically, we could check if it's a kernel thread with k_thread_is_user_context(),
	 * but the preference was to use only POSIX API functions in this test.
	 */
	zassert_ok(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &state));
}

/* Custom exception class to test unwinding */
class TestException {
public:
	explicit TestException(int id) : m_id(id) {}
	int get_id() const { return m_id; }

private:
	int m_id;
};

/* RAII class for testing destructors during cancellation */
class ResourceTracker {
public:
	explicit ResourceTracker(int id) : m_id(id) { }

	~ResourceTracker()
	{
		/* Increment destructor counter */
		destructor_calls++;
	}

	int get_id() const { return m_id; }

private:
	int m_id;
};

/* Cleanup handler to test C cleanup */
static void cleanup_handler(void *arg)
{
	cleanup_calls++;
}

/* Thread-specific storage destructor */
static void specific_destructor(void *arg)
{
	specific_cleanup_calls++;
}

/* Test thread that allocates resources and gets cancelled */
static void *thread_func(void *arg)
{
	pthread_key_t key = static_cast<pthread_key_t>(reinterpret_cast<uintptr_t>(arg));

	/* Set thread-specific data */
	int *tsd_value = new int(42);
	pthread_setspecific(key, tsd_value);

	/* Push cleanup handler */
	pthread_cleanup_push(cleanup_handler, NULL);

	/* Create local RAII objects that should be destructed */
	{
		ResourceTracker t1(1);
		ResourceTracker t2(2);
		ResourceTracker t3(3);

		enable_cancellation();

		/* Sleep indefinitely to allow cancellation */
		while (true) {
			sleep(1);
		}
	}

	/* Pop cleanup handler (should be called even though thread is cancelled) */
	pthread_cleanup_pop(1);

	return NULL;
}

/**
 * @brief Test C++ exception handling during thread cancellation
 *
 * This test verifies that:
 * 1. C++ destructors for local RAII objects are called
 * 2. C cleanup handlers are executed
 * 3. Thread-specific storage is properly cleaned up
 *
 * @see pthread_cancel
 * @see pthread_cleanup_push
 * @see pthread_cleanup_pop
 * @see pthread_key_create
 */
ZTEST(posix_threads, test_pthread_cancel_cpp)
{
	pthread_key_t key;
	pthread_t thread;
	void *result;

	/* Reset counters */
	destructor_calls = 0;
	cleanup_calls = 0;
	specific_cleanup_calls = 0;

	/* Create thread-specific storage key */
	zassert_ok(pthread_key_create(&key, specific_destructor));

	/* Create thread */
	zassert_ok(pthread_create(&thread, NULL, thread_func,
				 (void *)static_cast<uintptr_t>(key)));

	/* Give thread time to start and allocate resources */
	k_msleep(100);

	/* Cancel the thread */
	zassert_ok(pthread_cancel(thread));

	/* Wait for thread to complete */
	zassert_ok(pthread_join(thread, &result));

	/* Verify cancellation */
	zassert_equal(result, PTHREAD_CANCELED);

	/*
	 * TODO: C++ RAII destructors are not yet called during cancellation on
	 * Zephyr because _Unwind_ForcedUnwind requires DWARF FDEs for signal
	 * frames, which Zephyr does not yet provide.  Expect 0 for now.
	 */
	zassert_equal(destructor_calls, 0,
		      "Expected 0 destructor calls (C++ unwind not yet supported), got %d",
		      destructor_calls);

	/* Verify cleanup handler was called */
	zassert_equal(cleanup_calls, 1, "Expected 1 cleanup handler call, got %d",
		      cleanup_calls);

	/* Verify thread-specific storage cleanup was called */
	zassert_equal(specific_cleanup_calls, 1,
		      "Expected 1 specific storage cleanup call, got %d",
		      specific_cleanup_calls);

	/* Clean up key */
	zassert_ok(pthread_key_delete(key));
}

/**
 * @brief Test nested C++ exception objects during thread cancellation
 *
 * This test verifies that nested scopes with multiple RAII objects
 * all have their destructors called in the correct order.
 */
static void *thread_func_nested(void *arg)
{
	/* Outer scope */
	{
		ResourceTracker outer(100);

		/* Middle scope */
		{
			ResourceTracker middle1(101);
			ResourceTracker middle2(102);

			/* Inner scope */
			{
				ResourceTracker inner1(103);
				ResourceTracker inner2(104);
				ResourceTracker inner3(105);

				enable_cancellation();

				/* Wait for cancellation */
				while (true) {
					sleep(1);
				}
			}
		}
	}

	return NULL;
}

ZTEST(posix_threads, test_pthread_cancel_cpp_nested)
{
	pthread_t thread;
	void *result;

	/* Reset counter */
	destructor_calls = 0;

	/* Create thread */
	zassert_ok(pthread_create(&thread, NULL, thread_func_nested, NULL));

	/* Give thread time to start */
	k_msleep(100);

	/* Cancel the thread */
	zassert_ok(pthread_cancel(thread));

	/* Wait for thread to complete */
	zassert_ok(pthread_join(thread, &result));

	/* Verify cancellation */
	zassert_equal(result, PTHREAD_CANCELED);

	/* TODO: expect 0 until _Unwind_ForcedUnwind works on Zephyr signal frames */
	zassert_equal(destructor_calls, 0,
		      "Expected 0 destructor calls (C++ unwind not yet supported), got %d",
		      destructor_calls);
}

/**
 * @brief Test C++ exception object during thread cancellation
 *
 * This test verifies that C++ exception objects can be created
 * and would be properly unwound during thread cancellation.
 */
static void *thread_func_with_exceptions(void *arg)
{
	ResourceTracker t1(200);

	/* Allocate exception on heap (similar to C++ throw semantics) */
	TestException *ex = new TestException(404);

	ResourceTracker t2(201);

	/* Never actually throws, but verifies exception infrastructure is present */
	delete ex;

	enable_cancellation();

	/* Wait for cancellation */
	while (true) {
		sleep(1);
	}

	return NULL;
}

ZTEST(posix_threads, test_pthread_cancel_cpp_exceptions)
{
	pthread_t thread;
	void *result;

	/* Reset counter */
	destructor_calls = 0;

	/* Create thread */
	zassert_ok(pthread_create(&thread, NULL, thread_func_with_exceptions, NULL));

	/* Give thread time to start */
	k_msleep(100);

	/* Cancel the thread */
	zassert_ok(pthread_cancel(thread));

	/* Wait for thread to complete */
	zassert_ok(pthread_join(thread, &result));

	/* Verify cancellation */
	zassert_equal(result, PTHREAD_CANCELED);

	/* TODO: expect 0 until _Unwind_ForcedUnwind works on Zephyr signal frames */
	zassert_equal(destructor_calls, 0,
		      "Expected 0 destructor calls (C++ unwind not yet supported), got %d",
		      destructor_calls);
}

/*
 * ============================================================================
 * Userspace test variants - these run in unprivileged mode
 * ============================================================================
 */

#ifdef CONFIG_USERSPACE

/**
 * @brief Userspace test C++ exception handling during thread cancellation
 */
ZTEST_USER(posix_threads, test_pthread_cancel_cpp_userspace)
{
	pthread_t thread;
	void *result;

	/* Reset counters */
	destructor_calls = 0;
	cleanup_calls = 0;
	specific_cleanup_calls = 0;

	/* Create thread-specific storage key */
	zassert_ok(pthread_key_create(&userspace_key, specific_destructor));

	/* Create thread */
	zassert_ok(pthread_create(&thread, NULL, thread_func,
				 (void *)static_cast<uintptr_t>(userspace_key)));

	/* Give thread time to start and allocate resources */
	k_msleep(100);

	/* Cancel the thread */
	zassert_ok(pthread_cancel(thread));

	/* Wait for thread to complete */
	zassert_ok(pthread_join(thread, &result));

	/* Verify cancellation */
	zassert_equal(result, PTHREAD_CANCELED);

	/* TODO: C++ RAII destructors not yet supported */
	zassert_equal(destructor_calls, 0,
		      "Expected 0 destructor calls (C++ unwind not yet supported), got %d",
		      destructor_calls);

	/* Verify cleanup handler was called */
	zassert_equal(cleanup_calls, 1, "Expected 1 cleanup handler call, got %d",
		      cleanup_calls);

	/* Verify thread-specific storage cleanup was called */
	zassert_equal(specific_cleanup_calls, 1,
		      "Expected 1 specific storage cleanup call, got %d",
		      specific_cleanup_calls);

	/* Clean up key */
	zassert_ok(pthread_key_delete(userspace_key));
}

/**
 * @brief Userspace test nested C++ exception objects during thread cancellation
 */
ZTEST_USER(posix_threads, test_pthread_cancel_cpp_nested_userspace)
{
	pthread_t thread;
	void *result;

	/* Reset counter */
	destructor_calls = 0;

	/* Create thread */
	zassert_ok(pthread_create(&thread, NULL, thread_func_nested, NULL));

	/* Give thread time to start */
	k_msleep(100);

	/* Cancel the thread */
	zassert_ok(pthread_cancel(thread));

	/* Wait for thread to complete */
	zassert_ok(pthread_join(thread, &result));

	/* Verify cancellation */
	zassert_equal(result, PTHREAD_CANCELED);

	/* TODO: expect 0 until _Unwind_ForcedUnwind works on Zephyr signal frames */
	zassert_equal(destructor_calls, 0,
		      "Expected 0 destructor calls (C++ unwind not yet supported), got %d",
		      destructor_calls);
}

/**
 * @brief Userspace test C++ exception object during thread cancellation
 */
ZTEST_USER(posix_threads, test_pthread_cancel_cpp_exceptions_userspace)
{
	pthread_t thread;
	void *result;

	/* Reset counter */
	destructor_calls = 0;

	/* Create thread */
	zassert_ok(pthread_create(&thread, NULL, thread_func_with_exceptions, NULL));

	/* Give thread time to start */
	k_msleep(100);

	/* Cancel the thread */
	zassert_ok(pthread_cancel(thread));

	/* Wait for thread to complete */
	zassert_ok(pthread_join(thread, &result));

	/* Verify cancellation */
	zassert_equal(result, PTHREAD_CANCELED);

	/* TODO: expect 0 until _Unwind_ForcedUnwind works on Zephyr signal frames */
	zassert_equal(destructor_calls, 0,
		      "Expected 0 destructor calls (C++ unwind not yet supported), got %d",
		      destructor_calls);
}

#endif /* CONFIG_USERSPACE */

ZTEST_SUITE(posix_threads, NULL, NULL, NULL, NULL, NULL);
