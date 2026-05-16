
#ifndef FIRESTAFF_BITMAP_PROOF_H
#define FIRESTAFF_BITMAP_PROOF_H

/* Proof probe: load GRAPHICS.DAT, extract a wall bitmap, dump as PPM.
 * Run: firestaff_bitmap_proof /path/to/DATA/
 * Output: wall_proof.ppm */

int fs_bitmap_proof_run(const char *data_dir, const char *output_path);

#endif
