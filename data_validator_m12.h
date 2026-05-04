#ifndef FIRESTAFF_DATA_VALIDATOR_M12_H
#define FIRESTAFF_DATA_VALIDATOR_M12_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    M12_VALIDATOR_MAX_FILES = 80,
    M12_VALIDATOR_MD5_CAPACITY = 33,
    M12_VALIDATOR_PATH_CAPACITY = 512,
    M12_VALIDATOR_LABEL_CAPACITY = 128
};

typedef enum {
    M12_FILE_STATUS_UNKNOWN = 0,
    M12_FILE_STATUS_VALID,
    M12_FILE_STATUS_MISMATCH,
    M12_FILE_STATUS_MISSING,
    M12_FILE_STATUS_ERROR
} M12_FileValidationStatus;

typedef struct {
    char filename[M12_VALIDATOR_PATH_CAPACITY];
    char gameId[16];
    char expectedMd5[M12_VALIDATOR_MD5_CAPACITY];
    char actualMd5[M12_VALIDATOR_MD5_CAPACITY];
    char label[M12_VALIDATOR_LABEL_CAPACITY];
    M12_FileValidationStatus status;
} M12_FileValidationResult;

typedef struct {
    M12_FileValidationResult results[M12_VALIDATOR_MAX_FILES];
    size_t resultCount;
    size_t validCount;
    size_t mismatchCount;
    size_t missingCount;
    size_t errorCount;
    int scanned;
} M12_DataValidatorState;

/**
 * Initialize validator state to zero.
 */
void M12_DataValidator_Init(M12_DataValidatorState* state);

/**
 * Scan the given data directory against all known checksums from the
 * built-in checksum database (derived from asset_validator_checksums_m12.json).
 * Populates state->results with per-file status.
 */
void M12_DataValidator_Scan(M12_DataValidatorState* state, const char* dataDir);

/**
 * Return a human-readable summary string for the overall validation.
 * The returned pointer is to a static buffer — not thread-safe.
 */
const char* M12_DataValidator_Summary(const M12_DataValidatorState* state);

/**
 * Return a short status label for a given file validation status.
 */
const char* M12_DataValidator_StatusLabel(M12_FileValidationStatus status);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DATA_VALIDATOR_M12_H */
