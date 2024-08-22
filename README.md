# Minishell
This proyect is about developing a shell. It's a portion of a real shell regularly called "minishell". This proyect is a good
put in practice some of the most importan calls in a operating sysyem like, "**fork**", "**exec**", "**signal**", "**pipe**", 
"**dup**" and more.

Also is a Makefile is programed to facilite the execution and the use of the compiler gcc.


This also has programed some built-in functions just as:

## cd
#### run:
```
run: cd [path]
```
It changes the directory. If "path" is given as argument, it will change the current directory to the given "path".
If it is not given it will change to the path stored in the enviroment variable HOME.

## umask
#### run:
```
umask [value]
```

## time
```

```
## read
```

```
## limit
```

```
## set 
```

```


## Getting Started

This project was inspired to make easier the billing of the consumption of each month of cloud services, in this case AWS. 
It was needed to extract manually from the pdf invoices to account for some credits AWs gives.

So this script was created to eliminate this manual search avoding long efforts extracting this data, human error and saving time.

### Prerequisites

* You will use it a UNIX operating system.

* Any text editor, built-in in most UNIX system.

* Gcc compiler, also built-in in most UNIX system.

So no worries for any installation :wink:

## Running

Open your terminal where the files(main.c and Makefile) are.

Type:
```
make
```
this will generate two files, main.o and minishel.

Then type:
```
./minishel
```
and that's it you have your minishel running and ready for testing. :sunglasses:

![Execution](https://github.com/KoolRick/minishell/blob/main/readmeFiles/executingMinishell.gif)


## Built With

* [man](https://man7.org/linux/man-pages/dir_all_alphabetic.html) - This website you'll find all the commands used in this proyect.
