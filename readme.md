# musicopy

a simple utility that copies music and playlists from a folder to another folder

## 'amazing' features:

- glob include/exclude patterns
- doesn't copy existing files
- multiple different configuration sections for different media players or libraries
- stores a copy of your playlist to merge differences created on another media player

## dependencies

- [inih](https://github.com/benhoyt/inih)
- [cwalk](https://github.com/likle/cwalk)

## config file

the config file is in ini format. here's the options:

```rc
[default]
music_dir = ~/music
playlist_dir = ~/playlists

exclude = junk/**
	i don't like this music/**

# include = folders to include/**

# don't copy existing files
# lazy = only check file size
# full = compare sha1 hash
# none = blindly overwrite files
existing = lazy

```

