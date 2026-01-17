#pragma once

#include <syncstream>

#if defined(__cpp_lib_syncbuf) && __cpp_lib_syncbuf >= 201803L
#define CARP_HAS_SYNC_STREAM
#endif

#ifndef CARP_HAS_SYNC_STREAM
#include <iostream>
#include <sstream>
#include <mutex>
#endif // CARP_HAS_SYNC_STREAM

namespace Carp
{

#ifdef CARP_HAS_SYNC_STREAM
using OSyncStream = std::osyncstream;
#else // !CARP_HAS_SYNC_STREAM
class OSyncStream
{
private:
    std::ostringstream m_buffer;
    std::ostream* m_stream = nullptr;
    bool m_has_emitted = false;

    static std::mutex& GetMutex()
    {
        static std::mutex mutex;
        return mutex;
    }

public:
    explicit OSyncStream(std::ostream& os) : m_stream(&os) {}

    OSyncStream(const OSyncStream&) = delete;
    OSyncStream& operator=(const OSyncStream&) = delete;

    OSyncStream(OSyncStream&& oth) noexcept :
        m_buffer(std::move(oth.m_buffer)),
        m_stream(oth.m_stream),
        m_has_emitted(oth.m_has_emitted)
    {
        oth.m_stream = nullptr;
        oth.m_has_emitted = true;
    }

    OSyncStream& operator=(OSyncStream&& other) noexcept
    {
        if (this != &other)
        {
            Emit();
            
            m_buffer = std::move(other.m_buffer);
            m_stream = other.m_stream;
            m_has_emitted = other.m_has_emitted;
            
            other.m_stream = nullptr;
            other.m_has_emitted = true;
        }
        return *this;
    }

    ~OSyncStream()
    {
        Emit();
    }

    OSyncStream& Emit()
    {
        if (!m_has_emitted && m_stream && m_buffer.tellp() > 0)
        {
            std::lock_guard<std::mutex> lock(GetMutex());
            *m_stream << m_buffer.str();
            m_stream->flush();
            m_has_emitted = true;
        }
        return *this;
    }

    template<typename T>
    OSyncStream& operator<<(T&& value)
    {
        m_buffer << std::forward<T>(value);
        return *this;
    }

    OSyncStream& operator<<(std::ostream& (*manip)(std::ostream&))
    {
        if (manip == static_cast<std::ostream& (*)(std::ostream&)>(std::endl))
        {
            m_buffer << '\n';
        }
        else if (manip == static_cast<std::ostream& (*)(std::ostream&)>(std::flush))
        {
        }
        else
        {
            manip(m_buffer);
        }
        return *this;
    }
};
#endif // CARP_HAS_SYNC_STREAM

} // namespace Carp
