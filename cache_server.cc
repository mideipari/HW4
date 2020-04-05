#include <iostream>
#include "cache.hh"
#include "evictor.hh"
#include "fifo_evictor.hh"
#include "lru_evictor.hh"
#include <algorithm>
#include <cstring>
#include <set>
#include <string>

#include <cpprest/http_listener.h>
#include <cpprest/json.h>


using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace std;

//command line arguments code taken from https://stackoverflow.com/questions/865668/how-to-parse-command-line-arguments-in-c
char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}

FifoEvictor *Fifo = new FifoEvictor();
Cache *c;


class CommandHandler
{
public:
    CommandHandler() {}
    CommandHandler(utility::string_t url);
    pplx::task<void> open() { return m_listener.open(); }
    pplx::task<void> close() { return m_listener.close(); }
private:
    void handle_get(http_request message);
    void handle_post(http_request message);
    void handle_put(http_request message);
    void handle_del(http_request message);
    void handle_head(http_request message);
    http_listener m_listener;
};

CommandHandler::CommandHandler(utility::string_t url) : m_listener(url)
{
    m_listener.support(methods::GET, std::bind(&CommandHandler::handle_get, this, std::placeholders::_1));
    m_listener.support(methods::POST, std::bind(&CommandHandler::handle_post, this, std::placeholders::_1));
    m_listener.support(methods::PUT, std::bind(&CommandHandler::handle_put, this, std::placeholders::_1));
    m_listener.support(methods::DEL, std::bind(&CommandHandler::handle_del, this, std::placeholders::_1));
    m_listener.support(methods::HEAD, std::bind(&CommandHandler::handle_del, this, std::placeholders::_1));
}


void CommandHandler::handle_get(http_request message){
    string key = http::uri::decode(message.relative_uri().path());
    key = key.substr(1, key.length());
    Cache::size_type size;
    string value;
    if (c->get(key, size) != nullptr) {
        value = c->get(key, size);
        string output = "{ key: " + key + ", value: " + value + " }";
        message.reply(status_codes::OK, output);
    }
    else {
        message.reply(status_codes::NoContent, "NOT FOUND");
    }
}

void CommandHandler::handle_post(http_request message){
    string key = http::uri::decode(message.relative_uri().path());
    key = key.substr(1, key.length());
    Cache::size_type size;
    string value;
    if (key.compare("reset") == 0) {
        c->reset();
        message.reply(status_codes::OK, "CACHE RESET");
    }
    else {
        message.reply(status_codes::BadRequest, "INVALID URL");
    }
}

void CommandHandler::handle_head(http_request message){
        message.headers().add("Space-Used: ", to_string(c->space_used()));
        message.headers().add("Accept: ", "html");
        message.headers().add("HTTP Version: ", "1.1");
        message.headers().add("Content-Type", "application/json");
        //Can't figure out how to return these headers
}

void CommandHandler::handle_put(http_request message){
    string url = http::uri::decode(message.relative_uri().path());
    std::string delimiter = "/";
    string key = url.substr(1, url.find(delimiter, 1)-1);
    string value = url.substr(url.find(delimiter, 1)+1, url.length());
    Cache::val_type valuei = value.c_str();
    Cache::size_type size;
    c->set(key, valuei, sizeof(valuei));
    string output = "Key: " + key + " with value: " + value + " inserted.";
    message.reply(status_codes::OK, output);
}

void CommandHandler::handle_del(http_request message){
    string key = http::uri::decode(message.relative_uri().path());
    key = key.substr(1, key.length());
    Cache::size_type size;
    string value;
    if (c->get(key, size) != nullptr) {
        c->del(key);
        string output = "Key: " + key + " deleted.";
        message.reply(status_codes::OK, output);
    }
    else {
        message.reply(status_codes::NoContent, "NOT FOUND");
    }
}

int main(int argc, char * argv[]) {


    char * maxmemstring = getCmdOption(argv, argv + argc, "-m");
    char * serverstring = getCmdOption(argv, argv + argc, "-s");
    char * portstring = getCmdOption(argv, argv + argc, "-p");
    char * threadsstring = getCmdOption(argv, argv + argc, "-t");

    if (strcmp(maxmemstring, "0")){
        maxmemstring = "100";
    }
    if(strcmp(serverstring, "0")){
        serverstring = "http://127.0.0.1";
    }
    if(strcmp(portstring, "0")){
        portstring = "5555";
    }
    int maxmem = atoi(maxmemstring);
    int port = atoi(portstring);
    string server(serverstring);
    string sport(portstring);
    server = server + ":" + portstring;

    FifoEvictor *Fifo = new FifoEvictor();
    c = new Cache(maxmem, 0.75, Fifo);
    string address(serverstring);

    string ports(portstring);
    address += ":" + ports;
    try
    {
        uri_builder uri(address);
        auto addr = uri.to_uri().to_string();
        CommandHandler handler(addr);
        handler.open().wait();
        ucout << utility::string_t(U("Listening for requests at: ")) << addr << std::endl;
        ucout << U("Press ENTER key to quit...") << std::endl;
        std::string line;
        std::getline(std::cin, line);
        handler.close().wait();
    }
    catch (std::exception& ex)
    {
        ucout << U("Exception: ") << ex.what() << std::endl;
        ucout << U("Press ENTER key to quit...") << std::endl;
        std::string line;
        std::getline(std::cin, line);
    }

    return 0;
}