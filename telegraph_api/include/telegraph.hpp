#pragma once
#include <string>
#include <vector>
namespace telegraph{
    class TelegraphApi{
        std::string token;
        public:
        TelegraphApi(std::string token);
        
        std::string upload_image(std::string binary);
        std::string create_page(std::string title, std::vector<std::string> images, std::string text, std::string author_url = "", std::string author_name = "");
        //size_t get_views(std::string path);
        static std::string create_account(std::string short_name, std::string author_name = "", std::string author_url = "");
    };
    

}