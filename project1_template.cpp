//============================================================================
// Name        : FileSystem53.cpp
// Author      : Leonard Bejosano, Melody Truong, TC Nguyen, Steven Zhang
// Version     : 0.1.2.1.2a
// Copyright   : Your copyright notice
// Description : First Project Lab
//============================================================================

#include <iostream>
#include <string>
#include <fstream>
#include <iostream>
using namespace std;

class FileSystem53 {

    int B;  //Block length
	int K;  //Number of blocks for descriptor table
	char** ldisk;
	char** desc_table;  // Descriptor Table (in memory).
    // This is aka cache. It's contents should be
    // maintained to be same as first K blocks in disk.
	// Descriptor table format:
	// +-------------------------------------+
	// | bitmap | dsc_0 | dsc_1 | .. | dsc_N |
	// +-------------------------------------+
	//   bitmap: Each bit represent a block in a disk. MAX_BLOCK_NO/8 bytes
	//   dsc_0 : Root directory descriptor
	//   dsc_i : i'th descriptor. Each descriptor is FILE_SIZE_FIELD+ARRAY_SIZE bytes long.

	// Filesystem format parameters:
	int FILE_SIZE_FIELD = 1;     // Size of file size field in bytes. Maximum file size allowed in this file system is 192.
	int ARRAY_SIZE = 3;          // The length of array of disk block numbers that hold the file contents.
	int DESCR_SIZE = FILE_SIZE_FIELD+ARRAY_SIZE;
	int MAX_FILE_NO = 14;        // Maximum number of files which can be stored by this file system.
	int MAX_BLOCK_NO = 64;       // Maximum number of blocks which can be supported by this file system.
	int MAX_BLOCK_NO_DIV8 = MAX_BLOCK_NO/8;
	int MAX_FILE_NAME_LEN = 32;  // Maximum size of file name in byte.
	int MAX_OPEN_FILE = 3;       // Maximum number of files to open at the same time.
	int FILEIO_BUFFER_SIZE = 64; // Size of file io bufer
	int _EOF= -1;       // End-of-File

	struct desc_element{
		int blocks [ARRAY_SIZE];
		int size;

		desc_element(){
			blocks = {0,0,0};
			size = 0;
		}

	};

    public:

	/* Constructor of this File system.
		 *   1. Initialize IO system.
		 *   2. Format it if not done.
		 */
		FileSystem53(int l, int b, string storage){
			this.B = b;
			this.K = l;
			file_desc = new char*[K];

			for (int i = 0; i < K; i++){
				file_desc[i] = new char[B];
				for (int j = 0; j < B; j++){
					file_desc[i][j] = 0;
				}
			}

			ldisk = new char*[B-K];
			format();

		}

	// Open File Table(OFT).
	void OpenFileTable();

	// Allocate open file table
	int find_oft();

	//Deallocate
	void deallocate_oft(int index);

	/* Format file system.
	 *   1. Initialize the first K blocks with zeros.
	 *   2. Create root directory descriptor for directory file.
	 * Parameter(s):
	 *   none
	 * Return:
	 *   none
	 */
	void format(){

	//Formats data such that characters are appended with a null character to signify end of string.
	
		for (int i = 0; i < L; i++){
			file_desc[i] = new char[B];
			for (int j = 0; j < B; j++){
				file_desc[i][j] = 0;
			}
		}	
		
		for (int i = 0; i < B-L; i++){
			ldisk[i] = new char[B];
			for (int j = 0; j < B; j++){
				ldisk[i][j] = 0;
			}
		}	



	}


	/* Read descriptor
	 * Parameter(s):
	 *    no: Descriptor number to read
	 * Return:
	 *    Return char[4] of descriptor
	 */
	char* read_descriptor(int no);


	/* Clear descriptor
	 *   1. Clear descriptor entry
	 *   2. Clear bitmap
	 *   3. Write back to disk
	 * Parameter(s):
	 *    no: Descriptor number to clear
	 * Return:
	 *    none
	 */
	void clear_descriptor(int no);


	/* Write descriptor
	 *   1. Update descriptor entry
	 *   2. Mark bitmap
	 *   3. Write back to disk
	 * Parameter(s):
	 *    no: Descriptor number to write
	 *    desc: descriptor to write
	 * Return:
	 *    none
	 */
	void write_descriptor(int no, char* desc);


	/* Search for an unoccupied descriptor.
	 * If ARRAY[0] is zero, this descriptor is not occupied.
	 * Then it returns descriptor number.
	 */
	int find_empty_descriptor();


	/* Search for an unoccupied block.
	 *   This returns the first unoccupied block in bitmap field.
	 *   Return value -1 means all blocks are occupied.
	 * Parameter(s):
	 *    none
	 * Return:
	 *    Returns the block number
	 *    -1 if not found
	 */
	int find_empty_block();


