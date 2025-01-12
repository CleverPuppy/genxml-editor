#include "xml_parser.h"
#include "thirdparty/tinyxml2/tinyxml2.h"
#include <functional>
#include <iostream>
#include <sstream>

#define PARSE_ERROR(err_msg) std::cerr << "err_msg" << std::endl;

#define SUCCESS_OR_RETURN(err, err_msg, return_express)                        \
  if (err != tinyxml2::XMLError::XML_SUCCESS) {                                \
    PARSE_ERROR(err_msg);                                                      \
    return return_express;                                                     \
  }

static std::string ErrorAttributeMsg(tinyxml2::XMLElement *element,
                                     const std::string &attributeName,
                                     tinyxml2::XMLError errorCode) {
  std::stringstream ss;
  ss << "[ERROR] Line " << element->GetLineNum()
     << "Parse Failed. No attribute of " << attributeName << "\n";
  return ss.str();
}

static std::string ErrorElementMsg(tinyxml2::XMLElement *element,
                                   const std::string &reason) {
  std::stringstream ss;
  ss << "[ERROR] Line " << element->GetLineNum() << "Parse Failed;";
  if (!reason.empty()) {
    ss << " Reason: " << reason << ".";
  }
  ss << "\n";
  return ss.str();
}

static bool
ForeachChildNode(tinyxml2::XMLElement *element,
                 const std::string_view &nodeName,
                 std::function<bool(tinyxml2::XMLElement *element)> &&func) {
  bool ret = true;
  for (tinyxml2::XMLElement *child =
           element->FirstChildElement(nodeName.data());
       child; child = child->NextSiblingElement(nodeName.data())) {
    if (!func(child)) {
      ret = false;
    }
  }
  return ret;
}

static bool DoParseXMLBaseData(tinyxml2::XMLElement *element,
                               XMLBaseData &out) {
  assert(element);
  const char *strName = nullptr;
  const char *strInfo = nullptr;
  const char *strPrefix = nullptr;
  tinyxml2::XMLError error;
  error = element->QueryAttribute("name", &strName);
  SUCCESS_OR_RETURN(error, ErrorAttributeMsg(element, "name", error), false);

  // optional attributes
  element->QueryAttribute("prefix", &strPrefix);
  element->QueryAttribute("info", &strInfo);

  out.name = std::string(strName);
  if (strInfo)
    out.info->assign(strInfo);
  if (strPrefix)
    out.prefix->assign(strPrefix);
  return true;
}

static bool DoParseXMLValueData(tinyxml2::XMLElement *element,
                                XMLValueData &out) {
  if (!DoParseXMLBaseData(element, (XMLBaseData &)out)) {
    return false;
  }
  uint64_t value;
  tinyxml2::XMLError error = element->QueryAttribute("value", &value);
  SUCCESS_OR_RETURN(error, ErrorAttributeMsg(element, "value", error), false);
  out.value = value;
  return true;
}

static bool DoParseXMLFieldData(tinyxml2::XMLElement *element,
                                XMLFieldData &out) {
  if (!DoParseXMLBaseData(element, (XMLBaseData &)out)) {
    return false;
  }
  uint32_t start;
  uint32_t end;
  uint64_t defaultValue;
  bool haveDefaultValue;
  const char *strType;

  tinyxml2::XMLError error = element->QueryAttribute("start", &start);
  SUCCESS_OR_RETURN(error, ErrorAttributeMsg(element, "start", error), false);
  error = element->QueryAttribute("end", &end);
  SUCCESS_OR_RETURN(error, ErrorAttributeMsg(element, "end", error), false);
  error = element->QueryAttribute("type", &strType);
  SUCCESS_OR_RETURN(error, ErrorAttributeMsg(element, "end", error), false);

  if (tinyxml2::XMLError::XML_SUCCESS ==
      element->QueryAttribute("default", &defaultValue)) {
    haveDefaultValue = true;
  }

  std::optional<std::vector<XMLValueData>> choices;
  if (!ForeachChildNode(
          element, "value", [&choices](tinyxml2::XMLElement *element) {
            if (!choices.has_value()) {
              choices = std::vector<XMLValueData>();
            }
            XMLValueData data;
            if (!DoParseXMLValueData(element, data)) {
              PARSE_ERROR(
                  ErrorElementMsg(element, "Failed to get valid 'value' data"));
              return false;
            }
            choices->emplace_back(data);
            return true;
          })) {
    PARSE_ERROR(
        ErrorElementMsg(element, "Traverse child node of 'value' failed"));
    return false;
  }

  out.start = start;
  out.end = end;
  out.type = strType;
  if (haveDefaultValue)
    out.defaultValue = defaultValue;
  std::swap(out.choices, choices);

  return true;
}

