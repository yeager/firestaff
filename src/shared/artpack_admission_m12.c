#include "artpack_admission_m12.h"

#include <stdio.h>
#include <string.h>

static int m12_artpack_has_fsart_extension(const char* path) {
    const char* dot;
    char ext[8];
    size_t i;
    if (!path || path[0] == '\0') {
        return 0;
    }
    dot = strrchr(path, '.');
    if (!dot) {
        return 0;
    }
    snprintf(ext, sizeof(ext), "%s", dot);
    for (i = 0U; ext[i] != '\0'; ++i) {
        if (ext[i] >= 'A' && ext[i] <= 'Z') {
            ext[i] = (char)(ext[i] - 'A' + 'a');
        }
    }
    return strcmp(ext, ".fsart") == 0;
}

static void m12_artpack_receipt_init(const char* path,
                                     M12_ArtpackAdmissionReceipt* receipt) {
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->status = M12_ARTPACK_ADMISSION_EMPTY;
    receipt->fallbackVisualsPermitted = 0;
    if (path) {
        snprintf(receipt->path, sizeof(receipt->path), "%s", path);
    }
}

int M12_ArtpackAdmission_Check(const char* path,
                               M12_ArtpackAdmissionReceipt* outReceipt) {
    FILE* fp;
    unsigned char header[8];
    size_t got;
    long size;

    m12_artpack_receipt_init(path, outReceipt);
    if (!path || path[0] == '\0') {
        return 0;
    }
    if (!m12_artpack_has_fsart_extension(path)) {
        if (outReceipt) {
            outReceipt->status = M12_ARTPACK_ADMISSION_PATH_NOT_FSART;
        }
        return 0;
    }
    fp = fopen(path, "rb");
    if (!fp) {
        if (outReceipt) {
            outReceipt->status = M12_ARTPACK_ADMISSION_OPEN_FAILED;
        }
        return 0;
    }
    if (fseek(fp, 0L, SEEK_END) != 0) {
        fclose(fp);
        if (outReceipt) {
            outReceipt->status = M12_ARTPACK_ADMISSION_OPEN_FAILED;
        }
        return 0;
    }
    size = ftell(fp);
    if (size < 8L || fseek(fp, 0L, SEEK_SET) != 0) {
        fclose(fp);
        if (outReceipt) {
            outReceipt->status = M12_ARTPACK_ADMISSION_TOO_SMALL;
            outReceipt->fileSize = size > 0L ? (unsigned long long)size : 0ULL;
        }
        return 0;
    }
    got = fread(header, 1U, sizeof(header), fp);
    fclose(fp);
    if (got != sizeof(header)) {
        if (outReceipt) {
            outReceipt->status = M12_ARTPACK_ADMISSION_TOO_SMALL;
            outReceipt->fileSize = size > 0L ? (unsigned long long)size : 0ULL;
        }
        return 0;
    }
    if (outReceipt) {
        outReceipt->fileSize = (unsigned long long)size;
        snprintf(outReceipt->signature, sizeof(outReceipt->signature),
                 "%02X%02X%02X%02X",
                 header[0], header[1], header[2], header[3]);
    }
    if (memcmp(header, "FSAR", 4U) == 0) {
        if (outReceipt) {
            outReceipt->status = M12_ARTPACK_ADMISSION_ACCEPTED_FSAR;
            outReceipt->admitted = 1;
        }
        return 1;
    }
    if (header[0] == 'P' && header[1] == 'K' &&
        header[2] == 0x03 && header[3] == 0x04) {
        if (outReceipt) {
            outReceipt->status = M12_ARTPACK_ADMISSION_ACCEPTED_ZIP;
            outReceipt->admitted = 1;
        }
        return 1;
    }
    if (outReceipt) {
        outReceipt->status = M12_ARTPACK_ADMISSION_UNSUPPORTED_SIGNATURE;
    }
    return 0;
}

const char* M12_ArtpackAdmission_StatusName(M12_ArtpackAdmissionStatus status) {
    switch (status) {
        case M12_ARTPACK_ADMISSION_EMPTY: return "empty";
        case M12_ARTPACK_ADMISSION_PATH_NOT_FSART: return "path-not-fsart";
        case M12_ARTPACK_ADMISSION_OPEN_FAILED: return "open-failed";
        case M12_ARTPACK_ADMISSION_TOO_SMALL: return "too-small";
        case M12_ARTPACK_ADMISSION_UNSUPPORTED_SIGNATURE:
            return "unsupported-signature";
        case M12_ARTPACK_ADMISSION_ACCEPTED_FSAR: return "accepted-fsar";
        case M12_ARTPACK_ADMISSION_ACCEPTED_ZIP: return "accepted-zip";
        default: return "unknown";
    }
}
