use Time::HiRes qw(gettimeofday);
use IO::Socket::INET;
 
# auto-flush on socket
$| = 1;
 
# creating a listening socket
my $socket = new IO::Socket::INET (
    LocalHost => '128.61.26.47',
    LocalPort => '7777',
    Proto => 'tcp',
    Listen => 5,
    Reuse => 1
);
die "cannot create socket $!\n" unless $socket;
print "server waiting for client connection on port 7777\n";
 
while(1)
{
    # waiting for a new client connection
    my $client_socket = $socket->accept();
 
    # get information about a newly connected client
    my $client_address = $client_socket->peerhost();
    my $client_port = $client_socket->peerport();
    print "connection from $client_address:$client_port\n";
 
    # read up to 1024 characters from the connected client
    my $data = "";
    while($data ne "end") {
      $client_socket->recv($data, 1024);
      #print "data: $data\n";
      ($seconds, $micro) = gettimeofday;    
      $time2 = $seconds + $micro/1000000.0; 
      $client_socket->send($time2);
      # notify client that response has been sent
    }
    shutdown($client_socket, 1);
}
 
$socket->close();
