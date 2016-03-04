#ifndef MHNETWORK_H
#define MHNETWORK_H

#include "boost/asio.hpp"
#include "../control/rwhandler.h"

#include <thread>

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

using connect_fun = std::function<void(boost::system::error_code)>;

//客户端网络链接
class MHNetwork
{
public:
    MHNetwork();

    bool start(){
        try{
            boost::system::error_code err;
            client->get_socket().connect(srv_addr, err);

            //连接成功回调
            _connect_callback(err);

            if(!err){
                //连接成功
                connect_ok = true;
                client->handle_read();
            }
            else{
                connect_ok = false;
            }


        }
        catch(boost::system::system_error &err){
            qDebug() << "connect error";
        }
        catch(...){
            qDebug() << "unknow exception";
        }

        return connect_ok;
    }

    void set_server(std::string ip, short port){
        srv_addr = tcp::endpoint(address::from_string(ip), port);
    }

    void set_process_callback(read_fun callback){
        client->set_process_callback(callback);
    }

    void send(int cmdid, char* buf, int len){
        TCP_HEADER header;
        header.pack_len = sizeof(TCP_HEADER) + len;
        header.body_len = len;
        header.cmd_id = cmdid;
        header.ver = 1;

        client->send((char*)&header, sizeof(TCP_HEADER));
        client->send(buf, len);
    }

    //
    void set_connect_callback(connect_fun callback){
        _connect_callback = callback;
    }

private:

    connect_fun _connect_callback;

    bool connect_ok = false;
    io_service ios;
    std::shared_ptr<RWHandler> client;
    tcp::endpoint srv_addr;
};

#endif // MHNETWORK_H
