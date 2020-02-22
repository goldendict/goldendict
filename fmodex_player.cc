/* This file is (c) 2019 nonwill <nonwill@hotmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file
 * FMOD NON-COMMERCIAL LICENSE
 * FMOD Sound System, copyright ? Firelight Technologies Pty, Ltd., 1994-2016. */

#ifdef MAKE_FMODEX_PLAYER
#include "fmodex_player.hh"
#include "fmod_errors.h"
#include <QFile>

#ifdef __WIN32
#ifdef _MSC_VER
#ifdef __WIN64
const QString FmodexAudioPlayer::fmodex_dyl_name("fmodex64.dll");
#else
const QString FmodexAudioPlayer::fmodex_dyl_name("fmodex.dll");
#endif
#elif defined(__CYGWIN32__)
const QString FmodexAudioPlayer::fmodex_dyl_name("fmodex.dll");
#else
const QString FmodexAudioPlayer::fmodex_dyl_name;
#endif
#else
const QString FmodexAudioPlayer::fmodex_dyl_name;
#endif

bool FmodexAudioPlayer::available()
{
    return QLibrary::isLibrary(fmodex_dyl_name) && QFile::exists(fmodex_dyl_name);
}

typedef FMOD_RESULT (*FMOD_SYSTEM_CREATE)(FMOD_SYSTEM **system);
typedef FMOD_RESULT (*FMOD_SYSTEM_RELEASE)(FMOD_SYSTEM *system);
typedef FMOD_RESULT (*FMOD_SYSTEM_GETVERSION)(FMOD_SYSTEM *system, unsigned int *version);
typedef FMOD_RESULT (*FMOD_SYSTEM_INIT)(FMOD_SYSTEM *system, int maxchannels, FMOD_INITFLAGS flags, void *extradriverdata);
//typedef FMOD_RESULT (*FMOD_SYSTEM_CLOSE)(FMOD_SYSTEM *system);
typedef FMOD_RESULT (*FMOD_SYSTEM_CREATESOUND)(FMOD_SYSTEM *system, const char *name_or_data, FMOD_MODE mode, FMOD_CREATESOUNDEXINFO *exinfo, FMOD_SOUND **sound);
typedef FMOD_RESULT (*FMOD_SOUND_SETMODE)(FMOD_SOUND *sound, FMOD_MODE mode);
typedef FMOD_RESULT (*FMOD_SYSTEM_PLAYSOUND)(FMOD_SYSTEM *system, FMOD_CHANNELINDEX channelid, FMOD_SOUND *sound, FMOD_BOOL paused, FMOD_CHANNEL **channel);
typedef FMOD_RESULT (*FMOD_CHANNEL_ISPLAYING)(FMOD_CHANNEL *channel, FMOD_BOOL *isplaying);
typedef FMOD_RESULT (*FMOD_CHANNEL_STOP)(FMOD_CHANNEL *channel);
typedef FMOD_RESULT (*FMOD_SOUND_RELEASE)(FMOD_SOUND *sound);

struct FmodexAudioPlayer::FMOD_EX_API {
    FMOD_SYSTEM     *system;
    FMOD_SOUND       *sound;
    FMOD_CHANNEL    *channel;

    FMOD_SYSTEM_CREATE FMOD_System_Create;
    FMOD_SYSTEM_RELEASE FMOD_System_Release;
    FMOD_SYSTEM_GETVERSION FMOD_System_GetVersion;
    FMOD_SYSTEM_INIT FMOD_System_Init;
    //FMOD_SYSTEM_CLOSE FMOD_System_Close;
    FMOD_SYSTEM_CREATESOUND FMOD_System_CreateSound;
    FMOD_SOUND_SETMODE FMOD_Sound_SetMode;
    FMOD_SYSTEM_PLAYSOUND FMOD_System_PlaySound;
    FMOD_CHANNEL_ISPLAYING FMOD_Channel_IsPlaying;
    FMOD_CHANNEL_STOP FMOD_Channel_Stop;
    FMOD_SOUND_RELEASE FMOD_Sound_Release;
};

bool FmodexAudioPlayer::ERRCHECK(int result)
{
    if (result != FMOD_OK)
    {
        emit error( tr("FMOD ERR: (%1) - %2").arg(result).arg(FMOD_ErrorString(FMOD_RESULT(result))) );
        return true;
    }
    return false;
}

