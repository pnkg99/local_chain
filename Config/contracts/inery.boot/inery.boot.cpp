#include "inery.boot.hpp"
#include <inery/privileged.hpp>

namespace ineryboot {

void boot::onerror( ignore<uint128_t>, ignore<std::vector<char>> ) {
   check( false, "the onerror action cannot be called directly" );
}

void boot::activate( const inery::checksum256& feature_digest ) {
   require_auth( get_self() );
   inery::preactivate_feature( feature_digest );
}

void boot::reqactivated( const inery::checksum256& feature_digest ) {
   check( inery::is_feature_activated( feature_digest ), "protocol feature is not activated" );
}

}
