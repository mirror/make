$!
$! prepare_vms.com  - Build config.h-vms from master on VMS.
$!
$! This is used for building off the master instead of a release tarball.
$!
$!
$! First try ODS-5, Pathworks V6 or UNZIP name.
$!
$ config_template = f$search("sys$disk:[.src]mkconfig*h.in")
$ if config_template .eqs. ""
$ then
$!
$!  Try NFS, VMStar, or Pathworks V5 ODS-2 encoded name.
$!
$   config_template = f$search("sys$disk:[.src]mkconfig.h*in")
$   if config_template .eqs. ""
$   then
$       write sys$output "Could not find mkconfig.h.in!"
$       exit 44
$   endif
$ endif
$ config_template_file = f$parse(config_template,,,"name")
$ config_template_type = f$parse(config_template,,,"type")
$ config_template = "sys$disk:[.src]" + config_template_file + config_template_type
$!
$!
$! Pull the version from configure.ac
$!
$ open/read ac_file sys$disk:[]configure.ac
$ac_read_loop:
$ read ac_file/end=ac_read_loop_end line_in
$ key = f$extract(0, 7, line_in)
$ if key .nes. "AC_INIT" then goto ac_read_loop
$ package = f$element (1,"[",line_in)
$ package = f$element (0,"]",package)
$ version = f$element (2,"[",line_in)
$ version = f$element (0,"]",version)
$ac_read_loop_end:
$ close ac_file
$!
$ if (version .eqs. "")
$ then
$    write sys$output "Unable to determine version!"
$    exit 44
$ endif
$!
$!
$ outfile = "sys$disk:[.src]mkconfig.h"
$!
$! Note the pipe command is close to the length of 255, which is the
$! maximum token length prior to VMS V8.2:
$! %DCL-W-TKNOVF, command element is too long - shorten
$! PDS: Blown out; someone else will have to figure this out
$ pipe (write sys$output "sub,@PACKAGE@,make,WHOLE/NOTYPE" ;-
        write sys$output "sub,@PACKAGE_BUGREPORT@,bug-make@gnu.org,WHOLE/NOTYPE" ;-
        write sys$output "sub,@PACKAGE_NAME@,GNU Make,WHOLE/NOTYPE" ;-
        write sys$output "sub,@PACKAGE_TARNAME@,make,WHOLE/NOTYPE" ;-
        write sys$output "sub,@PACKAGE_URL@,https://www.gnu.org/software/make/,WHOLE/NOTYPE" ;-
        write sys$output "sub,@PACKAGE_VERSION@,''version',WHOLE/NOTYPE" ;-
        write sys$output "exit") |-
       edit/edt 'config_template'/out='outfile'/command=sys$pipe >nla0:
$!
$ write sys$output "GNU Make version: ", version, " prepared for VMS"