	/* Get one character.
	 *    Returns the character currently pointed by the internal file position
	 *    indicator of the specified stream. The internal file position indicator
	 *    is then advanced to the next character.
	 * Parameter(s):
	 *    index (descriptor number of the file to be added.)
	 * Return:
	 *    On success, the character is returned.
	 *    If a writing error occurs, EOF is returned.
	 */
	int fgetc(int index);


	/* Put one character.
	 *    Writes a character to the stream and advances the position indicator.
	 *    The character is written at the position indicated by the internal position
	 *    indicator of the file, which is then automatically advanced by one.
	 * Parameter(s):
	 *    c: character to write
	 *    index (descriptor number of the file to be added.)
	 * Return:
	 *    On success, the character written is returned.
	 *    If a writing error occurs, -2 is returned.
	 */
	int fputc(int c, int index);


	/* Check for the end of file.
	 * Parameter(s):
	 *    index (descriptor number of the file to be added.)
	 * Return:
	 *    Return true if end-of-file reached.
	 */
	bool feof(int index);


	/* Search for a file
	 * Parameter(s):
	 *    index: index of open file table
	 *    st: The name of file to search.
	 * Return:
	 *    index: An integer number position of found file entry.
	 *    Return -1 if not found.
	 */
	int search_dir(int index, string symbolic_file_name);


	/* Clear a file entry from directory file
	 *
	 * Parameter(s):
	 *    index: open file table index
	 *    start_pos:
	 *    length:
	 * Return:
	 *    none
	 */
	void delete_dir(int index, int start_pos, int len);


	/* File creation function:
	 *    1. creates empty file with file size zero.
	 *    2. makes/allocates descriptor.
	 *    3. updates directory file.
	 * Parameter(s):
	 *    symbolic_file_name: The name of file to create.
	 * Return:
	 *    Return 0 for successful creation.
	 *    Return -1 for error (no space in disk)
	 *    Return -2 for error (for duplication)
	 */
	int create(string symbolic_file_name);


	/* Open file with descriptor number function:
	 * Parameter(s):
	 *    desc_no: descriptor number
	 * Return:
	 *    index: index if open file table if successfully allocated.
	 *    Return -1 for error.
	 */
	int open_desc(int desc_no){
		if (file_no > 13){
			return -1;
		}
		for (int i = 0; i < 3; i++){
			if (openFileTable[i].isEmpty()){
				openFileTable[i].currentPosition = 0;
				openFileTable[i].fileDescriptorIndex = file_no;
				iosystem->read_block(i, 0);
				for (int j = 0; j < 64; j++){
					openFileTable[i].bufferReader[j] = iosystem->getCurrentBlock()[j];
				}
				return file_no;
			}
		}
		return -1;
	}


	/* Open file with file name function:
	 * Parameter(s):
	 *    symbolic_file_name: The name of file to open.
	 * Return:
	 *    index: An integer number, which is a index number of open file table.
	 *    Return -1 or -2 if it cannot be open.
	 */
	// TODOs:
			// 1. Open directory file
			// 2. Search for a file with given name
			//    Return -1 if not found.
			// 3. Get descriptor number of the found file
			// 4. Looking for unoccupied entry in open file table.
			//    Return -2 if all entry are occupied.
			// 5. Initialize the entry (descriptor number, current position, etc.)
			// 6. Return entry number
	int open(string symbolic_file_name){
		for (int i = 0; i < 14; i++){
			bool match = true;
			for (int j = 0; symbolicName[j] != 0; j++){		
				if (dir[i].symbolic_file_name[j] != symbolicName[j]){
					match = false;
					break;
				}
			}
			if (match){
				bool spaceAvailable = false;
				for (int j = 0; j < 3; j++){
					if (openFileTable[j].isEmpty()){
						openFileTable[j].currentPosition = 0;
						openFileTable[j].fileDescriptorIndex = i;
						iosystem->read_block(i, 0);
						for (int k = 0; k < 64; k++){
							openFileTable[j].bufferReader[k] = iosystem->getCurrentBlock()[k];
						}
						spaceAvailable = true;
					}
				}
				if (!spaceAvailable){
					return -1;
				}
				return i;
			}
		}
		return -1;
	}