FmodexAudioPlayer::FmodexAudioPlayer()
    : fmodex(0), fmodex_dl(fmodex_dyl_name)
{
    if(!fmodex_dl.load())
    {
        emit error(fmodex_dl.errorString());
        return;
    }
    fmodex = new FmodexAudioPlayer::FMOD_EX_API;
    memset(fmodex, 0, sizeof(FmodexAudioPlayer::FMOD_EX_API));
    fmodex->FMOD_System_Create = (FMOD_SYSTEM_CREATE)fmodex_dl.resolve("FMOD_System_Create");
    fmodex->FMOD_System_Release = (FMOD_SYSTEM_RELEASE)fmodex_dl.resolve("FMOD_System_Release");
    fmodex->FMOD_System_GetVersion = (FMOD_SYSTEM_GETVERSION)fmodex_dl.resolve("FMOD_System_GetVersion");
    fmodex->FMOD_System_Init = (FMOD_SYSTEM_INIT)fmodex_dl.resolve("FMOD_System_Init");
    //fmodex->FMOD_System_Close = (FMOD_SYSTEM_CLOSE)fmodex_dl.resolve("FMOD_System_Close");
    fmodex->FMOD_System_CreateSound = (FMOD_SYSTEM_CREATESOUND)fmodex_dl.resolve("FMOD_System_CreateSound");
    fmodex->FMOD_Sound_SetMode = (FMOD_SOUND_SETMODE)fmodex_dl.resolve("FMOD_Sound_SetMode");
    fmodex->FMOD_System_PlaySound = (FMOD_SYSTEM_PLAYSOUND)fmodex_dl.resolve("FMOD_System_PlaySound");
    fmodex->FMOD_Channel_IsPlaying = (FMOD_CHANNEL_ISPLAYING)fmodex_dl.resolve("FMOD_Channel_IsPlaying");
    fmodex->FMOD_Channel_Stop = (FMOD_CHANNEL_STOP)fmodex_dl.resolve("FMOD_Channel_Stop");
    fmodex->FMOD_Sound_Release = (FMOD_SOUND_RELEASE)fmodex_dl.resolve("FMOD_Sound_Release");

    if(ERRCHECK(fmodex->FMOD_System_Create(&fmodex->system)))
        clean();
    else
    {
        unsigned int version = 0;
        if(ERRCHECK(fmodex->FMOD_System_GetVersion(fmodex->system, &version)))
        {
            clean();
        }
        else if (version < FMOD_VERSION)
        {
            clean();
            emit error( tr("FMOD ERR: You are using an old version of FMOD %08x. This program requires %08x.").
                        arg(version).arg(FMOD_VERSION) );
        }
        else if(ERRCHECK(fmodex->FMOD_System_Init(fmodex->system, 1,
                                                  FMOD_INIT_STREAM_FROM_UPDATE |
                                                  FMOD_INIT_SYNCMIXERWITHUPDATE,
                                                  0)))
        {
            clean();
        }
    }
}
FmodexAudioPlayer::~FmodexAudioPlayer()
{
    if(fmodex) {
        stop();
        clean();
        fmodex_dl.unload();
    }
}
void FmodexAudioPlayer::clean()
{
    if(fmodex)
    {
        if(fmodex->system)
        {
            ERRCHECK(fmodex->FMOD_System_Release(fmodex->system));
        }
        delete fmodex;
        fmodex = 0;
    }
}

QString FmodexAudioPlayer::play( const char * data, int size )
{
    if(fmodex==nullptr)
        return tr( "FMOD Ex Library (%1) is invalid or not available!" ).arg(fmodex_dyl_name);
    stop();
    FMOD_CREATESOUNDEXINFO exinfo;
    memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    exinfo.length = size;

    FMOD_RESULT result = fmodex->FMOD_System_CreateSound(fmodex->system, data,
                                                         FMOD_HARDWARE | FMOD_OPENMEMORY,
                                                         &exinfo, &fmodex->sound);
    if(ERRCHECK(result))
    {
        return tr( "Failed to create sound. The sound type may not be supported by FMOD Ex." );
    }

    result = fmodex->FMOD_Sound_SetMode(fmodex->sound, FMOD_LOOP_OFF);
    ERRCHECK(result);
    result = fmodex->FMOD_System_PlaySound(fmodex->system,
                                           fmodex->channel == 0 ? FMOD_CHANNEL_FREE : FMOD_CHANNEL_REUSE,
                                           fmodex->sound, false, &fmodex->channel);
    ERRCHECK(result);

    return QString();
}

void FmodexAudioPlayer::stop()
{
    if(fmodex && fmodex->sound)
    {
        if(fmodex->channel)
        {
            FMOD_BOOL is_playing = false;
            FMOD_RESULT result = fmodex->FMOD_Channel_IsPlaying(fmodex->channel, &is_playing);
            if(is_playing)
                result = fmodex->FMOD_Channel_Stop(fmodex->channel);
            ERRCHECK(result);
        }
        FMOD_RESULT result = fmodex->FMOD_Sound_Release(fmodex->sound);
        ERRCHECK(result);
        fmodex->sound = 0;
    }
}


#endif // MAKE_FMODEX_PLAYER
