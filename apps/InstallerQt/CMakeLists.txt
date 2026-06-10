qt_add_executable(RepackInstallerGui
    src/main.cpp
    src/InstallerController.h
    src/InstallerController.cpp
)

target_link_libraries(RepackInstallerGui PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Concurrent
    RepackCore
)

qt_add_qml_module(RepackInstallerGui
    URI RepackInstaller
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/components/PrimaryButton.qml
        qml/components/ModeButton.qml
        qml/components/StatPill.qml
)

if (WIN32)
    set_target_properties(RepackInstallerGui PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

