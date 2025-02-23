#pragma once
#include <vector>
#include "../../core/inery/name.hpp"
#include "../../core/inery/crypto.hpp"

namespace inery {

  /**
   *  @defgroup master_key master Key
   *  @ingroup contracts
   *  @ingroup types
   *  @brief Maps master with its signing key, used for master schedule
   */

  /**
   *  Maps master with its signing key, used for master schedule
   *
   *  @ingroup master_key
   */
  struct master_key {

    /**
     *  Name of the master
     *
     *  @ingroup master_key
     */
    name             master_name;

    /**
     *  Block signing key used by this master
     *
     *  @ingroup master_key
     */
    public_key       block_signing_key;

    /// @cond OPERATORS

    friend constexpr bool operator < ( const master_key& a, const master_key& b ) {
      return a.master_name < b.master_name;
    }

    /// @endcond

    CDT_REFLECT(master_name, block_signing_key);
  };

   /**
    *  @defgroup master_schedule master Schedule
    *  @ingroup contracts
    *  @ingroup types
    *  @brief Defines both the order, account name, and signing keys of the active set of masters.
    */

   /**
    * Defines both the order, account name, and signing keys of the active set of masters.
    *
    * @ingroup master_schedule
    */
   struct master_schedule {
      /**
       * Version number of the schedule. It is sequentially incrementing version number
       */
      uint32_t                     version;

      /**
       * List of masters for this schedule, including its signing key
       */
      std::vector<master_key>    masters;

      CDT_REFLECT(version, masters);
   };

   /**
    *  @defgroup master_authority master Authority
    *  @ingroup contracts
    *  @ingroup types
    *  @brief Maps master with its a flexible authority structure, used for master schedule
    */

   /**
    * pairs a public key with an integer weight
    *
    * @ingroup master_authority
    */
   struct key_weight {
      /**
       * public key used in a weighted threshold multi-sig authority
       *
       * @brief public key used in a weighted threshold multi-sig authority
       */
      public_key key;

      /**
       * weight associated with a signature from the private key associated with the accompanying public key
       *
       * @brief weight of the public key
       */
      uint16_t   weight;

      CDT_REFLECT(key, weight);
   };

   /**
    * block signing authority version 0
    * this authority allows for a weighted threshold multi-sig per-master
    *
    * @ingroup master_authority
    *
    * @brief weighted threshold multi-sig authority
    */
   struct block_signing_authority_v0 {
      /**
       * minimum threshold of accumulated weights from component keys that satisfies this authority
       *
       * @brief minimum threshold of accumulated weights from component keys that satisfies this authority
       */
      uint32_t                    threshold;

      /**
       * component keys and their associated weights
       *
       * @brief component keys and their associated weights
       */
      std::vector<key_weight>     keys;

      bool is_valid()const;

      CDT_REFLECT(threshold, keys);
   };

   /**
    * variant of all possible block signing authorities
    *
    * @ingroup master_authority
    */
   using block_signing_authority = std::variant<block_signing_authority_v0>;

   /**
    * Maps master with its signing key, used for master schedule
    *
    * @ingroup master_authority
    *
    * @brief Maps master with its signing key
    */
   struct master_authority {

      /**
       * Name of the master
       *
       * @brief Name of the master
       */
      name             master_name;

      /**
       * The block signing authority used by this master
       */
      block_signing_authority       authority;

      friend constexpr bool operator < ( const master_authority& a, const master_authority& b ) {
         return a.master_name < b.master_name;
      }

      CDT_REFLECT(master_name, authority);
   };

   /**
    *  Returns back the list of active master names.
    *
    *  @ingroup master_schedule
    */
   std::vector<name> get_active_masters();

} /// namespace inery
