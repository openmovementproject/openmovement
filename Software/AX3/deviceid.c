#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int deviceIdFromFile(const char *filename) {
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL) { return -1; }
	unsigned char buffer[64] = {0};
	int count = fread(buffer, 1, sizeof(buffer), fp);
	if (count < sizeof(buffer) || buffer[0] != 'M' || buffer[1] != 'D') { fclose(fp); return -1; }
	int deviceId = buffer[5] | (buffer[6] << 8);
	int upperDeviceId = buffer[11] | (buffer[12] << 8);
	if (upperDeviceId != 0xffff) { deviceId |= upperDeviceId << 16; }
	fclose(fp);
	if (deviceId == 0 || deviceId == 0xffff) { return -1; } // 0 and 65535 are reserved IDs for "unidentified"
	return deviceId;
}

/*
#define OM_MAX_PATH (256)
#define OM_DEFAULT_FILENAME "CWA-DATA.CWA"
int deviceIdFromFilePath(const char *path) {
	char filename[OM_MAX_PATH];
#ifdef _WIN32
	char sep = '\\';
#else
	char sep = '/';
#endif
	strcpy(filename, path);
	if (filename[strlen(filename) - 1] != sep) {
		sprintf(filename + strlen(filename), "%c", sep);
	}
	sprintf(filename + strlen(filename), "%s", OM_DEFAULT_FILENAME);
	return deviceIdFromFile(filename);
}
*/

int main(int argc, char *argv[]) {
	if (argc < 2 || argc > 2 || argv[1][0] == '-') {
		printf("ERROR: File not specified\n");
		return -1;
	}
	const char *source = argv[1];
	// printf("DEBUG: Reading: %s\n", source);
	int deviceId;
	//deviceId = deviceIdFromFilePath(source);
	deviceId = deviceIdFromFile(source);
	printf("%d\n", deviceId);
	return 0;
}
