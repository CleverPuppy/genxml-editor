#include "xml_ui.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include "xml_types.h"
#include <functional>

inline bool ModalOKButton() {
  return ImGui::Button("Ok") ||  ImGui::Shortcut(ImGuiKey_Enter);
}

inline bool ModalCancelButton() {
  return ImGui::Button("Cancel") || ImGui::Shortcut(ImGuiKey_Escape);
}

static void RenderEditingValueTable(std::vector<XMLValueData> &data) {
  if (ImGui::BeginTable("Value Table", 4,
                        ImGuiTableFlags_Resizable)) {
    ImGui::TableSetupColumn("Name", 0);
    ImGui::TableSetupColumn("Value", 0);
    ImGui::TableSetupColumn("Info", 0);
    ImGui::TableSetupColumn("Operations", 0);
    ImGui::TableHeadersRow();
    for (int id = 0; id < data.size(); ++id) {
      XMLValueData &valueData = data[id];
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("%s", valueData.name.c_str());
      ImGui::TableNextColumn();
      ImGui::Text("%llu", valueData.value);
      ImGui::TableNextColumn();
      ImGui::Text("%s", valueData.info ? valueData.info->c_str() : "NA");
      ImGui::TableNextColumn();
      ImGui::PushID(id);
      if (ImGui::Button("delete")) {
        data.erase(data.begin() + id);
        ImGui::PopID();
        break;
      }
      ImGui::SameLine();
      if (ImGui::Button("edit")) {
        // todo
      }
      ImGui::PopID();
    }
    ImGui::EndTable();
  }
}

static void RenderEditingFieldTable(std::vector<XMLFieldData> &data) {
  if (ImGui::BeginTable("Field Table", 6,
                        ImGuiTableFlags_Resizable)) {
    ImGui::TableSetupColumn("Name", 0);
    ImGui::TableSetupColumn("Start", 0);
    ImGui::TableSetupColumn("End", 0);
    ImGui::TableSetupColumn("Info", 0);
    ImGui::TableSetupColumn("Choices", 0);
    ImGui::TableSetupColumn("Operations", 0);
    ImGui::TableHeadersRow();
    for (int id = 0; id < data.size(); ++id) {
      XMLFieldData &field = data[id];
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("%s", field.name.c_str());
      ImGui::TableNextColumn();
      ImGui::Text("%u", field.start);
      ImGui::TableNextColumn();
      ImGui::Text("%u", field.end);
      ImGui::TableNextColumn();
      ImGui::Text("%s", field.info ? field.info->c_str() : "NA");
      ImGui::TableNextColumn();
      if (field.choices) {
        for (const auto &choice : field.choices.value()) {
          ImGui::BulletText("%llu\t%s\t%s", choice.value, choice.name.c_str(),
                            (choice.info ? choice.info->c_str() : "NA"));
        }
      } else {
        ImGui::Text("NA");
      }
      ImGui::TableNextColumn();
      ImGui::PushID(id);
      if (ImGui::Button("delete")) {
        data.erase(data.begin() + id);
        ImGui::PopID();
        break;
      }
      ImGui::SameLine();
      if (ImGui::Button("edit")) {
        // todo
      }
      ImGui::PopID();
    }
    ImGui::EndTable();
  }
}

static void RenderOptionalText(const char *strCheckBoxLabel,
                               const char *strInputLabel, bool *bNeeded,
                               std::optional<std::string> &optionalStr) {

  if (ImGui::Checkbox(strCheckBoxLabel, bNeeded)) {
    if (!optionalStr)
      optionalStr = "";
  }
  if (*bNeeded) {
    ImGui::SameLine();
    ImGui::InputText(strInputLabel, &optionalStr.value(), 0, nullptr, nullptr);
  }
}

