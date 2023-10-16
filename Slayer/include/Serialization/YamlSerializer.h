#pragma once

#include "Core/Core.h"
#include "Core/Math.h"
#include "Serialization/Serialization.h"

#include <fstream>
#include <iostream>
#include <tuple>

#include "yaml-cpp/yaml.h"

namespace Slayer {
    class YamlSerializer : public Serializer<SerializationFlags::Read>
    {
    private:
        std::string path;
    public:
        YamlSerializer() = default;
        ~YamlSerializer() = default;

        YAML::Emitter out;

        template<typename T>
        void Serialize(T& value, const std::string& path)
        {
            out << YAML::BeginMap;
            value.Transfer(*this);
            out << YAML::EndMap;
            std::ofstream stream(path);
            stream << out.c_str();
            stream.close();
        }

        bool PushObject(const std::string& name = "")
        {
            if (!name.empty())
            {
                out << YAML::Key << name;
                out << YAML::Value;
            }
            out << YAML::BeginMap;
            return true;
        }

        void PopObject()
        {
            out << YAML::EndMap;
        }

        bool PushArray(const std::string& name)
        {
            out << YAML::Key << name;
            out << YAML::Value;
            out << YAML::BeginSeq;
            return true;
        }

        void PopArray()
        {
            out << YAML::EndSeq;
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
            out << YAML::BeginMap;
            value.Transfer(*this);
            out << YAML::EndMap;
        }

        bool PushArrayElement()
        {
            out << YAML::BeginMap;
            return true;
        }

        void PopArrayElement()
        {
            out << YAML::EndMap;
        }

        template<typename T>
        void Transfer(Shared<T> value, const std::string& name)
        {
            out << YAML::Key << name;
            out << YAML::Value;
            out << YAML::BeginMap;
            value->Transfer(*this);
            out << YAML::EndMap;
        }

        template<typename T>
        void Transfer(T* value, const std::string& name)
        {
            out << YAML::Key << name;
            out << YAML::Value;
            out << YAML::BeginMap;
            value.Transfer(*this);
            out << YAML::EndMap;
        }

        template<typename T>
        void Transfer(T& value, const std::string& name)
        {
            out << YAML::Key << name;
            out << YAML::Value;
            out << YAML::BeginMap;
            value.Transfer(*this);
            out << YAML::EndMap;
        }

        template<>
        void Transfer(float& value, const std::string& name)
        {
            out << YAML::Key << name << YAML::Value << value;
        }

        template<>
        void Transfer(char& value, const std::string& name)
        {
            out << YAML::Key << name << YAML::Value << value;
        }

        template<>
        void Transfer(uint32_t& value, const std::string& name)
        {
            out << YAML::Key << name << YAML::Value << value;
        }

        template<>
        void Transfer(AssetID& value, const std::string& name)
        {
            out << YAML::Key << name << YAML::Value << value;
        }

        template<>
        void Transfer(std::string& value, const std::string& name)
        {
            out << YAML::Key << name << YAML::Value << value;
        }

        void TransferAsset(AssetID& value, AssetType assetType, const std::string& name)
        {
            Transfer(value, name);
        }

        template<typename T>
        void TransferVector(Vector<T>& values, const std::string& name, const int32_t& maxCount = -1)
        {
            out << YAML::Key << name;
            out << YAML::Value;

            out << YAML::BeginSeq;

            for (auto& value : values)
            {
                out << YAML::BeginMap;
                value.Transfer(*this);
                out << YAML::EndMap;
            }

            out << YAML::EndSeq;
        }

        template<>
        void Transfer(Vec3& value, const std::string& name)
        {
            out << YAML::Key << name;
            out << YAML::Value;
            out << YAML::Flow;
            out << YAML::BeginSeq << value.x << value.y << value.z << YAML::EndSeq;
        }

        template<>
        void Transfer(Quat& value, const std::string& name)
        {
            out << YAML::Key << name;
            out << YAML::Value;
            out << YAML::Flow;
            out << YAML::BeginSeq << value.w << value.x << value.y << value.z << YAML::EndSeq;
        }

        void Transfer(Mat4& value, const std::string& name)
        {
            Vec3 position;
            Quat rotation;
            Vec3 scale;
            Vec3 skew;
            Vec4 perspective;
            if (glm::decompose(value, scale, rotation, position, skew, perspective))
                return;

            out << YAML::Key << name;
            out << YAML::Value;
            Transfer(position, "position");
            Transfer(rotation, "rotation");
            Transfer(scale, "scale");
        }
    };


    class YamlDeserializer : public Serializer<SerializationFlags::Write>
    {
    private:
        std::string path;
    public:
        YamlDeserializer() = default;
        ~YamlDeserializer() = default;

        std::stack<YAML::Node> nodeStack;
        // Index and size of the array
        std::stack<std::tuple<uint32_t, uint32_t>> indexStack;

        template<typename T>
        T Deserialize(T& obj, const std::string& path)
        {
            if (!std::filesystem::exists(path))
            {
                Log::Error("File does not exist: ", std::filesystem::absolute(path).string());
                SL_ASSERT(false);
            }
            auto node = YAML::LoadFile(path);
            nodeStack.push(node);
            obj.Transfer(*this);
            nodeStack.pop();
            return obj;
        }

