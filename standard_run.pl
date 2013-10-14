 for ($count = 1; $count < 4; $count++) {
    $out = `redis-cli flushall`;
    $out = `./khan ./test -s`;
    $out = `for f in test/\@laptop/*.mp3; do mv "\$f" test/\@favs$count/; done`;
    $out = `mv rename_times.txt measurements/selectors/selector/redis-100-run$count.txt`;
 }
