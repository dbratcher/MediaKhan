#!/usr/bin/perl
use strict;
use warnings;

# randomly plays mp3 files in a directory
my $dir = "/Users/drewbratcher/projects/MediaKhan/test/genre/Trance";
while(1) {
  
  # get files in directory
  opendir (DIR, $dir) or die $!;
  my @files;
  while(my $file = readdir(DIR)) {
    next unless ($file =~ m/\.mp3$/);
    push @files, $file;
  }
  closedir(DIR);
  
  # select one at random
  my $selected = $files[rand @files];
  
  #play via mp3
  system("mpg123",$dir."/".$selected);
}
