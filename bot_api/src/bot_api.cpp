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
static curl_slist * header = 0;
namespace{
size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string *data) {     //For curl reply
    if(data)
        data->append((char*) ptr, size * nmemb);
    return size * nmemb;
}
}
std::string BotApi::file_downloader(std::string file_path, size_t file_size){
    
auto * curl = curl_easy_init();
    if(!curl) return nullptr;
    
    std::string offset("");
    if(update_offset != 0){                                         //It's for not to getting updates you have already got. 
        offset+= "?offset="; 
        offset+= std::to_string(update_offset);
    }



    std::string url = "https://api.telegram.org/file/bot" + BotToken + "/" + file_path;  //forming get request
    
    std::string buffer("");
  

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());            //fucking magic
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);



    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return buffer;

}
BotApi::BotApi(std::string token) : BotToken(token)
{
    header = curl_slist_append(header, "Content-Type: application/json");
}

std::vector<std::shared_ptr<Update>> BotApi::get_updates()
{
    auto * curl = curl_easy_init();
    if(!curl) return std::vector<std::shared_ptr<Update>>();
    
    std::string offset("");
    if(update_offset != 0){                                         //It's for not to getting updates you have already got. 
        offset+= "?offset="; 
        offset+= std::to_string(update_offset);
    }



    std::string url = "https://api.telegram.org/bot" + BotToken + "/getUpdates" + offset;  //forming get request
    
    std::string buffer("");
  

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());            //fucking magic
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);



    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if(buffer.empty()) return std::vector<std::shared_ptr<Update>>();
#ifdef DBG_BOT_API
    std::cout << buffer << std::endl;
#endif    
    json update = json::parse(buffer);
    
    std::vector <std::shared_ptr<Update>> result;

    if (update.find("result") != update.end() && update.find("result")->size() > 0){    //json to normal data, but still fucking magic
        result.reserve(update["result"].size());
        for (auto it : update.find("result").value()){
            std::shared_ptr<Update> up = std::make_shared<Update>();
            up->update_id = it["update_id"];
            update_offset = up->update_id+1;
            if (it.find("message") != it.end()){
                auto msg = it["message"];
                auto message = up->message = std::make_shared<struct Message>();
                message->message_id = msg["message_id"];
                message->chat_id = msg["chat"]["id"];
                if (msg.find("from") != msg.end()){
                    up->from.is_bot = msg["from"]["is_bot"];
                    up->from.user_id = msg["from"]["id"];
                    if (msg["from"].find("username") != msg["from"].end())
                        up->from.username = msg["from"]["username"];
                }
                if (msg.find("document") != msg.end()){
                    std::string mime_type= msg["document"]["mime_type"];
                    if (mime_type.find("image") == std::string::npos)
                        continue;
                    else
                        message->file->binary = msg["document"]["file_id"];
                }
                else if(msg.find("photo") != msg.end()){
                    
                    message->file = std::make_shared<File>();
                    size_t file_size = 0;
                    json photos = msg["photo"];
                    for(auto &it : photos)
                        if(it["file_size"] < 5*1024*1024  && it["file_size"] > file_size){
                            message->file->file_id = it["file_id"];
                            file_size = it["file_size"];
                        }
                    if(!file_size) continue;
                    
                }
                else if (msg.find("text") != msg.end()){
                    message->is_document = false;
                    message->text = msg["text"];
                }
                else
                    continue;
                
            }
            else if(it.find("callback_query") != it.end()){
                
                up->callback_query =  std::make_shared<CallbackQuery>();
                if(it["callback_query"].find("message") != it["callback_query"].end()){
                    auto msg = up->callback_query->message = std::make_shared<struct Message>();
                    msg->chat_id = it["callback_query"]["message"]["chat"]["id"];
                    msg->message_id = it["callback_query"]["message"]["message_id"];
                    msg->text = it["callback_query"]["message"]["text"];
                }
                up->callback_query->query_data = it["callback_query"]["data"];
                up->from.user_id = it["callback_query"]["from"]["id"];
                up->from.is_bot = false;
                
                if (it["callback_query"]["from"].find("username") != it["callback_query"]["from"].end())
                    up->from.username = it["callback_query"]["from"]["username"];
                
            }
            else
                continue;
            result.push_back(up);
        }
        
    }
    return result;
}

