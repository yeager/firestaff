#ifndef FIRESTAFF_KNOWN_HASHES_H
#define FIRESTAFF_KNOWN_HASHES_H
int fs_known_hash_count(void);
const char *fs_known_hash_for_path(const char *path);
int fs_known_hash_expected_size(const char *path);
#endif
