# NovaEngine

Overview:
	An ambitious C project of mine from 2020 that was an attempt at creating an interpreted language. Was lost to a dead computer years ago. The original Git repository is unrecoverable but the program itself remains and it's final version however incomplete is stable.

Compatibility: 
	This program was developed on a Linux system, and so direct use on Windows is not currently supported. In order to run, either simply using a Linux system directly or alternatively, if using Windows, use the Windows Subsystem for Linux (WSL) like I did when recovering and ressurecting this project.

Detail:
	The language/grammar itself is very simple, supporting only addition, subtraction, and negative numbers in its final and current form. Division, multiplication, and modulus were the next planned features. The program includes a parser that utilized a stack-based approach to token parsing as well an example of a multi-threaded circular buffer, all, along with other features, complete and as well as planned, organized into a small framework that is pieced together like a lego set in the main file. All of this was written in C utilizing the pthreads library for multi-threading support, using only vim, gcc, make, and valgrind on a headless Ubuntu computer consulting only a C manual. This setup was taken on as a personal challenge during the initial covid lockdown, but was certainly Spartan, and in hindsight probably unnecessary. The program size itself is very small, taking up only 36KB on my system.

Look at:
	Circular buffer: novapipe.c/.h
	Parser: novacmp.c/.h
	Object-oriented C/Grammar: novaexe.c/.h
	"Lego-set" in action: novamain.c

Running:
  1. In a Linux console, type make in the top directory to run the makefile.
  2. Once make is done building there will be a "nova" file in the directory. This is the engine.
  3. To run the engine, enter ./nova.
  4. You will get a small header message indicating the language version/version of the interpreter.
  5. There is no console prompt, just the cursor. Enter only integers, addition (+) and subtraction (-) only, with no whitespace, ending with a semicolon. A single leading + or - is allowed between two numbers in order to implement negative numbers, where the outer symbol is the operator, and the inner one the sign. A simple number with a semicolon is also valid, as well as with an optional sign.
  6. Hit enter, and the engine computes the result and prints it to the console, and then awaits the next input.
  7. To quit, use Ctrl-C.

Note as well that the program can recover completely from some error states caused by bad input, as it was built to be forgiving. There are corner cases that are not covered by this.
