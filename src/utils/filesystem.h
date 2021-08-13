#ifndef __STD_EXT_FILESYSTEM_TOOLS_HPP__
#define __STD_EXT_FILESYSTEM_TOOLS_HPP__

#include <string>
#include <vector>
#include <filesystem>

namespace std::ext::filesystem
{
//    namespace filesystem
//    {
        std::vector<std::string> 
                          dir_files_list(const std::filesystem::path &dir_name, 
                          const std::string &filter = "");
        std::vector<std::string> 
                          dir_files_list_r(const std::filesystem::path &dir_name, 
                          const std::string &filter = "");
        std::vector<std::string> 
                          dir_dirs_list(const std::filesystem::path &dir_name, 
                          const std::string &filter = "");
        std::string find_file_up(const std::filesystem::path &from_path, 
                                const std::string &file_name);
        std::string file_get_contents(const std::filesystem::path &file_path);
//    }
}

/*
class filesystem
{
    public:
        filesystem ();
        virtual ~filesystem ();
    private:
};
*/

#endif /* end of include guard FILESYSTEM_HPP */

