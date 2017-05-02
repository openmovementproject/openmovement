# AX3 Rotation: Axis Inversions


## No rotation

```
+----+-----------------------+
|    |                  y    |
| /| |   ^              ^    |
| || |  |-| \/          |    |
| \| |  | | /\    x <--(+) z |
|    |                       |
+----+-----------------------+
```

Transform: (identity).


## Rotated 180 degrees around Y-axis: "flipped on short side"

```
+-----------------------+----+
|    y                  |    |
|    ^              '   | /| |
|    |          '' :':  | || |
| z (.)--> x    .. : :  | \| |
|                       |    |
+-----------------------+----+
```

Transform: *x'* = -*x*, *z'* = -*z*.


## Rotated 180 degrees around X-axis: "flipped on long side"

```
+----+-----------------------+
|    |                       |
| |\ |  : : ''    x <--(.) z |
| || |  :': ..          |    |
| |/ |   .              v    |
|    |                  y    |
+----+-----------------------+
```

Transform: *y'* = -*y*, *z'* = -*z*.



## Rotated 180 degrees around both X-axis and Y-axis: "rotated 180 degrees rotate Z-axis", "flipped on long and short sides"

```
+-----------------------+----+
|                       |    |
| z (+)--> x    \/ | |  | |\ |
|    |          /\ |-|  | || |
|    v              V   | |/ |
|    y                  |    |
+-----------------------+----+
```

Transform: *x'* = -*x*, *y'* = -*y*.

