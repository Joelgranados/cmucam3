#!/usr/bin/perl -w

use strict;

use constant TOOL_PREFIX => 'arm-none-eabi-';

my %aeabi_table = (
    dadd     => 'double-precision addition',
    ddiv     => 'double-precision division',
    dmul     => 'double-precision multiplication',
    drsub    => 'double-precision reverse subtraction',
    dsub     => 'double-precision subtraction',
    dneg     => 'double-precision negation',

    cdcmpeq  => 'double-precision non-excepting equality comparison',
    cdcmple  => 'double-precision 3-way compare',
    cdrcmple => 'double-precision reversed 3-way compare',
    dcmpeq   => 'double-precision = or !=',
    dcmplt   => 'double-precision <',
    dcmple   => 'double-precision <=',
    dcmpge   => 'double-precision >=',
    dcmpgt   => 'double-precision >',
    dcmpun   => 'double-precision isunordered',

    fadd     => 'single-precision addition',
    fdiv     => 'single-precision division',
    fmul     => 'single-precision multiplication',
    frsub    => 'single-precision reverse subtraction',
    fsub     => 'single-precision subtraction',
    fneg     => 'single-precision negation',

    cfcmpeq  => 'single-precision non-excepting equality comparison',
    cfcmple  => 'single-precision 3-way compare',
    cfrcmple => 'single-precision reversed 3-way compare',
    fcmpeq   => 'single-precision = or !=',
    fcmplt   => 'single-precision <',
    fcmple   => 'single-precision <=',
    fcmpge   => 'single-precision >=',
    fcmpgt   => 'single-precision >',
    fcmpun   => 'single-precision isunordered',

    d2iz     => 'double to integer C-style conversion',
    d2uiz    => 'double to unsigned C-style conversion',
    d2lz     => 'double to long long C-style conversion',
    d2ulz    => 'double to unsigned long long C-style conversion',
    f2iz     => 'float to integer C-style conversion',
    f2uiz    => 'float to unsigned C-style conversion',
    f2lz     => 'float to long long C-style conversion',
    f2ulz    => 'float to unsigned long long C-style conversion',

    d2f      => 'double to float conversion',
    f2d      => 'float to double conversion',

    i2d      => 'integer to double conversion',
    ui2d     => 'unsigned to double conversion',
    l2d      => 'long long to double conversion',
    ul2d     => 'unsigned long long to double conversion',
    i2f      => 'integer to float conversion',
    ui2f     => 'unsigned to float conversion',
    l2f      => 'long long to float conversion',
    ul2f     => 'unsigned long long to float conversion',
);


while (<>) {
    chomp;

    last if /^Cross Reference Table$/;
}

<>;
<>;

my %xref;
my $sym;

while (<>) {
    chomp;

    if (/^(\S+)/) {
	$sym = $1;
	$xref{$sym} = [];
    } elsif (/([^\/]+\.a\([^)]+\.o\))$/) {
	push @{$xref{$sym}}, $1;
    } elsif (/([^\/]+\.o)$/) {
	push @{$xref{$sym}}, $1;
    }
}

my $num_aeabi = 0;
foreach (keys %xref) {
    next unless  (/__aeabi_(.*)/);

    my $key = $_;
    my $aeabi_key = $1;
    my @list = @{$xref{$_}};

    if (@list) {
	my $desc = $aeabi_table{$aeabi_key};
	if (defined $desc) {
	    $num_aeabi++;
	    print "'$desc' ($key) used by:\n";
	    foreach (sort @list) {
		print "  $_\n";
	    }
	    print "\n";
	}
    }
}

if (!$num_aeabi) {
    print "No floating point detected!\n";
}
