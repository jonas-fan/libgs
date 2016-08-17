#ifndef GS_UNIX_SOCKET_H_
#define GS_UNIX_SOCKET_H_

#include "socket.h"

#ifdef __cplusplus
extern "C" {
#endif

const struct gs_socket_base_t * gs_unix_socket_base(void);

#ifdef __cplusplus
}
#endif

#endif  /* GS_UNIX_SOCKET_H_ */