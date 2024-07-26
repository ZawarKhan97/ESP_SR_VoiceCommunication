# set(requires
#     hardware_driver
#     sr_ringbuf
#     )

# idf_component_register(SRCS main.c
#                        INCLUDE_DIRS "."
#                        REQUIRES ${requires})

idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS ".")
   