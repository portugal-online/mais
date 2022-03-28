cmake_minimum_required(VERSION 3.16.1)

target_sources(${PROJECT_NAME}.elf
	PRIVATE
		UAIR_config_api.cc
		UAIR_io_audit.c
		UAIR_io_base.c
		UAIR_io_config.c
)
