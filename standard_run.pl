 for ($count = 1; $count < 4; $count++) {
    $out = `rm my_db.db`;
    $out = `./khan ./test -s`;
    $out = `for f in test/location/laptop/*.mp3; do mv "\$f" test/tags/favs$count/; done`;
    $out = `mv rename_times.txt measurements/inserts/local/bdb-4000-run$count.txt`;
 }
