$ save_verify = f$verify()
$ set noverify
$ set noon
$ goto skip_header
$!
$!	File:	COMP.COM
$!	Date:	August 23, 1985
$!	Author:	Robin Miller
$!
$!	Procedure to: Compile modules for VMODEM
$!
$!	Modifications:
$!
$!	Calling sequence: @COMP module_name
$!
$skip_header:
$!
$! Setup the variables:
$!
$!compile_options = "/DEBUG/NOOPTIMIZE/LIST"
$ compile_options = "/LIST"
$ library_name	  = "VMODEM"
$ save_status	  = 0
$!
$! Check for the C file specified.
$!
$ if f$search ("''p1'.C") .nes. "" then $ goto check_lbr
$ write sys$output "File ''p1'.C does not exist ... exiting ..."
$ goto exit_compile
$!
$! Check for the object module library.
$!
$check_lbr:
$ if f$search ("''library_name'.OLB") .nes. "" then $ goto do_compile
$ write sys$output -
	"Library file ''library_name'.OLB does not exist ... exiting ..."
$ goto exit_compile
$!
$! Finally, do the C compile and library insertion.
$!
$do_compile:
$ if f$search ("''p1'.OBJ") .nes. "" then $ delete 'p1'.obj;*
$ if f$search ("''p1'.LIS") .nes. "" then $ delete 'p1'.lis;*
$ CC'compile_options' 'p1'
$ save_status = $status
$ if (.not. $status) then $ goto exit_compile
$ set nocontrol=y
$ library 'library_name' 'p1'
$ set control=y
$ delete 'p1'.obj;*
$ if f$search ("''p1'.OBJ") .nes. "" then $ delete 'p1'.obj;*
$!
$! all done:
$!
$exit_compile:
$ if save_verify then $ set verify
$ exit save_status
