from gmusicapi import Musicmanager

mm = Musicmanager()
mm.login()
songs = mm.get_all_songs()

print len(songs)
