/* This file is (c) 2018 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef AUDIOPLAYERUI_HH_INCLUDED
#define AUDIOPLAYERUI_HH_INCLUDED

#include "audioplayerinterface.hh"

/// This class template ensures that its managed checkable audio UI element
/// is interactable when checked to allow stopping playback at any time.
/// When the UI element becomes unchecked after a setPlaybackState() call, the
/// proper interactability based on playability, as specified by the last
/// setPlayable() call, is restored.
template< class CheckableUiElement >
class AudioPlayerUi
{
  Q_DISABLE_COPY( AudioPlayerUi )
public:
  /// This pointer to member typically points to setEnabled() or setVisible().
  typedef void ( CheckableUiElement::* SetInteractablePtr )( bool );

  explicit AudioPlayerUi( CheckableUiElement & audioUiElement,
                          SetInteractablePtr setInteractablePtr ) :
    uiElement( audioUiElement ),
    setInteractable( setInteractablePtr ),
    interactableWhenUnchecked( true )
  {}

  AudioPlayerInterface::State playbackState() const
  {
    return uiElement.isChecked() ? AudioPlayerInterface::PlayingState
                                 : AudioPlayerInterface::StoppedState;
  }

  void setPlaybackState( AudioPlayerInterface::State state )
  {
    const bool checked = ( state == AudioPlayerInterface::PlayingState );
    uiElement.setChecked( checked );
    if( checked || !interactableWhenUnchecked )
      ( uiElement.*setInteractable )( checked );
  }

  void setPlayable( bool playable )
  {
    interactableWhenUnchecked = playable;
    if( !uiElement.isChecked() )
      ( uiElement.*setInteractable )( interactableWhenUnchecked );
  }

private:
  CheckableUiElement & uiElement;
  const SetInteractablePtr setInteractable;
  bool interactableWhenUnchecked;
};

#endif // AUDIOPLAYERUI_HH_INCLUDED
