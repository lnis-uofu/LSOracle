use strict;
use warnings;

my $nr_rounds;
my %vars;

my $cnf = shift;
open my $cnffd, '<', $cnf or die $!;
while ($_ = <$cnffd>) {
	chomp;

	if (m/^[c\*] parameter nr_rounds = (\d+)$/) {
		$nr_rounds = $1;
	} elsif (my ($var, $width, $name) = m/^[c\*] var (\d+)\/(\d+) (.*)$/) {
		$vars{$name} = $var;
	}
}
close $cnffd;

my %valuation;

my $output = shift;
open my $outputfd, '<', $output or die $!;
while ($_ = <$outputfd>) {
	chomp;

	next if m/^c/;
	s/^v //;

	for (split m/\s+/) {
		if (m/^-x?(\d+)$/) {
			$valuation{$1} = 0;
		} elsif (m/^x?(\d+)$/) {
			$valuation{$1} = 1;
		}
	}
}
close $outputfd;

printf "%u\n", $nr_rounds;

printf "%08x %08x %08x %08x %08x %08x %08x %08x\n%08x %08x %08x %08x %08x %08x %08x %08x\n",
	value("w[0]"), value("w[1]"), value("w[2]"), value("w[3]"),
	value("w[4]"), value("w[5]"), value("w[6]"), value("w[7]"),
	value("w[8]"), value("w[9]"), value("w[10]"), value("w[11]"),
	value("w[12]"), value("w[13]"), value("w[14]"), value("w[15]");

printf "%08x %08x %08x %08x %08x\n",
	value("h_in0"), value("h_in1"),
	value("h_in2"), value("h_in3"),
	value("h_in4");

printf "%08x %08x %08x %08x %08x\n",
	value("h_out0"), value("h_out1"),
	value("h_out2"), value("h_out3"),
	value("h_out4");

sub value {
	my $name = shift;
	my $var = $vars{$name};

	my $value = 0;
	for (my $i = 0; $i < 32; ++$i) {
		$value |= ($valuation{$var + $i} || 0) << $i;
	}

	return $value;
}
