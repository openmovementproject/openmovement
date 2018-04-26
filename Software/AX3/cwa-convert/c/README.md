# cwa-convert - CWA Data Conversion

The conversion program "exports" raw accelerometer data from the CWA file 
(which has a naturally, slightly-variable rate from the underlying sensor), 
and can be run as follows:

```bash
cwa-convert "filename.cwa" -out "filename.temp.csv"
```

You should replace `cwa-convert` with the full path to the executable file on your system:

* If you are on Windows you can obtain a pre-built executable from [AX3-Utils-Win-3.zip](https://github.com/digitalinteraction/openmovement/blob/master/Downloads/AX3/AX3-Utils-Win-3.zip?raw=true), 
and if you have the [AX3 software](https://github.com/digitalinteraction/openmovement/wiki/AX3-GUI#downloading-and-installing), you should already have the executable at:

```cmd
"%ProgramFiles(x86)%\Open Movement\OM GUI\Plugins\Convert_CWA\cwa-convert.exe"
```

* If you are on Linux, Mac or Windows Subsystem for Linux, you can build the binary yourself with these shell commands (XCode required on Mac):

```bash
wget -P cwa-convert.build https://github.com/digitalinteraction/openmovement/raw/master/Software/AX3/cwa-convert/c/{main.c,cwa.{c,h}}
cd cwa-convert.build && gcc main.c cwa.c -lm -O2 -o ../cwa-convert && cd -
```


A list of other command-line options is available with:

```bash
cwa-convert --help
```

If you would prefer to deal with precisely-timed, re-sampled data, see the [omconvert](https://github.com/digitalinteraction/openmovement/blob/master/Software/AX3/omconvert/README.md) program.
