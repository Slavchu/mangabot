#include <telegraph.hpp>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
using json = nlohmann::json;
using namespace telegraph;


size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string* data) {
    data->append((char*) ptr, size * nmemb);
    return size * nmemb;
}
void string_space_cleaner(std::string &str){
    for(int i = 0; i < str.size(); i++){
        if(str[i] == ' ') str[i]='+';
        else if(str[i] == '+') {str.insert(str.begin()+i-1, '\\'); i++;}
    }
}
telegraph::TelegraphApi::TelegraphApi(std::string token){
    this->token = token;

}

std::string telegraph::TelegraphApi::upload_image(std::string bin)
{
    if(bin.empty()) return std::string();
    std::string result;
    
    // ##

    CURL *curl;
    CURLcode res;

    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;
    struct curl_slist *headerlist = NULL;
    static const char buf[] = "Expect:";

    curl_global_init(CURL_GLOBAL_ALL);

    // set up the header
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "cache-control:",
                 CURLFORM_COPYCONTENTS, "no-cache",
                 CURLFORM_END);

    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "content-type:",
                 CURLFORM_COPYCONTENTS, "multipart/form-data",
                 CURLFORM_END);

    curl_formadd(&formpost, &lastptr,
                 CURLFORM_COPYNAME, "file", 
                 CURLFORM_BUFFER, "data",
                 CURLFORM_BUFFERPTR, bin.data(),
                 CURLFORM_BUFFERLENGTH, bin.size(),
                 CURLFORM_END);

    curl = curl_easy_init();

    headerlist = curl_slist_append(headerlist, buf);
    if (curl)
    {

        curl_easy_setopt(curl, CURLOPT_URL, "https://telegra.ph/upload");

        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);

        curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        
        return result;
    }
    curl_formfree(formpost);
    curl_slist_free_all(headerlist);
    return result;
}

std::string telegraph::TelegraphApi::create_page(std::string title, std::vector<std::string> images, std::string text, std::string author_url, std::string author_name){
    std::string url = "https://api.telegra.ph/createPage?";
    url+="access_token="+token;
    string_space_cleaner(title);
    url+="&title=" + title;
    if(author_name.size()){
        string_space_cleaner(author_name);
        url+="&author_name"+author_name;
        
    }
    if(author_url.size()){
        string_space_cleaner(author_url);
        url+="&author_url="+author_url;
    }
    json content = json::array();
    string_space_cleaner(text);
    json jtext = {{"tag", "p"}, {"children", json::array({text})}};
    content.push_back(jtext);
    if(images.size()){
        for(int i = 0; i < images.size(); i++){
            json img;
            img["tag"] = "figure";
            img["children"] = json::array();
            images[i].erase(0,1);
            images[i].erase(images[i].end()-1);

            images[i] = json::parse(images[i])["src"]; 
           
            json image;
            image["tag"]="img";
            image["attrs"]["src"] = images[i];
            img["children"].push_back(image);

            content.push_back(img);
        
        }
        std::string scontent = content.dump();
        for(int j = 0; j < scontent.size(); j++){
                if(scontent[j] == '/'){
                    scontent.insert( j, "\\");
                    j+=2;
                }
        }

        url+= "&content="+scontent;
        url+="&return_content=true";
        
    }
    auto curl = curl_easy_init();

    if(curl){
        std::string reply;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &reply);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        json jreply = json::parse(reply);
        if(jreply["ok"] == true){
            return jreply["result"]["url"];
        }
    }
    return std::string();
}

std::string telegraph::TelegraphApi::create_account(std::string short_name, std::string author_name, std::string author_url){
    auto curl = curl_easy_init();
    if(curl){
        std::string reply;
        for(int i = 0; i < short_name.size(); i++){
            if(short_name[i] == ' '){
                short_name[i] = '+';
            }
        }
        std::string request = "short_name=" + short_name;
        if(!author_name.empty()){
            for(int i = 0; i < author_name.size(); i++){
                if(author_name[i] == ' '){
                    author_name[i] = '+';
                }
            }
            request+= "&author_name=" + author_name;
        }
        if(!author_url.empty())
            request+="&author_url="+author_url;
        curl_easy_setopt(curl, CURLOPT_URL, std::string("https://api.telegra.ph/createAccount?" + request).c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &reply);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if(reply.empty()) return reply;
        json reply_body = json::parse(reply);
        if(reply_body["ok"] == true){

            auto &result= reply_body["result"];
            return result["access_token"];
        }

    }
    return std::string();
}

