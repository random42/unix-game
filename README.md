## How to run

Compile:

```bash
make
```

Set environment in *.env* file.

```bash
# set env variables
export $(egrep -v '^#' .env | xargs)
# execute
./bin/master
```
