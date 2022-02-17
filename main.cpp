

#include <coroutine>
#include <future>
#include <algorithm>
#include <atomic>
#include <cassert>

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
	std::promise<std::coroutine_handle<>> p;

	bool await_ready() { return false; }
	auto await_suspend(std::coroutine_handle<> h)
	{
		h.resume();
		return std::noop_coroutine();
	}

	void await_resume() {}
};

int main()
{
	std::atomic<bool> b = false;
	TestAwaitable a;
	auto f = a.p.get_future();

	TestTask t1 = [](std::atomic<bool>& b, TestAwaitable& a) ->TestTask {


		assert(b.exchange(true) == false);

		co_await a;

		co_await std::suspend_always{};
	}(b, a);

	t1.handle.resume();

	return 0;
}

