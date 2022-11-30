## Entire project documentation and structure description

This project are using **LLVM**, if you know about it, it will be easy for you to understand.<br> 


Project directory explainations:

- CFG(文法)txt: CFG syntax and simple description. The detailed description can be found in the experiment report.


- hpp files

  - **util.hpp** : Header file for the parsing and parsing section that defines the classes and enumeration constants in the parsing and declares the parsing methods.
  - **ParExp1.hpp** : Declares the interface between parsing classes and intermediate code generation and global variables for semantic parsing.
  - **codegen1.hpp** : Code generation header that declares the many classes related to the language structure and the global variables and methods used to generate the intermediate code for **LLVM**.

- cpp files

  - **token.cpp** : Use DFA for lexical analysis.
  - **praser.cpp** : Recurse descendingly CFG parsing, syntax analysis, using LL(1) algorithm.
  - **ParExp1.cpp** : defines the interface between the AST generated by parsing and the classes designed later, and carries out semantic analysis (symbol table + type check).
  - **codegen1.cpp** : Base on **LLVM** to achieve many classes of code generation functions. The main function in this file calls functions in the process of the DFA construction, token reading, syntax tree generation, semantic analysis, conversion to syntax tree of the newly defined class, and intermediate code generation.
  - **codegen_opt.cpp** : On the basis of codegen1.cpp, supplement optimization code to optimize the module code, and part to embed C code for functions like input and output in IR.

