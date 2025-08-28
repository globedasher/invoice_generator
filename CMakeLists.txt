cmake_minimum_required(VERSION 3.16)
project(InvoiceGeneratorGUI VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Qt5 REQUIRED COMPONENTS Core Widgets PrintSupport)

# Enable automatic handling of MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Add executable
add_executable(InvoiceGeneratorGUI
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/CSVParser.cpp
    src/CSVParser.h
    src/PDFGenerator.cpp
    src/PDFGenerator.h
    src/TemplateEditor.cpp
    src/TemplateEditor.h
    src/HighlightManager.cpp
    src/HighlightManager.h
    src/InvoiceData.h
    ui/MainWindow.ui
)

# Link Qt libraries
target_link_libraries(InvoiceGeneratorGUI Qt5::Core Qt5::Widgets Qt5::PrintSupport)

# Set target properties
set_target_properties(InvoiceGeneratorGUI PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

# Copy sample files to build directory
configure_file(${CMAKE_SOURCE_DIR}/orders.csv ${CMAKE_BINARY_DIR}/orders.csv COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/config.xlsx ${CMAKE_BINARY_DIR}/config.xlsx COPYONLY)