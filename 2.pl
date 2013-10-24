use Time::HiRes  qw(gettimeofday);

my $last = 0;
my $count = 0;
my $dir = './test/tags/favs';
my $seconds = 0;
my $microseconds = 0;
while(1) {
  $count = `ls -l test/tags | grep "favs2" | awk '{print \$2}'`;
  ($seconds, $microseconds) = gettimeofday;
  if($count ne $last) {
    print $count;
    print " ";
    $time = $seconds + $microseconds/1000000.0;
    print "$time\n";
  }
  $last = $count;
}
