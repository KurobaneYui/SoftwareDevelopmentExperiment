BackupTool: compile/DisplayUI.o compile/FileInfo.o compile/FilePacker.o compile/FileCompressor.o compile/InodeRecorder.o compile/Netdisk_client.o compile/netTools.o
	g++ -O2 compile/DisplayUI.o compile/FileInfo.o compile/FilePacker.o compile/FileCompressor.o compile/InodeRecorder.o compile/Netdisk_client.o compile/netTools.o -o BackupTool --std=c++17 -I includes

compile/DisplayUI.o: sources/DisplayUI.cpp
	g++ -O2 -c sources/DisplayUI.cpp -o compile/DisplayUI.o --std=c++17 -I includes

compile/FileCompressor.o: sources/FileCompressor.cpp
	g++ -O2 -c sources/FileCompressor.cpp -o compile/FileCompressor.o --std=c++17 -I includes

compile/FileInfo.o: sources/FileInfo.cpp
	g++ -O2 -c sources/FileInfo.cpp -o compile/FileInfo.o --std=c++17 -I includes

compile/FilePacker.o: sources/FilePacker.cpp
	g++ -O2 -c sources/FilePacker.cpp -o compile/FilePacker.o --std=c++17 -I includes

compile/InodeRecorder.o: sources/InodeRecorder.cpp
	g++ -O2 -c sources/InodeRecorder.cpp -o compile/InodeRecorder.o --std=c++17 -I includes

compile/Netdisk_client.o: sources/Netdisk_client.cpp
	g++ -O2 -c sources/Netdisk_client.cpp -o compile/Netdisk_client.o --std=c++17 -I includes

compile/netTools.o: sources/netTools.cpp
	g++ -O2 -c sources/netTools.cpp -o compile/netTools.o --std=c++17 -I includes