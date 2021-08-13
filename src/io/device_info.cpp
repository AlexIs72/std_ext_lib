#include <iostream>
#include <sstream>
//#include <boost/filesystem.hpp>

#include <utils/filesystem.h>
#include <utils/string.h>

#include "device_info.h"

//namespace fs = boost::filesystem;
/*
device_info::device_info()
{
    
}
*/

device_info::device_info(const std::filesystem::path &p) :
    m_bus_num(-1),
    m_dev_num(-1),
    m_device_path(p)
{
    // TODO ned to use std::filesystem::path features
    std::vector<std::string> items = std::ext::string::split(p.c_str(), '/');
    if(items.size())
    {
        m_device_name = items.back();
    }
}

device_info::~device_info()
{
}

// cppcheck-suppress unusedFunction
std::vector<device_info> device_info::devices_list(const std::filesystem::path &path)
{
//    std::filesystem::directory_iterator end_iter;
    std::vector<device_info>    list;    

/*    for ( std::filesystem::directory_iterator dir_itr( path );
          dir_itr != end_iter;
          ++dir_itr )
*/
    for(auto& entry: std::filesystem::directory_iterator(path))
    {
//        std::cout << dir_itr->path()/*.filename()*/ << " = " << fs::read_symlink(dir_itr->path());
//            fs::is_directory( dir_itr->status()) << std::endl;
//        if(fs::is_directory( dir_itr->status()))
//        {
//            std::string path = dir_itr->path().c_str();
	    std::filesystem::path p = entry.path();
//std::cout << p.c_str() << std::endl;
//            std::string symlink = std::filesystem::read_symlink(p).c_str();
            if(!std::filesystem::read_symlink(p).empty())
            {
//                path = std::filesystem::/*read_symlink*/canonical(p).c_str();
		p = std::filesystem::/*read_symlink*/canonical(p);
            }
            device_info di(p);
            list.push_back(di);  
//            std::cout << path << std::endl;
//        }

//        std::cout << dir_itr->path()/*.filename()*/ << std::endl;                                    
    }  

    return list;
}

// cppcheck-suppress unusedFunction
void device_info::read_device_info()
{
//    std::string device_path = get_device_path();
  //std::string bus_path = get_base_bus_path();

//std::cout << "device_path = " << m_device_path.c_str() << std::endl;

    std::string tmp = std::ext::filesystem::find_file_up(m_device_path, "busnum");
    if(!tmp.empty())
    {
        m_bus_num = std::stoi(std::ext::filesystem::file_get_contents(tmp));
    }

    tmp = std::ext::filesystem::find_file_up(m_device_path, "devnum");
    if(!tmp.empty())
    {
        m_dev_num = std::stoi(std::ext::filesystem::file_get_contents(tmp));
    }

    std::string product_id_file = std::ext::filesystem::find_file_up(m_device_path, "idProduct");
    m_product_id = std::ext::string::rtrim(std::ext::filesystem::file_get_contents(product_id_file));
    //std::cout << "id_product_file = " << id_product_file << std::endl;
//    std::cout << "content = " << std::ext::filesystem::file_get_contents(id_product_file) << std::endl;

    std::string vendor_id_file = std::ext::filesystem::find_file_up(m_device_path, "idVendor");
    m_vendor_id = std::ext::string::rtrim(std::ext::filesystem::file_get_contents(vendor_id_file));
//    std::cout << "id_vendor_file = " << id_vendor_file << std::endl;
//    std::cout << "content = " << std::ext::filesystem::file_get_contents(id_vendor_file) << std::endl;

    std::string manufacturer_file = std::ext::filesystem::find_file_up(m_device_path, "manufacturer");
    m_manufacturer = std::ext::string::rtrim(std::ext::filesystem::file_get_contents(manufacturer_file));
//    std::cout << "manufacturer_file = " << manufacturer_file << std::endl;
//    std::cout << "content = " << std::ext::filesystem::file_get_contents(manufacturer_file) << std::endl;

//    std::cout << "bus_path = " << bus_path << std::endl;
/*
    std::ifstream uevent_file(device_path + "/uevent");
    if(uevent_file.good())
    {
        std::string line;
        while(uevent_file)
        {
            std::getline(uevent_file, line);
            if(!line.empty())
            {
                std::vector<std::string> param = std::ext::string::split(line, '=');
                if(param.size() && param[0] == "PRODUCT")
                {
//                    std::cout << line << std::endl;
                    std::vector<std::string> items = std::ext::string::split(param[1], '/');
                    if(items.size())
                    {
                        m_vendor_id = items[0];
                        m_product_id = items[1];
                    }
                }
            }
        }
    }
*/
/*    std::ifstream mnf_file(bus_path + "/manufacturer");
    if(mnf_file.good())
    {
        std::string line;
        while(mnf_file)
        {
            std::getline(mnf_file, m_manufacturer);
//          :cout << line << std::endl;
        }
    }
*/
}

// cppcheck-suppress unusedFunction
std::string device_info::uniq_dev_id() const
{
    std::stringstream ss;
    
    if(m_bus_num > 0)
    {
        ss << m_bus_num << ":";
    }

    if(m_dev_num > 0)
    {
        ss << m_dev_num << ":";
    }
    
    ss << m_vendor_id << ":" << m_product_id ;
    
    return ss.str();
}
