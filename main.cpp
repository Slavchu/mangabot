#include "bot.hpp"
#include "telegraph.hpp"
int main(){
    auto telegraph =  telegraph::TelegraphApi(telegraph::TelegraphApi::create_account("fucking name", "fucking autor"));
    std::cout << telegraph.create_page("somepage" , {telegraph.upload_image("tmp/pic.jpg"),telegraph.upload_image("tmp/pic.jpg")}, "FUCK YOU") << std::endl;
    mangabot::Mangabot();

    return 0;
}