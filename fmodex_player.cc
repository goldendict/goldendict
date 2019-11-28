/* This file is (c) 2018 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifdef MAKE_FMODEX_PLAYER

#include <QByteArray>
#include "fmodex_player.hh"

bool FmodexAudioPlayer::ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        emit error( tr("FMOD error! (%1) %2").arg(result).arg(FMOD_ErrorString(result)) );
        return true;
    }
    return false;
}

FmodexAudioPlayer::FmodexAudioPlayer()
    : system(0), sound(0), channel(0)
{
    FMOD_RESULT result = FMOD_System_Create(&system);
    if(ERRCHECK(result))
        system = 0;
    else
    {
        unsigned int version = 0;
        result = FMOD_System_GetVersion(system, &version);
        if(ERRCHECK(result))
        {
            clean();
        }
        if (version < FMOD_VERSION)
        {
            clean();
            emit error( tr("Error! You are using an old version of FMOD %08x. This program requires %08x").
                        arg(version).arg(FMOD_VERSION) );
        }
        else
        {
            result = FMOD_System_Init(system, 1, FMOD_INIT_NORMAL, 0);
            if(ERRCHECK(result))
                clean();
        }
    }
}
FmodexAudioPlayer::~FmodexAudioPlayer()
{
    stop();
    clean();
    system = 0;
}
void FmodexAudioPlayer::clean()
{
    if(system)
    {
        FMOD_RESULT result = FMOD_System_Close(system);
        ERRCHECK(result);
        result = FMOD_System_Release(system);
        ERRCHECK(result);
    }
}

QString FmodexAudioPlayer::play( const char * data, int size )
{
  stop();
  FMOD_CREATESOUNDEXINFO exinfo;
  memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
  exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
  exinfo.length = size;

  FMOD_RESULT result = FMOD_System_CreateSound(system, data, FMOD_HARDWARE | FMOD_OPENMEMORY, &exinfo, &sound);
  if(ERRCHECK(result))
  {
      return tr( "Couldn't create sound for playing." );
  }

  result = FMOD_Sound_SetMode(sound, FMOD_LOOP_OFF);
  ERRCHECK(result);
  result = FMOD_System_PlaySound(system, channel == 0 ? FMOD_CHANNEL_FREE : FMOD_CHANNEL_REUSE, sound, false, &channel);
  ERRCHECK(result);

  return QString();
}

void FmodexAudioPlayer::stop()
{
    if(sound)
    {
        if(channel)
        {
            FMOD_RESULT result = FMOD_Channel_Stop(channel);
            ERRCHECK(result);
        }
        FMOD_RESULT result = FMOD_Sound_Release(sound);
        ERRCHECK(result);
        sound = 0;
    }
}


#endif // MAKE_FMODEX_PLAYER
