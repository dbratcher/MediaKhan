 for ($count = 3; $count < 4; $count++) {
    $out = `./khan ./test -s`; 
    $out = `for f in test/\@laptop/*.mp3; do mv "\$f" test/\@favs2$count/; done`;
    $out = `mv rename_times.txt measurements/selectors/selector/redis-4000-run$count.txt`;
}
