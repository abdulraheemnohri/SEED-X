add_library(seed_engine STATIC src/agent_kernel.cpp src/guardian.cpp src/policy.cpp src/audit_log.cpp src/event_bus.cpp src/local_ai.cpp)
target_include_directories(seed_engine PUBLIC include)
target_compile_features(seed_engine PUBLIC cxx_std_20)
target_compile_options(seed_engine PRIVATE -Wall -Wextra -Wpedantic -Werror=return-type)
