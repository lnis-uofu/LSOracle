use strict;
use warnings;

use IO::Select;

my $s = IO::Select->new();

my $prog1 = shift;
my $prog2 = shift;

my $time0 = time;

open(my $fd1, '-|', 'unbuffer', $prog1, @ARGV) or die $!;
$s->add($fd1);

open(my $fd2, '-|', 'unbuffer', $prog2, @ARGV) or die $!;
$s->add($fd2);

while ($s->count) {
	for my $fd ($s->can_read) {
		my $line = <$fd>;
		unless (defined $line) {
			$s->remove($fd);
			next;
		}

		chomp $line;

		my $time = time - $time0;
		printf "\e[33m%5u %s\n", $time, $line if ($fd == $fd1);
		printf "\e[32m%5u %s\n", $time, $line if ($fd == $fd2);
	}
}
