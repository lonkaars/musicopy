# musicopy

a simple utility that copies music and playlists from a folder to another folder

## 'amazing' features:

- [x] glob include/exclude patterns
- [x] doesn't copy existing files
- [x] multiple different configuration sections for different media players or libraries
- [ ] stores a copy of your playlist to merge differences created on another media player

## dependencies

- [inih](https://github.com/benhoyt/inih)
- [cwalk](https://github.com/likle/cwalk)

## config file

the config file is in ini format. here's the options:

```rc
[default]
music_dir = ~/music
playlist_dir = ~/playlists

target_music_dir = /mnt/player/Music
target_playlist_dir = /mnt/player/Playlists

# multi-line ini works too (multiple definitions
# of the same key also works)
# exclude = junk/**
# 	i don't like this music/**

# include = folders to include/**

# don't copy existing files
# none = blindly overwrite files
# lazy = only overwrite if a file doesn't exists
# size = only overwrite if file size doesn't match
# hash = only overwrite if sha1 hash doesn't match
existing = lazy

```

