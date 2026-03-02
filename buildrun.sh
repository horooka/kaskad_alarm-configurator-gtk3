glib-compile-resources --target=icons_resource.c --generate-source icons.gresource.xml

g++ -c -g -DFORCE_PACKING -std=c++17 -I. -IFastParam-gcc $(pkg-config --cflags glibmm-2.4) -o FastParam-gcc/BinDataStruct.o FastParam-gcc/BinDataStruct.cpp &&
  g++ -c -g -DFORCE_PACKING -std=c++17 -I. -IFastParam-gcc $(pkg-config --cflags glibmm-2.4) -o FastParam-gcc/StationNames.o FastParam-gcc/StationNames.cpp &&
  g++ -c -g -DFORCE_PACKING -std=c++17 -I. -IFastParam-gcc $(pkg-config --cflags glibmm-2.4) -o FastParam-gcc/fastparam_api.o FastParam-gcc/fastparam_api.cpp &&
  g++ -g src/main.cpp src/utils.cpp src/tinyxml2.cpp icons_resource.c \
    FastParam-gcc/BinDataStruct.o FastParam-gcc/StationNames.o FastParam-gcc/fastparam_api.o \
    -o kaskad_alarm-configurator-gtk3 \
    -Iinclude $(pkg-config --cflags --libs gtkmm-3.0) -std=c++17 -Wall -Wextra -Wno-class-memaccess &&
  ./kaskad_alarm-configurator-gtk3

# G_DEBUG=all ./kaskad_alarm-configurator-gtk3
