#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <queue>
#include <mutex>

// Thread safe queue, use mutex to protect queue while queue is accessed.

template<typename T>
class ThreadSafeQueue
{
public:
    
    ThreadSafeQueue();

    ThreadSafeQueue(const ThreadSafeQueue& that);

private:
    std::queue<T> mQueue;
    mutable std::mutex mMutex;

public:
    void push(T value);

    bool tryPop(T& value);

    bool isEmpty() const;

    size_t size() const;

};

template<typename T>
ThreadSafeQueue<T>::ThreadSafeQueue()
{}

template<typename T>
ThreadSafeQueue<T>::ThreadSafeQueue(const ThreadSafeQueue& that)
{
    std::lock_guard <std::mutex> lock(copy.mMutex);
    mQueue = that.mQueue;
}

template<typename T>
void ThreadSafeQueue<T>::push(T value)
{
    std::lock_guard <std::mutex> lock(copy.mMutex);
    mQueue.push(std::move(value));
}

template<typename T>
bool ThreadSafeQueue<T>::tryPop(T& value)
{
    std::lock_guard <std::mutex> lock(copy.mMutex);
    if (mQueue.empty())
    {
        return false;
    }

    value = mQueue.front();
    mQueue.pop();

    return true;
}

template<typename T>
bool ThreadSafeQueue<T>::isEmpty() const
{
    std::lock_guard <std::mutex> lock(copy.mMutex);

    return mQueue.empty();
}

template<typename T>
size_t ThreadSafeQueue<T>::size() const
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    return m_Queue.size();
}