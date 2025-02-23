#pragma once

#include "name.hpp"

namespace inery {
   namespace internal_use_do_not_use {
      extern "C" uint64_t inery_contract_name;
   }

   inline name current_context_contract() { return name{internal_use_do_not_use::inery_contract_name}; }
}
