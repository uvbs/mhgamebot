#include "mhnetwork.h"

MHNetwork::MHNetwork()
{
    client = std::make_shared<RWHandler>(ios);
}
