#ifndef _DT_ERROR_H_
#define _DT_ERROR_H_

#include <errno.h>
#include <stddef.h>

/* error handling */
#define DTERROR(e) (-(e))
#define DTUNERROR(e) (-(e))

/*common*/
#define DTERROR_NONE               DTERROR(0)
#define DTERROR_FAIL               DTERROR(1)

/*dtplayer*/
#define DTERROR_READ_FAILED        DTERROR(100)
#define DTERROR_READ_TIMEOUT       DTERROR(101)
#define DTERROR_READ_AGAIN         DTERROR(102)

#define DTERROR_READ_EOF           DTERROR(150)

//dthost
#define DTERROR_INVALID_CMD        DTERROR(200)
//dtport
//dtaudio
//dtvideo

#endif
