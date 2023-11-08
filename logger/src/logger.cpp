#include "../include/logger.hpp"
#include <sstream>
void Logger::add_log(std::string log){
    logger_mtx.lock();
    log_file << log << std::endl;    
#ifdef LOG_TO_CONSOLE
    std::cout << log << std::endl;
#endif
    logger_mtx.unlock();
}
