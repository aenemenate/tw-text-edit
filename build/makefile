Objs = main.obj helpers.obj editor_actions.obj editor_data.obj color_palette.obj \
       text_buffer.obj menu_bar.obj buffer_list.obj text_dropdown.obj
       

SRC_DIR = ../src

TARGET  = ../bin/tw-text-edit.exe

all: $(TARGET)

$(TARGET):$(Objs)
 link /out:$@ /SUBSYSTEM:WINDOWS /DEBUG:FULL /LIBPATH:"C:\Program Files\boost_1_73_0\stage\lib" ..\libs\BearLibTerminal.lib $**

# src

main.obj:$(SRC_DIR)/main.cpp $(SRC_DIR)/base_types.h $(SRC_DIR)/editor_data.h $(SRC_DIR)/color_palette.h \
         $(SRC_DIR)/buffer_list.h  $(SRC_DIR)/menu_bar.h $(SRC_DIR)/text_dropdown.h $(SRC_DIR)/text_buffer.h
  cl /c /std:c++17 /Z7 /MT /O2 /I"C:\Program Files\boost_1_73_0" /EHsc $(SRC_DIR)/main.cpp
  
helpers.obj:$(SRC_DIR)/helpers.cpp $(SRC_DIR)/helpers.h
  cl /c /std:c++17 /Z7 /MT /O2 /I"C:\Program Files\boost_1_73_0" /EHsc $(SRC_DIR)/helpers.cpp

editor_actions.obj:$(SRC_DIR)/editor_actions.cpp $(SRC_DIR)/editor_actions.h $(SRC_DIR)/editor_data.h
  cl /c /std:c++17 /Z7 /MT /O2 /I"C:\Program Files\boost_1_73_0" /EHsc $(SRC_DIR)/editor_actions.cpp
  
editor_data.obj:$(SRC_DIR)/editor_data.cpp $(SRC_DIR)/editor_data.h $(SRC_DIR)/base_types.h $(SRC_DIR)/color_palette.h \
                $(SRC_DIR)/buffer_list.h  $(SRC_DIR)/menu_bar.h $(SRC_DIR)/text_dropdown.h $(SRC_DIR)/text_buffer.h
  cl /c /std:c++17 /Z7 /MT /O2 /I"C:\Program Files\boost_1_73_0" /EHsc $(SRC_DIR)/editor_data.cpp
  
color_palette.obj:$(SRC_DIR)/color_palette.cpp $(SRC_DIR)/color_palette.h
  cl /c /std:c++17 /Z7 /MT /O2 /I"C:\Program Files\boost_1_73_0" /EHsc $(SRC_DIR)/color_palette.cpp
  
text_buffer.obj:$(SRC_DIR)/text_buffer.cpp $(SRC_DIR)/text_buffer.h $(SRC_DIR)/base_types.h $(SRC_DIR)/color_palette.h
  cl /c /std:c++17 /Z7 /MT /O2 /I"C:\Program Files\boost_1_73_0" /EHsc $(SRC_DIR)/text_buffer.cpp

menu_bar.obj:$(SRC_DIR)/menu_bar.cpp $(SRC_DIR)/menu_bar.h $(SRC_DIR)/color_palette.h $(SRC_DIR)/editor_actions.h \
             $(SRC_DIR)/base_types.h $(SRC_DIR)/editor_data.h 
  cl /c /std:c++17 /Z7 /MT /O2 /I"C:\Program Files\boost_1_73_0" /EHsc $(SRC_DIR)/menu_bar.cpp

buffer_list.obj:$(SRC_DIR)/buffer_list.cpp $(SRC_DIR)/buffer_list.h $(SRC_DIR)/color_palette.h $(SRC_DIR)/base_types.h \
                $(SRC_DIR)/text_buffer.h
  cl /c /std:c++17 /Z7 /MT /O2 /I"C:\Program Files\boost_1_73_0" /EHsc $(SRC_DIR)/buffer_list.cpp

text_dropdown.obj:$(SRC_DIR)/text_dropdown.cpp $(SRC_DIR)/text_dropdown.h $(SRC_DIR)/color_palette.h $(SRC_DIR)/helpers.h \
                  $(SRC_DIR)/base_types.h $(SRC_DIR)/text_buffer.h $(SRC_DIR)/editor_data.h $(SRC_DIR)/editor_actions.h
  cl /c /std:c++17 /Z7 /MT /O2 /I"C:\Program Files\boost_1_73_0" /EHsc $(SRC_DIR)/text_dropdown.cpp