//this is a huge piece of shit i've written.
void BotApi::send_message(const std::string& message, 
                         size_t chat_id, 
                         size_t reply_to_message_id, 
                         std::shared_ptr<IKeyboard> keyboard, 
                         bool disable_notification,
                         const std::string& parse_mode ){

    auto curl = curl_easy_init();
    if(!curl){
        return;
    }
    
    
    json PostMessage;
    if(keyboard){
        PostMessage["reply_markup"] = keyboard->reply_markup();
          keyboard.reset();
     }
     PostMessage["chat_id"] = chat_id;
     PostMessage["text"] = message;
     PostMessage["disable_notification"] = disable_notification;
     if (reply_to_message_id)
         PostMessage["reply_to_message_id"] = reply_to_message_id;
     if (parse_mode != "")
         PostMessage["parse_mode"] = parse_mode;

     std::string PostRequest = PostMessage.dump();
#ifdef DBG_BOT_API
    std::cout << PostRequest << std::endl;
#endif
    std::string url = "https://api.telegram.org/bot" + BotToken + "/sendMessage";    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, PostRequest.size());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, PostRequest.c_str());
#ifdef DBG_BOT_API
    std::string reply;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &reply);

#endif
#ifndef DBG_BOT_API
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, 0);

#endif

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,writeFunction);


   
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    

    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
#ifdef DBG_BOT_API
    std::cout << reply << std::endl;

#endif
}

void BotApi::send_remove_keyboard(const std::string& message, size_t chat_id, size_t reply_to_message_id, bool disable_notification, const std::string& parse_mode){ //Don't fucking touch it, it just works
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

    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
}

void BotApi::edit_message_text(const std::string& message, size_t chat_id, size_t message_id, std::shared_ptr<InlineKeyboard> keyboard, const std::string& parse_mode){
     auto curl = curl_easy_init();
    if(!curl){
        return;
    }
    
    
    json PostMessage;
    if(keyboard){
        PostMessage["reply_markup"] = keyboard->reply_markup();
        keyboard.reset();
    }
    PostMessage["chat_id"] = chat_id;
    PostMessage["message_id"] = message_id;
    PostMessage["text"] = message;
    if (parse_mode != "")
         PostMessage["parse_mode"] = parse_mode;
    std::string PostRequest = PostMessage.dump();
    #ifdef DBG_BOT_API
    std::cout << PostRequest << std::endl;
#endif
    std::string url = "https://api.telegram.org/bot" + BotToken + "/editMessageText";    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, PostRequest.size());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, PostRequest.c_str());
#ifdef DBG_BOT_API
    std::string reply;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &reply);

#endif
#ifndef DBG_BOT_API
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, 0);

#endif

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,writeFunction);


   
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    

    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
#ifdef DBG_BOT_API
    std::cout << reply << std::endl;

#endif
}

void BotApi::download_document(File &file){
    auto curl = curl_easy_init();
    if(!curl){
        return;
    }
    
    
    json PostMessage;
    PostMessage["file_id"] = file.file_id;
    
    
   
    std::string PostRequest = PostMessage.dump();
#ifdef DBG_BOT_API
    std::cout << PostRequest << std::endl;
#endif
    std::string reply;
    std::string url = "https://api.telegram.org/bot" + BotToken + "/getFile";    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, PostRequest.size());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, PostRequest.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &reply);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,writeFunction);
   
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    
    curl_easy_perform(curl);
    if(reply.empty()) return;
    json jreply = json::parse(reply);
    std::string file_path = jreply["result"]["file_path"];
    file.binary = file_downloader(file_path, (size_t)jreply["result"]["file_size"]);
    

    
    curl_easy_cleanup(curl);

    
}

std::string ReplyKeyboard::reply_markup(){
    json ReplyMarkup;
    ReplyMarkup["resize_keyboard"] = true;
    ReplyMarkup["one_time_keyboard"] = one_time_keyboard;
    ReplyMarkup["keyboard"] = json::array();
    ReplyMarkup["is_persistent"] = true;
    auto &arr = ReplyMarkup["keyboard"];
    for (int y = 0; y < this->keyboard.size(); y++)
    {
        arr[y] = json::array();
        for (int x = 0; x < this->keyboard[y].size(); x++)
        {
            json button;

            button["text"] = this->keyboard[y][x];
            arr[y].push_back(button);
        }
    }
    return ReplyMarkup.dump();
}

std::string InlineKeyboard::reply_markup()
{
    json ReplyMarkup;
    ReplyMarkup["inline_keyboard"] = json::array();
    for(int y = 0; y < keyboard.size(); y++){
        ReplyMarkup["inline_keyboard"][y]= json::array();
        for (int x = 0; x < this->keyboard[y].size(); x++)
        {
            json button;
            if(this->keyboard[y][x].text.empty()) continue;
            button["text"] = this->keyboard[y][x].text;
            if(!this->keyboard[y][x].callback_data.empty()){
                button["callback_data"] = keyboard[y][x].callback_data;
            }
            else continue;
            if(!this->keyboard[y][x].url.empty()){
                button["url"] = keyboard[y][x].url;
            }
            ReplyMarkup["inline_keyboard"][y].push_back(button);
        }
    }
    return ReplyMarkup.dump();
}
