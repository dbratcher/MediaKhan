from gmusicapi import Musicmanager
from re import match

mm = Musicmanager()
mm.login()
songs = mm.get_all_songs()

def get_all_titles():
  titles = [song['title'] for song in songs]
  print len(titles), " loaded from google music\n"
  return titles

def get_metadata(title, attr):
  for song in songs:
    if song['title'] == title:
      if title == "Winter":
        print song
      res =  song.get(attr, "na")
      if(match('^[a-zA-Z\. ]+$', res)):
        print "valid"
      else:
        print "not valid = ", res
        res = "na"
      return res
