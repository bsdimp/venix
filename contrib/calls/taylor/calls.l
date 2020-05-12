.TH CALLS P1 PUBLIC
.ad
.SH NAME
calls \- print out calling pattern of functions in a program
.SH SYNOPSIS
calls [flag[s]] [function[s]] .....[flags and functions]
.SH DESCRIPTION
calls accepts from standard input a file which should
be the source of a programme, and outputs the analysed calling pattern to
standard output. 
It is intended to help analyse the flow of a programme by laying out
the functions called in the manner used in "Software Tools". For example:
.br
	main
.br
		abc
.br
			a
.br
				<< abc <<
.br
			b
.br
		def
.br
			d
.br
			>> e [external]
.br

in which main calls abc and def, abc calls a and b which are defined
within the same source file, and def calls d (defined in the same source)
and e, which is presumably a library programme or defined in a different
source file.
The function a calls abc in a recursive loop.
.br
FLAGS
.in +15
.ti -10
-t or -terse	Provides a terse form of output, in which the calling pattern
for any function is printed only once, on the first occurrence of the
function. Subsequent occurrences output the function name, and a notation
.br
	... [see line xx]
.br
This is the default case.
.ti -10
-v or -verbose	Full output of function calling patterns on every occurrence.
.ti -10
-w or -width nn	Set the output paper width to nn. Default is 132.

.in -15
FUNCTION LIST
.br
Function names within the input programme may be selected as roots
of the layout (default "main" in C). For example, using the previous programme:
.br
	calls def abc<programme
.br
	def
.br
		d
.br
		>> e [external]
.br
	
.br
	abc
.br
		a
.br
			<< abc <<
.br
		b
.br

	Functions not referenced within the source file are shown as
.br
		>>function [external]

.br
	Recursive references are shown as
.br
		<< function <<

.SH AUTHOR
M.M.Taylor (DCIEM)
.br
Modified for V7 and stdio, Alexis Kwan (HCR for DCIEM)
.SH BUGS
#define's with arguments are treated as functions.
.br
Many intended features are not implemented:
	Flags -t -v (terse and verbose) -g (list globals used) -f -p (Fortran
and Pascal languages).




