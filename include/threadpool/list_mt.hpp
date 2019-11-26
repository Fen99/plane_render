#pragma once

#include <list>
#include <atomic>
#include <thread>
#include <iostream>

namespace plane_render {

// list на спинлоках: list multithread
template<typename T>
class ListMT
{
public:
    class ListMTAccessor // Для блокировки и доступа
    {
    public:
        std::list<T>* operator->() { return &parent_->list_; }

        void Release() // Разблокируем доступ (не CAS: мы владельцы), после этого акцессор невалиден
        {
            if (!parent_)
                return;

            parent_->locked_.store(false);
            parent_ = nullptr;
        }

        ListMTAccessor(ListMTAccessor&& another)
        {
            another.parent_ = nullptr;
        }
        ListMTAccessor& operator=(ListMTAccessor&& another)
        {
            Release();
            another.parent_ = nullptr;
            return *this;
        }
        ~ListMTAccessor() { Release(); }

    private:
        ListMTAccessor(ListMT<T>* parent) : parent_(parent)
        {
            bool expected = false;
            while (!parent_->locked_.compare_exchange_strong(expected, true)) // Ждем, пока сможем захватить
            {
                expected = false;
                std::this_thread::yield();
            }
        }
        ListMTAccessor& operator=(const ListMTAccessor&) = delete;
        ListMTAccessor(const ListMTAccessor&) = delete; // Только для возврата объектов

    private:
        ListMT<T>* parent_ = nullptr;
        friend class ListMT<T>;
    };

public:
    ListMT() : locked_(false) {}
    ListMT(const ListMT&) = delete;
    ListMT& operator=(const ListMT&) = delete;

    // Для односложных операций (блокирует на 1 операцию)
    ListMTAccessor operator->()
    {
        return GetAccessor();
    }

    // Для операций со многими вызовами (блокирует надолго)
    ListMTAccessor GetAccessor()
    {
        return ListMTAccessor(this);
    }

private:
    std::atomic_bool locked_;
    std::list<T> list_;
};

} // namespace plane_render
