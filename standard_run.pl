use File::Basename;

 for ($count = 1; $count < 4; $count++) {
    $out = `redis-cli flushall`;
    $out = `./khan ./test -s`;
    print "startup complete\n";
    opendir(D, "test/location/laptop/") || die "Can't opendir\n";
    $i = 0;
    @files = ();
    while($f = readdir(D)) {
      if($f=~/mp3/) {
        push @files, $f;
      }
    }
    for $f (@files) {
        $i += 1;
        if($i%100==0) {
          print "moving file $i\n";
        }
        rename("test/\@laptop/$f","test/\@favs/$f");
    }
    closedir(D);
    $out = `mv rename_times.txt measurements/selectors/selector/redis-4000-run$count.txt`;
}
