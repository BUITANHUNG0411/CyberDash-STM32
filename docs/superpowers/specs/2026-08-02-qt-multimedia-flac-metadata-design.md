# Qt Multimedia FLAC Metadata Design

## Goal

After a library scan, the Music player must select a visible current track and
show metadata from valid FLAC files without parsing FLAC or Vorbis bytes in
project code.

## Scope

- Use only Qt Multimedia's public `QMediaPlayer` and `QMediaMetaData` API.
- Read `Title`, artist (`ContributingArtist`, with `AlbumArtist` fallback),
  `AlbumTitle`, and `CoverArtImage` where Qt's media backend exposes them.
- Preserve the filename-derived title/artist and generated gradient as safe
  fallbacks when a tag or cover is missing.
- Select the first discovered track when no persisted resume index is valid.

## Architecture

`MusicScanner` remains the worker-thread owner of recursive file enumeration.
For each discovered path it creates or reuses a scanner-local `QMediaPlayer`,
sets the local file source, and waits asynchronously for Qt Multimedia to
publish metadata or complete/fail media loading. It emits one `SongData` only
after that bounded per-file attempt resolves, then advances to the next path.

`MusicPlayerViewModel` remains the GUI-thread `QAbstractListModel` owner. It
inserts received songs and, for the first row of a fresh scan, publishes
`currentIndex = 0`; a valid persisted index still takes precedence when the
scan finishes. QML requires no changes because it already binds display text
to the PathView current item.

## Failure Handling

- A missing, unsupported, or incomplete metadata response cannot block the
  scan indefinitely; the scanner moves on after a bounded Qt event-loop wait.
- Missing tag fields use the existing filename fallback.
- Missing or unusable cover art keeps the existing generated color artwork.
- Playback remains independently lazy-created by the ViewModel; scanner
  metadata loading must not start audio output.

## Tests

- A fresh scan with a discovered song selects index zero and emits its change.
- A scanner metadata result is preserved by the model roles.
- Metadata absence leaves deterministic filename/color fallback values intact.
- Existing multimedia-disabled playback tests remain deterministic.
