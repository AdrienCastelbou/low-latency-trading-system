#pragma once

#include <vector>
#include "../common.hpp"

namespace common::data_structures
{
    template<typename T>
    class MemoryPool final
    {
        public:
            MemoryPool() = delete;
            MemoryPool(size_t n);
            MemoryPool(const MemoryPool&) = delete;
            MemoryPool(const MemoryPool&&) = delete;
            MemoryPool& operator=(const MemoryPool&) = delete;
            MemoryPool& operator=(const MemoryPool&&) = delete;

            template<typename... Args>
            T* allocate(Args... args);

            void deallocate(T* obj);
        private:
            void updateNextFreeIndex();

            struct ObjBlock;

            std::vector<ObjBlock>   _data;
            size_t                  _nextFreeIdx;
            
    };

    template<typename T>
    MemoryPool<T>::MemoryPool(size_t n) : _data(n, {T(), true}), _nextFreeIdx(0)
    {
        common::assert(reinterpret_cast<const ObjBlock *>(&(_data[0]._object)) == &(_data[0]), "T object should be first member of ObjectBlock.");
    }

    template<typename T>
    template<typename... Args>
    T* MemoryPool<T>::allocate(Args... args)
    {
        auto objBlock = &_data[_nextFreeIdx];

#if !defined(NDEBUG)
        common::assert(objBlock->_isFree, "Expected free ObjectBlock at index:" + std::to_string(_nextFreeIdx));
#endif
        T* obj = &(objBlock->_object);
        obj = new(obj) T(args...);
        objBlock->_isFree = false;
        updateNextFreeIndex();
        return obj;
    }

    template<typename T>
    void MemoryPool<T>::updateNextFreeIndex()
    {
        const auto iniFreeIdx = _nextFreeIdx;

        while (!_data[_nextFreeIdx]._isFree)
        {
            ++_nextFreeIdx;
            if (_nextFreeIdx == _data.size()) [[unlikely]]
            {
                _nextFreeIdx = 0;
            }
            if (_nextFreeIdx == iniFreeIdx) [[unlikely]]
            {
                common::fatal("Memory Pool out of space");
            }
        }
    }

    template<typename T>
    void MemoryPool<T>::deallocate(T* obj)
    {
        const auto objIndex = reinterpret_cast<ObjBlock*>(obj) - &_data[0];
#if !defined(NDEBUG)
        common::assert(objIndex >= 0 && static_cast<size_t>(objIndex) < _data.size(), "Element being deallocated does not belong to this Memory pool.");
        common::assert(!_data[objIndex]._isFree, "Expected in-use ObjectBlock at index:" + std::to_string(objIndex));
#endif
        _data[objIndex]._isFree = true;
    }

    template<typename T>
    struct MemoryPool<T>::ObjBlock
    {
        T       _object;
        bool    _isFree;
    };
} // namespace memory
