# Catalina / Xcode12に対応

CXX = clang++
CXXFLAGS = -std=c++11 -g -fobjc-arc
LDFLAGS = -isysroot $(shell xcrun --sdk macosx --show-sdk-path) -framework Foundation -framework CoreFoundation -F/Library/Apple/System/Library/PrivateFrameworks -framework MobileDevice

OBJS = main.o common.o Options.o GetDevice.o help.o listdevice.o deviceinfo.o listapp.o listappname.o listappraw.o fileshare_mkdir.o fileshare_ls.o fileshare_rename.o fileshare_rm.o fileshare_pull.o fileshare_push.o listfilesharedapp.o file_ls.o file_pull.o file_rm.o app_ls.o app_pull.o install_app.o uninstall_app.o location.o location_reset.o show_devmode.o syslog.o

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: ta2ool

ta2ool: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)

main.o: main.mm
	$(CXX) $(CXXFLAGS) -c -o $@ $<

GetDevice.o: GetDevice.mm
	$(CXX) $(CXXFLAGS) -c -o $@ $<

install: ta2ool
	install ta2ool /usr/local/bin

clean:
	@rm -rf *.o
	@rm -rf ta2ool
