#pragma once
#include <mutex>
#include <vector>
#include <fstream>
#include <exception>
#ifdef LOG_TO_CONSOLE
#include <iostream>
#endif
class LogException : std::exception{
    std::string exc_text;
    public:

    LogException(std::string exception ) : exc_text(exception){

    }
    virtual const char* what() const noexcept override {
        return exc_text.c_str();
    }
};
class Logger{
    std::ofstream log_file;
    std::mutex logger_mtx;

    public:
    Logger(std::string filename = "log.txt"){
        log_file.open(filename, std::ios::app);
        if(!log_file.is_open()){
            throw LogException("File not oppened");
        }
    }
    void add_log(std::string log);

};