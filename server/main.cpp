#include"Server.h"

int main(){
    Server::Ptr server = Server::get_instance();

    server->run();
    return 0;
}