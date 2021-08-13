#ifndef __DEVICE_INFO_HPP__
#define __DEVICE_INFO_HPP__

#include <vector>
#include <string>

#include <filesystem>

//#include <boost/filesystem.hpp>

//namespace fs = boost::filesystem;

class device_info
{
    public:
//        device_info ();
        explicit device_info (const std::filesystem::path &p);
        virtual ~device_info ();

        static std::vector<device_info> devices_list(const std::filesystem::path &path);

        inline bool is_virtual() const
        {
            return m_device_path.string().find("/virtual/") != std::string::npos; 
        }

        inline std::filesystem::path path() const
        {
            return m_device_path;
        }

        inline std::string device_name() const
        {
            return m_device_name;
        }

        inline int bus_num() const
        {
            return m_bus_num;
        }

        inline int dev_num() const
        {
            return m_dev_num;
        }

        inline std::string vendor_id() const
        {
/*          if(m_vendor_id.empty())
            {
                read_device_info();
            }
            */
            return m_vendor_id;
        }

        inline std::string product_id() const
        {
/*          if(m_product_id.empty())
            {
                read_device_info();
            }
  */        
            return m_product_id;
        }

        inline std::string manufacturer() const
        {
/*            if(m_manufacturer.empty())
            {
                read_device_info();
            }
  */          
            return m_manufacturer;
        }

/*        virtual inline std::filesystem::path get_device_path() const
        {
//          fs::path p(m_device_path);
//          return p.parent_path().c_str();
          return m_device_path;
        }
*/
        std::string uniq_dev_id() const;
        
/*        virtual inline const std::string get_base_bus_path() const
        {
            fs::path p(get_base_device_path());
            return p.parent_path().c_str();
        }
*/        
        void read_device_info();

    private:
        int             m_bus_num;
        int             m_dev_num;
        std::filesystem::path     m_device_path;
        std::string     m_device_name;
        std::string     m_vendor_id;
        std::string     m_product_id;
        std::string     m_manufacturer;

};

#endif /* end of include guard DEVICE_INFO_HPP */

