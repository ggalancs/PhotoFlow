IF(NOT HAVE_REGEX_H)
  ADD_SUBDIRECTORY(regex)
ENDIF()

ADD_SUBDIRECTORY(lensfun)

# also build getopt on windows
IF(WIN32 AND BUILD_TESTS)
  ADD_SUBDIRECTORY(getopt)
ENDIF()