	/* File Read function:
	 *    This reads a number of bytes from the the file indicated by index.
	 *    Reading should start from the point pointed by current position of the file.
	 *    Current position should be updated accordingly after read.
	 * Parameter(s):
	 *    index: File index which indicates the file to be read.
	 *    mem_area: buffer to be returned
	 *    count: number of byte(s) to read
	 * Return:
	 *    Actual number of bytes returned in mem_area[].
	 *    -1 value for error case "File hasn't been open"
	 *    -2 value for error case "End-of-file"
	 TODOs:
			 1. Read the open file table using index.
			    1.1 Get the file descriptor number and the current position.
			    1.2 Can't get proper file descriptor, return -1.
			 2. Read the file descriptor
			    2.1 Get file size and block array.
			 3. Read 'count' byte(s) from file and store in mem_area[].
			    3.1 If current position crosses block boundary, call read_block() to read the next block.
			    3.2 If current position==file size, stop reading and return.
			    3.3 If this is called when current position==file size, return -2.
			    3.4 If count > mem_area size, only size of mem_area should be read.
			    3.5 Returns actual number of bytes read from file.
			    3.6 Update current position so that next read() can be done from the first byte haven't-been-read.
    */
	int read(int index, char* mem_area, int count);


	/* File Write function:
	 *    This writes 'count' number of 'value'(s) to the file indicated by index.
	 *    Writing should start from the point pointed by current position of the file.
	 *    Current position should be updated accordingly.
	 * Parameter(s):
	 *    index: File index which indicates the file to be read.
	 *    value: a character to be written.
	 *    count: Number of repetition.
	 * Return:
	 *    >0 for successful write
	 *    -1 value for error case "File hasn't been open"
	 *    -2 for error case "Maximum file size reached" (not implemented.)
	 */
	int write(int index, char value, int count);


	/* Setting new read/write position function:
	 * Parameter(s):
	 *    index: File index which indicates the file to be read.
	 *    pos: New position in the file. If pos is bigger than file size, set pos to file size.
	 * Return:
	 *    0 for successful write
	 *    -1 value for error case "File hasn't been open"
	 */
	int lseek(int index, int pos);


	/* Close file function:
	 * Parameter(s):
	 *    index: The index of open file table
	 * Return:
	 *    none
	 */
	void close(int index){
		if (openFileTable[index].isEmpty()){
			return;
		}
		openFileTable[index] = OFT();
	}


	/* Delete file function:
	 *    Delete a file
	 * Parameter(s):
	 *    symbolic_file_name: a file name to be deleted.
	 * Return:
	 *    Return 0 with success
	 *    Return -1 with error (ie. No such file).
	 */
	int deleteFile(string fileName);


	/* Directory listing function:
	 *    List the name and size of files in the directory. (We have only one directory in this project.)
	 *    Example of format:
	 *       abc 66 bytes, xyz 22 bytes
	 * Parameter(s):
	 *    None
	 * Return:
	 *    None
	 */
	void directory(){
		for (int i=0; i < 14; i++){
			dirEntry entry = dir[i];
			if (&entry){
				char* fileDesc = new char[64];
				iosystem->read_block(entry.indexForDesc, fileDesc);
				std::cout << entry.symbolic_file_name << " " << fileDesc[0] << std::endl;
			}
		}
	}

	/*------------------------------------------------------------------
	  Disk management functions.
	  These functions are not really a part of file system.
	  They are provided for convenience in this emulated file system.
	  ------------------------------------------------------------------
	 Restores the saved disk image in a file to the array.
	 */
	void restore(){

		string line;
		std::ifstream saveFile("saveFile.txt");
		if (saveFile.is_open()){
			while (getline(saveFile, line)){
				std::stringstream ss;
				ss << line;
				int blockNumber;
				std::string content;
				ss >> blockNumber;
				ss >> content;

				if (blockNumber < L){
					std::strcpy(file_desc[blockNumber], content.c_str());
				}
				else{
					std::strcpy(ldisk[blockNumber - L], content.c_str());
				}	
			}
			saveFile.close();
		}
		else{
			std::cout << "It brokedededed" << std::endl;
		}

	}

	// Saves the array to a file as a disk image.
	void save(){

		std::ofstream saveFile("saveFile.txt");
		for (int i = 0; i < L; i++){
			if (file_desc[i] == nullptr){
				continue;
			}
			saveFile << i << " ";
			for (int j = 0; j < B; j++){
				saveFile << file_desc[i][j];
				if (file_desc[i][j] == 0){
					saveFile << std::endl;
					break;
				}
			}
		}

		for (int i = 0; i < B - L; i++){
			if (ldisk[i] == nullptr){
				continue;
			}
			saveFile << (i + L) << " ";
			for (int j = 0; j < B; j++){
				saveFile << ldisk[i][j];
				if (ldisk[i][j] == 0){
					saveFile << std::endl;
					break;
				}
			}
		}
		saveFile.close();

	}

	// Disk dump, from block 'start' to 'start+size-1'.
	void diskdump(int start, int size);

	// 
	void read_block(int i, char* p)
	{	
		for (int b = 0; b < B; b++)
		{
			char c = ldisk[i][b];
			p[b] = c;
		}
	}

	void write_block(int i, char* p)
	{	
		for (int b = 0; b < B; b++)
		{
			char c = p[b];
			ldisk[i][b] = c;
		}
	}

};