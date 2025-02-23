/**
 *  @file
 *  @copyright defined in ine/LICENSE
 */
#pragma once

#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @addtogroup chain
 *  @ingroup c_api
 *  @brief Defines %C API for querying internal chain state
 *  @{
 */

 /**
  *  Gets the set of active masters.
  *
  *  @param masters - Pointer to a buffer of account names
  *  @param datalen - Byte length of buffer, when passed 0 will return the size required to store full output.
  *
  *  @return uint32_t - Number of bytes actually populated
  *  @pre `masters` is a pointer to a range of memory at least `datalen` bytes long
  *  @post the passed in `masters` pointer gets the array of active masters.
  *
  *  Example:
  *
  *  @code
  *  capi_name masters[21];
  *  uint32_t bytes_populated = get_active_masters(masters, sizeof(capi_name)*21);
  *  @endcode
  */
__attribute__((inery_wasm_import))
uint32_t get_active_masters( capi_name* masters, uint32_t datalen );

#ifdef __cplusplus
}
#endif
/// @}
