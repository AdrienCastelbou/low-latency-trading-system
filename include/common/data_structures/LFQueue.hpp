#pragma once
#include <vector>
#include <atomic>
#include "../common.hpp"

namespace common::data_structures
{
    template<typename T>
    class LFQueue final
    {
        public:
            LFQueue() = delete;
            LFQueue(size_t n);
            LFQueue(const LFQueue&) = delete;
            LFQueue(const LFQueue&&) = delete;
            LFQueue& operator=(const LFQueue&) = delete;
            LFQueue& operator=(const LFQueue&&) = delete;

            size_t size() const noexcept;
            T* getNextToWriteTo() noexcept;
            void updateWriteIndex() noexcept;
            T* getNextToRead() noexcept;
            void updateReadIndex() noexcept;

        private:
            std::vector<T> _data;
            std::atomic_size_t _nextWriteIndex;
            std::atomic_size_t _nextReadIndex;
            std::atomic_size_t _size;
    };
    
    
    template<typename T>
    LFQueue<T>::LFQueue(size_t n) : _data(n, T())
    {
    }

    template<typename T>
    size_t LFQueue<T>::size() const noexcept
    {
        return _size.load();
    }

    template<typename T>
    T* LFQueue<T>::getNextToWriteTo() noexcept
    {
        return &_data[_nextWriteIndex];
    }

    template<typename T>
    void LFQueue<T>::updateWriteIndex() noexcept
    {
        _nextWriteIndex = (_nextWriteIndex + 1) % _data.size();
        _size++;
    }

    template<typename T>
    T* LFQueue<T>::getNextToRead() noexcept
    {
        return _nextReadIndex == _nextWriteIndex ? nullptr : &_data[_nextReadIndex];
    }

    template<typename T>
    void LFQueue<T>::updateReadIndex() noexcept
    {
        _nextReadIndex = (_nextReadIndex + 1) % _data.size();
        common::assert(_size != 0, "Read an invalid element");
        _size--;
    }
} // namespace common::data_structures