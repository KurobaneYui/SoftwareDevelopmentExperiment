########################################################################################################################################
# lines below must comment unless only one target active
########################################################################################################################################

InodeRecorderTest: ./InodeRecorderTest.cpp ../compile/InodeRecorder.o
	g++ -O2 ./InodeRecorderTest.cpp ../compile/InodeRecorder.o -o ./InodeRecorderTest --std=c++17 -I ../includes

FilePackerTest: ./FilePackerTest.cpp ../compile/FilePacker.o
	g++ -O2 ./FilePackerTest.cpp ../compile/FilePacker.o -o ./FilePackerTest --std=c++17 -I ../includes

FileInfoTest: ./FileInfoTest.cpp ../compile/FileInfo.o ../compile/FilePacker.o ../compile/InodeRecorder.o
	g++ -O2 ./FileInfoTest.cpp ../compile/FileInfo.o ../compile/FilePacker.o ../compile/InodeRecorder.o -o ./FileInfoTest --std=c++17 -I ../includes

FileCompressorTest: ./FileCompressorTest.cpp ../compile/FileCompressor.o ../compile/FilePacker.o
	g++ -O2 ./FileCompressorTest.cpp ../compile/FileCompressor.o ../compile/FilePacker.o -o ./FileCompressorTest --std=c++17 -I ../includes

########################################################################################################################################
# lines below must not be comment
########################################################################################################################################

../compile/DisplayUI.o: ../sources/DisplayUI.cpp
	g++ -O2 -c ../sources/DisplayUI.cpp -o ../compile/DisplayUI.o --std=c++17 -I ../includes

../compile/FileCompressor.o: ../sources/FileCompressor.cpp
	g++ -O2 -c ../sources/FileCompressor.cpp -o ../compile/FileCompressor.o --std=c++17 -I ../includes

../compile/FileInfo.o: ../sources/FileInfo.cpp
	g++ -O2 -c ../sources/FileInfo.cpp -o ../compile/FileInfo.o --std=c++17 -I ../includes

../compile/FilePacker.o: ../sources/FilePacker.cpp
	g++ -O2 -c ../sources/FilePacker.cpp -o ../compile/FilePacker.o --std=c++17 -I ../includes

../compile/InodeRecorder.o: ../sources/InodeRecorder.cpp
	g++ -O2 -c ../sources/InodeRecorder.cpp -o ../compile/InodeRecorder.o --std=c++17 -I ../includes

../compile/Netdisk_client.o: ../sources/Netdisk_client.cpp
	g++ -O2 -c ../sources/Netdisk_client.cpp -o ../compile/Netdisk_client.o --std=c++17 -I ../includes

../compile/netTools.o: ../sources/netTools.cpp
	g++ -O2 -c ../sources/netTools.cpp -o ../compile/netTools.o --std=c++17 -I ../includes