# Welcome to LM-SDSL
This is our implementation used in the following paper:

```
@inproceedings{shareghicompact,
  author={Shareghi, Ehsan and Petri, Matthias and Haffari, Gholamreza and Cohn, Trevor},
  title={Compact, Efficient and Unlimited Capacity: Language Modeling with Compressed Suffix Trees},
  booktitle={Proceedings of the 2015 Conference on Empirical Methods in Natural Language Processing, 2015, September 19-21, 2015, Lisbon, Portugal},
  year={2015},
}
```

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

