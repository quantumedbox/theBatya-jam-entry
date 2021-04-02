#include <stdio.h>
#include <string.h>

#include <windows.h>

// This modules should be a separate dll
// Main should check, was this module modified or not
// Server should demand specification of this module

typedef enum machineError {
	MACHINE_OUT_OF_MEMORY,
	MACHINE_INVALID_DATA,
}
MachineError_T;

typedef struct
{
    BYTE    Used20CallingMethod;
    BYTE    SMBIOSMajorVersion;
    BYTE    SMBIOSMinorVersion;
    BYTE    DmiRevision;
   DWORD    Length;
    BYTE    SMBIOSTableData[];
}
RawSMBIOSData;

DWORD MAKEDWORD(BYTE bytes[5])
{
	DWORD word;
	word |= bytes[0] << 24;
	word |= bytes[1] << 16;
	word |= bytes[2] << 8;
	word |= bytes[3];
	return word;
}

int byteStringLen(BYTE* str)
{
	int len = 0;
	while ((*str) != '\0') ++len;
	return len;
}

void exitWithCode(MachineError_T error)
{
	printf("Error while getting machine UUID\n");
	switch(error) {
	case MACHINE_OUT_OF_MEMORY:
		printf("Cannot allocate buffer\n");
	case MACHINE_INVALID_DATA:
		printf("Cannot get the data\n");
	}
	exit(error);
}

RawSMBIOSData* getSMBIOSData()
{
	BYTE s[] = "RSMB";
	DWORD RSMB = MAKEDWORD(s);

	DWORD smBiosDataSize = 0;
	RawSMBIOSData* smBiosData = NULL;
	DWORD bytesWrittten = 0;

	smBiosDataSize = GetSystemFirmwareTable(RSMB, 0, NULL, 0);

	smBiosData = (RawSMBIOSData*)HeapAlloc(GetProcessHeap(), 0, smBiosDataSize);
	if (!smBiosData)
		exitWithCode(MACHINE_OUT_OF_MEMORY);

	bytesWrittten = GetSystemFirmwareTable(RSMB, 0, smBiosData, smBiosDataSize);
	if (bytesWrittten != smBiosDataSize)
		exitWithCode(MACHINE_INVALID_DATA);

	printf("got %d bytes of BIOS data\n", smBiosData->Length);

	return smBiosData;
}

// TODO
void printSMBIOSData(RawSMBIOSData* biosData)
{
	for (int i = 0; i < biosData->Length; i++)
	{
		if (i < biosData->Length-1 && 			 \
			biosData->SMBIOSTableData[i] == 0 && \
			biosData->SMBIOSTableData[i+1] == 0)
		{
			i += biosData->SMBIOSTableData[i+3];
			while (i < biosData->Length-1 && biosData->SMBIOSTableData[i+1] != 0)
			{
				printf("inedx: %d", i);
				printf("%s\n", biosData->SMBIOSTableData[i]);
				i += byteStringLen(&biosData->SMBIOSTableData[i]);
			}
		}
	}
}

int main()
{
	RawSMBIOSData* data = getSMBIOSData();
	printSMBIOSData(data);
	return 0;
}
