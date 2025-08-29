idf_component_register(
                       SRCS "blink_example_main.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES driver DRV8313_Driver PWM
                       )
component_compile_options(-O0)
