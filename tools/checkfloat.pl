#!/usr/bin/perl -w

# Copyright 2007  Adam Goode
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


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



my %syms;
my @all_addrs;

open(OBJDUMP, '-|', TOOL_PREFIX . "objdump", "-d", "--", @ARGV);
while(<OBJDUMP>) {
    chomp;

    next unless (/([0-9A-Fa-f]+):.*<__aeabi_([^>+]+)>/);
    my $addr = $1;
    my $sym = $2;

    next unless defined $aeabi_table{$sym};
    push @{$syms{$sym}}, $addr;
    push @all_addrs, $addr;
}

close OBJDUMP;


if (!%syms) {
    print "No floating point found!\n";
    exit;
}


my %addr2line;
my $i = 0;

#print join(' ', @all_addrs), "\n";
open(ADDR2LINE, '-|', TOOL_PREFIX . "addr2line", "-f", "-e", $ARGV[0], @all_addrs);
while(<ADDR2LINE>) {
    chomp;
    my $funcname = $_;
    chomp(my $line = <ADDR2LINE>);

    $addr2line{$all_addrs[$i++]} = "$funcname : $line";
}
close ADDR2LINE;

foreach (sort keys %syms) {
    my $desc = $aeabi_table{$_};
    my %lines;

    print "'$desc' (__aeabi_$_) used by:\n";
    my @lines = map({$addr2line{$_}} @{$syms{$_}});
    foreach (@lines) {
	next if defined $lines{$_};
	print "  $_\n";
	$lines{$_} = 1;
    }
    print "\n";
}
