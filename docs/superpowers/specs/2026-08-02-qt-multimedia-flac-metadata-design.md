# Qt Multimedia FLAC Metadata Display Design

## Goal

After a library scan, the Music player must select a visible current track and
show the embedded title, artist, album, and cover art from valid FLAC files
without external libraries or project-owned FLAC/Vorbis parsing.

## Confirmed Root Cause

Runtime inspection against the user's FLAC library confirmed that Qt delivers
`Title`, `ContributingArtist`, and `AlbumTitle` through the scanner and into
`MusicPlayerViewModel`. The current-track C++ properties also return those
values after the first row is inserted.

The cover is lost at the Qt metadata mapping boundary. On this Linux Qt/FFmpeg
backend, the embedded FLAC picture is exposed as `QMediaMetaData::ThumbnailImage`;
`QMediaMetaData::CoverArtImage` is null. The current mapper checks only
`CoverArtImage`, so it retains an empty `SongData::coverArt` even though the file
contains a valid attached picture.

## Scope

- Use only Qt's public C++ API and the repository's existing Qt Multimedia
  dependency.
- Read `Title`, artist (`ContributingArtist`, with `AlbumArtist` fallback),
  `AlbumTitle`, and an embedded image where Qt's media backend exposes it.
- Prefer `CoverArtImage`; fall back to `ThumbnailImage` when the preferred key
  is null.
- Preserve the filename-derived title/artist and generated gradient as safe
  fallbacks when a tag or cover is missing.
- Select the first discovered track when no persisted resume index is valid.
- Keep QML bound to the ViewModel's current-track properties rather than a
  transient `PathView` delegate.

## Non-Goals

- Adding TagLib, FFmpeg APIs, or another third-party metadata library.
- Parsing FLAC metadata blocks or Vorbis comments in project code.
- Changing, normalizing, or rewriting the user's music files.
- Adding streaming, playlist management, or media-library persistence.

## Architecture

`MusicScanner` remains the worker-thread owner of recursive file enumeration.
For each discovered path it creates a scanner-local `QMediaPlayer`, sets the
local file source, and waits for Qt Multimedia to publish metadata or
complete/fail media loading. It emits one `SongData` only after that bounded
per-file attempt resolves, then advances to the next path.

`MusicScanner::applyQtMetaData` maps text tags first. For cover art it reads
`CoverArtImage`; when that value does not contain a usable `QImage`, it reads
`ThumbnailImage`. A usable image is encoded as the existing PNG data URI. If
neither key contains an image or PNG encoding fails, the mapper leaves the
existing fallback cover value unchanged.

`MusicPlayerViewModel` remains the GUI-thread `QAbstractListModel` owner. It
inserts received songs and, for the first row of a fresh scan, publishes
`currentIndex = 0` together with the effective current-track notification; a
valid persisted index still takes precedence when the scan finishes. QML reads
`currentTitle`, `currentArtist`, and `currentCoverArt`, avoiding dependence on
the creation timing of a `PathView` delegate.

## Failure Handling

- A missing, unsupported, or incomplete metadata response cannot block the
  scan indefinitely; the scanner moves on after a bounded Qt event-loop wait.
- Missing tag fields use the existing filename fallback.
- Missing or unusable cover art keeps the existing cover value and generated
  color artwork.
- Playback remains independently lazy-created by the ViewModel; scanner
  metadata loading must not start audio output.
- Production code does not log tag values or encoded cover data.

## Tests

- A metadata record containing only `ThumbnailImage` produces a PNG data URI.
- `CoverArtImage` takes precedence when both public image keys are populated.
- Null images preserve an existing fallback cover value.
- A fresh scan with a discovered song selects index zero, emits the effective
  current-track notification, and exposes its title, artist, and cover.
- Metadata absence leaves deterministic filename/color fallback values intact.
- Automated tests use generated in-memory images and do not depend on the
  user's personal Music directory.
- Runtime verification may read one real FLAC file but never modifies it.

## Alternatives Considered

### Capture the attached picture through `QVideoSink`

This would treat the FLAC attached picture as a video frame. It adds media
decode lifecycle and timing complexity, and may require starting the player.
It is unnecessary because the current backend already supplies the image via
`ThumbnailImage`.

### Parse FLAC metadata blocks in C++

This avoids backend-specific Qt metadata mapping, but requires bounded parsing
of FLAC blocks, Vorbis comments, picture sizes, corrupt files, and text
encodings. The larger security and maintenance surface is not justified for
the confirmed failure.

## Decision Log

- Use Qt's two public image keys with `CoverArtImage` first and
  `ThumbnailImage` second. This is the smallest fix at the confirmed failure
  boundary and adds no dependency.
- Keep current-track presentation state in `MusicPlayerViewModel`. This makes
  QML reactive to model selection without relying on delegate lifetime.
- Keep filename, existing cover, and generated-color fallbacks. A malformed or
  incomplete FLAC remains visible and cannot stall the library scan.
