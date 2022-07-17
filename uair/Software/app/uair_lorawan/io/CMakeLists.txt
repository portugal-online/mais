cmake_minimum_required(VERSION 3.16.1)

target_sources(${PROJECT_NAME}.elf
	PRIVATE
		${CMAKE_CURRENT_LIST_DIR}/UAIR_config_api.cc
		$<$<BOOL:${UNITTESTS}>:${CMAKE_CURRENT_LIST_DIR}/UAIR_config_api_t.cc>
		${CMAKE_CURRENT_LIST_DIR}/UAIR_io_audit.cc
		${CMAKE_CURRENT_LIST_DIR}/UAIR_io_base.c
		$<$<BOOL:${UNITTESTS}>:${CMAKE_CURRENT_LIST_DIR}/UAIR_io_base_t.cc>
		${CMAKE_CURRENT_LIST_DIR}/UAIR_io_config.cc
		$<$<BOOL:${UNITTESTS}>:${CMAKE_CURRENT_LIST_DIR}/UAIR_io_config_t.cc>
)
