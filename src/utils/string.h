#ifndef __STD_EXT_STRING_TOOLS_HPP__
#define __STD_EXT_STRING_TOOLS_HPP__

#include <vector>
#include <string>
//#include <string>

namespace std::ext
{
    namespace string 
    {
        const std::string trim_defaults = "\t\n\v\f\r ";
        bool starts_with(const char *s, const char *a[], size_t size);
        bool starts_with(const std::string &s, const std::vector<std::string> &v);
        bool starts_with(const std::string &s1, const std::string &s2);
        std::vector<std::string> split(const std::string &s, char delim);
        std::string ltrim(const std::string& str, 
                                const std::string& chars = trim_defaults);
        std::string rtrim(const std::string& str, 
                                const std::string& chars = trim_defaults);
        std::string trim(const std::string& str, 
                                const std::string& chars = trim_defaults);
        void toupper(char* str);
        void toupper(std::string &s);
        std::string toupper_copy(const std::string &s);
    }
}
/*
class string
{
    public:
        string ();
        virtual ~string ();
    private:
};
*/
#endif /* end of include guard STRING_HPP */

