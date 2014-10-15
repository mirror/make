Overview:                                                       -*-text-mode-*-
---------

  This version of GNU make has been tested on:
  OpenVMS V8.3/V8.4 (Alpha) and V8.4 (Integrity) AND V7.3 (VAX)

  This version of GNU Make is intended to be run from DCL to run
  make scripts with a special syntax that is described below.  It
  likely will not be able to run unmodified Unix makefiles.

  There is an older implementation of GNU Make that was ported to GNV.
  Work is now in progress to merge that port to get a single version
  of GNU Make available.  When that merge is done, GNU Make will auto
  detect that it is running under a Posix shell and then operate as close to
  GNU Make on Unix as possible.

  The descriptions below are for running GNU make from DCL or equivalent.


Recipe differences:
-------------------

  GNU Make for OpenVMS can not currently run native Unix make files because of
  differences in the implementation that it is not aware of the GNV packages.

  I am trying to document the current behavior in this section.  This is based
  on the information in the file NEWS. and running the test suite.
  TODO: More tests are needed to validate and demonstrate the OpenVMS
  expected behavior.

  The format for recipes are a combination of Unix macros, a subset of
  simulated UNIX commands, some shell emulation, and OpenVMS commands.
  This makes the resulting makefiles unique to the OpenVMS port of GNU make.

  If you are creating a OpenVMS specific makefile from scratch, you should also
  look at MMK (Madgoat Make) available at https://github.com/endlesssoftware/mmk
  MMK uses full OpenVMS syntax and a persistent subprocess is used for the
  recipe lines, allowing multiple line rules.

  The default makefile search order is "makefile.vms", "gnumakefile",
  "makefile".  TODO: See if that lookup is case sensitive.

  When Make is invoked from DCL, it will create a foreign command
  using the name of executable image, with any facility prefix removed,
  for the duration of the make program, so it can be used internally
  to recursively run make().  The macro MAKE_COMMAND will be set to
  this foreign command.

  When make is launched from an exec*() command from a C program,
  the foreign command is not created.  The macro MAKE_COMMAND will be
  set to the actual command passed as argv[0] to the exec*() function.

  If the DCL symbol or logical name GNV$MAKE_USE_MCR exists, then
  the macro MAKE_COMMAND will be set to be an "MCR" command with the
  absolute path used by DCL to launch make.  The foreign command
  will not be created.

  The macro MAKE is set to be the same value as the macro MAKE_COMMAND
  on all platforms.

  Each recipe command is normally run as a separate spawned processes,
  except for the cases documented below where a temporary DCL command
  file may be used.

  BUG: Testing has shown that the commands in the temporary command files
  are not always created properly.  This issue is still under investigation.

  Any macros marked as exported are temporarily created as DCL symbols
  for child images to use.  DCL symbol substitution is not done with these
  commands.
  TODO: Add symbol substitution.

  When a temporary DCL command file is used, DCL symbol substitution
  will work.

  Command lines of excessive length are broken and written to a command file
  in sys$scratch:. There's no limit to the lengths of commands (and no need
  for .opt files :-) any more.

  The '<', '>' and '>>' redirection has been implemented by using
  temporary command files.  These will be described later.

  The DCL symbol or logical name GNV$MAKE_USE_CMD_FILE when set to a
  string starting with one of '1','T', or 'E' for "1", "TRUE", or "ENABLE",
  then temporary DCL command files are always used for running commands.
  In this case, the exported environment environment variables are
  created by command file.  BUG: Environment variables that hold values
  with dollar signs in them are not exported correctly.

  GNU Make generally does text comparisons for the targets and sources.  The
  make program itself can handle either Unix or OpenVMS format filenames, but
  normally does not do any conversions from one format to another.
  TODO: The OpenVMS format syntax handling is incomplete.
  TODO: ODS-5 EFS support is missing.
  BUG: The internal routines to convert filenames to and from OpenVMS format
  do not work correctly.

  Note: In the examples below, line continuations such as a backslash may have
  been added to make the examples easier to read in this format.
  BUG: That feature does not completely work at this time.

  Since the OpenVMS utilities generally expect OpenVMS format paths, you will
  usually have to use OpenVMS format paths for rules and targets.
  BUG: Relative OpenVMS paths may not work in targets, especially combined
  with vpaths.  This is because GNU make will just concatenate the directories
  as it does on Unix.

  The variables $^ and $@ separate files with commas instead of spaces.
  While this may seem the natural thing to do with OpenVMS, it actually
  causes problems when trying to use other make functions that expect the
  files to be separated by spaces.  If you run into this, you need the
  following workaround to convert the output.
  TODO: Look at have the $^ and $@ use spaces like on Unix and have
  and easy to use function to do the conversions and have the built
  in OpenVMS specific recipes and macros use it.

  Example:

