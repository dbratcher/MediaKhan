use Time::HiRes  qw(gettimeofday);

my $last = 0;
my $count = 0;
my $dir = './test/tags/favs';
my $seconds = 0;
my $microseconds = 0;
while(1) {
  $count = `ls -l test/tags | grep "favs"`;
  ($seconds, $microseconds) = gettimeofday;
  my $time = $seconds;
  $time + ($microseconds/1000000.0);
  if($count ne $last) {
    print $count;
    print " ";
    print $time;
    print "\n";
  }
  $last = $count;
}
