cmake_minimum_required(VERSION 3.16.1)

target_sources(${PROJECT_NAME}.elf
	PRIVATE
		${CMAKE_CURRENT_LIST_DIR}/UAIR_config_api.cc
		${CMAKE_CURRENT_LIST_DIR}/UAIR_io_audit.c
		${CMAKE_CURRENT_LIST_DIR}/UAIR_io_base.c
		$<$<BOOL:${UAIR_UNIT_TESTS}>:${CMAKE_CURRENT_LIST_DIR}/UAIR_io_base_t.cc>
		${CMAKE_CURRENT_LIST_DIR}/UAIR_io_config.cc
		$<$<BOOL:${UAIR_UNIT_TESTS}>:${CMAKE_CURRENT_LIST_DIR}/UAIR_io_config_t.cc>
)
