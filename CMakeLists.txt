cmake_minimum_required(VERSION 3.16)

project(magicpods VERSION 1.0.1 LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Widgets WebSockets Linguist)

qt_standard_project_setup(REQUIRES 6.8
    I18N_TRANSLATED_LANGUAGES en
)

set(QML_IMPORT_PATH "${CMAKE_BINARY_DIR}/qml" CACHE STRING "Qt Creator extra qml import paths" FORCE)

set(APP_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/app")
set(APP_QML_DIR "${APP_SOURCE_DIR}/qml")
set(APP_I18N_TS_FILE_BASE "src/app/i18n/locale")

set_source_files_properties(src/app/qml/Units.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

file(GLOB_RECURSE APP_RESOURCE_FILES CONFIGURE_DEPENDS
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "${APP_QML_DIR}/assets/*"
)

qt_add_executable(magicpods
    src/app/cpp/Backend.cpp
    src/app/cpp/BackendManager.cpp
    src/app/cpp/DeckyManager.cpp
    src/app/cpp/DesktopManager.cpp
    src/app/cpp/main.cpp
    src/app/cpp/TrayIcon.cpp
    src/app/cpp/TrayIconManager.cpp
)

qt_add_resources(magicpods "fonts"
    PREFIX "/fonts"
    FILES
        src/app/fonts/NotoSans-Thin.ttf
        src/app/fonts/NotoSans-ThinItalic.ttf
        src/app/fonts/NotoSans-ExtraLight.ttf
        src/app/fonts/NotoSans-ExtraLightItalic.ttf
        src/app/fonts/NotoSans-Light.ttf
        src/app/fonts/NotoSans-LightItalic.ttf
        src/app/fonts/NotoSans-Regular.ttf
        src/app/fonts/NotoSans-Italic.ttf
        src/app/fonts/NotoSans-Medium.ttf
        src/app/fonts/NotoSans-MediumItalic.ttf
        src/app/fonts/NotoSans-SemiBold.ttf
        src/app/fonts/NotoSans-SemiBoldItalic.ttf
        src/app/fonts/NotoSans-Bold.ttf
        src/app/fonts/NotoSans-BoldItalic.ttf
        src/app/fonts/NotoSans-ExtraBold.ttf
        src/app/fonts/NotoSans-ExtraBoldItalic.ttf
        src/app/fonts/NotoSans-Black.ttf
        src/app/fonts/NotoSans-BlackItalic.ttf
)

qt_add_qml_module(magicpods
    URI magicpods
    OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml/magicpods"
    QML_FILES
        src/app/qml/Units.qml
        src/app/qml/Main.qml
        src/app/qml/PopupAnimation.qml        
        src/app/qml/components/Animations.qml
        src/app/qml/components/BatteryPopup.qml
        src/app/qml/components/BatteryVertical.qml
        src/app/qml/components/FormRow.qml
        src/app/qml/components/Heading.qml
        src/app/qml/components/HelpMessage.qml
        src/app/qml/components/InlineMessage.qml
        src/app/qml/components/Label.qml
        src/app/qml/components/Separator.qml
        src/app/qml/pages/AboutPage.qml
        src/app/qml/pages/BatteryPage.qml
        src/app/qml/pages/HeadphonesPage.qml
        src/app/qml/pages/SettingsPage.qml
        src/app/qml/pages/ColorSample.qml
    RESOURCES
        ${APP_RESOURCE_FILES}
)

set_target_properties(magicpods PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_include_directories(magicpods PRIVATE
    src/app/cpp
    src/app/dependencies
)

target_link_libraries(magicpods PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
    Qt6::WebSockets
)

target_compile_definitions(magicpods PRIVATE
    MAGICPODS_VERSION="${PROJECT_VERSION}"
    MAGICPODS_APP_SOURCE_DIR="${APP_SOURCE_DIR}"
)

qt_add_translations(magicpods
    SOURCE_TARGETS magicpods
    TS_FILE_BASE "${APP_I18N_TS_FILE_BASE}"
    MERGE_QT_TRANSLATIONS
    QT_TRANSLATION_CATALOGS qtbase
    RESOURCE_PREFIX i18n
)

install(TARGETS magicpods
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET magicpods
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
