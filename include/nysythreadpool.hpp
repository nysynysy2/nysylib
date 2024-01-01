#pragma once
#include <thread>
#include <memory>
#include <future>
#include <functional>
#include <list>
#include "nysylockfreequeue.hpp"

namespace nysy {
	template<std::size_t CacheCapacity = 10>
		class ThreadPool {
		public:
			ThreadPool(std::size_t thread_count = std::thread::hardware_concurrency()) {
				for (std::size_t add_counter{ 0 }; add_counter < thread_count; ++add_counter) {
					m_threads.emplace_back(&ThreadPool::execute, this);
				}
			};
			ThreadPool(const ThreadPool&) = delete;
			template<typename Fn, typename...Args>auto add_task(Fn&& func, Args&&... args) {
				auto task_ptr{ std::make_shared< std::packaged_task<std::invoke_result_t<Fn, Args...>(void)> >([=]()mutable { return func(args...); }) };
				auto future{ (*task_ptr).get_future() };
				m_tasks.push(std::function{ [task_ptr]() { (*task_ptr)(); } });
				++m_tasks_left;
				return future;
			}
			void close() {
				if (!(m_is_stopped.exchange(true))) {
					for (std::size_t quit_task_counter{ 0 }; quit_task_counter < m_threads.size(); ++quit_task_counter) {
						add_task([]() {});
					}
					m_threads.clear();
				}
			}
			void wait() { while (!m_is_stopped && m_tasks_left != 0); }
			~ThreadPool() { close(); }
		private:
			void execute() {
				do {
					auto task{ m_tasks.pop() };
					if (m_is_stopped.load())return;
					task();
					--m_tasks_left;
				} while (!m_is_stopped.load());
			}
			std::list<std::jthread> m_threads;
			LockFreeQueue< std::function<void(void)>, CacheCapacity > m_tasks{};
			std::atomic<std::size_t> m_tasks_left{ 0 };
			std::atomic<bool> m_is_stopped{ false };
	};
}