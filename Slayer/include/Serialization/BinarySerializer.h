#pragma once

#include "Core/Core.h"
#include "Core/Log.h"
#include "Serialization/Serialization.h"

#include <fstream>
#include <iostream>
#include <tuple>

namespace Slayer {
    class BinarySerializer : public Serializer<SerializationFlags::Read>
    {
    private:

    public:
        BinarySerializer() = default;
        ~BinarySerializer() = default;

        template<typename T>
        void Serialize(T& value, const std::string& path)
        {

        }

        bool PushObject(const std::string& name = "")
        {

            return true;
        }

        void PopObject()
        {

        }

        bool PushArray(const std::string& name)
        {

            return true;
        }

        void PopArray()
        {

        }

        bool Next()
        {
            SL_ASSERT(false && "Not implemented");
            return false;
        }

        bool IsValid(std::string& name)
        {
            SL_ASSERT(false && "Not implemented");
            return false;
        }

        template<typename T>
        void TransferArrayElement(T& value)
        {

        }

        bool PushArrayElement()
        {

            return true;
        }

        void PopArrayElement()
        {

        }

        template<typename T>
        void Transfer(Shared<T> value, const std::string& name)
        {

        }

        template<typename T>
        void Transfer(T* value, const std::string& name)
        {

        }

        template<typename T>
        void Transfer(T& value, const std::string& name)
        {

        }

        template<>
        void Transfer(float& value, const std::string& name)
        {

        }

        template<>
        void Transfer(char& value, const std::string& name)
        {

        }

        template<>
        void Transfer(uint32_t& value, const std::string& name)
        {

        }

        template<>
        void Transfer(AssetID& value, const std::string& name)
        {

        }

        template<>
        void Transfer(std::string& value, const std::string& name)
        {

        }

        template<typename T>
        void TransferVector(Vector<T>& values, const std::string& name, const int32_t& maxCount = -1)
        {

        }

        template<>
        void Transfer(Vec3& value, const std::string& name)
        {

        }

        template<>
        void Transfer(Quat& value, const std::string& name)
        {

        }
    };


    class BinaryDeserializer : public Serializer<SerializationFlags::Write>
    {
    private:
        const char* m_data;
        char* m_current;
        uint32_t m_size;
    public:
        BinaryDeserializer() = default;
        ~BinaryDeserializer() = default;

        template<typename T>
        void CopyData(T* dst, const size_t size)
        {
            Copy(m_current, dst, size);
            m_current += size;
        }

        template<typename T>
        void Deserialize(T& obj, const char* data, size_t size)
        {
            m_data = data;
            m_current = (char*)data;
            m_size = size;
            obj.Transfer(*this);
        }

        bool PushObject(const std::string& name = "")
        {

            return false;
        }

        void PopObject()
        {

        }

        bool PushArray(const std::string& name)
        {

            return false;
        }

        void PopArray()
        {

        }

        bool PushArrayElement()
        {

            return false;
        }

        void PopArrayElement()
        {

        }

        bool Next()
        {

            return false;
        }

        bool IsValid(std::string& name)
        {

            return false;
        }

        template<typename T>
        void TransferArrayElement(T& value)
        {

        }

        template<typename T>
        void Transfer(T& value, const std::string& name)
        {
            value.Transfer(*this);
        }

        template<typename T>
        void Transfer(Shared<T> value, const std::string& name)
        {

        }

        template<typename T>
        void Transfer(T* value, const std::string& name)
        {

        }

        template<>
        void Transfer(float& value, const std::string& name)
        {
            SL_ASSERT((m_current <= m_data + m_size) && "Out of bounds.");
            Copy(m_current, &value, sizeof(float));
            m_current += sizeof(float);
        }

        template<>
        void Transfer(uint8_t& value, const std::string& name)
        {
            SL_ASSERT((m_current <= m_data + m_size) && "Out of bounds.");
            Copy(m_current, &value, sizeof(uint8_t));
            m_current += sizeof(uint8_t);
        }

        template<>
        void Transfer(int32_t& value, const std::string& name)
        {
            SL_ASSERT((m_current <= m_data + m_size) && "Out of bounds.");
            Copy(m_current, &value, sizeof(int32_t));
            m_current += sizeof(int32_t);
        }

        template<>
        void Transfer(uint32_t& value, const std::string& name)
        {
            SL_ASSERT((m_current <= m_data + m_size) && "Out of bounds.");
            Copy(m_current, &value, sizeof(uint32_t));
            m_current += sizeof(uint32_t);
        }

        template<>
        void Transfer(uint64_t& value, const std::string& name)
        {
            SL_ASSERT((m_current <= m_data + m_size) && "Out of bounds.");
            Copy(m_current, &value, sizeof(uint64_t));
            m_current += sizeof(uint64_t);
        }

        template<>
        void Transfer(std::string& value, const std::string& name)
        {
            SL_ASSERT((m_current <= m_data + m_size) && "Out of bounds.");

            uint32_t size;
            Copy(m_current, &size, sizeof(uint32_t));
            m_current += sizeof(uint32_t);

            SL_ASSERT((m_current + size <= m_data + m_size) && "Out of bounds.");

            value.resize(size);
            Copy(m_current, value.data(), size);
            m_current += size;
        }

        template<typename T>
        void TransferVector(Vector<T>& values, const std::string& name, const int32_t& maxCount = -1)
        {
            uint32_t size = 0;
            Copy(m_current, &size, sizeof(uint32_t));
            m_current += sizeof(uint32_t);

            SL_ASSERT((m_current <= m_data + m_size) && "Out of bounds.");

            values.resize(size);
            for (uint32_t i = 0; i < size; ++i)
            {
                Transfer(values[i], std::to_string(i));
            }
        }

        template<typename T>
        void TransferVectorPacked(Vector<T>& values, const std::string& name)
        {
            uint32_t size = 0;
            Copy(m_current, &size, sizeof(uint32_t));
            m_current += sizeof(uint32_t);

            if (!(m_current + size * sizeof(T) <= m_data + m_size))
                Log::Error(name, "Out of bounds size:", size, "sizeof(T):", sizeof(T), "m_size:", m_size);
            SL_ASSERT((m_current + size * sizeof(T) <= m_data + m_size) && "Out of bounds.");

            values.resize(size);
            CopyData(values.data(), size * sizeof(T));
        }

        template<typename T, typename U>
        void TransferDict(Dict<T, U>& values, const std::string& name)
        {
            uint32_t size = 0;
            Copy(m_current, &size, sizeof(uint32_t));
            m_current += sizeof(uint32_t);

            SL_ASSERT((m_current + size * sizeof(T) <= m_data + m_size) && "Out of bounds.");

            for (uint32_t i = 0; i < size; ++i)
            {
                T key;
                Transfer(key, "key");
                U value;
                Transfer(value, "value");

                values[key] = value;
            }
        }

        template<>
        void Transfer(Vec3& value, const std::string& name)
        {
            Transfer(value.x, "x");
            Transfer(value.y, "y");
            Transfer(value.z, "z");
        }

        template<>
        void Transfer(Quat& value, const std::string& name)
        {
            Transfer(value.w, "w");
            Transfer(value.x, "x");
            Transfer(value.y, "y");
            Transfer(value.z, "z");
        }

        template<>
        void Transfer(Mat4& value, const std::string& name)
        {
            if (m_current >= m_data + m_size)
                SL_ASSERT(false && "Out of bounds");
            Copy(m_current, &value, sizeof(Mat4));
            m_current += sizeof(Mat4);
        }
    };
}