#ifndef MHNETWORK_H
#define MHNETWORK_H

#include "boost/asio.hpp"
#include "../control/rwhandler.h"

#include <thread>

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

using connect_fun = std::function<void(int, boost::system::error_code)>;

//客户端网络链接
class MHNetwork
{
public:
    MHNetwork();

    void stop();
    void start();

    void set_server(std::string ip, short port);

    void set_process_callback(read_fun callback);
    void set_error_callback(error_fun callback);

    void send(int cmdid, const char* buf, int len);

    //
    void set_connect_callback(connect_fun callback);

private:
    std::thread* run_thread;
    connect_fun _connect_callback;
    io_service ios;
    std::shared_ptr<RWHandler> client;
    tcp::endpoint srv_addr;
};

#endif // MHNETWORK_H
