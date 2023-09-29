#pragma once 

#include "Core/Core.h"
#include "Core/Math.h"
#include "Core/Containers.h"
#include "Resources/Asset.h"
#include "Resources/ResourceManager.h"
#include "Serialization/Serialization.h"

#include "imgui.h"

namespace Slayer::Editor {

    class PropertySerializer : public Serializer<SerializationFlags::ReadWrite>
    {
    public:
        PropertySerializer() = default;
        ~PropertySerializer() = default;

        const ImGuiTreeNodeFlags_ treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen;

        template<typename T>
        void Transfer(T* value, const std::string& name)
        {
            if (ImGui::TreeNodeEx(name.c_str(), treeNodeFlags))
            {
                value->Transfer(*this);
                ImGui::TreePop();
            }
        }

        template<typename T>
        void Transfer(T& value, const std::string& name)
        {
            if (ImGui::TreeNodeEx(name.c_str(), treeNodeFlags))
            {
                value.Transfer(*this);
                ImGui::TreePop();
            }
        }

        template<>
        void Transfer(float& value, const std::string& name)
        {
            ImGui::DragFloat(name.c_str(), &value);
        }

        template<>
        void Transfer(uint32_t& value, const std::string& name)
        {
            ImGui::DragInt(name.c_str(), (int*)&value);
        }

        template<>
        void Transfer(AssetID& value, const std::string& name)
        {
            ImGui::InputInt(name.c_str(), (int*)&value, 1, 100, ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_CharsDecimal);
        }

        void TransferAsset(AssetID& value, AssetType assetType, const std::string& name)
        {
            auto* rm = ResourceManager::Get();
            const Map<AssetID, AssetRecord>& assetRecords = rm->GetAssetRecords(assetType);
            std::string currentAssetName = assetRecords.find(value) != assetRecords.end() ? assetRecords.at(value).name : "None";

            if (ImGui::BeginCombo(name.c_str(), currentAssetName.c_str()))
            {
                for (auto& [assetId, assetRecord] : assetRecords)
                {
                    if (ImGui::Selectable(assetRecord.name.c_str()))
                    {
                        value = assetId;
                    }
                }

                ImGui::EndCombo();
            }
        }

        template<>
        void Transfer(std::string& value, const std::string& name)
        {
            ImGui::InputText(name.c_str(), value.data(), value.size());
        }

        template<typename T>
        void TransferVector(Vector<T>& values, const std::string& name)
        {
            if (ImGui::TreeNode(name.c_str()))
            {
                for (auto& value : values)
                {
                    ImGui::PushID(&value);
                    value.Transfer(*this);
                    ImGui::PopID();
                }

                ImGui::TreePop();
            }
        }

        template<>
        void Transfer(Vec3& value, const std::string& name)
        {
            ImGui::DragFloat3(name.c_str(), &value.x);
        }

        template<>
        void Transfer(Quat& value, const std::string& name)
        {
            ImGui::DragFloat4(name.c_str(), &value.w);
        }

        template<>
        void Transfer(Mat4& value, const std::string& name)
        {
            ImGui::DragFloat4("Row 1", &value[0][0]);
            ImGui::DragFloat4("Row 2", &value[1][0]);
            ImGui::DragFloat4("Row 3", &value[2][0]);
            ImGui::DragFloat4("Row 4", &value[3][0]);
        }

    };
}