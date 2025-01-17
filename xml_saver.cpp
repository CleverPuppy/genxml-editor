#include "xml_saver.h"
#include "tinyxml2.h"
#include "xml_types.h"
#include <cassert>
#include <iostream>

static void ToXml(tinyxml2::XMLElement *element, const XMLBaseData &data) {
  assert(element);
  element->SetAttribute("name", data.name.c_str());
  if (data.info.has_value()) {
    element->SetAttribute("info", data.info->c_str());
  }
  if (data.prefix.has_value()) {
    element->SetAttribute("prefix", data.prefix.value().c_str());
  }
}

static void ToXml(tinyxml2::XMLElement *element, const XMLValueData &data) {
  ToXml(element, dynamic_cast<const XMLBaseData &>(data));
  element->SetAttribute("value", data.value);
}

static void ToXml(tinyxml2::XMLElement *element,
                  const XMLFieldData &fieldData) {
  ToXml(element, dynamic_cast<const XMLBaseData &>(fieldData));
  element->SetAttribute("start", fieldData.start);
  element->SetAttribute("end", fieldData.end);
  element->SetAttribute("type", fieldData.type.c_str());
  if (fieldData.defaultValue.has_value()) {
    element->SetAttribute("default", fieldData.defaultValue.value());
  }
  if (fieldData.choices.has_value()) {
    for (const XMLValueData &valueData : fieldData.choices.value()) {
      tinyxml2::XMLElement *pChild = element->InsertNewChildElement("value");
      ToXml(pChild, valueData);
    }
  }
}

static void ToXml(tinyxml2::XMLElement *element, const XMLEnumData &enumData) {
  ToXml(element, dynamic_cast<const XMLBaseData &>(enumData));
  for (const XMLValueData &valueData : enumData.values) {
    tinyxml2::XMLElement *pChild = element->InsertNewChildElement("value");
    ToXml(pChild, valueData);
  }
}

static void ToXml(tinyxml2::XMLElement *element,
                  const XMLStructData &structData) {
  ToXml(element, dynamic_cast<const XMLBaseData &>(structData));
  element->SetAttribute("length", structData.length);
  for (const XMLFieldData &fieldData : structData.fields) {
    tinyxml2::XMLElement *pChild = element->InsertNewChildElement("field");
    ToXml(pChild, fieldData);
  }
}

static void ToXml(tinyxml2::XMLElement *element, const XMLDocData &docData) {
  ToXml(element, dynamic_cast<const XMLBaseData &>(docData));
  for (const XMLEnumData &enumData : docData.enumerates) {
    auto *pChild = element->InsertNewChildElement("enum");
    ToXml(pChild, enumData);
  }

  for (const XMLStructData &structData : docData.structures) {
    auto *pChild = element->InsertNewChildElement("struct");
    ToXml(pChild, structData);
  }
}

bool SaveToFile(const XMLDocData &data, const char* file) {
  tinyxml2::XMLDocument doc;
  doc.InsertFirstChild(doc.NewElement("genxml"));
  ToXml(doc.RootElement(), data);
  tinyxml2::XMLError result = doc.SaveFile(file);
  if (result != tinyxml2::XMLError::XML_SUCCESS) {
    std::cerr << "Error: Save xml to '" << file << "' failed." << std::endl;
  } else {
    std::cout << "Save to '" << file << "' success." << std::endl;
  }
  return result;
}