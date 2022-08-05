# grpc-playground

### build

to build, just do `./build.sh`

if you want to specify just one target to build, you can do one of the following:

```
./build.sh client
./build.sh inner_server
./build.sh outer_server
```

### run

you have to open 3 terminals and do the following:

- terminal 1:

```
./run.sh outer_server
```

- terminal 2:

```
./run.sh inner_server
```

- terminal 3:

```
./run.sh client X
```

where `X` is anumber of threads you'd like to spawn

