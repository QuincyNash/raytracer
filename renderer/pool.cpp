#include "pool.hpp"

#include <condition_variable>
#include <iostream>

// Initialize pool with given number of threads
ThreadPool::ThreadPool(size_t num_threads) : stop(false) {
  for (size_t i = 0; i < num_threads; ++i) {
    workers.emplace_back([this] {
      while (true) {
        std::function<void()> task;
        {
          // Acquire lock and wait for tasks
          std::unique_lock<std::mutex> lock(this->mtx);
          this->cv.wait(lock,
                        [this] { return this->stop || !this->tasks.empty(); });
          if (this->stop && this->tasks.empty()) return;
          task = std::move(this->tasks.front());
          this->tasks.pop();
        }
        task();
      }
    });
  }
}

// Destructor: join all threads
ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(mtx);
    stop = true;
  }
  cv.notify_all();
  for (std::thread& worker : workers) {
    if (worker.joinable()) {
      worker.join();
    }
  }
}

void ThreadPool::wait() {
  std::unique_lock<std::mutex> lock(mtx);
  cv_finished.wait(lock, [this] { return active.load() == 0; });
}

void ThreadPool::clear_tasks() {
  std::unique_lock<std::mutex> lock(mtx);
  while (!tasks.empty()) tasks.pop();
}