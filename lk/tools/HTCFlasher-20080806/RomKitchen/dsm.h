struct FileEntry {
  unsigned long NameLength;
  unsigned long Flags;
  unsigned long Offset;
  unsigned long Base; // Base address this file was originally linked with.
  unsigned long Size;
};
struct FileEntry FileEntry;

struct DeviceManifestHeader
{
  unsigned long StructSize; // Size of this structure (in bytes)
  unsigned long PackageFlags; // package specific identifiers.
  unsigned long ProcessorID; // what processor (matches defines in winnt.h)
  unsigned long OSVersion; // what version of the operating system was this built to.
  unsigned long PlatformID; // what was the target platform.
  unsigned long NameLength; // length of filename in bytes.
  unsigned long NameOffset; // offset to Friendly name of package
  unsigned long DependentCount; // How many entries in Dependent GUID list.
  unsigned long DependentOffset; // How many bytes from the front of the file are the dependent GUID structs.
  unsigned long ShadowCount; // How many entries in shadow GUID list.
  unsigned long ShadowOffset; // How many bytes from front of file is the array of shadowed package GUIDs.
  unsigned long FileCount; // How many files are there listed in this manifest.
  unsigned long FileListOffset; // How many bytes from the front of file to the first FileEntry.
  unsigned long CERTData; // number of bytes of digital certificate data
  unsigned long CERTDataOffset; // How many bytes from the front of file to the certificate data.
  unsigned short PackageVersion1; // Version of this package
  unsigned short PackageVersion2; // Version of this package
  unsigned short PackageVersion4; // Version of this package
  unsigned short PackageVersion3; // Version of this package
  unsigned short PrevPkgVersion1; // Version of package that this package updates. (0) for Canonical
  unsigned short PrevPkgVersion2; // Version of package that this package updates. (0) for Canonical
  unsigned short PrevPkgVersion4; // Version of package that this package updates. (0) for Canonical
  unsigned short PrevPkgVersion3; // Version of package that this package updates. (0) for Canonical
  char unknown[8];
  char guidPackage[16]; // GUID of this package
  char unknown2[16];
};
struct DeviceManifestHeader DeviceManifestHeader;
