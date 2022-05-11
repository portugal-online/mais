#ifndef CCONDITION_H__
#define CCONDITION_H__

#include <queue>
#include <mutex>
#include <condition_variable>

template <class T>
class CCondition
{
public:
  CCondition(void)
    : d()
    , m()
    , c()
  {}

  CCondition(const T &initial)
    : d(initial)
    , m()
    , c()
  {}

  ~CCondition(void)
  {}

  void set(const T &t)
  {
    std::lock_guard<std::mutex> lock(m);
    d = t;
    c.notify_one();
  }

  void wait(const T &value)
  {
    std::unique_lock<std::mutex> lock(m);
    while( d!=value)
    {
      c.wait(lock);
    }
  }
  void waitnot(const T &value)
  {
    std::unique_lock<std::mutex> lock(m);
    while( d==value)
    {
      c.wait(lock);
    }
  }
    const T &get() const { return d; }
    void reset(const T&t)
    {
        d = t;
    }
private:
  T d;
  mutable std::mutex m;
  std::condition_variable c;
};
#endif
