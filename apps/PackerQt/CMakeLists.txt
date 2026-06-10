qt_add_executable(RepackStudio
    src/main.cpp
    src/PackerController.h
    src/PackerController.cpp
)

target_link_libraries(RepackStudio PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Concurrent
    RepackCore
)

qt_add_qml_module(RepackStudio
    URI RepackStudio
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/components/MetricCard.qml
        qml/components/SideNavButton.qml
        qml/components/PrimaryButton.qml
        qml/components/TextFieldRow.qml
)

if (WIN32)
    set_target_properties(RepackStudio PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

