
#ifdef _WIN32
    #include <windows.h>
#else
    #include <iconv.h>
    #include <errno.h>
#endif
  #include <string>

std::wstring to_wstring(const std::string& s)
{
#ifdef _WIN32
    int size_needed = MultiByteToWideChar(CP_UTF8, 0,
                                          s.data(), (int)s.size(),
                                          nullptr, 0);

    std::wstring result(size_needed, 0);

    MultiByteToWideChar(CP_UTF8, 0,
                        s.data(), (int)s.size(),
                        result.data(), size_needed);

    return result;

#else
    // Linux / Unix: convert UTF‑8 → UTF‑32 using iconv
    iconv_t cd = iconv_open("WCHAR_T", "UTF-8");
    if (cd == (iconv_t)-1)
        return L""; // conversion not available

    size_t in_bytes = s.size();
    size_t out_bytes = (s.size() + 1) * sizeof(wchar_t);

    std::wstring result;
    result.resize(s.size() + 1);

    char* in_buf = const_cast<char*>(s.data());
    char* out_buf = reinterpret_cast<char*>(result.data());

    if (iconv(cd, &in_buf, &in_bytes, &out_buf, &out_bytes) == (size_t)-1)
    {
        iconv_close(cd);
        return L"";
    }

    iconv_close(cd);
    return result;
#endif
}
