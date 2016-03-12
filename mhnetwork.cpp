#include "mhnetwork.h"

MHNetwork::MHNetwork()
{
    client = std::make_shared<RWHandler>(ios);
    client->set_id(0);
}

