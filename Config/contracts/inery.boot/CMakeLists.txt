if (INERY_COMPILE_TEST_CONTRACTS)
   add_contract(inery.boot inery.boot ${CMAKE_CURRENT_SOURCE_DIR}/src/inery.boot.cpp)

   target_include_directories(inery.boot
      PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/include)

   set_target_properties(inery.boot
      PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

   configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/inery.boot.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/inery.boot.contracts.md @ONLY )

   target_compile_options( inery.boot PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
else()
   configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/bin/inery.boot.abi ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY )
   configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/bin/inery.boot.wasm ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY )
endif()

