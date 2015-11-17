# Find which tx inputs and ouputs are mine

How to develop on top of [Monero](https://getmonero.org/)? One way is to use json-rpc calls from
any language capable of this, for example, as
[shown in python](http://moneroexamples.github.io/python-json-rpc/). 

# Aim: check which transaction's outputs belong to a given address



# Pre-requsits




# C++ code
The two most interesting C++ files in this example are `MicroCore.cpp` and `main.cpp`.
Therefore, I will present only these to files here. Full source code is
at [github](https://github.com/moneroexamples/access-blockchain-in-cpp). The surfce code can
also slighly vary with the code here, as it can be updated more frequently than 
the code presented here. So for the latest version
of this example, please check the github repository directly.

## MicroCore.cpp


```c++

}
```

# Output example 1
Executing the program as follows:

```bash
```

Results in the following output:

```

Total xmr received: 4.800000000000
```
These results agree with those obtained using [XMR test](http://xmrtests.llcoins.net/checktx.html).

# Output example 2

Executing the program as follows:

```bash
./xmreg01 --address 41vEA7Ye8Bpeda6g59v5t46koWrVn2PNgEKgzquJjmiKCFTsh9gajr8J3pad49rqu581TAtFGCH9CYTCkYrCpuWUG9GkgeB --viewkey fed77158ec692fe9eb951f6aeb22c3bda16fe8926c1aac13a5651a9c27f34309 --txhash ba807a90792f9202638e7288eff05949ccffbc54fd6a108571b65b963fee573a
```
Results in the following output:

```
```

These results agree also with those obtained using [XMR test](http://xmrtests.llcoins.net/checktx.html).

#Output example 3
Executing the program as follows:
```bash
```
Results in the following output:

```bash
```

These results also agree with those obtained using [XMR test](http://xmrtests.llcoins.net/checktx.html).

#Output example 4
Executing the program as follows:

```bash
./xmreg01 --address 41vEA7Ye8Bpeda6g59v5t46koWrVn2PNgEKgzquJjmiKCFTsh9gajr8J3pad49rqu581TAtFGCH9CYTCkYrCpuWUG9GkgeB --viewkey fed77158ec692fe9eb951f6aeb22c3bda16fe8926c1aac13a5651a9c27f34309 --txhash 60b6c42f1a3bea6ecf2adb8a4f98753be5a0a3e032a98beb7bdc44a325cea7e6
```

Results in the following output:

```bash
```
These results agree also with those obtained using [XMR test](http://xmrtests.llcoins.net/checktx.html).

## Compile this example
The dependencies are same as those for Monero, so I assume Monero compiles
correctly. If so then to download and compile this example, the following
steps can be executed:

```bash
# download the source code
git clone https://github.com/moneroexamples/finding-mine-tx-ins-and-outs.git

# enter the downloaded sourced code folder
cd finding-mine-tx-ins-and-outs

# create the makefile
cmake .

# compile
make
```

After this, `tx_ins_and_outs` executable file should be present in access-blockchain-in-cpp
folder. How to use it, can be seen in the above example outputs.


## How can you help?

Constructive criticism, code and website edits are always good. They can be made through github.

Some Monero are also welcome:
```
48daf1rG3hE1Txapcsxh6WXNe9MLNKtu7W7tKTivtSoVLHErYzvdcpea2nSTgGkz66RFP4GKVAsTV14v6G3oddBTHfxP6tU
```
