# AX Device Rotation: Axis Inversions

Where `*` denotes the thinner, slightly tapered end of the puck.

## AX6 Accel Axes and Gyro Rotation

```
+---+-----------------------+
|   |                  y    |
|   |                  ^    |
|   |   ^              |    |
| * |  |-| \/         .-->  |
|   |  | | /\     ^  ' |    |
|   |        x <--|---(+)z| |
|   |             \_   <-'  |
+---+-----------------------+
```


## Axis at no rotation

```
+---+-----------------------+
|   |                  y    |
|   |   ^              ^    |
| * |  |-| \/          |    |
|   |  | | /\    x <--(+) z |
|   |                       |
+---+-----------------------+
```

Transform: (identity).


## Rotated 180 degrees around Y-axis: "flipped on short side"

```
+-----------------------+---+
|    y                  |   |
|    ^              '   |   |
|    |          '' :':  | * |
| z (.)--> x    .. : :  |   |
|                       |   |
+-----------------------+---+
```

Transform: *x'* = -*x*, *z'* = -*z*.


## Rotated 180 degrees around X-axis: "flipped on long side"

```
+---+-----------------------+
|   |                       |
|   |  : : ''    x <--(.) z |
| * |  :': ..          |    |
|   |   .              v    |
|   |                  y    |
+---+-----------------------+
```

Transform: *y'* = -*y*, *z'* = -*z*.



## Rotated 180 degrees around both X-axis and Y-axis: "rotated 180 degrees rotate Z-axis", "flipped on long and short sides"

```
+-----------------------+---+
|                       |   |
| z (+)--> x    \/ | |  |   |
|    |          /\ |-|  | * |
|    v              V   |   |
|    y                  |   |
+-----------------------+---+
```

Transform: *x'* = -*x*, *y'* = -*y*.


## Making the transform on .WAV files

Use the UI to generate a resampled .WAV file for your .CWA file by performing one of the analysis steps. 

Install "Audacity": http://www.audacityteam.org/

Run "Audacity". 

Select: "Edit" / "Preferences" / "Import/Export" / "When exporting tracks to an audio file" / "Use custom mix (for example to export a 5.1 multichannel file)", "OK". 

Select: "Edit" / "Preferences" / "Quality" / "Sampling" / "Default Sample Format" / "16-bit". 

Select: "File" / "Open" / select the .wav file / "Open". 

Perform one or more of these actions:
- for x' = -x: Click on "Left"  under "sample 1" / "Effect" / "Invert".
- for y' = -y: Click on "Right" under "sample 2" / "Effect" / "Invert".
- for z' = -z: Click on "Mono"  under "sample 3" / "Effect" / "Invert".

Select: "File" / "Export Audio" / select the original .wav file / "Save" / "OK" / "OK". 

Use the UI to perform one of the analysis steps again, the inverted-axis .WAV file will be used.  


