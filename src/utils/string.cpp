#include <sstream>
#include <algorithm>

#include "string.h"

bool std::ext::string::starts_with(const std::string &s1, const std::string &s2)
{
    return s1.rfind(s2, 0) != std::string::npos;
}

bool starts_with(const std::string &s, const std::vector<std::string> &v)
{
    for(const std::string &item : v)
    {
        if(std::ext::string::starts_with(s, item))
        {
            return true;
        }
    }
    return false;
}

bool std::ext::string::starts_with(const char *s, const char *a[], size_t size)
{
    for(size_t i=0; i<size; i++)
    {
        if(std::ext::string::starts_with(s, a[i]))
        {
            return true;
        }
    }

    return false;
}

// cppcheck-suppress unusedFunction
std::vector<std::string> std::ext::string::split(const std::string &s, char delim)
{
    std::stringstream ss(s.data());
    std::string token;
    std::vector<std::string> v;

    while(std::getline(ss, token, delim)) {
        v.push_back(token);
    }

    return v;
}

std::string std::ext::string::ltrim(const std::string& str, 
                                    const std::string& chars /*= "\t\n\v\f\r "*/)
{
    return std::string(str).erase(0, str.find_first_not_of(chars));
//    return ;
}
 
std::string std::ext::string::rtrim(const std::string& str, 
                                    const std::string& chars /*= "\t\n\v\f\r "*/)
{
    return std::string(str).erase(str.find_last_not_of(chars) + 1);
//    return str;
}

// cppcheck-suppress unusedFunction 
std::string std::ext::string::trim(const std::string& str, 
                                    const std::string& chars /*= "\t\n\v\f\r "*/)
{
    return ltrim(rtrim(str, chars), chars);
}

void std::ext::string::toupper(char* str)
{ 
//    int i=0;
//    while (*str)
    for(int i=0; *(str+i)!=0; i++)
    {
        *(str+i) = ::toupper(*(str+i));
    }
}

void std::ext::string::toupper(std::string &s)
{
//    std::string cmd = "Hello World";
    for_each(s.begin(), s.end(), [](char& in){ in = ::toupper(in); });
}

// cppcheck-suppress unusedFunction
std::string std::ext::string::toupper_copy(const std::string &s)
{
    std::string copy_s = s;
    toupper(copy_s);
    return copy_s;
}


/*
string::string()
{
    
}

string::~string()
{
}
*/
