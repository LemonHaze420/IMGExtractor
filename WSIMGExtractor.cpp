#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <istream>
#include <ostream>
#include <fstream>
#include <vector>

#include <Windows.h>

#define APP_TITLE   "IMG Extractor"
#define APP_VER		"v1.0"
#define FOURCC(d,c,b,a)		(a << 24 | b << 16 | c << 8 | d)

struct IMGHeader {
	int identifier;		// LINR
	int headerSize;		// size of the total TOC
	int numEntries;		// number of entries within the IMG data
	int xxxx;			// reserved / unknown value
};

struct IMGEntry {
	char filename[0x10];	// filename
	int contentSize;		// actual content size
	int unknownValue;		// usually either 0x8000 or 0x4000
};

void PrintUsage()
{
	printf("\n\n\t.\IMGExtractor.exe <IMG file> <output directory>\n");
}

int main(int argc, char ** argp)
{
	printf("%s %s by LemonHaze\n", APP_TITLE, APP_VER);
	if (argc < 2) {
		PrintUsage();
		return -1;
	} else {
#		ifdef _DEBUG
			printf("args; \n\t[0]: %s\n\t[1]: %s\n\n", argp[1], argp[2]);
#		endif

		std::string IMGPath = argp[1];
		std::string IDXPath = IMGPath.substr(0, IMGPath.find_last_of('.')) + ".IDX";

		std::ifstream imgStream(IMGPath, std::ios::binary);
		std::ifstream idxStream(IDXPath, std::ios::binary);
		if (imgStream.good() && idxStream.good()) {
			IMGHeader hdr;
			idxStream.read(reinterpret_cast<char*>(&hdr), sizeof(IMGHeader));

			if (hdr.identifier == FOURCC('L', 'I', 'N', 'R')) 
			{	
				printf("[LINR] Header Size: \t0x%X\n[LINR] Num Entries: \t%d\n", hdr.headerSize, hdr.numEntries);

				// collect all entries from the .IDX file out of the pair..
				std::vector<IMGEntry> entries;
				for (int entryIdx = 1; entryIdx <= hdr.numEntries; ++entryIdx) 
				{
					IMGEntry entry;
					idxStream.read(reinterpret_cast<char*>(&entry), sizeof(IMGEntry));
					entries.push_back(entry);

					printf("[%d/%d] Filename: %s \tSize = 0x%X \tUnknownValue = 0x%X \tCurrFileOffset = 0x%X\n", entryIdx, hdr.numEntries, entry.filename, entry.contentSize, entry.unknownValue, (int)imgStream.tellg());
				}
				idxStream.close();

				// now extract all entries from the .IMG..
				for (auto& entry : entries) 
				{
					std::string path = argp[2];
					path.append("\\");
					path.append(entry.filename);

					std::ofstream outputFile(path, std::ios::binary);
					if (outputFile.good()) {
						char* fileBuffer = new char[entry.contentSize];
						memset(fileBuffer, 0x00, entry.contentSize);
						imgStream.read(fileBuffer, entry.contentSize);

						outputFile.write(fileBuffer, entry.contentSize);
						outputFile.close();
						delete[] fileBuffer;
					}
					else 
						printf("Couldn't open %s for writing. Check paths and/or permissions!\n", path.c_str());
				}
			} else 
				printf("Invalid header: %X\n", hdr.identifier);
		
			imgStream.close();
		}
	}

	return 0;
}