# Welcome to LM-SDSL

# compile instructions

1. Check out reprository
2. `git submodule init`
3. `git submodule update`
4. `cd build`
5. `cmake ..`

# usage

Create a collection:

```
./create-collection.x -i toyfile.txt -c ../collections/toy
```

Build index

```
./build-index.x -c ../collections/toy/
```

Query index

```
./query-index-knm.x -c ../collections/toy/ -p toyquery.txt -n 3
```

# Single CST

To call the single CST method, pass -b:

```
./query-index-knm.x -c ../collections/toy/ -p toyquery.txt -n 3 -b
```

## Running unit tests ##

```
rm -r ../collections/unittest/
./create-collection.x -i ../UnitTestData/data/training.data -c ../collections/unittest
./unit-test.x 
```

