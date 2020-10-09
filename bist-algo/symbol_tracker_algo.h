#pragma once

#include "helix.hh"
#include "bist_algo_base.h"
#include <string>
#include <math.h>
#include <stdint.h>
#include <vector>

namespace helix
{
  class symbol_tracker_algo final :
    public algo_base
  {
  public:
    explicit symbol_tracker_algo(std::weak_ptr<session> s, std::vector<std::string> symbols);
    ~symbol_tracker_algo();
    static symbol_tracker_algo* create_new_algo(std::weak_ptr<session> session, 
                                                std::vector<std::string> symbol);
  private:
    int tick(event* ev) override;
    std::unique_ptr<struct trace_fmt_ops> impl;
  };

}


