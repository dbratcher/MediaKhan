use File::Basename;

 for ($count = 2; $count < 4; $count++) {
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
        rename("test/location/laptop/$f","test/tags/favs$count/$f");
    }
    closedir(D);
    $out = `mv start_times.txt measurements/inserts/remote/config1/start-redis-4000-run$count.txt`;
    $out = `touch start_times.txt`;
}
