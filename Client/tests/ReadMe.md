# 测试用例说明

## 编译说明

在本目录下，有Makefile用于快速编译

Makefile文件中分为上下两节：下半部分为依赖的模块，除非确定编译期不需要，否则请不要注释；上半部分为测试用例编译，请只保留需要测试的用例，而注释其他用例的编译（由于make只把第一行当作目标代码，因此都不注释将只能编译第一个测试用例）

编译启用了`-O2`优化

## 用例说明

* InodeRecorderTest：本测试用例测试InodeRecorder模块。无其他依赖文件（夹）
* FilePackerTest：本测试用例测试FilePacker模块。依赖本目录下的testFileRWTruncate文件
* FileInfoTest：本测试用例测试FileInfo模块。依赖FilePacker模块和InodeRecorder模块，依赖上级目录下的test_directory文件夹作为待备份文件夹，依赖本目录下BackupFileTemporyDirectory文件夹存放备份文件，依赖本目录下BackupFileRestoryFolder文件夹存放还原的文件（夹）
* FileCompressorTest：本测试用例测试FileComprerssor模块。依赖FilePacker模块和InodeRecorder模块，依赖本目录下CompressorTestFolder目录内的BACKUP.BACKUP文件作为待压缩文件

## 运行测试

分别修改Makefile后进行编译，编译后直接运行查看输出即可。例如：`./FileCompressorTest`运行FileCompressorTest测试例程