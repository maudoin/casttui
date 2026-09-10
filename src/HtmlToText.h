#pragma once

#include <RapidXml/rapidxml.hpp>

#include <string>

inline void html_to_text(rapidxml::xml_node<wchar_t>* node, std::wstring& out, int listLevel = 0)
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
      std::wstring name = node->name();

      if (name == L"p")
      {
          out += L"\n\n";  // paragraph break
      }
      else if (name == L"br")
      {
          out += L"\n";
      }
      else if (name == L"li")
      {
          out += std::wstring(listLevel * 2, ' ') + L"- ";
      }
      else if (name == L"ul" || name == L"ol")
      {
          listLevel++;
      }
      else if (name == L"h1" || name == L"h2" || name == L"h3")
      {
          out += L"\n";
      }
      else if (name == L"script" || name == L"style")
      {
          return; // ignore
      }

      // Recurse into children
      for (xml_node<wchar_t>* child = node->first_node(); child; child = child->next_sibling())
      {
          html_to_text(child, out, listLevel);
      }

      // After closing certain tags
      if (name == L"p" || name == L"h1" || name == L"h2" || name == L"h3")
      {
          out += L"\n";
      }

      return;
    }
    default:
      return;
  }
}

inline std::wstring html_to_text(std::wstring const& data)
{
  using namespace rapidxml;
  xml_document<wchar_t> doc;    // character type defaults to char
  wchar_t* text = const_cast<wchar_t*>(data.data());
  try
  {
    doc.parse<parse_non_destructive>(text);    // 0 means default parse flags
  }
  catch(std::exception const&)
  {
  }

  std::wstring out;
  html_to_text(doc.first_node(), out);
  return out;
}
