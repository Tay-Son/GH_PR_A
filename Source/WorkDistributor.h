#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <atomic>

namespace GS {
	class WorkDistributor {

	private:
		bool is_execute_;


		std::atomic<size_t> num_working_;
		std::atomic<size_t> num_remain_works_;

		size_t num_threads_;

		std::vector<std::thread> workers_;
		std::queue<std::function<void()>> works_;
		std::mutex mtx_;
		std::mutex mtx_work_e_;
		std::condition_variable cv_work_q_;
		std::condition_variable cv_work_e_;

		bool is_stopped_;

	public:
		WorkDistributor(size_t num_threads);
		~WorkDistributor();

		void ReserveWork(std::function<void()> work);
		void ExecuteWork();
		void ExecuteWork2();

	private:
		void WorkerThread();
		void WorkerThread2();
	};

}
