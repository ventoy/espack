# espack
Make a directory to an ESP disk image.  
把目录整体打包成一个 ESP 分区的磁盘镜像。

# usage
1. Decompress the released zip file, for example `espack-1.0.zip`
2. Put all you files/directories/subdirectories... under `ESP` folder.
3. Run `sh esp.sh` in linux or click the 'esp.bat' in Windows.
4. A image file `esp.img` will be created in current directory.

# 使用方法
1. 下载并解压发布的 zip 文件，例如 `espack-1.0.zip`
2. 把你所有需要的文件、目录、子目录等都放到解压后的 `ESP` 目录下。此目录下的结构即为最终ESP分区内的结构。
3. 对于Linux系统，在终端执行 `sh esp.sh` ，对于Windows系统，直接点击 `esp.bat` 这个批处理即可。
4. 打包成功后会在当前目录下生成一个 `esp.img` 文件，即为ESP分区的镜像文件。

# Note/说明
1. By default I put a `BOOTX64.EFI` file in the `EFI` directory as an example, it's UEFI shell.efi. You can delete it.  
   默认 `ESP` 里面放了一个 `BOOTX64.EFI` 文件作为例子，是 UEFI 的 shell.efi 文件。你可以把它删除。


