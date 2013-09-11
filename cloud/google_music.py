from gmusicapi import Musicmanager
from re import match

mm = Musicmanager()
mm.login()

songs = {};

def convert(input):
  if isinstance(input, dict):
    return {convert(key): convert(value) for key, value in input.iteritems()}
  elif isinstance(input, list):
    return [convert(element) for element in input]
  elif isinstance(input, unicode):
    return input.encode('utf-8')
  else:
    return input

def get_all_titles():
  songs = mm.get_all_songs()
  songs = convert(songs)
  titles = [song['title'] + ".mp3" for song in songs]
  print len(titles), " loaded from google music\n"
  titles = [str(title) for title in titles]
  return titles

def get_metadata(title, attr):
  title = title[:-4]
  for song in songs:
    if song['title'] == title:
      if title == "Winter":
        print song
      res =  song.get(attr, "na")
      res = str(res)
      return res

def get_song(title, path):
  title = title[:-4]
  an_id = 'null'
  for song in songs:
    if song['title'] == title:
      an_id = song['id']
  filename, audio = mm.download_song(an_id)
  with open(path, 'wb') as f:
    f.write(audio)

def upload_song(path):
  mm.upload(path)
