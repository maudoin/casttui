#pragma once

#include <RapidXml/rapidxml.hpp>

#include <string>

inline void html_to_text(rapidxml::xml_node<>* node, std::string& out, int listLevel = 0)
{
  using namespace rapidxml;
  if (!node) return;

  switch (node->type())
  {
    case node_type::node_data:
      out += node->value();
      return;

    case node_type::node_element:
    {
      std::string name = node->name();

      if (name == "p")
      {
          out += "\n\n";  // paragraph break
      }
      else if (name == "br")
      {
          out += "\n";
      }
      else if (name == "li")
      {
          out += std::string(listLevel * 2, ' ') + "- ";
      }
      else if (name == "ul" || name == "ol")
      {
          listLevel++;
      }
      else if (name == "h1" || name == "h2" || name == "h3")
      {
          out += "\n";
      }
      else if (name == "script" || name == "style")
      {
          return; // ignore
      }

      // Recurse into children
      for (xml_node<>* child = node->first_node(); child; child = child->next_sibling())
      {
          html_to_text(child, out, listLevel);
      }

      // After closing certain tags
      if (name == "p" || name == "h1" || name == "h2" || name == "h3")
      {
          out += "\n";
      }

      return;
    }
    default:
      return;
  }
}

inline std::string html_to_text(std::string const& data)
{
  using namespace rapidxml;
  xml_document<> doc;    // character type defaults to char
  char* text = const_cast<char*>(data.data());
  doc.parse<parse_non_destructive>(text);    // 0 means default parse flags

  std::string out;
  html_to_text(doc.first_node(), out);
  return out;
}
