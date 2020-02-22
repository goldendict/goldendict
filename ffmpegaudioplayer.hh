/* This file is (c) 2018 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef FFMPEGAUDIOPLAYER_HH_INCLUDED
#define FFMPEGAUDIOPLAYER_HH_INCLUDED

#include "audioplayerinterface.hh"
#include "ffmpegaudio.hh"

#ifdef MAKE_FFMPEG_PLAYER

namespace Ffmpeg
{

class AudioPlayer : public AudioPlayerInterface
{
    Q_OBJECT
public:
    AudioPlayer() : as(new AudioService)
    {
        connect( as, SIGNAL( error( QString ) ),
                 this, SIGNAL( error( QString ) ) );
        as->init();
    }

    ~AudioPlayer()
    {
        delete as;
    }

    virtual QString play( const char * data, int size )
    {
        as->playMemory( data, size );
        return QString();
    }

    virtual void stop()
    {
        as->stop();
    }
private:
    AudioService *as;
};

}

#endif // MAKE_FFMPEG_PLAYER

#endif // FFMPEGAUDIOPLAYER_HH_INCLUDED
