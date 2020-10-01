#pragma once

#include "helix.hh"

#include <string>

namespace helix {

namespace nasdaq {

class nordic_itch_protocol : public protocol {
    std::string _name;
public:
    static bool supports(const std::string& name);
    explicit nordic_itch_protocol(std::string name);
    virtual session* new_session(void *) override;
};

}

}
