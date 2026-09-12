#pragma once
#include <string>
#include <string_view>
#include <unordered_map>

inline std::wstring decode_escape(std::wstring_view esc)
{
    static const std::unordered_map<std::wstring_view, std::wstring_view> map = {
        {L"nbsp",  L" "},
        {L"amp",   L"&"},
        {L"lt",    L"<"},
        {L"gt",    L">"},
        {L"quot",  L"\""},
        {L"apos",  L"'"},
    };

    if (auto it = map.find(esc); it != map.end())
        return std::wstring(it->second);

    // numeric escapes: &#123; or &#x1F;
    if (!esc.empty() && esc[0] == L'#') {
        bool hex = (esc.size() > 1 && (esc[1] == L'x' || esc[1] == L'X'));
        try {
            unsigned long code =
                hex ? std::stoul(std::wstring(esc.substr(2)), nullptr, 16)
                    : std::stoul(std::wstring(esc.substr(1)), nullptr, 10);
            return std::wstring(1, static_cast<wchar_t>(code));
        } catch (...) {}
    }

    // unknown escape → return literally "&xxx;"
    return L"&" + std::wstring(esc) + L";";
}

inline std::wstring html_to_text(std::wstring_view html)
{
    std::wstring out;
    int listLevel = 0;

    auto emit_tag = [&](std::wstring_view tag) {
        if (tag == L"p") {
            out += L"\n";
        } else if (tag == L"br") {
            out += L"\n";
        } else if (tag == L"li") {
            out += std::wstring(listLevel * 2, L' ') + L"- ";
        } else if (tag == L"ul" || tag == L"ol") {
            listLevel++;
        } else if (tag == L"h1" || tag == L"h2" || tag == L"h3") {
            out += L"\n";
        }
    };

    auto emit_close_tag = [&](std::wstring_view tag) {
        if (tag == L"p" || tag == L"h1" || tag == L"h2" || tag == L"h3") {
            out += L"\n";
        } else if (tag == L"ul" || tag == L"ol") {
            listLevel = std::max(0, listLevel - 1);
        }
    };

    size_t i = 0;
    while (i < html.size()) {

        // ---------- ESCAPES ----------
        if (html[i] == L'&') {
            size_t semi = html.find(L';', i + 1);
            if (semi != std::wstring_view::npos) {
                std::wstring_view esc = html.substr(i + 1, semi - (i + 1));
                out += decode_escape(esc);
                i = semi + 1;
                continue;
            }
        }

        // ---------- TAGS ----------
        if (html[i] == L'<') {
            size_t j = html.find(L'>', i + 1);
            if (j == std::wstring_view::npos) break;

            std::wstring_view tag = html.substr(i + 1, j - (i + 1));

            bool closing = false;
            if (!tag.empty() && tag[0] == L'/') {
                closing = true;
                tag.remove_prefix(1);
            }

            // strip attributes
            size_t sp = tag.find(L' ');
            if (sp != std::wstring_view::npos)
                tag = tag.substr(0, sp);

            // ignore script/style blocks
            if (tag == L"script" || tag == L"style") {
                std::wstring closeTag = L"</" + std::wstring(tag) + L">";
                size_t end = html.find(closeTag, j + 1);
                if (end == std::wstring_view::npos) return out;
                i = end + closeTag.size();
                continue;
            }

            if (!closing)
                emit_tag(tag);
            else
                emit_close_tag(tag);

            i = j + 1;
            continue;
        }

        // ---------- TEXT ----------
        out += html[i];
        ++i;
    }

    return out;
}
