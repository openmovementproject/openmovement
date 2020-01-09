# Packed unpacker by Dan Jackson (to be tested!)
numUnpack = function(packedData) {
  count = length(packedData)
  xyz = integer(count * 3)
  for (i in 1:count) {
    # eezzzzzz zzzzyyyy yyyyyyxx xxxxxxxx
    value = packedData[i]

    # Isolate each axis component
    x = bitwAnd(           value     , 0x03ff)
    y = bitwAnd(bitwShiftR(value, 10), 0x03ff)
    z = bitwAnd(bitwShiftR(value, 20), 0x03ff)

    # Signed
    if (x >= 0x200) { x = x - 0x400 }
    if (y >= 0x200) { y = y - 0x400 }
    if (z >= 0x200) { z = z - 0x400 }

    # Scale
    shift = bitwAnd(bitwShiftR(value, 30), 0x03)
    x = bitwShiftL(x, shift)
    y = bitwShiftL(y, shift)
    z = bitwShiftL(z, shift)

    # Store
    xyz[(i - 1) * 3 + 1] = x
    xyz[(i - 1) * 3 + 2] = y
    xyz[(i - 1) * 3 + 3] = z
  }
  data = matrix(xyz, ncol=3, byrow=T)
  data
}


# --- Test code ---
filename <- "D:/temp/test.cwa"

fid <- file(filename, "rb")
readChar(fid, 1024, useBytes = TRUE) # skip file header

readChar(fid, 30, useBytes = TRUE)	 # skip block header
packedData <- readBin(fid, integer(), size = 4, endian = "little", n = 120)
readChar(fid, 2, useBytes = TRUE)	 # skip checksum

data = numUnpack(packedData)
print(data[1:120,])

close(fid)

