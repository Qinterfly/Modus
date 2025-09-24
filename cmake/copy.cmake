
file(GLOB FULLSRCS ${SRCS})

foreach(SRC IN LISTS FULLSRCS)
    if(EXISTS ${SRC})
        message("Copying file ${SRC} to ${DEST}")
        file(COPY ${SRC} DESTINATION ${DEST})
    else()
        message("Could not find file ${SRC} for copying")
    endif()
endforeach()
