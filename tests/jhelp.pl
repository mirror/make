#!/usr/bin/env perl
# -*-perl-*-
#
# This script helps us test jobserver/parallelism without a lot of unreliable
# (and slow) sleep calls.  Written in Perl to get portable sub-second sleep.
#
# It can run the following steps based on arguments:
#  -t <secs>  : maximum # of seconds the script can run; else we fail.
#               Default is 4 seconds.
#  -e <word>  : echo <word> to stdout
#  -f <word>  : echo <word> to stdout AND create an (empty) file named <word>
#  -w <word>  : wait for a file named <word> to exist

# Force flush
$| = 1;

my $timeout = 4;

sub op {
    my ($op, $nm) = @_;

    defined $nm or die "Missing value for $op\n";

    if ($op eq '-e') {
        print "$nm\n";
        return 1;
    }

    if ($op eq '-f') {
        print "$nm\n";
        open(my $fh, '>', $nm) or die "$nm: open: $!\n";
        close(my $fh);
        return 1;
    }

    if ($op eq '-w') {
        if (-f $nm) {
            return 1;
        }
        select(undef, undef, undef, 0.1);
        return 0;
    }

    if ($op eq '-t') {
        $timeout = $nm;
        return 1;
    }

    die("Invalid command: $op $nm\n");
}

my $start = time();
while (@ARGV) {
    if (op($ARGV[0], $ARGV[1])) {
        shift;
        shift;
    }
    if ($start + $timeout < time()) {
        die("Timeout after ".(time()-$start-1)." seconds\n");
    }
}

exit(0);
