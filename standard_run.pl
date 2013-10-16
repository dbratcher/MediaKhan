 for ($count = 1; $count < 4; $count++) {
    $out = `./khan ./test -s`;
    $out = `for f in test/location/laptop/*.mp3; do mv "\$f" test/tags/favs$count/; done`;
    $out = `mv rename_times.txt measurements/selectors/selector/redis-4000-run$count.txt`;
 }