static bool DoParseXMLEnumData(tinyxml2::XMLElement *element,
                               XMLEnumData &out) {
  assert(element);
  if (!DoParseXMLBaseData(element, (XMLBaseData &)out)) {
    return false;
  }

  std::vector<XMLValueData> values;
  if (!ForeachChildNode(element, "value",
                        [&values](tinyxml2::XMLElement *element) {
                          // std::cout << "Parse" <<element->GetText() << std::endl;
                          XMLValueData valueData;
                          if (!DoParseXMLValueData(element, valueData)) {
                            return false;
                          }
                          values.emplace_back(std::move(valueData));
                          return true;
                        })) {
    PARSE_ERROR(
        ErrorElementMsg(element, "Traverse child node of 'value' failed"));
    return false;
  }

  if (values.empty()) {
    PARSE_ERROR(
        ErrorElementMsg(element, "No 'value' belong to this 'enum' node"));
    return false;
  }

  std::swap(out.values, values);

  return true;
}

static bool DoParseXMLStructData(tinyxml2::XMLElement *element,
                                 XMLStructData &out) {
  assert(element);
  if (!DoParseXMLBaseData(element, (XMLBaseData &)out)) {
    return false;
  }
  uint32_t length;
  std::vector<XMLFieldData> fields;
  tinyxml2::XMLError error;

  error = element->QueryAttribute("length", &length);
  SUCCESS_OR_RETURN(error, ErrorAttributeMsg(element, "length", error), false);

  if (!ForeachChildNode(
          element, "field", [&fields](tinyxml2::XMLElement *element) {
            XMLFieldData data;
            if (!DoParseXMLFieldData(element, data)) {
              PARSE_ERROR(ErrorElementMsg(element, "Not a valid 'field'"));
              return false;
            }
            fields.emplace_back(std::move(data));
            return true;
          })) {
    PARSE_ERROR(
        ErrorElementMsg(element, "Traverse child node of 'field' failed"));
    return false;
  }

  if (fields.empty()) {
    PARSE_ERROR(ErrorElementMsg(element, "Have no 'field' nodes"));
    return false;
  }

  out.length = length;
  std::swap(out.fields, fields);

  return true;
}

static bool DoParseXMLDocData(tinyxml2::XMLDocument &xmldoc, XMLDocData &out) {
  tinyxml2::XMLElement *genxmlNode = xmldoc.FirstChildElement("genxml");
  if (!genxmlNode) {
    PARSE_ERROR("no genxml root node.");
    return false;
  }

  XMLDocData docData;

  // Parse the 'enum's
  if (!ForeachChildNode(
          genxmlNode, "enum", [&docData](tinyxml2::XMLElement *element) {
            XMLEnumData enumData;
            if (!DoParseXMLEnumData(element, enumData)) {
              PARSE_ERROR(
                  ErrorElementMsg(element, "Invalid 'enum' definition"));
              return false;
            }
            docData.enumerates.emplace_back(std::move(enumData));
            return true;
          })) {
    PARSE_ERROR("Error happends when parse 'enum' definitions");
    return false;
  }

  // parse the 'struct's
  if (!ForeachChildNode(
          genxmlNode, "struct", [&docData](tinyxml2::XMLElement *element) {
            std::cout << "Parse " << element->Attribute("name") << std::endl;
            XMLStructData Data;
            if (!DoParseXMLStructData(element, Data)) {
              PARSE_ERROR(
                  ErrorElementMsg(element, "Invalid 'struct' definition"));
              return false;
            }
            docData.structures.emplace_back(std::move(Data));
            return true;
          })) {
    PARSE_ERROR("Error happends when parse 'struct' definitions");
    return false;
  }

  std::swap(out, docData);
  return true;
}

bool XMLParserContext::init() {
  if (validContext) {
    std::cout << "Already Inited. Skit." << std::endl;
    return true;
  }
  tinyxml2::XMLError error = doc.LoadFile(filename.c_str());
  if (error != tinyxml2::XMLError::XML_SUCCESS) {
    return false;
  }

  if (!DoParseXMLDocData(doc, parsedDoc)) {
    std::cerr << "Parse XML doc [" << filename << "] failed." << std::endl;
    return false;
  }

  validContext = true;
  return true;
}
