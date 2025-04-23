# FILE = 

CC            = gcc
CXX           = g++
CFLAGS    += -DLINUX -pipe -O2 -Wall -W -fopenmp -I/usr/include -I/opt/EDTpdv/tiff-4.0.3/libtiff -I/lib/x86_64-linux-gnu/ -I/opt/pleora/ebus_sdk/Ubuntu-12.04-x86_64/include -L/usr/lib/x86_64-linux-gnu -I/usr/lib/x86_64-linux-gnu -I/usr/lib/ -I/opt/pleora/ebus_sdk/Ubuntu-12.04-x86_64/lib/genicam/bin/Linux64_x64 -I/opt/pleora/ebus_sdk/Ubuntu-12.04-x86_64/lib 


LIBS      = -lncurses \
/usr/lib/gcc/x86_64-redhat-linux/4.8.2/libgomp.a \
/usr/lib64/libtiff.so.5 \
/usr/cfitsio-4.4.1/lib/libcfitsio.a \
/usr/lib64/libdl.so \
/usr/local/lib/libpicam.so.0 \
/usr/local/lib/libpicam.so \
-lz # <-- Add this line for linking zlib


# all: 
# 	@clear
# 	@rm -f *.o
# 	@rm -f *~
# 	@rm -f obj/*
# 	@echo "Compiling server..."
# 	@g++ -c -o obj/server.o server.c
# 	@echo "Done."
# 	@echo "Compiling camera..."	
# 	@g++ -c -o obj/camera.o $(CFLAGS) camera.cpp 	
# 	@echo "Done."
# 	@echo "Compiling server... " 
# 	@g++ obj/server.o obj/camera.o $(CFLAGS) $(LIBS) -o bin/camserver_cit camserver.c
# 	@echo "Done."
all: 
	@clear
	@rm -f *.o
	@rm -f *~
	@rm -f obj/*
	@echo "Compiling server..."
	@g++ -c -o obj/server.o server.c
	@echo "Done."
	@echo "Compiling camera..."	
	@g++ -c -o obj/camera.o $(CFLAGS) camera.cpp 	
	@echo "Done."

	@echo "Done."
	@echo "Compiling server... " 
	@g++ obj/server.o obj/camera.o $(CFLAGS) $(LIBS) -o bin/camserver_cit camserver.c

	@echo "Done."


clean:
	@echo "Cleaning... "
	rm -f *.o
	rm -f *~
	rm -f obj/*
	@echo "Done."

