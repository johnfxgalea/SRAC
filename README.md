# SRAC
SRAC - a Simple Return Address Checker ![SRAC](SRACLOGO.png)

## Building Test Example

To build:
```
cd test
mkdir build
cd build
cmake ..
make
```

To run:
```
./test AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
```

## Building SRAC - QBDI

To build:
```
cd SRAC-QBDI
mkdir build
cd build
cmake ..
make
```

To run:
```
LD_PRELOAD=PATH/TO/libQBDI.so ./srac AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
```

## Building SRAC - QBDI - FRIDA


