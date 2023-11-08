#include "bot.hpp"
#include "telegraph.hpp"


int main(){
    CategoryUser usr(0);
    usr.get_manga(1, ECategory::ALREADY_READ);
    auto bot = mangabot::Mangabot();
    bot.start_bot();
    return 0;
}