comma := ,
empty :=
space := $(empty) $(empty)

foo: $(addsuffix .3,$(subs $(comma),$(space),$^)


  Makefile variables are looked up in the current environment. You can set
  symbols or logicals in DCL and evaluate them in the Makefile via
  $(<name-of-symbol-or-logical>).  Variables defined in the Makefile
  override OpenVMS symbols/logicals.

  OpenVMS logical and symbols names show up as "environment" using the
  origin function.  when the "-e" option is specified, the origion function
  shows them as "environment override".  On Posix the test scripts indicate
  that they should show up just as "environment".

  When GNU make reads in a symbol or logical name into the environment, it
  converts any dollar signs found to double dollar signs for convenience in
  using DCL symbols and logical names in recipes.  When GNU make exports a
  DCL symbol for a child process, if the first dollar sign found is followed
  by second dollar sign, then all double dollar signs will be convirted to
  single dollar signs.

  The variable $(ARCH) is predefined as IA64, ALPHA or VAX respectively.
  Makefiles for different OpenVMS systems can now be written by checking
  $(ARCH).  Since IA64 and ALPHA are similar, usually just a check for
  VAX or not VAX is sufficient.

  You may have to update makefiles that assume VAX if not ALPHA.

ifeq ($(ARCH),VAX)
  $(ECHO) "On the VAX"
else
  $(ECHO) "On the ALPHA  or IA64"
endif

  Empty commands are handled correctly and don't end in a new DCL process.

  The exit command needs to have OpenVMS exit codes.  To pass a Posix code
  back to the make script, you need to encode it by multiplying it by 8
  and then adding %x1035a002 for a failure code and %x1035a001 for a
  success.  Make will interpret any posix code other than 0 as a failure.
  TODO: Add an option have simulate Posix exit commands in recipes.

  Lexical functions can be used in pipes to simulate shell file test rules.

  Example:

  Posix:
b : c ; [ -f $@ ] || echo >> $@

  OpenVMS:
b : c ; if f$$search("$@") then pipe open/append xx $@ ; write xx "" ; close xx


  You can also use pipes and turning messages off to silently test for a
  failure.

x = %x1035a00a

%.b : %.c
<tab>pipe set mess/nofac/noiden/nosev/notext ; type $^/output=$@ || exit $(x)


Runtime issues:

  The OpenVMS C Runtime has a convention for encoding a Posix exit status into
  to OpenVMS exit codes.  These status codes will have the hex value of
  0x35a000.  OpenVMS exit code may also have a hex value of %x10000000 set on
  them.  This is a flag to tell DCL not to write out the exit code.

  To convert an OpenVMS encoded Posix exit status code to the original code
  You subtract %x35a000 and any flags from the OpenVMS code and divide it by 8.

  WARNING: Backward-incompatibility!
  The make program exit now returns the same encoded Posix exit code as on
  Unix. Previous versions returned the OpenVMS exit status code if that is what
  caused the recipe to fail.
  TODO: Provide a way for scripts calling make to obtain that OpenVMS status
  code.

  Make internally has two error codes, MAKE_FAILURE and MAKE_TROUBLE.  These
  will have the error "-E-" severity set on exit.

  MAKE_TROUBLE is returned only if the option "-q" or "--question" is used and
  has a Posix value of 1 and an OpenVMS status of %x1035a00a.

  MAKE_FAILURE has a Posix value of 2 and an OpenVMS status of %x1035a012.

  Output from GNU make may have single quotes around some values where on
  other platforms it does not.  Also output that would be in double quotes
  on some platforms may show up as single quotes on VMS.

  There may be extra blank lines in the output on VMS.
  https://savannah.gnu.org/bugs/?func=detailitem&item_id=41760

  There may be a "Waiting for unfinished jobs..." show up in the output.

  Error messages generated by Make or Unix utilities may slightly vary from
  Posix platforms.  Typically the case may be different.

  When make deletes files, on posix platforms it writes out 'rm' and the list
  of files.  On VMS, only the files are writen out, one per line.
  TODO: VMS

  There may be extra leading white space or additional or missing whitespace
  in the output of recipes.

  GNU Make uses sys$scratch: for the tempfiles that it creates.

  The OpenVMS CRTL library maps /tmp to sys$scratch if the TMP: logical name
  does not exist.  As the CRTL may use both sys$scratch: and /tmp internally,
  if you define the TMP logical name to be different than SYS$SCRATCH:,
  you may end up with only some temporary files in TMP: and some in SYS$SCRATCH:

  The default include directory for including other makefiles is
  SYS$SYSROOT:[SYSLIB] (I don't remember why I didn't just use
  SYS$LIBRARY: instead; maybe it wouldn't work that way).
  TODO:  A better default may be desired.

  If the device for a file in a recipe does not exist, on OpenVMS an error
  message of "stat: <file>: no such device or address" will be output.

  Make ignores success, informational, or warning errors (-S-, -I-, or
  -W-).  But it will stop on -E- and -F- errors. (unless you do something
  to override this in your makefile, or whatever).


Unix compatibilty features:
---------------------------

  The variable $(CD) is implemented as a built in Change Directory
  command. This invokes the 'builtin_cd'  Executing a 'set default'
  recipe doesn't do the trick, since it only affects the subprocess
  spawned for that command.
  TODO: Need more info on how to use and side effects

  Unix shell style I/O redirection is supported. You can now write lines like:
  "<tab>mcr sys$disk:[]program.exe < input.txt > output.txt &> error.txt"
  BUG: This support is not handling built in make macros with "<" in them
  properly.

  Posix shells have ":" as a null command.  OpenVMS generates a DCL warning
  when this is encountered.  It would probably be simpler to have OpenVMS just
  handle this instead of changing all the tests that use this feature.
  https://savannah.gnu.org/bugs/index.php?41761

  A note on appending the redirected output.  A simple mechanism is
  implemented to make ">>" work in action lines. In OpenVMS there is no simple
  feature like ">>" to have DCL command or program output redirected and
  appended to a file. GNU make for OpenVMS already implements the redirection
  of output. If such a redirection is detected, an ">" on the action line,
  GNU make creates a DCL command procedure to execute the action and to
  redirect its output. Based on that, now ">>" is also recognized and a
  similar but different command procedure is created to implement the
  append. The main idea here is to create a temporary file which collects
  the output and which is appended to the wanted output file. Then the
  temporary file is deleted. This is all done in the command procedure to
  keep changes in make small and simple. This obviously has some limitations
  but it seems good enough compared with the current ">" implementation.
  (And in my opinion, redirection is not really what GNU make has to do.)
  With this approach, it may happen that the temporary file is not yet
  appended and is left in SYS$SCRATCH.

  The temporary file names look like "CMDxxxxx.". Any time the created
  command procedure can not complete, this happens. Pressing Ctrl+Y to
  abort make is one case. In case of Ctrl+Y the associated command
  procedure is left in SYS$SCRATCH as well. Its name is CMDxxxxx.COM.

  The CtrlY handler now uses $delprc to delete all children. This way also
  actions with DCL commands will be stopped. As before the CtrlY handler
  then sends SIGQUIT to itself, which is handled in common code.

  Temporary command files are now deleted in the OpenVMS child termination
  handler. That deletes them even if a Ctrl+C was pressed.
  TODO: Does the previous section about >> leaving files still apply?

  The behavior of pressing Ctrl+C is not changed. It still has only an effect,
  after the current action is terminated. If that doesn't happen or takes too
  long, Ctrl+Y should be used instead.


Build Options:

  Added support to have case sensitive targets and dependencies but to
  still use case blind file names. This is especially useful for Java
  makefiles on VMS:

<TAB>.SUFFIXES :
<TAB>.SUFFIXES : .class .java
<TAB>.java.class :
<TAB><TAB>javac "$<"
<TAB>HelloWorld.class :      HelloWorld.java

  A new macro WANT_CASE_SENSITIVE_TARGETS in config.h-vms was introduced.
  It needs to be enabled to get this feature; default is disabled.
  TODO: This should be a run-time setting based on if the process
  has been set to case sensitive.


Unimplemented functionality:

  The new feature "Loadable objects" is not yet supported. If you need it,
  please send a change request or submit a bug report.

  The new option --output-sync (-O) is accepted but has no effect: GNU make
  for OpenVMS does not support running multiple commands simultaneously.


Self test failures and todos:
-----------------------------

  GNU make was not currently translating the OpenVMS encoded POSIX values
  returned to it back to the Posix values.  I have temporarily modified the
  Perl test script to compensate for it.  This should be being handled
  internally to Make.
  TODO: Verify and update the Perl test script.

  The features/parallelism test was failing. OpenVMS is executing the rules
  in sequence not in parallel as this feature was not implemented.
  GNU Make on VMS no longer claims it is implemented.
  TODO: Implement it.

  The vpath feature may need the targets to be in OpenVMS format.  To be
  consistent with other target processing, this restriction should be removed.
  TODO: Verify this after recent changes.

  The features/vpathgpath test is failing.  Reason has not yet been determined.

  The misc/bs-nl test is failing.  This is where a line is continued with a
  backslash.

  The options/dash-e test is failing.  Need to determine how to do overrides
  on VMS.

  The options/dash-k test is failing.  Test is not stopping when it should.

  The options/dash-n test is failing.  The "+" handling is not working.
  MAKEFLAG appears not to work.

  Symlink support is not present.  Symlinks are supported by OpenVMS 8.3 and
  later.

  The targets/INTERMEDIATE and targets/SECONDARY tests are failing.
  When make deletes files, on posix platforms it writes out 'rm' and the list
  of files.  On vms, only the files are writen out, one per line.

  The variables/GNUMAKEFLAGS and variables/MAKE_RESTARTS are failing.

  The variables/MAKEFILES test is failing.  Reason not yet determined.

  The variables/MAKEFLAGS test is failing.  Looks like the child is failing.

  The variables/automatic test is failing.
  The $^D, $^F, $+D, $+F cases are failing.

  The variables/undefine test is failing.  Undefine of multi-line define fails.

  Error messages should be supressed with the "-" at the beginning of a line.
  On openVMS they were showing up.  TODO: Is this still an issue?

  The internal vmsify and unixify OpenVMS to/from UNIX are not handling logical
  names correctly.


Build instructions:
------------------

  Don't use the HP C V7.2-001 compiler, which has an incompatible change
  how __STDC__ is defined. This results at least in compile time warnings.

Make a 1st version
       $ @makefile.com  ! ignore any compiler and/or linker warning
       $ copy make.exe 1st-make.exe

  Use the 1st version to generate a 2nd version as a test.
       $ mc sys$disk:[]1st-make clean  ! ignore any file not found messages
       $ mc sys$disk:[]1st-make

  Verify your 2nd version by building Make again.
       $ copy make.exe 2nd-make.exe
       $ mc sys$disk:[]2nd-make clean
       $ mc sys$disk:[]2nd-make


Running the tests:
------------------

  Running the tests on OpenVMS requires the following software to be installed
  as most of the tests are Unix oriented.

  * Perl 5.18 or later.
    https://sourceforge.net/projects/vmsperlkit/files/
  * GNV 2.1.3 + Updates including a minimum of:
    * Bash 4.3.30
    * ld_tools 3.0.2
    * coreutils 8.21
   https://sourceforge.net/p/gnv/wiki/InstallingGNVPackages/
   https://sourceforge.net/projects/gnv/files/

   As the test scripts need to create some foreign commands that persist
   after the test is run, it is recommend that either you use a subprocess or
   a dedicated login to run the tests.

   To get detailed information for running the tests:

   $ set default [.tests]
   $ @run_make_tests help

   Running the script with no parameters will run all the tests.

   After the the test script has been run once in a session, assuming
   that you built make in sys$disk:[make], you can redefined the
   "bin" logical name as follows:

   $ define bin sys$disk:[make],gnv$gnu:[bin]

   Then you can use Perl to run the scripts.

   $ perl run_make_tests.pl


Acknowlegements:
----------------

See NEWS. for details of past changes.

  These are the currently known contributers to this port.

  Hartmut Becker
  John Malmberg
  Michael Gehre
  John Eisenbraun
  Klaus Kaempf
  Mike Moretti
  John W. Eaton