- **Makefile** : You can use Makefile to generate an executable file. There are two types of executable files, (NAME) is not optimized and (NAME1) with embedded C functions with 5 optional optimizations (depending on LLVM's design for each optimization pass, the optimizations can be any combination). You can specify the name of the build file by editing the value of (NAME) or (NAME1). Do not run the make command in multiple threads at the same time. (Please refer to Makefile.)

- **input** : "input" is the input file of the default custom syntax (that is, the result of a CFG grammar derivation). The program reads the file and checks for errors. If there are no errors, the intermediate code is generated.

- inputrecord: Optional content and history of "input ".

- **Executable files** :

  - final_ori11: No-optimized executable file. The input file is used as input to output intermediate code or error information to the terminal.
  - final_optwithC11: executable file with optimization (checked optimizations for 4.1 4.2 4.3 in experiment, other optimizations can be re-made by disabling corresponding comments). I embed `printi(__int16_t)` function declaration to print integers, others as the same, please refer to Chapter 5 of experiment report for more.

- test folder

  - testn: indicates the code of the nTH test sample in the experiment report.
  - testn_ori: Put the code in testn into the input file and run the intermediate code output by final_ori11.
  - testn_opt: Put the code in testn into the input file and execute the optimized intermediate code output by final_optwithC11.

- Other Documents

  - printi. c: defines the function `printi`. You can compile with command

    ```bash
    clang -S -emit-llvm printi.c -o printi.ll
    ```

    The readable LLVM bytecode file printi.ll can be generated, and the executable file can be compiled together with the intermediate code generated by this program. (This file is entirely for embedding print functionality.)

    ```bash
    clang printi.ll output.ll -o output
    ```

    The resulting output can be executed and printed.



#### The operation method is explained again here:

Use **make** to get the executable file, edit the file "input", to the input you want to test, execute the program to get the result (errors or generated intermediate code if no errors (optimized or not optimized)). Copy the portion of the terminal output

```bash
; ModuleID = '666 compiling principal PYF YHR'
source_filename = "666 compiling principal PYF YHR"
```

Copy from this line to the end and put it into an empty **".ll"** file.
<br>
Then run it as described above or as specified in the report.
<br>
Because of the mature architecture of LLVM, the generated intermediate code can go on to generate object code, which is supported by both theory and practice.


<br>
The whole project did not establish a new folder, convenient for readers to directly execute, avoid possible mistakes of readers. The executable file can be generated by referring to the content in the **fifth section** of the experiment report.
<br>
<br>

<p align=right> -- Yifei Pang




<br><br>
Translation in Chinese:
<br>

工程目录解析:

- 文法.txt：实现的CFG语法以及简单描述。具体描述可见于实验报告。

- hpp文件：

  - **util.hpp**：词法分析、语法分析部分的头文件，定义了词法语法分析中的类、枚举常量，声明分析方法。
  - **ParExp1.hpp**：声明了语法分析类到中间代码生成的接口以及用于语义分析的全局变量。
  - **codegen1.hpp**：代码生成部分头文件，声明了和语言结构相关的众多类以及用于**LLVM**生成中间代码的全局变量和方法。

- cpp文件：

  - **token.cpp**：使用DFA进行词法分析。
  - **praser.cpp**：递归下降的进行CFG解析、语法分析，使用LL(1)算法。
  - **ParExp1.cpp**：定义了语法分析生成的AST到后续设计的类之间的接口，并且进行语义分析（符号表+类型检查）。
  - **codegen1.cpp**：基于**LLVM**实现了众多类的代码生成函数。main函数在这个文件中，进行DFA构造、token读取、语法树生成、语义分析、转换为新定义类的语法树、中间代码生成、（中间代码优化）所有过程的函数调用。
  - **codegen_opt.cpp**：在codegen1.cpp的基础上设计了优化模块代码以及将C函数（代码）嵌入的模块（即生成的IR可以调用C代码进行包括输入输出的操作）。

- **Makefile**：可以使用make来生成可执行文件，可执行文件有两种类型，Name类型为不带任何优化的版本，Name1类型为带有嵌入C函数，带有5个可选优化的版本（根据LLVM每个优化pass各自独立的设计，优化类型可以任意组合）；你可以通过编辑Name的值指定生成文件的名字，请勿同时在多个线程执行make命令。（相关内容已经下载Makefile中）

- **input**：input文件为默认的自定义语法的输入文件（即一种CFG文法的推导结果），程序将读取文件并且检查错误，无错误则生成中间代码。

- inputrecord：一些input可选的内容以及历史记录。

- **可执行文件**：

  - final_ori11：未带优化的可执行文件，以input文件为输入，输出中间代码或者输出错误信息到终端。
  - final_optwithC11：带优化的可执行文件（勾选了实验中4.1 4.2 4.3的优化，其他优化可以通过打开注释重新make），嵌入`printi(__int16_t)`函数声明打印整数，其他同上，具体请参看实验报告第5章。

- test文件夹

  - testn：实验报告中第n个测试样例的代码。
  - testn_ori：testn中的代码放入input文件，执行final_ori11输出的中间代码。
  - testn_opt：testn中的代码放入input文件，执行final_optwithC11输出的优化过的中间代码。
  
- 其他文件

  - printi.c：定义了printi函数，使用编译命令

    ```bash
    clang -S -emit-llvm printi.c -o printi.ll
    ```

生成可读LLVM字节码文件printi.ll，后续可与本程序执行生成的中间代码共同编译生成可执行文件。（该文件完全是为了嵌入打印功能）

```bash
clang printi.ll output.ll -o output
```

生成的output可以执行并且进行打印。



#### 操作方法再次讲解：

make执行得到可执行程序，编辑input内容为你想测试的输入内容，执行程序可以得到结果（报错或者无错误情况下生成中间代码（优化or未优化））。拷贝终端输出的部分

```asm
; ModuleID = '666 compiling principal PYF YHR'
source_filename = "666 compiling principal PYF YHR"
```

从这行开始拷贝到结束，然后放入到一个空的.ll文件中。
<br>
之后按照上述方式运行或者按照报告中指定的方式进行运行即可。
<br>
由于使用了LLVM成熟的架构，生成的中间代码完全可以继续生成目标代码，理论和实践都是支持的。


<br>
整个工程没有建立新的文件夹，方便读者直接执行，避免读者可能的错误。可执行文件可以参照实验报告第5张的内容生成。




<p align=right>——庞懿非

