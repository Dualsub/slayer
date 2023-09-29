#pragma once

namespace Slayer {
    template<typename T>
    class Singleton
    {
    protected:
        static T* s_instance;
    public:
        Singleton() = default;
        ~Singleton() = default;

        static T* Get() { return s_instance; }
        static void SetInstance(T* instance) { s_instance = instance; }
    };


    template<typename T>
    T* Singleton<T>::s_instance = nullptr;
}