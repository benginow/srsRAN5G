set_directory_properties(PROPERTIES LABELS "du_high_fuzz")

include_directories(../../..)

add_executable(du_high_benchmark du_high_benchmark.cpp)
target_link_libraries(du_high_benchmark srsran_du_high f1ap_du_test_helpers srsran_pcap gtest)
add_test(du_high_benchmark du_high_benchmark)
