# FAT32 File System Shell

## Project Description

This project involves creating a user space shell application capable of interpreting a FAT32 file system image. The shell will provide various commands to interact with the file system image, ensuring it remains uncorrupted and robust.

The main objectives of this project include:

* Understanding and working with file allocation tables and endianness.
* Implementing file access operations within a FAT32 file system image.
* Ensuring the utility adheres to strict guidelines without using existing kernel code or other FAT32 utility code.

A FAT32 file image, fat32.img, is provided, along with the FAT32 specification document, fatspec.pdf. These resources are essential for correctly interpreting the file system image


## Features
   
#### open
```
open <filename>  
```
Opens a FAT32 image file. If the file is not found, an error message is displayed. If a file system is already open, an error message is displayed.

#### close
```
close
```
Closes the currently opened FAT32 image file. If no file system is open, an error message is displayed. Any subsequent commands (except open) will result in an error if no file system is open.

#### save
```
save
```
Writes the memory-resident FAT32 image to the current working directory using the original filename. If no file system is open, an error message is displayed.

```
save <new filename>
```
Writes the memory-resident FAT32 image to the current working directory using the specified new filename. The original disk image remains unmodified. If no file system is open, an error message is displayed.


#### quit / exit
```
quit
exit   
```
Exits the program with status zero.

#### info
```
info
```
Displays information about the file system in both hexadecimal and decimal formats, including:

* BPB_BytesPerSec
* BPB_SecPerClus
* BPB_RsvdSecCnt
* BPB_NumFATS
* BPB_FATSz32
* BPB_ExtFlags
* BPB_RootClus
* BPB_FSInfo

#### stat
```
stat <filename> or <directory name>
```
Prints the attributes and starting cluster number of the specified file or directory. For directories, the size is zero. If the file or directory does not exist, an error message is displayed.

#### get
```
get <filename>
```
Retrieves the specified file from the FAT32 image and places it in the current working directory. If the file does not exist, an error message is displayed
```
get <filename> <new filename>
```
Retrieves the specified file from the FAT32 image and places it in the current working directory with the new filename. If the file does not exist, an error message is displayed.

#### put (work in progress)
```
put <filename> 
```
Reads a file from the current working directory and places it in the memory-resident FAT32 image. If the file does not exist, an error message is displayed.
```
put <filename> <new filename>
```
Reads a file from the current working directory and places it in the memory-resident FAT32 image using the new filename. If the file does not exist, an error message is displayed.


#### cd
```
cd <directory>
```
Changes the current working directory to the specified directory. Supports relative and absolute paths.

#### ls
```
ls
```
Lists the contents of the current directory, excluding deleted files or system volume names. Supports listing "." and "..".

#### read
```
read <filename> <position> <number of bytes> <OPTION>
```
Reads from the specified file starting at the given position and outputs the specified number of bytes. By default, values are printed as hexadecimal integers. The following option flags can change the format of the output:

* -ascii: Prints the bytes as ASCII characters.
* -dec: Prints the bytes as decimal integers.

#### del
```
del <filename>
```
Deletes the specified file from the file system.

#### undel
```
undel <filename>
```
Undeletes the specified file from the file system.
