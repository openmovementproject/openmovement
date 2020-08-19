# AX3/AX6 FAQ

## Maximum recording duration

Approximate maximum recording duration from a single full battery charge.

| Device  | Configuration  |   Sample Capacity |  6.25 Hz |    12.5 Hz |      25 Hz |    50 Hz |  100 Hz | 200 Hz | 400 Hz | 800 Hz | 1600 Hz | 3200 Hz |
|---------|----------------|------------------:|---------:|-----------:|-----------:|---------:|--------:|-------:|-------:|-------:|--------:|--------:|
| AX3     | packed         | 119009040 samples |        - | 65d (110d) |  48d (55d) |     27d* |  13.5d* |   6.8d |   3.4d |  41.3h |   20.6h |   10.3h |
| AX3     | unpacked       |  79339360 samples | ? (146d) | 65d? (73d) |      36.5d |      18d |     9d* |   4.5d |   2.3d |  27.5h |   13.7h |    6.8h |
| AX6     | accel. only    | 158714880 samples | ? (293d) |      146d? |        73d |    36.5d |  18.3d* |   9.1d |   4.5d |  55.1h |   27.5h |       - |
| AX6     | accel. + gyro. |  79357440 samples |        - |          - | 9d (36.7d) | 9d (18d) |     9d* |   4.5d |   2.3d |  27.5h |   13.7h |       - |

* `d` days (<= 400 Hz)
* `h` hours (> 800 Hz)
* `-` an unsupported configuration. 
* `?` an unknown value, or uncertainty in the figure where given. 
* `()` where a recording is likely to be battery-limited, times in brackets are the storage capacity (which could only be reached if recharged). 
* `*` where the storage- and battery-limits are similar, which is reached first may be device-dependent. 
