Objs = main.obj
       

SRC_DIR = ../src

TARGET  = ../bin/tw-text-edit.exe

all: $(TARGET)

$(TARGET):$(Objs)
 link /out:$@ /SUBSYSTEM:WINDOWS /DEBUG:FULL /LIBPATH:"C:\Program Files\boost_1_73_0\stage\lib" ..\libs\BearLibTerminal.lib $**

# src

main.obj:$(SRC_DIR)/main.cpp
  cl /c /std:c++17 /Z7 /MT /O2 /I"C:\Program Files\boost_1_73_0" /EHsc $(SRC_DIR)/main.cpp
