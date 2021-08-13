#include <fstream>
#include <iostream>
#include <sstream>

#include <regex>

#include "filesystem.h"


namespace fs = std::filesystem; 

// https://felixmorgner.com/blog/c++/2017/06/17/filtering-files-with-boost.html
// https://stackoverflow.com/questions/1257721/can-i-use-a-mask-to-iterate-files-in-a-directory-with-boost

static std::string escapeRegex(const std::string &str) {
  std::regex esc("([\\^\\.\\$\\|\\(\\)\\[\\]\\*\\+\\?\\/\\\\])");                                                         
  std::string rep("\\\\\\1");
  return regex_replace(str, esc, rep, std::regex_constants::match_default | 
                                        std::regex_constants::format_sed);
}

static std::string wildcardToRegex(const std::string &pattern) {
  std::regex esc("\\\\([\\*\\?])");
  std::string rep(".\\1");
  return regex_replace(escapeRegex(pattern), esc, rep, std::regex_constants::match_default | 
                                                        std::regex_constants::format_sed);
}


std::vector<std::string> 
std::ext::filesystem::dir_files_list(const std::filesystem::path &dir_name, 
                                const std::string &filter/* = ""*/)
{
    const  std::regex rfilter(wildcardToRegex(filter));
//    std::smatch   what;
    std::vector<std::string>	v;
    
    if(!fs::exists(dir_name))
    {
        return v;
    }
    
    try
    {
        for (auto &entry : fs::directory_iterator(dir_name))
        {
            std::string s_path = entry.path().filename().string();
            if(fs::is_regular_file(entry) &&
                std::regex_match(s_path, rfilter))
            {
                v.push_back(s_path);
            }
        }
    } 
    catch(const fs::filesystem_error &e)
    {
    }

    return v;
}

std::vector<std::string> 
std::ext::filesystem::dir_files_list_r(const std::filesystem::path &dir_name, 
                                const std::string &filter/* = ""*/)
{
    const  std::regex rfilter(wildcardToRegex(filter));
    std::smatch   what;
    std::vector<std::string>	v;
    
    if(!fs::exists(dir_name))
    {
        return v;
    }
    
    try
    {
        for (auto &entry: fs::recursive_directory_iterator(dir_name))
        {
            std::string s_path = entry.path(); //.filename().string();
            if(fs::is_regular_file(entry) &&
                std::regex_match(s_path, what, rfilter)
                )
            {
                continue;
            }
            v.push_back(s_path/*entry.path().filename().c_str()*/);
        }
    } 
    catch(const fs::filesystem_error &e)
    {
    }

    return v;
}

std::vector<std::string> 
std::ext::filesystem::dir_dirs_list(const std::filesystem::path &dir_name, 
                                const std::string &filter/* = ""*/)
{
    const  std::regex rfilter(filter);
//    std::smatch   what;
    std::vector<std::string>	v;
    
    if(!fs::exists(dir_name))
    {
        return v;
    }
    
    try
    {
        for (auto &entry: fs::directory_iterator(dir_name))
        {
            std::string s_path = entry.path();
            if(fs::is_directory(entry) &&
                std::regex_match(s_path, /*what,*/ rfilter)
                )
            {
                v.push_back(s_path);
            }
        }
    } 
    catch(const fs::filesystem_error &e)
    {
    }

    return v;
}

std::string std::ext::filesystem::find_file_up(const std::filesystem::path &from_path, 
                                                const std::string &file_name)
{
    std::string res;
    fs::path base_p(from_path);
    fs::path p;

    while(!base_p.empty())
    {
        p = base_p;
        p.append(file_name);
        if(fs::exists(p))
        {
            return p.c_str();
        }
	
	if(base_p == base_p.parent_path())
	{
	    break;
	}    
	fs::path p2(base_p);
        base_p = p2.parent_path();
    }

    return "";
}

std::string 
std::ext::filesystem::file_get_contents(const std::filesystem::path &file_path)
{
    if(file_path.empty())
    {
        return "";
    }
    std::ifstream f(file_path); //taking file as inputstream
    std::string str;
    if(f) {
        std::ostringstream ss;
        ss << f.rdbuf(); // reading data
        str = ss.str();
    }

    return str;
}
