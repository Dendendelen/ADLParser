# General Purpose ADL Parser

A parser and transpiler for the Analysis Description Language, designed to allow quick and intuitive creation of high-energy analyses in a form that is easily human-readable and framework-independent. This tool allows for raw output from the parser, along with output in the form of multiple analysis frameworks for HEP.

## Installation

To compile the ADL parser and transpiler, run:

```
make
```

in the relevant directory. The executable produced can then be run.

## Instructions

The syntax for the tool is:

```
main FILENAME.adl [timber]|[coffea]|[lex]|[parse]|[alil]
```

This will output to standard output. To create an output file, simply pipe into the desired target.

### Arguments

* **`timber`**: Transpile the ADL to be run in the TIMBER analysis framework
* **`coffea`**: Transpile the ADL to be run in the Coffea analysis framework
* **`alil`**: Compile the ADL into Analysis-Level Instruction Language (ALIL), an intermediate imperative language used to facilitate further transpiling or running of the code
* **`lex`**: Perform the tokenizing step of the parsing; output the ADL text broken into its tokens
* **`parse`**: Perform the parsing, outputting a GraphViz DOT file, which can then be turned into an image by running `make dot`
