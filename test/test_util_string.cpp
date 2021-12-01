#include "util_string.h"
#include <iostream>

int main()
{
    std::string str("abc efg hig");

    auto res1 = utility::split(str);
    for (auto s: res1) std::cout << s << std::endl;

    auto res2 = utility::split(str, "c h");
    for (auto s: res2) std::cout << s << std::endl;

    return 0;
}