/*




            HEY, STOP!!!!!!!!!!!!!!
            SO FUCKING BAD CODE HERE
            I just want to save your eyes
            Don't go here, it just works








*/
#include <bot_api.hpp>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#ifdef DBG_BOT_API
#include <iostream>
#endif
using json = nlohmann::json;

size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string* data) {     //For curl reply
    if(data)
        data->append((char*) ptr, size * nmemb);
    return size * nmemb;
}



std::vector <Update> BotApi::get_updates(){
    auto * curl = curl_easy_init();
    if(!curl) return std::vector<Update>();
    
    std::string offset = "";
    if(update_offset != 0){                                         //It's for not to getting updates you have already got. 
        offset+= "?offset="; 
        offset+= std::to_string(update_offset);
    }



    std::string url = "https://api.telegram.org/bot" + BotToken + "/getUpdates" + offset;  //forming get request
    
    std::string buffer;
  

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());            //fucking magic
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);



    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    json update = json::parse(buffer);
    
    std::vector <Update> result;

    if (update.find("result") != update.end() && update.find("result")->size() > 0){    //json to normal data, but still fucking magic
        for (auto &it : update.find("result").value()){
            Update up;
            up.update_id = it["update_id"];
            update_offset = up.update_id+1;
            if (it.find("message") != it.end()){
                auto msg = it["message"];
                struct Message message;
                message.message_id = msg["message_id"];
                message.chat_id = msg["chat"]["id"];
                if (msg.find("from") != msg.end()){
                    message.from.is_bot = msg["from"]["is_bot"];
                    message.from.user_id = msg["from"]["id"];
                    if (msg["from"].find("username") != msg["from"].end())
                        message.from.username = msg["from"]["username"];
                }
                if (msg.find("document") != msg.end()){
                    message.is_document = true;

                    if (msg["document"]["mime_type"] != "application/pdf")
                        continue;
                    else
                        message.text = msg["document"]["file_id"];
                }
                else if (msg.find("text") != msg.end()){
                    message.is_document = false;
                    message.text = msg["text"];
                }
                else
                    continue;
                up.message = message;
            }
            else
                continue;
            result.push_back(up);
        }
        
    }
    return result;
}

//this is a huge piece of shit i've written.
void BotApi::send_message(std::string message, size_t chat_id, size_t reply_to_message_id,std::vector<std::vector<std::string>> buttons, bool one_time_keyboard ,bool disable_notification, std::string parse_mode){

    auto curl = curl_easy_init();
    if(!curl){
        return;
    }
    
    
    json PostMessage;
     if(buttons.size()){
        json ReplyMarkup;
        ReplyMarkup["resize_keyboard"] = true;
        ReplyMarkup["one_time_keyboard"] = one_time_keyboard;
        ReplyMarkup["keyboard"] = json::array();
        ReplyMarkup["is_persistent"] = true;
        auto &arr =ReplyMarkup["keyboard"];
        for(int y = 0; y < buttons.size(); y++){
            arr[y] = json::array();
            for(int x = 0; x < buttons[y].size(); x++ ){
                json keyboard;
                
                keyboard["text"] = buttons[y][x];
                arr[y].push_back(keyboard);
            }
        }
#ifdef DBG_BOT_API
        std::cout << ReplyMarkup.dump() << std::endl;
#endif
        PostMessage["reply_markup"] = ReplyMarkup.dump(); 

        
    }
    PostMessage["chat_id"] = chat_id;
    PostMessage["text"] = message;
    PostMessage["disable_notification"] = disable_notification;
    if(reply_to_message_id) PostMessage["reply_to_message_id"] = reply_to_message_id;
    if (parse_mode != "") PostMessage["parse_mode"] = parse_mode;
   
    std::string PostRequest = PostMessage.dump();
#ifdef DBG_BOT_API
    std::cout << PostRequest << std::endl;
#endif
    std::string url = "https://api.telegram.org/bot" + BotToken + "/sendMessage";    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, PostRequest.size());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, PostRequest.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,writeFunction);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, 0);

    curl_slist * header = 0;
    header = curl_slist_append(header, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);


    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
}

void BotApi::send_remove_keyboard(std::string message, size_t chat_id, size_t reply_to_message_id, bool disable_notification, std::string parse_mode){ //Don't fucking touch it, it just works
    auto curl = curl_easy_init();
    if(!curl){
        return;
    }
    
    
    json PostMessage;
     
    json ReplyMarkup;
    ReplyMarkup["remove_keyboard"] = true; 
    PostMessage["reply_markup"] = ReplyMarkup.dump(); 

        
    
    PostMessage["chat_id"] = chat_id;
    PostMessage["text"] = message;
    PostMessage["disable_notification"] = disable_notification;
    if(reply_to_message_id) PostMessage["reply_to_message_id"] = reply_to_message_id;
    if (parse_mode != "") PostMessage["parse_mode"] = parse_mode;
   
    std::string PostRequest = PostMessage.dump();
#ifdef DBG_BOT_API
    std::cout << PostRequest << std::endl;
#endif
    std::string url = "https://api.telegram.org/bot" + BotToken + "/sendMessage";    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, PostRequest.size());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, PostRequest.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, 0);

    curl_slist * header = 0;
    header = curl_slist_append(header, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
}

void BotApi::forward_message(size_t chat_id, size_t from_chat_id, size_t message_id){
    auto curl = curl_easy_init();
    if(!curl){
        return;
    }
    
    
    json PostMessage;
    PostMessage["chat_id"] = chat_id;
    PostMessage["from_chat_id"] = from_chat_id;
    PostMessage["message_id"] = message_id; 
    PostMessage["protect_content"] = "true";
    std::string PostRequest = PostMessage.dump();
#ifdef DBG_BOT_API 
    std::cout << PostRequest;
#endif
    std::string url = "https://api.telegram.org/bot" + BotToken + "/copyMessage";    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, PostRequest.size());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, PostRequest.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, 0);

    curl_slist * header = 0;
    header = curl_slist_append(header, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
}