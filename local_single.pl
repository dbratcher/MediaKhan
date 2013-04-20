#test script 
for($num_files = 2; $num_files <= 10; $num_files+=2) {
  print "runs with $num_files num files\n";
  for($run = 1; $run <= 3; $run++) {
    print "RUN $run\n";
    `fusermount -u /tmp/fuse/`;
    `rm -f ~/projects/mediakhan/khan_root/*.mp3`;
    `rm -f ~/projects/mediakhan/khan_root2/*.mp3`;
    `rm -f ~/projects/mediakhan/khan_root3/*.mp3`;
    `cp ~/projects/mediakhan/media/1.mp3 ~/projects/mediakhan/khan_root`;
    for($file = 2; $file <= $num_files; $file++) {  
      `cp ~/projects/mediakhan/media/$file.mp3 ~/projects/mediakhan/khan_root3`;
    }
    $input = <STDIN>;
    `~/packages/redis/src/redis-cli flushall`;
    `~/projects/mediakhan/khan /tmp/fuse/ /home/drew/projects/mediakhan/stores.txt`;
    `mkdir /tmp/fuse/mp3/all_mp3s/localize_1.mp3`;
  }
}

