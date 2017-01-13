CONFIG -= app_bundle qt
CONFIG += c++11

INCLUDEPATH += \
    ../SFGUI/include \
    ../SFMLF/SFML/include

LIBS += \
    -L../SFGUI/lib/static-std -lsfgui-s \
    -L../SFMLF/lib -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lfreetype -ljpeg -lwinmm -lgdi32

DEFINES += SFML_STATIC
DEFINES += SFGUI_STATIC

SOURCES += main.cpp
