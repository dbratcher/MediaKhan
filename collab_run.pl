use Time::HiRes  qw(gettimeofday);
for (my $run = 1; $run < 4; $run++) {
  open(DATA, ">redis-100-run$run.txt");
  for (my $split = 0; $split < 100; $split++) {
    print "starting splt $split\n";
    my $start = 0;
    my $i = 0;
    my $out = `redis-cli flushall`;
    $out = `./khan ./test -s`;
    my @files = <test/location/laptop/*.mp3>;
    foreach my $file (@files) {
      $out = `mv $file test/tags/favs`;
      if($i == $split) {
        ($seconds, $microseconds) = gettimeofday;
        $start = $seconds + $microseconds/1000000.0;
      }
      $i+=1;
    } 
    ($seconds, $microseconds) = gettimeofday;
    my $end = $seconds + $microseconds/1000000.0;
    print DATA ($end - $start);
    print DATA "\n";
  }
  close(DATA);
}
