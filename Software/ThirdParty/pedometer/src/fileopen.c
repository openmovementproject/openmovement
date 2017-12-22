// File Open Dialog from a Console Application

#ifdef _WIN32
#include <stdio.h>
#include <windows.h>

const char *GetFilename(const char *initialFilename)
{
	static char szFileName[MAX_PATH];
	static char szFileTitle[MAX_PATH];
	OPENFILENAMEA ofn;

	*szFileName = '\0';
	*szFileTitle = '\0';

	if (initialFilename != NULL) { strcpy_s(szFileName, MAX_PATH, initialFilename); }

	ofn.lStructSize       = sizeof(ofn);
	ofn.hwndOwner         = GetConsoleWindow(); // GetFocus();
	ofn.lpstrFilter       = "OMX & CWA Files (.omx, .cwa)\0*.omx;*.cwa\0All Files (*.*)\0*.*\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter    = 0;
	ofn.nFilterIndex      = 0;
	ofn.lpstrFile         = szFileName;
	ofn.nMaxFile          = MAX_PATH;
	ofn.lpstrInitialDir   = ".";
	ofn.lpstrFileTitle    = szFileTitle;
	ofn.nMaxFileTitle     = MAX_PATH;
	ofn.lpstrTitle        = "Open File";
	ofn.lpstrDefExt       = "";
	ofn.Flags             = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	if (!GetOpenFileNameA(&ofn)) { return NULL; }

	return szFileName;
}

#if 0
int main(int argc, char *argv[])
{
	const char *filename = NULL;

	// Use command-line filename if specified (e.g. if drag-and-drop to executable)
	if (argc > 1) { filename = argv[1]; }

	// If not specified, use file open dialog
#ifdef _WIN32
	if (filename == NULL)
	{
		filename = GetFilename(NULL);
	}
#endif

	// Error if no file specified
	if (filename == NULL)
	{
		fprintf(stderr, "ERROR: File not specified\r\n.");
		return -1;
	}

	// TODO: Do something with the file
	fprintf(stderr, "FILE: %s\r\n", filename);

	return 0;
}
#endif

#endif
