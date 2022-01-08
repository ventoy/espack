# espack
Make a directory to an ESP disk image.  (把目录整体打包成一个 ESP 分区的磁盘镜像。)

# usage
1. Download the released zip file from [(here)](https://github.com/ventoy/espack/releases), for example `espack-1.0.zip` and then decompress it.
2. Put all you files/directories/subdirectories... under `ESP` folder. The structure of the `ESP` directory is the structure of the final ESP partition image.
3. Run `sh esp.sh` in linux or click the `esp.bat` in Windows.
4. An image file `esp.img` (you can rename it as you want) will be created in current directory.

# Note
1. By default I put a `BOOTX64.EFI` file in the `EFI` directory as an example, it's UEFI shell.efi. You can replace it with your UEFI app.  
2. You can pack different EFI files for different architectures together, for example `BOOTX64.EFI` `BOOTIA32.EFI` `BOOTAA64.EFI` ...     
  
  
  
  
# 使用方法
1. 从 [(这里)](https://github.com/ventoy/espack/releases) 下载发布的 zip 文件，例如 `espack-1.0.zip`，然后解压开。
2. 把你所有需要的文件、目录、子目录等都放到解压后的 `ESP` 目录下。此目录的结构即为最终ESP分区的结构。
3. 对于Linux系统，在终端执行 `sh esp.sh` ，对于Windows系统，直接点击 `esp.bat` 这个批处理即可。
4. 打包成功后会在当前目录下生成一个 `esp.img` 文件（你可以任意重命名），即为ESP分区的镜像文件。

# 说明
1. 默认 `ESP` 里面放了一个 `BOOTX64.EFI` 文件作为例子，是 UEFI 的 shell.efi 文件。你可以使用自己的EFI文件替换它。
2. 你可以把不同构架的EFI文件打包在一起，比如  `BOOTX64.EFI` `BOOTIA32.EFI` `BOOTAA64.EFI`

