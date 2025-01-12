#ifndef __XML_PARSER_H__
#define __XML_PARSER_H__

#include "xml_types.h"
#include "thirdparty/tinyxml2/tinyxml2.h"
#include <fstream>
#include <optional>
#include <string>



class XMLParserContext {
public:
  std::string filename;

  XMLParserContext(const XMLParserContext&) = delete;
  XMLParserContext& operator=(const XMLParserContext&) = delete;

  explicit XMLParserContext(const std::string& filename):
    filename(filename), validContext(false) {}
  bool init();
  ~XMLParserContext() = default;

private:
  bool validContext;
  tinyxml2::XMLDocument doc;
  XMLDocData parsedDoc;
};

#endif