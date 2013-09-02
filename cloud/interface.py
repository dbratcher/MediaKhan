from gmusicapi import Musicmanager

mm = Musicmanager()
mm.login()
songs = mm.get_all_songs()

def get_all_titles():
  titles = [song['title'] for song in songs]
  return titles

def get_artist(title):
  for song in songs:
    if song['title'] == title:
      return song['artist']
