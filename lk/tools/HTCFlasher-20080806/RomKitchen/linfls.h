#define FLASH_LAYOUT_SIG_SIZE 8

typedef enum  _REGION_TYPE { XIP, READONLY_FILESYS, FILESYS } REGION_TYPE;

struct FlashRegion
{
    REGION_TYPE regionType;
    unsigned long       dwStartPhysBlock;
    unsigned long       dwNumPhysBlocks;
    unsigned long       dwNumLogicalBlocks;
    unsigned long       dwSectorsPerBlock;
    unsigned long       dwBytesPerBlock;
    unsigned long       dwCompactBlocks;

};
struct FlashRegion FlashRegion;

struct FlashLayoutSector {
    // Signiture to identify this sector
    char abFLSSig[FLASH_LAYOUT_SIG_SIZE];
    // Size in bytes of reserved entries array
    unsigned long cbReservedEntries;
    // Size in bytes of region array
    unsigned long cbRegionEntries;
    // ReservedEntry[] and FlashRegion[] are located
    // immediately after this struct.
};
struct FlashLayoutSector FlashLayoutSector;

// Partition Table
// end of master boot record contains 4 partition entries
struct PARTENTRY {
        char            Part_BootInd;           // If 80h means this is boot partition
        char            Part_FirstHead;         // Partition starting head based 0
        char            Part_FirstSector;       // Partition starting sector based 1
        char            Part_FirstTrack;        // Partition starting track based 0
        char            Part_FileSystem;        // Partition type signature field
        char            Part_LastHead;          // Partition ending head based 0
        char            Part_LastSector;        // Partition ending sector based 1
        char            Part_LastTrack;         // Partition ending track based 0
        unsigned long   Part_StartSector;       // Logical starting sector based 0
        unsigned long   Part_TotalSectors;      // Total logical sectors in partition
};
struct PARTENTRY PARTENTRY;

