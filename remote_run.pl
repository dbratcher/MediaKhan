 for ($count = 1; $count < 4; $count++) {
    $out = `./khan ./test -s`; 
    $out = `for f in test/location/laptop/*.mp3; do mv "\$f" test/tags/favs$count/; done`;
    $out = `mv start_times.txt measurements/inserts/remote/local_config/start-redis-100-run$count.txt`;
}