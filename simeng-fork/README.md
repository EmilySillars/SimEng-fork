# Emily's SimEng Forked Repo

## Setup

#### Clone the repo and checkout the correct branch

1. ```
   git clone https://github.com/EmilySillars/SimEng-fork.git
   ```

2. ```
   cd SimEng-fork
   ```

3. ```
   git checkout sme-bsc-contest
   ```

## What Next?

QUESTION: WHAT VERSION OF LLVM is my pistachio build????

### Build SimEng

(based on [these instructions](https://uob-hpc.github.io/SimEng/user/building_simeng.html#building))

0. Make sure you know the location of your prebuilt LLVM libraries so you can set the `-DLLVM_DIR` flag correctly; it will look something like `-DLLVM_DIR=/path-to-llvm-build/lib/cmake/llvm`

1. ```
   cd SimEng-fork
   cmake -G Ninja -B build -S . \
           -DCMAKE_BUILD_TYPE=RelWithDebInfo \
           -DSIMENG_USE_EXTERNAL_LLVM=ON \
           -DLLVM_DIR=/home/hoppip/llvm-project-pistachio/build/lib/cmake/llvm .
   ```

2. ```
   cmake --build build -j8
   ```

3. ```
   cmake --build build --target test
   ```

4. ```
   cmake --build build --target install
   ```

### Run a binary on hardware simulated by SimEng

- Run default program on default hardware: /usr/local/bin/simeng



## Investigate hand written matmul

- I need arm compiler to run the matmul on my simeng build: https://developer.arm.com/Tools%20and%20Software/Arm%20Compiler%20for%20Linux#Downloads

- ```
  bash <(curl -L https://developer.arm.com/-/cdn-downloads/permalink/Arm-Compiler-for-Linux/Package/install.sh)
  ```

- looks like I need ubuntu!!

- ```
  Please choose the operating system on which you want to install Arm Compiler for Linux.
      1. AmazonLinux-2
      2. AmazonLinux-2023
      3. RHEL-7
      4. RHEL-8
      5. RHEL-9
      6. SLES-15
      7. Ubuntu-20.04
      8. Ubuntu-22.04
  ```

- 

https://github.com/UoB-HPC/SME_Evaluation_Resources/blob/main/SME_matmul_fp32/matmul.c

More documentation: https://uob-hpc.github.io/SimEng/user/configuring_simeng.html

Questions I have:

- how do I know which hex value represents which instruction + its operands? Is this in the manual somewhere?

## Troubleshooting

1. ```
   [hoppip@inf-205-98 SimEng-fork]$ cmake --build build --target test
   ninja: error: unknown target 'test', did you mean 'help'?
   ```

2. ```
   [hoppip@inf-205-98 SimEng-fork]$ cmake --build build --target install
   [0/1] Install the project...
   -- Install configuration: "RelWithDebInfo"
   -- Installing: /usr/local/include/simeng
   CMake Error at cmake_install.cmake:46 (file):
     file INSTALL cannot make directory "/usr/local/include/simeng": Permission
     denied.
   
   
   FAILED: CMakeFiles/install.util 
   cd /home/hoppip/SimEng-fork/build && /usr/bin/cmake -P cmake_install.cmake
   ninja: build stopped: subcommand failed.
   ```

   Maybe a solution?
   ```
   sudo cmake --build build --target install
   ```

   This is kind of weird now though, because to run simeng, I have to access the executable from my home directory at `/usr/local/bin/simeng`

3. 