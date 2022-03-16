#pragma once

#include "WorkDistributor.h"

namespace GS {
	WorkDistributor::WorkDistributor(size_t num_threads) :
		num_threads_(num_threads),
		num_remain_works_(0),
		is_execute_(false),
		is_stopped_(false)
	{
		workers_.reserve(num_threads_);
		for (size_t i = 0; i < num_threads_; i++) {
			workers_.emplace_back([this]() {this->WorkerThread(); });
		}
	}

	WorkDistributor::~WorkDistributor() {
		is_stopped_ = false;
		cv_work_q_.notify_all();

		for (auto& t : workers_) { t.join(); }
	}

	void WorkDistributor::ReserveWork(std::function<void()> work) {
		std::lock_guard<std::mutex> lock(mtx_);
		works_.push(std::move(work));
		num_remain_works_.operator++(std::memory_order_acq_rel);
	}

	void WorkDistributor::ExecuteWork() {
		std::unique_lock<std::mutex> lock(mtx_);
		is_execute_ = true;
		cv_work_q_.notify_all();
		cv_work_e_.wait(lock, [this]() {return (num_working_ == 0 && works_.empty()); });
		is_execute_ = false;
	}
	void WorkDistributor::ExecuteWork2() {
		cv_work_q_.notify_all();
		while (true) {
			std::unique_lock<std::mutex> lock(mtx_);
			if (num_remain_works_ == 0) {
				return;
			}
			lock.unlock();
		}
	}

	void WorkDistributor::WorkerThread() {
		std::function<void()> work;

		while (true) {
			std::unique_lock<std::mutex> lock(mtx_);
			cv_work_q_.wait(lock, [this]() {return (is_execute_ && !works_.empty()); });
			//cv_work_q_.wait(lock, [this]() {return (is_execute_); });
			while (!works_.empty()) {
				work = std::move(works_.front());
				works_.pop();
				++num_working_;
				lock.unlock();

				work();

				lock.lock();
				--num_working_;
				cv_work_e_.notify_one();
			}
		}
	}

	void WorkDistributor::WorkerThread2() {
		std::function<void()> work;
		std::unique_lock<std::mutex> lock(mtx_, std::defer_lock);

		while (true) {
			cv_work_q_.wait(lock, [this]() {return !works_.empty() || !is_stopped_; });
			lock.lock();
			while (true) {
				if (!is_stopped_) { return; }
				work = std::move(works_.front());
				works_.pop();
				lock.unlock();

				work();

				lock.lock();
				num_remain_works_--;
				if (num_remain_works_ == 0) {
					cv_work_e_.notify_one();
					lock.unlock();
					break;
				}
			}
		}
	}

}