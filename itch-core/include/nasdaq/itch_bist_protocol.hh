#pragma once

#include "helix.hh"

#include <string>

namespace helix {

namespace nasdaq {

class itch_bist_protocol : public protocol {
    std::string _name;
public:
    explicit itch_bist_protocol(std::string name);
    static bool supports(const std::string& name);
    virtual session* new_session(void *) override;
};

}

}
