use Time::HiRes qw(gettimeofday);
use IO::Socket::INET;
 
# auto-flush on socket
$| = 1;
 
# create a connecting socket
my $socket = new IO::Socket::INET (
    PeerHost => '128.61.26.47',
    PeerPort => '7777',
    Proto => 'tcp',
);
die "cannot connect to the server $!\n" unless $socket;
print "connected to the server\n";

$diff1 = 0;
$diff2 = 0;
$num_runs = 1000; 
# data to send to a server
for($run=0; $run < $num_runs; $run++) {
  my $time2 = "";
  #print "run $run\n";
  ($seconds, $micro) = gettimeofday;
  $time1 = $seconds + $micro/1000000.0;
  $socket->send($time1);
  # receive a response of up to 1024 characters from server
  $socket->recv($time2, 1024);
  #print "recieved $time2\n";
  ($seconds, $micro) = gettimeofday;
  $time3 = $seconds + $micro/1000000.0;
  $diff1 += $time2 - $time1;
  $diff2 += $time3 - $time2;
  #print "diff1: $diff1\n";
  #print "diff2: $diff2\n\n";
}
$socket->send("end");
shutdown($socket, 1);

$diff1/=$num_runs;
$diff2/=$num_runs;
print "diff1: $diff1\n";
print "diff2: $diff2\n";
$socket->close();
