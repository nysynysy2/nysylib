#ifndef _NYSY_LOCK_FREE_QUEUE_
#define _NYSY_LOCK_FREE_QUEUE_
#include <semaphore>
#include <vector>
#include <memory>
#include <atomic>
#include <limits>

namespace nysy {

	template<typename Elem, std::size_t Size>
	class LockFreeQueue {
		std::counting_semaphore<Size> m_free_space_sem{ Size }, m_data_sem{ 0 };
		std::atomic<std::size_t> m_next_data_index{ 0 }, m_next_free_space_index{ 0 };
		std::unique_ptr<Elem[]> m_arr = std::make_unique<Elem[]>(Size);
	public:
		LockFreeQueue() = default;
		LockFreeQueue(const LockFreeQueue&) = delete;
		void push(Elem&& elem) {
			m_free_space_sem.acquire();
			m_arr[std::atomic_fetch_add(&m_next_free_space_index, 1) % Size] = std::forward<Elem>(elem);
			m_data_sem.release();
		}
		Elem pop() {
			m_data_sem.acquire();
			Elem tmp( std::move(m_arr[std::atomic_fetch_add(&m_next_data_index, 1) % Size]) );
			m_free_space_sem.release();
			return tmp;
		}
	};

}
#endif