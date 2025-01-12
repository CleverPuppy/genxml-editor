#ifndef __XML_TYPES_H__
#define __XML_TYPES_H__

#include <string>
#include <optional>
#include <vector>

struct XMLBaseData {
  std::string name;
  std::optional<std::string> prefix;
  std::optional<std::string> info;
};

struct XMLValueData : public XMLBaseData {
  uint64_t value;
};

struct XMLFieldData : public XMLBaseData {
  uint32_t start;
  uint32_t end;
  std::string type;
  std::optional<uint64_t>defaultValue;
  std::optional<std::vector<XMLValueData> > choices;
};

// support 'enum' and 'structure'
struct XMLEnumData : public XMLBaseData {
  std::vector<XMLValueData> values;
};

struct XMLStructData : public XMLBaseData {
  uint32_t length;
  std::vector<XMLFieldData> fields;
};

struct XMLDocData: public XMLBaseData {
  std::vector<XMLStructData> structures;
  std::vector<XMLEnumData> enumerates;
};

#endif