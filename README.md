# Minishell
This project is about developing a shell. It's a portion of a real shell regularly called "minishell". This project is a good
put in practice some of the most important calls in a operating system like, "**fork**", "**exec**", "**signal**", "**pipe**", 
"**dup**" and more.

Also, there is a Makefile is programed to facilitate the execution and the use of the compiler gcc.

This minishell includes input prompts where users can enter commands, and supports redirection (<, >, >>) for input/output control. 
Pipelines (|) are used to connect the output of one command to the input of another. It also allows running commands in the background 
using &, enabling multitasking. Additionally, it manages signals like SIGINT and SIGQUIT to handle interruptions, ensuring smooth 
operation and process control.

This also has programed some built-in functions just as: cd, umask, time, limit, set, read.

## cd
#### run:
```
cd [path]
```
It changes the directory. If **path** is given as argument, it will change the current directory to the given **path**.
If it is not given it will change to the path stored in the environment variable HOME.

## umask
#### run:
```
umask [value]
```
Change the file creation mask defined. If **value** is given it change the current mask. If it not given it will show 
the current mask.

## time
#### run:
```
time [command]
```
It times the execution. If **command** is given it will execute the command and time it. If it not given it will show  
the time of the minishell and the children process.

## limit
#### run:
```
limit [resource[maximum]]
```
It sets maximum resource limits for current process and its child. If **resource** is given but no **maximum**, it will
show the current limits for **resource**. If both **resource** and **maximum** is given it will set the **maximum** limit to the
**resource** given. If none are given it will show all the limits defined.

## set 
#### run:
```
set [variable[value...]]
```
Assigns value to the environment values. If **variable** is given but no **value** it will show the value of **variable** 
in case both **variable** and **value** are given , it will set **value** to the **variable**. If no arguments are given
it will show all the environment variables and its values.

## read
#### run:
```
read variable [variable...]
```
Assigns value to the environment values. It modded version of set. This method can assign value to multiple variable in 
the same command. It uses spaces and tabulators to divide the token for the variables and values. It will assign the
first word to the first variable, the second word to the second so goes on until last variable will have the rest of the
words.


## Getting Started

### Prerequisites

* You will use it a UNIX operating system.

* Gcc compiler, also built-in in most UNIX system.

So no worries for any installation :wink:

_In case you want modify or add functionalities, you will need a text editor, built-in in most UNIX system, and modify the main.c file._

## Running

Open your terminal where the files(main.c and Makefile) are.

Type:
```
make
```
This will generate the files necessary to run the minishell, **main.o** and **minishell**.

Then type:
```
./minishell
```

![Execution](https://github.com/KoolRick/minishell/blob/main/readmeFiles/executingMinishell.gif)

and that's it you have your minishell running and ready for testing. :sunglasses:

## Built With

* [man](https://man7.org/linux/man-pages/dir_all_alphabetic.html) - This website you'll find all the commands used in this project.
