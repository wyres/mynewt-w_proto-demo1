/**
 * Wyres private code
 * Socket like device access manager
 * This allows a generic paradigm (sockets) to be used to access 'device' instances (eg UART or I2C), with a standard set of methods + synchronisation
 * Devices must be created at init time directly, and they register with this manager to become accessible. Multiple app accesses are possible to the
 * same device instance.
 */
#include "os/os.h"

#include "wutils.h"

#include "lowpowermgr.h"

#define MAX_LPCBFNS MYNEWT_VAL(MAX_LPCBFNS)

// Registered callbacks fns
static LP_CBFN_t _devices[MAX_LPCBFNS];        // TODO should be a mempool
static LP_MODE _curMode=LP_RUN;

void LPMgr_register(LP_CBFN_t cb) {
    // TODO add to list
    _devices[0]=cb;
}

void LPMgr_run() {
    _curMode = LP_RUN;
}
void LPMgr_deepsleep() {
    _curMode = LP_DEEPSLEEP;
}

