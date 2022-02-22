

#include <coroutine>
#include <future>
#include <algorithm>
#include <atomic>
#include <cassert>
#include <iostream>

struct TestTask
{
	struct promise_type
	{
		std::suspend_always initial_suspend() { return {}; }
		std::suspend_always final_suspend() noexcept { return {}; }

		TestTask get_return_object() {
			return { std::coroutine_handle<promise_type>::from_promise(*this) };
		}
		void return_void() {}
		void unhandled_exception() {}
	};
	std::coroutine_handle<promise_type> handle;

	TestTask() = default;
	TestTask(const TestTask&) = delete;
	TestTask(TestTask&& t) : handle(std::exchange(t.handle, nullptr)) {}
	TestTask(std::coroutine_handle<promise_type> h) : handle(std::coroutine_handle<promise_type>(h)) {}

	~TestTask()
	{
		if (handle)
			handle.destroy();
	}
};

struct TestAwaitable
{
	bool await_ready() { return false; }
	auto await_suspend(std::coroutine_handle<> h)
	{
		std::cout << "TestAwaitable await_suspend" << std::endl;
		h.resume();
		return std::noop_coroutine();
	}

	void await_resume() {}
};

int recusive_resume()
{
	bool b = false;
	TestAwaitable a;

	TestTask t1 = [](TestAwaitable& a, bool& b) ->TestTask {

		std::cout << "initial_suspend resumed" << std::endl;
		assert(b == false);
		b = true;

		co_await a;

		std::cout << "TestAwaitable resumed" << std::endl;

		co_await std::suspend_always{};
	}(a, b);

	t1.handle.resume();

	return 0;
}

struct DestroyAwaitable
{
	bool await_ready() { return false; }
	auto await_suspend(std::coroutine_handle<> h)
	{
		std::cout << "DestroyAwaitable await_suspend" << std::endl;
		h.destroy();

		// asan triggered here. Seems this call writes to the coro of h.
		return std::noop_coroutine();
	}
	void await_resume() {}
};

int recusive_destroy()
{
	bool b = false;

	TestTask t1 = [](bool& b) ->TestTask {

		std::cout << "initial_suspend resumed" << std::endl;
		assert(b == false);
		b = true;

		co_await DestroyAwaitable{};

		std::cout << "TestAwaitable resumed" << std::endl;

		co_await std::suspend_always{};
	}(b);

	t1.handle.resume();
	t1.handle = {};
	return 0;
}


int main()
{
	recusive_resume();
	recusive_destroy();
	return 0;
}