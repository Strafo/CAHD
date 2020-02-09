# Correlation-Aware Anonymization of High-Dimensional Data 

C++ implementation of the paper _On the Anonymization of Sparse High-Dimensional Data_ https://www.researchgate.net/publication/4330924_On_the_Anonymization_of_Sparse_High-Dimensional_Data

**Usage**

Download release binary from https://github.com/Strafo/CAHD/releases. (ELF 64-bit LSB executable, x86-64)

```
$./Cahd --help
  Allowed options:
    -h [ --help ]               produce help message
    -i [ --input ] arg          name of the input file
    -o [ --output ] arg         name of the anonymized output file
    -p [ --privacy ] arg        privacy degree
    -a [ --alpha ] arg          search width parameter
    -s [ --sensitiveitems ] arg sensitive items separated by -
    -v [ --verbose ]            display additional information
    -d [ --debug ]              display debug information

```

**Example**

With the binary located in the parent folder of the CAD project:
```
.
├── Cahd (bianary file)
├── BMS1formatted100.csv
```

The command:

```
$ ./Cahd -i BMS1formatted1000.csv -p 4 -a 3 -s 10877-18423  -v -d

```
will print to the console some debug
