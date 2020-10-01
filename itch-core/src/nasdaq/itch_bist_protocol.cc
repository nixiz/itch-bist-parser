#include "nasdaq/itch_bist_protocol.hh"
#include "nasdaq/itch_bist_handler.hh"
#include "nasdaq/binaryfile.hh"

namespace helix {

namespace nasdaq {

bool itch_bist_protocol::supports(const std::string& name)
{
    return name == "nasdaq-binaryfile-itch-bist";
}

itch_bist_protocol::itch_bist_protocol(std::string name)
    :_name{std::move(name)}
{
}

session* itch_bist_protocol::new_session(void *data)
{
    if (_name == "nasdaq-binaryfile-itch-bist") {
        return new binaryfile_session<itch_bist_handler>(data);
    } else {
        throw std::invalid_argument("unknown protocol: " + _name);
    }
}

}

}
