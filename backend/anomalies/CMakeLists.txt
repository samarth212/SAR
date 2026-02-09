add_library(anomalies
    priceAnomaly.cpp
    volumeAnomaly.cpp
    spreadAnomaly.cpp
)

target_include_directories(anomalies PUBLIC ${CMAKE_CURRENT_LIST_DIR}/..)

target_link_libraries(anomalies PUBLIC nlohmann_json::nlohmann_json)