        bool PushObject(const std::string& name = "")
        {
            auto node = nodeStack.top();
            if (node[name])
            {
                nodeStack.push(node[name]);
                return true;
            }
            return false;
        }

        void PopObject()
        {
            nodeStack.pop();
        }

        bool PushArray(const std::string& name)
        {
            auto node = nodeStack.top();
            if (node[name])
            {
                indexStack.push({ uint32_t(0), (uint32_t)node[name].size() });
                nodeStack.push(node[name]);
                return true;
            }
            return false;
        }

        void PopArray()
        {
            indexStack.pop();
            nodeStack.pop();
        }

        bool PushArrayElement()
        {
            auto& [index, size] = indexStack.top();
            auto& node = nodeStack.top();
            if (node[index - 1])
            {
                nodeStack.push(node[index - 1]);
                return true;
            }

            return false;
        }

        void PopArrayElement()
        {
            nodeStack.pop();
        }

        bool Next()
        {
            auto& [index, size] = indexStack.top();
            if (index < size)
            {
                index++;
                return true;
            }
            return false;
        }

        bool IsValid(std::string& name)
        {
            auto& node = nodeStack.top();
            if (node[name])
            {
                return true;
            }
            return false;
        }

        template<typename T>
        void TransferArrayElement(T& value)
        {
            // Get from index stack
            auto& [index, size] = indexStack.top();
            auto node = nodeStack.top();
            if (node[index - 1])
            {
                nodeStack.push(node[index - 1]);
                value.Transfer(*this);
                nodeStack.pop();
            }
        }

        template<typename T>
        void Transfer(T& value, const std::string& name)
        {
            auto node = nodeStack.top();
            if (node[name])
            {
                nodeStack.push(node[name]);
                value.Transfer(*this);
                nodeStack.pop();
            }
        }

        template<typename T>
        void Transfer(Shared<T> value, const std::string& name)
        {
            auto node = nodeStack.top();
            if (node[name])
            {
                nodeStack.push(node[name]);
                value->Transfer(*this);
                nodeStack.pop();
            }
        }

        template<typename T>
        void Transfer(T* value, const std::string& name)
        {
            auto node = nodeStack.top();
            if (node[name])
            {
                nodeStack.push(node[name]);
                value->Transfer(*this);
                nodeStack.pop();
            }
        }

        template<>
        void Transfer(float& value, const std::string& name)
        {
            auto node = nodeStack.top();
            if (node[name])
                value = node[name].as<float>();
        }

        template<>
        void Transfer(char& value, const std::string& name)
        {
            auto node = nodeStack.top();
            if (node[name])
                value = node[name].as<char>();
        }

        template<>
        void Transfer(uint32_t& value, const std::string& name)
        {
            auto node = nodeStack.top();
            if (node[name])
                value = node[name].as<uint32_t>();
        }

        template<>
        void Transfer(uint64_t& value, const std::string& name)
        {
            auto node = nodeStack.top();
            if (node[name])
                value = node[name].as<uint64_t>();
        }

        template<>
        void Transfer(std::string& value, const std::string& name)
        {
            auto node = nodeStack.top();
            if (node[name])
                value = node[name].as<std::string>();
        }

        void TransferAsset(AssetID& value, AssetType assetType, const std::string& name)
        {
            Transfer(value, name);
        }

        template<typename T>
        void TransferVector(Vector<T>& values, const std::string& name, const int32_t& maxCount = -1)
        {
            auto parentNode = nodeStack.top();
            auto node = parentNode[name];
            if (node && node.IsSequence())
            {
                values.resize(node.size());
                for (int i = 0; i < node.size(); i++)
                {
                    nodeStack.push(node[i]);
                    values[i].Transfer(*this);
                    nodeStack.pop();
                }
            }
        }

        template<>
        void Transfer(Vec3& value, const std::string& name)
        {
            auto node = nodeStack.top()[name];
            if (!node)
                return;
            if (!node.IsSequence() || node.size() != 3)
                return;

            value.x = node[0].as<float>();
            value.y = node[1].as<float>();
            value.z = node[2].as<float>();
        }

        template<>
        void Transfer(Quat& value, const std::string& name)
        {
            auto node = nodeStack.top()[name];
            if (!node)
                return;
            if (!node.IsSequence() || node.size() != 4)
                return;

            value.w = node[0].as<float>();
            value.x = node[1].as<float>();
            value.y = node[2].as<float>();
            value.z = node[3].as<float>();
        }

        template<>
        void Transfer(Mat4& value, const std::string& name)
        {
            auto node = nodeStack.top()[name];
            if (node[name])
                return;

            Vec3 position;
            Transfer(position, "position");
            Quat rotation;
            Transfer(rotation, "rotation");
            Vec3 scale;
            Transfer(scale, "scale");

            value = glm::translate(Mat4(1.0f), position) * glm::toMat4(rotation) * glm::scale(Mat4(1.0f), scale);
        }
    };
}