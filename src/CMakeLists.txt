cmake_minimum_required(VERSION 3.0.0)
project( Algorithm )

# Adding target sources
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCES)
file(GLOB HEADERS "*.h")

if(UNIX)
	add_compile_options(-lboost_thread -lboost_system)
endif()

# Creating target
add_library( ${PROJECT_NAME} ${SOURCES} ${HEADERS} )


# Linking with boost
SET(BOOST_ROOT ${CONAN_BOOST_ROOT})
SET(BOOST_INCLUDEDIR ${CONAN_INCLUDE_DIRS_BOOST})
set(BOOST_LIBRARYDIR ${CONAN_LIB_DIRS_BOOST})
set(BOOST_LIBRARY_DIR ${CONAN_LIB_DIRS_BOOST})

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.70.0 EXACT 
	REQUIRED COMPONENTS 
		unit_test_framework 
		regex 
		system 
		filesystem 
		date_time 
		chrono 
		thread 
		locale 
		iostreams
)
target_include_directories(${PROJECT_NAME} PRIVATE ${BOOST_INCLUDEDIR})
target_link_directories(${PROJECT_NAME} PRIVATE  ${BOOST_LIBRARYDIR})
target_link_libraries(${PROJECT_NAME} 
	${CONAN_LIBS} 
	Boost::unit_test_framework 
	Boost::regex 
	Boost::system 
	Boost::filesystem
	Boost::date_time 
	Boost::chrono 
	Boost::thread 
	Boost::locale  
	Boost::iostreams
)

# Standard setup of target
target_compile_definitions(${PROJECT_NAME} 
    PRIVATE NOMINMAX
)
if(UNIX)
	set(CMAKE_C_FLAGS " -fpermissive")
	set(CMAKE_CXX_FLAGS " -fpermissive")
	find_library(pthread STATIC)
	target_link_libraries(${PROJECT_NAME} pthread)
endif()
