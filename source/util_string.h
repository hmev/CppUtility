#include <string>
#include <vector>
#include <regex>

namespace util
{
    std::vector<std::string> split(const std::string& str, const std::string splits = " ")
    {
        std::vector<std::string> result;

        std::regex reg("[^"+splits+"]+");
        std::smatch matched;
        if (std::regex_match(str, matched, reg))
        {
            for (auto m: matched) result.push_back(m);
        }
        else 
        {
            result.push_back(str);
        }

        return std::move(result);
    }

    std::string join(const std::vector<std::string>& str, const char joiner)
    {
        std::string result = *(str.begin());
        for (auto iter = str.begin() + 1; iter != str.end(); iter++)
        {
            result += joiner + (*iter);
        }
        return result;
    }
}