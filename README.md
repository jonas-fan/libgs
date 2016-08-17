# libgs

Simple socket wrapper for Linux platform.

## Build
```
$ git clone https://github.com/yjfan/libgs.git
$ cd libgs/
$ mkdir build/
$ cd build/
$ cmake ..
$ make -j4
```

## Test
```
$ cd libgs/build/tests/
$ ./receiver -b /tmp/uds.ipc
```

```
$ cd libgs/build/tests/
$ ./sender -c /tmp/uds.ipc -m "Hello world!"
```
