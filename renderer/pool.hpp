#pragma once

#include <functional>
#include <queue>
#include <thread>
#include <vector>

// Simple thread pool for parallel task execution
class ThreadPool {
 private:
  std::vector<std::thread> workers;         // Worker threads
  std::queue<std::function<void()>> tasks;  // Task queue
  std::mutex mtx;                           // Mutex for task queue
  std::condition_variable cv;               // Task available condition
  std::condition_variable cv_finished;      // All tasks finished condition
  std::atomic<int> active{0};               // Active task count
  bool stop;

 public:
  ThreadPool(size_t num_threads);
  ~ThreadPool();

  template <class F>
  void enqueue(F&& f) {
    {
      std::unique_lock<std::mutex> lock(mtx);
      active++;

      tasks.emplace([this, func = std::forward<F>(f)]() {
        func();
        active--;                  // Mark task as done
        cv_finished.notify_one();  // Notify waiting threads
      });
    }
    cv.notify_one();
  }

  int size() const { return workers.size(); }
  void wait();
  void clear_tasks();
};