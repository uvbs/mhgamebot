#include "mhnetwork.h"

MHNetwork::MHNetwork()
{
    client = std::make_shared<RWHandler>(ios);
    client->set_id(0);
    run_thread = nullptr;
}

void MHNetwork::send(int cmdid, const char* buf, int len)
{
    TCP_HEADER header;
    header.body_len = len;
    header.cmd_id = cmdid;

    client->send((char*)&header, sizeof(header));
    client->send(buf, len);
}

void MHNetwork::stop(){
    ios.stop();
    client->close_socket();
}

void MHNetwork::start()
{
    boost::system::error_code err;
    client->get_socket().connect(srv_addr, err);

    //连接回调
    _connect_callback(client->get_id(), err);


    client->handle_read();

    //ios线程
    if(run_thread == nullptr)
    {
        run_thread = new std::thread([this](){
            ios.run();
            qDebug() << "ios thread exit";
            run_thread = nullptr;
        });
    }
}

void MHNetwork::set_server(std::string ip, short port){
    srv_addr = tcp::endpoint(address::from_string(ip), port);
}

void MHNetwork::set_process_callback(read_fun callback){
    client->set_process_callback(callback);
}

void MHNetwork::set_error_callback(error_fun callback){
    client->set_error_callback(callback);
}

void MHNetwork::set_connect_callback(connect_fun callback){
    _connect_callback = callback;
}

