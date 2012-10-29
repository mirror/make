#                                                                    -*-perl-*-
$description = "Test the load operator.";

$details = "Test dynamic loading of modules.";

# Don't do anything if this system doesn't support "load"
exists $FEATURES{load} or return -1;

# First build a shared object
# Provide both a default and non-default load symbol

unlink(qw(testload.c testload.so));

open(my $F, '> testload.c') or die "open: testload.c: $!\n";
print $F <<'EOF' ;
#include <string.h>
#include <stdio.h>

void define_new_function (void *, const char *, int, int, int,
                          char *(*)(char *, char **, const char *));

char *variable_buffer_output (char *, const char *, unsigned int);

static char *
func_test(char *o, char **argv, const char *funcname)
{
    return variable_buffer_output (o, funcname, strlen (funcname));
}

int
testload_gmake_setup ()
{
    define_new_function (0, "func-a", 1, 1, 1, func_test);
    return 1;
}

int
explicit_setup ()
{
    define_new_function (0, "func-b", 1, 1, 1, func_test);
    return 1;
}
EOF
close($F) or die "close: testload.c: $!\n";

run_make_test('testload.so: testload.c ; @$(CC) -g -shared -fPIC -o $@ $<',
              '', '');

# TEST 1
run_make_test(q!
all: ; @echo $(func-a foo) $(func-b bar)
load ./testload.so
!,
              '', "func-a\n");

# TEST 2
# Load a different function
run_make_test(q!
all: ; @echo $(func-a foo) $(func-b bar)
load ./testload.so(explicit_setup)
!,
              '', "func-b\n");

# TEST 3
# Verify the .LOADED variable
run_make_test(q!
all: ; @echo $(filter ./testload.so,$(.LOADED)) $(func-a foo) $(func-b bar)
load ./testload.so(explicit_setup)
!,
              '', "./testload.so func-b\n");

# TEST 4
# Check multiple loads
run_make_test(q!
all: ; @echo $(filter ./testload.so,$(.LOADED)) $(func-a foo) $(func-b bar)
load ./testload.so
load ./testload.so(explicit_setup)
!,
              '', "./testload.so func-a\n");

unlink(qw(testload.c testload.so)) unless $keep;

# This tells the test driver that the perl test script executed properly.
1;
