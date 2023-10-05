#include "bot.hpp"
#include "telegraph.hpp"


int main(){
    auto bot = mangabot::Mangabot();
    bot.start_bot();
    return 0;
}