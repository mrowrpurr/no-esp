if (MSVC)
    add_compile_definitions(
            UNICODE
            _UNICODE
            NOMINMAX
            _CRT_SECURE_NO_WARNINGS # Stop telling me to use strcpy_s please
    )
endif()