#pragma once

#include "Core/Core.h"
#include "Core/Math.h"
#include <fstream>
#include <iostream>
#include "Serialization/Serialization.h"

namespace Slayer {

    class JsonSerializer
    {
    private:
        std::string m_output = "";
        uint32_t m_indent = 4;
        uint32_t m_curr_indent = 0;
    public:
        JsonSerializer() = default;
        ~JsonSerializer() = default;

        void PushVariable(const std::string& name, const std::string& value)
        {
            m_output += std::string(m_curr_indent * m_indent, ' ');
            m_output += "\"" + name + "\": " + value + ",\n";
        }

        void PushObject(const std::string& name)
        {
            m_output += std::string(m_curr_indent * m_indent, ' ');
            if (name.empty())
                m_output += "{\n";
            else
                m_output += "\"" + name + "\": {\n";
            m_curr_indent++;
        }

        void PopObject()
        {
            // RFind and remove the last comma.
            m_output = m_output.substr(0, m_output.rfind(",")) + "\n";
            std::cout << m_output << std::endl;

            m_curr_indent--;
            m_output += std::string(m_curr_indent * m_indent, ' ');
            m_output += "},\n";
        }

        void PushArray(const std::string& name)
        {
            m_output += std::string(m_curr_indent * m_indent, ' ');
            m_output += "\"" + name + "\": [\n";
            m_curr_indent++;
        }

        void PopArray()
        {
            // RFind and remove the last comma.
            m_output = m_output.substr(0, m_output.rfind(",")) + "\n";

            m_curr_indent--;
            m_output += std::string(m_curr_indent * m_indent, ' ');
            m_output += "],\n";
        }

        template<typename T>
        void Serialize(const std::string& path, T& data)
        {
            m_output.clear();
            Transfer("", data);
            
            std::ofstream out(path);
            out << m_output;
            out.close();
        }

        template<typename T>
        void TransferVector(const std::string& name, std::vector<T>& data, const int32_t& maxCount = -1)
        {
            PushArray(name);
            for (auto& element : data)
            {
                Transfer("", element);
            }
            PopArray();
        }

        template<typename T>
        void Transfer(const std::string& name, T& data)
        {
            PushObject(name);
            data.Serialize(*this);
            PopObject();
        }

        template<>
        void Transfer(const std::string& name, float& data)
        {
            PushVariable(name, std::to_string(data));
        }

        template<>
        void Transfer(const std::string& name, uint32_t& data)
        {
            PushVariable(name, std::to_string(data));
        }

        template<>
        void Transfer(const std::string& name, std::string& data)
        {
            PushVariable(name, "\"" + data + "\"");
        }

        template<>
        void Transfer(const std::string& name, Vec2& data)
        {
            PushObject(name);
            Transfer("x", data.r);
            Transfer("y", data.g);
            PopObject();
        }

        template<>
        void Transfer(const std::string& name, Vec3& data)
        {
            PushObject(name);
            Transfer("x", data.x);
            Transfer("y", data.y);
            Transfer("z", data.z);
            PopObject();
        }

        template<>
        void Transfer(const std::string& name, Vec4& data)
        {
            PushObject(name);
            Transfer("x", data.x);
            Transfer("y", data.y);
            Transfer("z", data.z);
            Transfer("w", data.w);
            PopObject();
        }

        template<>
        void Transfer(const std::string& name, Mat4& data)
        {
            PushObject(name);
            for (size_t i = 0; i < 4 * 4; i++)
            {
                Transfer(std::to_string(i), data[i]);
            }
            PopObject();
        }

        template<>
        void Transfer(const std::string& name, Quat& data)
        {
            PushObject(name);
            Transfer("x", data.x);
            Transfer("y", data.y);
            Transfer("z", data.z);
            Transfer("w", data.w);
            PopObject();
        }
    };

    class JsonDeserializer
    {
    public:
        bool Deserialize(ComponentStore& store, const std::string& data)
        {

        }
    };


}