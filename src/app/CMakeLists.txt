qt6_add_executable(LonelyRunnerApp
    main.cpp
    mainwindow.cpp
    manualtestpanel.cpp
    rangetestpanel.cpp
    verifyfilepanel.cpp
    rangeworker.cpp
    animationwidget.cpp
    thememanager.cpp
    optionspanel.cpp
)

set_target_properties(LonelyRunnerApp PROPERTIES AUTOMOC ON)

target_link_libraries(LonelyRunnerApp PRIVATE
    lonelyrunner
    Qt6::Widgets
    Qt6::Concurrent
)
