#ifndef FIRESTAFF_ARTPACK_ADMISSION_M12_H
#define FIRESTAFF_ARTPACK_ADMISSION_M12_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    M12_ARTPACK_ADMISSION_EMPTY = 0,
    M12_ARTPACK_ADMISSION_PATH_NOT_FSART,
    M12_ARTPACK_ADMISSION_OPEN_FAILED,
    M12_ARTPACK_ADMISSION_TOO_SMALL,
    M12_ARTPACK_ADMISSION_UNSUPPORTED_SIGNATURE,
    M12_ARTPACK_ADMISSION_ACCEPTED_FSAR,
    M12_ARTPACK_ADMISSION_ACCEPTED_ZIP
} M12_ArtpackAdmissionStatus;

typedef struct {
    M12_ArtpackAdmissionStatus status;
    int admitted;
    int fallbackVisualsPermitted;
    unsigned long long fileSize;
    char path[512];
    char signature[8];
} M12_ArtpackAdmissionReceipt;

int M12_ArtpackAdmission_Check(const char* path,
                               M12_ArtpackAdmissionReceipt* outReceipt);
const char* M12_ArtpackAdmission_StatusName(M12_ArtpackAdmissionStatus status);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_ARTPACK_ADMISSION_M12_H */
