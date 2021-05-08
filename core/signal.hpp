#ifndef CORE_SIGNAL_HPP_
#define CORE_SIGNAL_HPP_

#include <functional>
#include <vector>

namespace CORE
{
    // Signal which calls every callee and disgregards the return type
    template <typename ReturnType, typename... Args>
    class SignalEveryone
    {
    public:
        explicit SignalEveryone() noexcept {}

        void Register(const std::function<ReturnType(Args...)>& arCalleeFunction)
        {
            mCallees.push_back(arCalleeFunction);
        }

        void operator()(Args... args) const
        {
            for (const auto& Call : mCallees)
                Call(args...);
        }

    private:
        std::vector<std::function<ReturnType(Args...)>> mCallees;
    };

    /// Special case for functions without parameters
    template <typename ReturnType>
    class SignalEveryone<ReturnType, void>
    {
    public:
        explicit SignalEveryone() noexcept {}

        void Register(const std::function<ReturnType()>& arCalleeFunction)
        {
            mCallees.push_back(arCalleeFunction);
        }

        void operator()() const
        {
            for (const auto& Call : mCallees)
                Call();
        }

    private:
        std::vector<std::function<ReturnType()>> mCallees;
    };
}

#endif
