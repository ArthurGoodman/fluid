CONFIG -= app_bundle qt
CONFIG += c++11

INCLUDEPATH += \
    ..\SFML\include \
    ..\SFGUI\include

LIBS += \
    -L..\SFGUI\lib\static-std -lsfgui-s \
    -L..\SFML\lib -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lfreetype -ljpeg -lwinmm -lgdi32

DEFINES += SFML_STATIC
DEFINES += SFGUI_STATIC

SOURCES += main.cpp
