#pragma once

#include <stdio.h>
#include <string.h>

#define MP_VECTOR micropather::MPVector

namespace micropather
{
    template <typename T>
    class MPVector
    {
        public:
        MPVector()
            : m_allocated(0)
            , m_size(0)
            , m_buf(0)
        {
        }

        ~MPVector()
        {
            delete[] m_buf;
        }

        void clear()
        {
            m_size = 0;
        }

        void resize(unsigned s)
        {
            capacity(s);
            m_size = s;
        }

        T& operator[](unsigned i)
        {
            return m_buf[i];
        }

        const T& operator[](unsigned i) const
        {
            return m_buf[i];
        }

        void push_back(const T& t)
        {
            capacity(m_size + 1);
            m_buf[m_size++] = t;
        }

        unsigned size() const
        {
            return m_size;
        }

        private:
        void capacity(unsigned cap)
        {
            if (m_allocated < cap)
            {
                unsigned newAllocated = cap * 3 / 2 + 16;
                T*       newBuf = new T[newAllocated];
                memcpy(newBuf, m_buf, sizeof(T) * m_size);
                delete[] m_buf;
                m_buf = newBuf;
                m_allocated = newAllocated;
            }
        }

        unsigned m_allocated;

        unsigned m_size;
        T*       m_buf;
    };
}; // namespace micropather