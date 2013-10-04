use Time::HiRes  qw(gettimeofday);

my $last = 0;
while(1) {
  my $dir = './test/tags/favs';
  my $seconds = 0;
  my $microseconds = 0;
  ($seconds, $microseconds) = gettimeofday;
  my $time = $seconds + ($microseconds/100000);
  my @files = <$dir/*>;
  my $count = @files;
  if($count ne $last) {
    print $count;
    print " ";
    print $time;
    print "\n";
  }
  $last = $count;
}