void XMLEditEnumUI::Render() {
  ImGui::InputText("Enum name", &currentEditing.name, 0, nullptr, nullptr);
  RenderOptionalText("Have prefix?", "prefix", &bHavePrefix,
                     currentEditing.prefix);
  RenderOptionalText("Have info?", "enum info", &bHaveInfo,
                     currentEditing.info);

  if (ImGui::BeginTable("Enum Values", 4,
                        ImGuiTableFlags_Resizable)) {
    ImGui::TableSetupColumn("Name", 0);
    ImGui::TableSetupColumn("Value", 0);
    ImGui::TableSetupColumn("Info", 0);
    ImGui::TableSetupColumn("Operations", 0);
    ImGui::TableHeadersRow();
    int id = 0;
    for (auto it = currentEditing.values.begin();
         it != currentEditing.values.end(); ++it) {
      auto &valueData = *it;
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("%s", valueData.name.c_str());
      ImGui::TableNextColumn();
      ImGui::Text("%llu", valueData.value);
      ImGui::TableNextColumn();
      ImGui::Text("%s", valueData.info ? valueData.info->c_str() : "NA");
      ImGui::TableNextColumn();
      ImGui::PushID(id);
      if (ImGui::Button("delete")) {
        currentEditing.values.erase(it);
        ImGui::PopID();
        break;
      }
      ImGui::SameLine();
      if (ImGui::Button("edit")) {
        // todo
      }
      ImGui::PopID();
      ++id;
    }
    ImGui::EndTable();
  }

  if (ImGui::Button("Add value")) {
    ImGui::OpenPopup("Edit Value");
  }
  if (ImGui::BeginPopupModal("Edit Value", NULL, ImGuiWindowFlags_Modal | ImGuiWindowFlags_AlwaysAutoResize)) {
    valueEdiotr.Render();
    if (ModalOKButton()) {
      currentEditing.values.push_back(valueEdiotr.currentEditing);
      valueEdiotr = XMLEditValueUI();
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ModalCancelButton()) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
  ImGui::NewLine();
}

void XMLEditStructUI::Render() {
  ImGui::Text("Struct:");
  ImGui::InputText("Name", &currentEditing.name);
  ImGui::InputScalar("Length", ImGuiDataType_U32, &currentEditing.length);
  RenderOptionalText("Have info?", "Info", &bHaveInfo, currentEditing.info);
  RenderEditingFieldTable(currentEditing.fields);

  if (ImGui::Button("Add Field")) {
	ImGui::OpenPopup("Edit Field");
  }
  if (ImGui::BeginPopupModal("Edit Field")) {
	fieldEditor.Render();
	if (ModalOKButton()) {
		currentEditing.fields.emplace_back(fieldEditor.currentEditing);
		fieldEditor = XMLEditFieldUI();
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ModalCancelButton()) {
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndPopup();
  }
}

void XMLEditFieldUI::Render() {
  ImGui::Text("Feild:");
  ImGui::InputText("Name", &currentEditing.name);
  ImGui::InputScalar("Start bit", ImGuiDataType_U32, &currentEditing.start);
  ImGui::InputScalar("End bit", ImGuiDataType_U32, &currentEditing.end);
  RenderOptionalText("Have info?", "Info", &bHaveInfo, currentEditing.info);
  RenderOptionalText("Have prefix?", "Prefix", &bHavePrefix,
                     currentEditing.prefix);

  if (currentEditing.choices) {
    RenderEditingValueTable(currentEditing.choices.value());
  }
  if (ImGui::Button("Add choise")) {
	if (!currentEditing.choices) currentEditing.choices = std::vector<XMLValueData>();
    ImGui::OpenPopup("Edit Choice");
  }
  if (ImGui::BeginPopupModal("Edit Choice")) {
    valueEditor.Render();
    if (ModalOKButton()) {
      currentEditing.choices->push_back(valueEditor.currentEditing);
      valueEditor = XMLEditValueUI();
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ModalCancelButton()) {
      ImGui::CloseCurrentPopup();
    }
	ImGui::EndPopup();
  }
}

void XMLEditValueUI::Render() {
  ImGui::Text("Editing Value:");
  ImGui::InputText("Name", &currentEditing.name);
  ImGui::InputScalar("Value", ImGuiDataType_U64, &currentEditing.value);
  RenderOptionalText("Have info?", "Info", &bHaveInfo, currentEditing.info);
}
