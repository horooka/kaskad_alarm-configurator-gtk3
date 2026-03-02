glib-compile-resources --target=icons_resource.c --generate-source icons.gresource.xml

g++ -c -g -DFORCE_PACKING -std=c++17 -I. -IFastParam-gxx $(pkg-config --cflags glibmm-2.4) -o FastParam-gxx/BinDataStruct.o FastParam-gxx/BinDataStruct.cpp &&
  g++ -c -g -DFORCE_PACKING -std=c++17 -I. -IFastParam-gxx $(pkg-config --cflags glibmm-2.4) -o FastParam-gxx/StationNames.o FastParam-gxx/StationNames.cpp &&
  g++ -c -g -DFORCE_PACKING -std=c++17 -I. -IFastParam-gxx $(pkg-config --cflags glibmm-2.4) -o FastParam-gxx/FastParamConst.o FastParam-gxx/FastParamConst.cpp &&
  g++ -c -g -DFORCE_PACKING -std=c++17 -I. -IFastParam-gxx $(pkg-config --cflags glibmm-2.4) -o FastParam-gxx/ParamSelect.o FastParam-gxx/ParamSelect.cpp &&
  g++ -c -g -DFORCE_PACKING -std=c++17 -I. -IFastParam-gxx $(pkg-config --cflags glibmm-2.4) -o FastParam-gxx/FastParamMain.o FastParam-gxx/FastParamMain.cpp &&
  g++ -g src/main.cpp src/utils.cpp src/tinyxml2.cpp icons_resource.c \
    FastParam-gxx/BinDataStruct.o FastParam-gxx/StationNames.o FastParam-gxx/FastParamConst.o FastParam-gxx/ParamSelect.o FastParam-gxx/FastParamMain.o \
    -o kaskad_alarm-configurator-gtk3 \
    -I. -Iinclude $(pkg-config --cflags --libs gtkmm-3.0) -std=c++17 -Wall -Wextra -Wno-class-memaccess &&
  ./kaskad_alarm-configurator-gtk3
