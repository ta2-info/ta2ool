CXX = clang++
CXXFLAGS = -std=c++11 -g -fobjc-arc
LDFLAGS = -isysroot $(shell xcrun --sdk macosx --show-sdk-path) -framework Foundation -framework CoreFoundation -F/System/Library/PrivateFrameworks -framework MobileDevice

OBJS = main.o common.o Options.o GetDevice.o help.o listdevice.o deviceinfo.o listapp.o listappname.o listappraw.o fileshare_ls.o fileshare_pull.o listfilesharedapp.o file_ls.o file_pull.o file_rm.o app_ls.o app_pull.o install_app.o uninstall_app.o location.o location_reset.o

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: ta2ool

ta2ool: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)

main.o: main.mm
	$(CXX) $(CXXFLAGS) -c -o $@ $<

GetDevice.o: GetDevice.mm
	$(CXX) $(CXXFLAGS) -c -o $@ $<

