#ifndef SAFE_QUEUE__
#define SAFE_QUEUE__

#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
using namespace std::chrono_literals;

// A threadsafe-queue.
template <class T>
    class CQueue
{
public:
    CQueue(void)
        : q()
    , m()
        , c()
    {}

        ~CQueue(void)
        {}

        // Add an element to the queue.
        void enqueue(T t)
        {
            std::lock_guard<std::mutex> lock(m);
            q.push(t);
            c.notify_one();
        }

        // Get the "front"-element.
        // If the queue is empty, wait till a element is avaiable.
        T dequeue(void)
        {
            std::unique_lock<std::mutex> lock(m);
            while(q.empty())
            {
                // release lock as long as the wait and reaquire it afterwards.
                c.wait(lock);
            }
            T val = q.front();
            q.pop();
            return val;
        }

        bool timed_dequeue(uint32_t timeout_ms, T &target)
        {
            bool ret = false;
            std::unique_lock<std::mutex> lock(m);

            if(q.empty())
            {
                if (c.wait_for(lock, timeout_ms*1ms, [this]{return !q.empty();}))
                {
                    target = q.front();
                    q.pop();
                    ret = true;
                }
                else
                {
                    ret = false;
                }
            } else {
                target = q.front();
                q.pop();
                ret = true;
            }
            return ret;
        }

        bool empty() {
            std::unique_lock<std::mutex> lock(m);
            return q.empty();
        }
    private:
        std::queue<T> q;
        mutable std::mutex m;
        std::condition_variable c;
};
#endif
