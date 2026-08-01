# Music Track Information Layout Design

## Goal

Display the current song title and artist in the Music control panel after a
successful scan, without changing the working C++ metadata pipeline.

## Confirmed Root Cause

Runtime inspection with the user's FLAC library confirmed that embedded title
and artist values reach `MusicPlayerViewModel` and are returned by
`currentTitle()` and `currentArtist()` after the first scanned row becomes
current.

The presentation failure is in `MusicPlayer.qml`. The track-information
`Column` has no explicit width, while both child `Text` items bind their width
to `parent.width - 80`. The positioner's implicit width depends on those child
widths, producing a circular geometry dependency that collapses the text area.
Cover art is rendered in a separate item and is therefore unaffected.

## Scope

- Give the track-information area explicit horizontal bounds.
- Reserve the Scan button's existing 60-pixel area and a theme-token margin.
- Keep title and artist bound directly to `MusicViewModel.currentTitle` and
  `MusicViewModel.currentArtist`.
- Preserve centered text and right-side elision for long metadata.
- Keep QML declarative with no imperative JavaScript.

## Architecture

The track-information `Column` receives an ID and anchors its left edge to the
track row, its right edge to the left edge of the Scan button, and its vertical
center to the row. The Scan button receives an ID but keeps its existing size
and right alignment. Both child `Text` items use `width: parent.width`.

This removes the parent/child width cycle and prevents text from occupying the
button area. No C++ interface, model role, scanner behavior, or multimedia
dependency changes.

## Alternatives Considered

- `width: parent.width - 80` on the track-information column is smaller but
  duplicates button geometry as a magic number.
- Replacing the row with `RowLayout` is valid but changes more layout structure
  than required for this isolated defect.

## Failure Handling

- An empty current title continues to show `Scanning...` or `No Music` through
  the existing declarative binding.
- An empty artist remains blank.
- Long values elide within the bounded text area and cannot overlap Scan.

## Tests

- Add a focused QML source-contract test to `tst_music_playback` that requires
  explicit track-info bounds tied to the Scan button.
- Reject the old `width: parent.width - 80` child-text pattern.
- Run the project QML lint, module `qmllint`, Zero-JavaScript scan, full build,
  all CTest targets, and offscreen smoke.

## Decision Log

- Anchor track information to the Scan button instead of duplicating its
  width. This is responsive, declarative, and the smallest root-cause fix.
- Leave metadata ownership in C++. The ViewModel values are already correct;
  changing parsing would fix the wrong layer.
